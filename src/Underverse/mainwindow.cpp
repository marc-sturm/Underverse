#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QInputDialog>

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

	connect(ui->search, SIGNAL(textEdited(QString)), ui->browser, SLOT(setSearchTerms(QString)));
	connect(ui->browser, SIGNAL(fileSelected(QString)), this, SLOT(loadFile(QString)));

	connect(qApp, SIGNAL(focusObjectChanged(QObject*)), this, SLOT(updateToolBar()));

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
	QMessageBox::about(this, "About " + QApplication::applicationName(),  "<p>" + QApplication::applicationName() + " " + QApplication::applicationVersion() +"<p>It is provided under the <a href=\"http://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html\">GNU General Public License (GPL) Version 2.0</a>.<p>This program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.");
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
	if (file_=="" || !modified_) return;

    Helper::storeTextFile(file_, QStringList() << ui->plain->toPlainText());
    modified_ = false;
    updateWindowTitle();
}

void MainWindow::on_actionClose_triggered()
{
	loadFile("");
}

void MainWindow::on_actionToggleEditing_triggered()
{
	ui->plain->setVisible(!ui->plain->isVisible());
	updateWidths();
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

void MainWindow::on_actionOpenDataFolder_triggered()
{
	QDesktopServices::openUrl(QUrl(Settings::string("data_folder")));
}

void MainWindow::on_actionMarkdownHelp_triggered()
{
	QWebView view;
	view.page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	connect(&view, SIGNAL(linkClicked(QUrl)), this, SLOT(openExternalLink(QUrl)));
	view.setHtml(markdown(Helper::fileText(":/Resources/MarkdownHelp.md")));
	GUIHelper::showWidgetAsDialog(&view, "Markdown help", false);
}

void MainWindow::on_actionAddImage_triggered()
{
	//get image file name
	QString image = QFileDialog::getOpenFileName(this, "Select image file", Settings::string("image_folder"), "Image (*.jpg *.png *.gif *.tiff);;All files (*.*)");
	if (image=="") return;
	Settings::setString("image_folder", QFileInfo(image).canonicalPath());

	//copy image to 'images' folder of markdown file
	QString folder = QFileInfo(file_).path();
	QDir(folder).mkdir("images");
	QString image_name = QFileInfo(image).fileName();
	QFile::copy(image, folder + "/images/" + image_name);

	//insert text
	ui->plain->textCursor().insertText("![" + image_name + "](images/" + image_name + ")");
}

void MainWindow::on_actionAddLinkGlobal_triggered()
{
	QString url = QInputDialog::getText(this, "Enter link URL", "URL:");
	if (url=="") return;

	QString text = QInputDialog::getText(this, "Enter link text", "Text:", QLineEdit::Normal, url);
	if (text=="") return;

	//insert text
	ui->plain->textCursor().insertText("[" + text + "](" + url + ")");
}

void MainWindow::on_actionAddLinkMarkdown_triggered()
{
	//select notes page
	NotesBrowser browser;
	QString data_folder = Settings::string("data_folder");
	browser.setBaseDirectory(data_folder);
	bool accepted = GUIHelper::showWidgetAsDialog(&browser, "Select page", true);
	if (!accepted) return;

	//insert text
	QString file = browser.selectedFile();
	file = file.replace(data_folder, "");
	ui->plain->textCursor().insertText("[" + QFileInfo(file).fileName().replace(".md", "") + "](" + file + ")");
}

void MainWindow::on_actionAddLinkAttachment_triggered()
{
	//get attachment file name
	QString attachment = QFileDialog::getOpenFileName(this, "Select attachment file", Settings::string("attachment_folder"));
	if (attachment=="") return;
	Settings::setString("attachment_folder", QFileInfo(attachment).canonicalPath());

	//copy attachment to 'attachments' folder of markdown file
	QString folder = QFileInfo(file_).path();
	QDir(folder).mkdir("attachments");
	QString attachment_name = QFileInfo(attachment).fileName();
	QFile::copy(attachment, folder + "/attachments/" + attachment_name);

	//insert text
	ui->plain->textCursor().insertText("[" + attachment_name + "](attachments/" + attachment_name + ")");
}

void MainWindow::on_actionSearch_triggered()
{
	ui->search->setFocus();
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
	QString url_str =  url.toString().replace("file://","");
	QString data_folder = Settings::string("data_folder");
	if (QFile::exists(data_folder + url_str))
	{
		loadFile(data_folder + url_str);
	}
	else
	{
		QDesktopServices::openUrl(url);
	}
}

void MainWindow::loadFile(QString filename)
{
	askWetherToStoreFile();

	if (filename!="" && !QFile::exists(filename))
	{
		QMessageBox::warning(this, "File missing", "File ' " + filename + "' does not exist!");
		filename = "";
	}

    file_ = filename;
	if (file_=="")
    {
        ui->plain->setPlainText("");
		ui->plain->setEnabled(false);
    }
    else
	{
		ui->plain->setPlainText(Helper::fileText(file_));
		ui->plain->setEnabled(true);
		addRecentFile(file_);
    }

    modified_ = false;
    updateWindowTitle();
	updateToolBar();

	//update browser
	if (fileInNotesFolder(file_))
	{
		ui->browser->show();
		ui->search->show();
		ui->browser->setSelectedFile(file_);
	}
	else if (file_=="")
	{
		ui->browser->show();
		ui->search->show();
	}
	else
	{
		ui->browser->hide();
		ui->search->hide();
	}
	updateWidths();
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

void MainWindow::updateToolBar()
{
	bool has_focus = ui->plain->hasFocus();
	bool file_open = (file_!="");
	bool in_notes = fileInNotesFolder(file_);

	ui->actionAddImage->setEnabled(has_focus && file_open && in_notes);
	ui->actionAddLinkGlobal->setEnabled(has_focus && file_open);
	ui->actionAddLinkMarkdown->setEnabled(has_focus && file_open && in_notes);
	ui->actionAddLinkAttachment->setEnabled(has_focus && file_open && in_notes);
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
	QString data_folder = Settings::string("data_folder");
	if (data_folder=="")
	{
		data_folder = qApp->applicationDirPath() + "/data/";
		QDir(data_folder).mkpath(".");
	}
	Settings::setString("data_folder", data_folder);
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

void MainWindow::updateWidths()
{
	QList<int> widths;

	int browser_with = ui->browser->isVisible() ? 250 : 0;
	widths << browser_with;

	int w = width() - browser_with;
	if (ui->plain->isVisible())
	{
		widths << w/2 << w/2;
	}
	else
	{
		widths << 0 << w;
	}
	ui->splitter->setSizes(widths);
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

bool MainWindow::fileInNotesFolder(QString filename)
{
	QString data_path = QFileInfo(Settings::string("data_folder")).canonicalFilePath();
	QString file_path = QFileInfo(filename).canonicalFilePath();

	return file_path.startsWith(data_path);
}
