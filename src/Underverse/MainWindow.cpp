#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QInputDialog>
#include <QScrollBar>
#include <QProcess>
#include <QToolButton>
#include <QToolTip>
#include <QStandardPaths>
#include <QTextBrowser>
#include "MainWindow.h"
#include "Settings.h"
#include "Helper.h"
#include "Exceptions.h"
#include "GUIHelper.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	connect(ui->notes_browser, SIGNAL(searchTermsChanged(QStringList)), ui->editor, SLOT(setHighlightStrings(QStringList)));
	connect(ui->notes_browser, SIGNAL(fileSelected(QString)), this, SLOT(loadFile(QString)));
	connect(ui->editor, SIGNAL(modificationStateChanged()), this, SLOT(updateWindowTitle()));

	installEventFilter(this);

	initSettings();
	applySettings();
	updateRecentFilesMenu();

    if (qApp->arguments().count()==2)
    {
        loadFile(qApp->arguments().at(1));
    }
    else
    {
        loadFile(""); //init GUI
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateWidths();
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, "About " + QApplication::applicationName(),  "<p>" + QApplication::applicationName() + " " + QApplication::applicationVersion() +"<p>A free markdown editor under MIT license.");
}

void MainWindow::on_actionNew_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Create new file", Settings::string("open_folder"), "MarkDown files (*.md);;All files (*.*)");
    if (filename=="") return;
    Settings::setString("open_folder", QFileInfo(filename).canonicalPath());

    Helper::touchFile(filename);
    loadFile(filename);
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file", Settings::string("open_folder"), "MarkDown files (*.md);;All files (*.*)");
    if (filename=="") return;
    Settings::setString("open_folder", QFileInfo(filename).canonicalPath());

    loadFile(filename);
}

void MainWindow::on_actionSave_triggered()
{
	ui->editor->storeFile();
}

void MainWindow::on_actionClose_triggered()
{
    loadFile("");
}

void MainWindow::on_actionToggleEditing_triggered()
{
	ui->editor->toggleEditArea();
    updateWidths();
}

void MainWindow::on_actionOpenNotesFolder_triggered()
{
	QDesktopServices::openUrl(QUrl(ui->editor->baseFolder()));
}

void MainWindow::on_actionOpenSettingsFiles_triggered()
{
	foreach(QString file, Settings::files())
	{
		QDesktopServices::openUrl("file:///" + file);
	}
}

void MainWindow::on_actionMarkdownHelp_triggered()
{
	MarkdownEditor* editor = new MarkdownEditor();
	editor->loadFile(":/Resources/MarkdownHelp.md");
	editor->toggleEditingEnabled();
	auto dlg = GUIHelper::createDialog(editor, "Markdown help", "", false);
	dlg->exec();
}

void MainWindow::on_actionSearch_triggered()
{
	if (notes_mode_)
    {
		ui->notes_browser->setFocus();
    }
    else
    {
		QStringList terms = QInputDialog::getText(this, "Search", "terms").split(' ', Qt::SkipEmptyParts);
		ui->notes_browser->setSearchTerms(terms);
    }
}

void MainWindow::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());
    QString filename = action->text();
    if (!QFile::exists(filename))
    {
        QMessageBox::warning(this, "File does not exist!", "File does not exist: " + filename);
        removeRecentFile(filename);
        return;
    }
    loadFile(filename);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	ui->editor->clear();
	event->accept();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		if (keyEvent->key() == Qt::Key_Search)
		{
			ui->notes_browser->setFocus();
			return true;
		}
	}

	return QObject::eventFilter(obj, event);
}

void MainWindow::loadFile(QString filename)
{
	filename = filename.trimmed();
    if (filename!="" && !QFile::exists(filename))
    {
        QMessageBox::warning(this, "File missing", "File ' " + filename + "' does not exist!");
        filename = "";
    }

	if (filename=="")
	{
		ui->editor->clear();
    }
    else
	{
		ui->editor->loadFile(filename);
		addRecentFile(ui->editor->file());
    }

	//update browser
	if (filename.isEmpty() || ui->editor->file().startsWith(ui->editor->baseFolder()))
	{
		ui->notes_browser->show();
		notes_mode_ = true;

		if (!filename.isEmpty())
		{
			ui->notes_browser->setSelectedFile(filename);
		}
    }
    else
	{
		ui->notes_browser->hide();
		notes_mode_ = false;
	}
	updateWidths();
}

void MainWindow::updateWindowTitle()
{
    QString title = qApp->applicationName();
	if (!ui->editor->file().isEmpty())
    {
		title += " - " + ui->editor->file() + (ui->editor->isModified() ? "*" : "");
    }
    setWindowTitle(title);
}

void MainWindow::addRecentFile(QString filename)
{
	//skip if on notes folder
	if ((QFileInfo(filename).canonicalPath() + "/").startsWith(ui->editor->baseFolder())) return;

	QStringList files = Settings::stringList("recent_files", true);

    files.prepend(filename.replace("\\", "/"));
    files.removeDuplicates();
    while(files.count()>15) files.pop_back();

    Settings::setStringList("recent_files", files);

    updateRecentFilesMenu();
}

void MainWindow::removeRecentFile(QString filename)
{
	QStringList files = Settings::stringList("recent_files", true);
    files.removeAll(filename.replace("\\", "/"));
    Settings::setStringList("recent_files", files);

	updateRecentFilesMenu();
}

QByteArray MainWindow::execute(QString exe, QStringList args, QString wd)
{
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	if (!wd.isEmpty()) process.setWorkingDirectory(wd);
	process.start(exe, args);
	if (!process.waitForFinished(-1))
	{
		QByteArray output = process.readAll();
		THROW(Exception, "Could not execute '" + exe + " " + args.join("\n") + "':\n" + output);
	}
	return process.readAll();
}

void MainWindow::initSettings()
{
	//data folder
	QString data_folder = Settings::string("data_folder", true);
    if (data_folder=="")
	{
		data_folder = qApp->applicationDirPath() + "/data/";
		QDir(data_folder).mkpath(".");
		Settings::setString("data_folder", data_folder);
	}

	//editor
	if (!Settings::contains("font")) Settings::setString("font", "Courier New");
	if (!Settings::contains("font_size")) Settings::setInteger("font_size", 10);
	if (!Settings::contains("tab_width")) Settings::setInteger("tab_width", 4);

	//misc
	if (!Settings::contains("open_folder")) Settings::setString("open_folder", qApp->applicationDirPath());
}

void MainWindow::applySettings()
{
    //general
	QString data_folder = Settings::string("data_folder");
	ui->notes_browser->setBaseDirectory(data_folder);
	ui->editor->setBaseFolder(data_folder);

    //editor
    QFont font(Settings::string("font"), Settings::integer("font_size"));
	ui->editor->setFont(font);
	ui->editor->setTabStopWidth(Settings::integer("tab_width") * QFontMetrics(font).horizontalAdvance(' '));
}

void MainWindow::updateRecentFilesMenu()
{
    ui->menuRecentFiles->clear();

	QStringList files = Settings::stringList("recent_files", true);
    foreach(QString file, files)
    {
        ui->menuRecentFiles->addAction(file, this, SLOT(openRecentFile()));
    }
}

void MainWindow::updateWidths()
{
	int browser_width = ui->notes_browser->isVisible() ? 250 : 0;
	ui->splitter->setSizes(QList<int>() << browser_width << (width()-browser_width));
}
