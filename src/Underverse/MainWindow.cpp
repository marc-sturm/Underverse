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
	ui->search_dock->hide();
	connect(ui->editor, SIGNAL(modificationStateChanged()), this, SLOT(updateWindowTitle()));
	connect(ui->search, SIGNAL(textEdited(QString)), this, SLOT(searchTermsChanged()));
	installEventFilter(this);

	initSettings();
	applySettings();
	updateRecentFilesMenu();

    if (qApp->arguments().count()==2)
    {
        loadFile(qApp->arguments().at(1));
	}
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, "About " + QApplication::applicationName(),  "<p>" + QApplication::applicationName() + " " + QApplication::applicationVersion() +"<p>A free simple markdown editor.");
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
	ui->editor->clear();
	updateWindowTitle();
}

void MainWindow::on_actionToggleEditing_triggered()
{
	ui->editor->toggleEditArea();
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
	ui->search_dock->show();
	ui->search->setFocus();
	ui->search->selectAll();
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
			on_actionSearch_triggered();
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
		return;
    }

	ui->editor->loadFile(filename);
	updateWindowTitle();
	addRecentFile(ui->editor->file());
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

void MainWindow::searchTermsChanged()
{
	ui->editor->setHighlightStrings(ui->search->text().split(' ', Qt::SkipEmptyParts));
}

void MainWindow::addRecentFile(QString filename)
{
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
	//editor
	if (!Settings::contains("font")) Settings::setString("font", "Courier New");
	if (!Settings::contains("font_size")) Settings::setInteger("font_size", 10);
	if (!Settings::contains("tab_width")) Settings::setInteger("tab_width", 4);

	//misc
	if (!Settings::contains("open_folder")) Settings::setString("open_folder", qApp->applicationDirPath());
}

void MainWindow::applySettings()
{
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

