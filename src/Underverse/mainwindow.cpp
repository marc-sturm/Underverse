#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

#include "MainWindow.h"
#include "Settings.h"
#include "Helper.h"
#include "Exceptions.h"
#include "SettingsDialog.h"
#include "GUIHelper.h"
#include "MarkDownHighlighter.h"

#include <markdown.h>
#include <html.h>
#include <buffer.h>


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, file_()
	, modified_(false)
{
    ui->setupUi(this);
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(askWetherToStoreFile()));

    ui->html->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(ui->html->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(openExternalLink(QUrl)));

	connect(ui->plain, SIGNAL(textChanged()), this, SLOT(textChanged()));

	connect(ui->browser, SIGNAL(fileSelected(QString)), this, SLOT(loadFile(QString)));

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

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);
	int browser_with = 250;
	int w = (width() - browser_with)/2;
	ui->splitter->setSizes(QList<int>() << browser_with << w << w);
}

QByteArray MainWindow::fileText(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        THROW(FileAccessException, "Could not open file for reading: '" + filename + "'!");
    }

    return file.readAll();
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, "About " + QApplication::applicationName(),  "<p>" + QApplication::applicationName() + " " + QApplication::applicationVersion() +"<p>It is provided under the <a href=\"http://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html\">GNU General Public License (GPL) Version 2.0</a>.<p>This program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.");
}

void MainWindow::on_actionOpen_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, "Open file", Settings::string("open_folder"),"MarkDown files (*.md);;All files (*.*)");
    if (filename=="") return;

    Settings::setString("open_folder", QFileInfo(filename).canonicalPath());
    loadFile(filename);
}

void MainWindow::on_actionSave_triggered()
{
	if (file_=="" || !modified_) return;

    Helper::storeTextFile(file_, QStringList() << ui->plain->toPlainText());
    modified_ = false;
    updateWindowTitle();
}

void MainWindow::on_actionClose_triggered()
{
	loadFile("");
}

void MainWindow::on_actionSettings_triggered()
{
	SettingsDialog dlg(this);
    if (dlg.exec()==QDialog::Accepted)
    {
		applySettings();
		//update HTML in case style changed
		updateHTML();
    }
}

void MainWindow::on_actionMarkdownHelp_triggered()
{
	QWebView view;
	view.page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	connect(&view, SIGNAL(linkClicked(QUrl)), this, SLOT(openExternalLink(QUrl)));
    view.setHtml(markdown(fileText(":/Resources/MarkdownHelp.md")));
    GUIHelper::showWidgetAsDialog(&view, "Markdown help", false);
}

void MainWindow::textChanged()
{
	updateHTML();

    //update window title
    modified_ = true;
	updateWindowTitle();
}

void MainWindow::updateHTML()
{
	QString text = markdown(ui->plain->toPlainText());
	QString base_url = "file:/" + QFileInfo(file_).canonicalPath() + "/";
	ui->html->setHtml(text, base_url);
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

void MainWindow::openExternalLink(QUrl url)
{
    QDesktopServices::openUrl(url);
}

void MainWindow::loadFile(QString filename)
{
	askWetherToStoreFile();

	if (!QFile::exists(filename))
	{
		QMessageBox::warning(this, "File missing", "File ' " + filename + "' is does not exist!");
		filename = "";
	}

    file_ = filename;
    if (filename=="")
    {
        ui->plain->setPlainText("");
    }
    else
	{
        ui->plain->setPlainText(fileText(filename));
        addRecentFile(filename);
    }

    modified_ = false;
    updateWindowTitle();

	//update browser
	QString data_path = QFileInfo(Settings::string("data_folder")).canonicalFilePath();
	QString file_path = QFileInfo(filename).canonicalFilePath();
	if (file_path.startsWith(data_path))
	{
		ui->browser->show();
		ui->browser->setSelectedFile(filename);
	}
	else
	{
		ui->browser->hide();
	}
}

void MainWindow::updateWindowTitle()
{
	QString title = qApp->applicationName();
	if (file_!="")
	{
		title += " - " + file_ + (modified_ ? "*" : "");
	}
	setWindowTitle(title);
}

void MainWindow::addRecentFile(QString filename)
{
    QStringList files = Settings::stringList("recent_files");

    files.prepend(filename.replace("\\", "/"));
    files.removeDuplicates();
    while(files.count()>15) files.pop_back();

    Settings::setStringList("recent_files", files);

    updateRecentFilesMenu();
}

void MainWindow::removeRecentFile(QString filename)
{
    QStringList files = Settings::stringList("recent_files");
    files.removeAll(filename.replace("\\", "/"));
    Settings::setStringList("recent_files", files);

    updateRecentFilesMenu();
}

void MainWindow::initSettings()
{
	//general
	QString default_data_path = QFileInfo(qApp->applicationDirPath() + "\\data\\").canonicalFilePath();
	Settings::setString("data_folder", Settings::string("data_folder", default_data_path));
	//editor
	Settings::setString("font", Settings::string("font", "Courier New"));
	Settings::setInteger("font_size", Settings::integer("font_size", 10));
	Settings::setInteger("tab_width", Settings::integer("tab_width", 4));
	//view
	Settings::setString("style", Settings::string("style", "GitHub"));
	//misc
    Settings::setString("open_folder", Settings::string("open_folder", qApp->applicationDirPath()));
}

void MainWindow::applySettings()
{
	//general
	ui->browser->setBaseDirectory(Settings::string("data_folder"));

	//editor
    QFont font(Settings::string("font"), Settings::integer("font_size"));
	ui->plain->setTabStopWidth(Settings::integer("tab_width") * QFontMetrics(font).width(' '));
	ui->plain->setFont(font);
}

void MainWindow::updateRecentFilesMenu()
{
    ui->menuRecentFiles->clear();

    QStringList files = Settings::stringList("recent_files");
    foreach(QString file, files)
    {
        ui->menuRecentFiles->addAction(file, this, SLOT(openRecentFile()));
	}
}

void MainWindow::askWetherToStoreFile()
{
	if (file_=="" || !modified_) return;

	QMessageBox box(this);
	box.setWindowTitle(QApplication::applicationName());
	box.setText("Save changes to '" + file_ + "'?");
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	box.setDefaultButton(QMessageBox::No);
	if (box.exec() == QMessageBox::Yes)
	{
		on_actionSave_triggered();
	}
}

QString MainWindow::markdown(QString in)
{
    if(in.size()==0) return "";

    struct buf *ib, *ob;
    struct sd_callbacks cbs;
    struct html_renderopt opts;
    struct sd_markdown *mkd;

    QByteArray qba = in.toUtf8();
    const char* txt = qba.constData();
    ib = bufnew(qba.size());
    bufputs(ib,txt);
    ob = bufnew(64);
    sdhtml_renderer(&cbs,&opts,0);
    mkd = sd_markdown_new(0,16,&cbs,&opts);
    sd_markdown_render(ob,ib->data,ib->size,mkd);
    sd_markdown_free(mkd);

	QString output = "<html><head><meta charset=\"utf-8\"><link type=\"text/css\" rel=\"stylesheet\" href=\"qrc:/Resources/" + Settings::string("style") + ".css\"/></head><body>";
	output.append(QString::fromUtf8(bufcstr(ob)));
	output.append("</body></html>");

	return output;
}
