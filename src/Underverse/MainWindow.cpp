#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QInputDialog>
#include <QScrollBar>
#include <QProcess>

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
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToClose()));

    connect(ui->html, SIGNAL(anchorClicked(QUrl)), this, SLOT(openExternalLink(QUrl)));
    connect(ui->plain, SIGNAL(textChanged()), this, SLOT(textChanged()));

    connect(ui->search, SIGNAL(textEdited(QStringList)), ui->browser, SLOT(setSearchTerms(QStringList)));
    connect(ui->search, SIGNAL(textEdited(QStringList)), this, SLOT(updateHTML()));
    connect(ui->browser, SIGNAL(fileSelected(QString)), this, SLOT(loadFile(QString)));

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(updateToolBar()));

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

    QStringList text = ui->plain->toPlainText().split("\n");
    while(text.last().trimmed().isEmpty())
    {
        text.removeLast();
    }
    Helper::storeTextFile(file_, text);
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

void MainWindow::on_actionOpenNotesFolder_triggered()
{
    QDesktopServices::openUrl(QUrl(notesFolder()));
}

void MainWindow::on_actionMarkdownHelp_triggered()
{
    QTextBrowser* view = new QTextBrowser();
    view->setOpenExternalLinks(false);
    connect(view, SIGNAL(anchorClicked(QUrl)), this, SLOT(openExternalLink(QUrl)));
    view->setHtml(markdownToHtml(Helper::fileText(":/Resources/MarkdownHelp.md")));
    auto dlg = GUIHelper::createDialog(view, "Markdown help", "", false);
	dlg->setMinimumSize(800, 600);
    dlg->exec();
}

void MainWindow::on_actionAddImage_triggered()
{
    //get image file name
    QString image = QFileDialog::getOpenFileName(this, "Select image file", notesFolder(), "Image (*.jpg *.png *.gif *.tiff);;All files (*.*)");
	if (image=="") return;

    //copy image to 'images' folder of markdown file
    QDir(fileFolder()).mkdir("images");
    QString image_name = QFileInfo(image).fileName();
    QFile::copy(image, fileFolder() + "/images/" + image_name);

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
    NotesBrowser* browser = new NotesBrowser();
	browser->setMinimumSize(800, 600);
    browser->setBaseDirectory(notesFolder());
    auto dlg = GUIHelper::createDialog(browser, "Select page", "", true);
    if (dlg->exec()==QDialog::Rejected) return;

    //insert text
    QString file = browser->selectedFile();
    file = file.replace(notesFolder(), "");
    ui->plain->textCursor().insertText("[" + QFileInfo(file).fileName().replace(".md", "") + "](" + file + ")");
}

void MainWindow::on_actionAddLinkAttachment_triggered()
{
    //get attachment file name
    QString attachment = QFileDialog::getOpenFileName(this, "Select attachment file", notesFolder());
    if (attachment=="") return;

    //copy attachment to 'attachments' folder of markdown file
    QDir(fileFolder()).mkdir("attachments");
    QString attachment_name = QFileInfo(attachment).fileName();
    QFile::copy(attachment, fileFolder() + "/attachments/" + attachment_name);

    //insert text
    ui->plain->textCursor().insertText("[" + attachment_name + "](attachments/" + attachment_name + ")");
}

void MainWindow::on_actionSearch_triggered()
{
	if (notes_mode_)
    {
        ui->search->setFocus();
    }
    else
    {
        QString terms = QInputDialog::getText(this, "Search", "terms");
        ui->search->setText(terms);
    }
}

void MainWindow::on_actionOpenNotes_triggered()
{
	loadFile("");
}

void MainWindow::on_actionDebug_triggered()
{
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
    QString text = markdownToHtml(ui->plain->toPlainText());

    //highlight search terms
    const QString start_tag = "<span style=\"background-color: yellow;\">";
    const QString end_tag = "</span>";
    int index = -1;
    foreach(const QString& term, ui->search->terms())
    {
        int from_index = 0;
        while((index = text.indexOf(term, from_index, Qt::CaseInsensitive))!=-1)
        {
            text.insert(index, start_tag);
            text.insert(index+start_tag.length()+term.length(), end_tag);
            from_index =index+start_tag.length()+term.length()+end_tag.length();
        }
    }

    //store scrollbar position
    int scroll_pos = ui->html->verticalScrollBar()->value();

    //update
    ui->html->setSearchPaths(QStringList() << fileFolder());
    ui->html->setHtml(text);

    //restore scrollbar position
    ui->html->verticalScrollBar()->setValue(scroll_pos);
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
    QString url_str = url.toString().trimmed();

    //web link
    if (url_str.startsWith("http://") || url_str.startsWith("https://"))
    {
        QDesktopServices::openUrl(url);
        return;
    }

    //link relative to notes folder
    if (fileIsInNotesFolder() && QFile::exists(notesFolder() + url_str))
    {
        loadFile(notesFolder() + url_str);
        return;
    }

    //link relative to current file
    if (QFile::exists(fileFolder() + url_str))
    {
        loadFile(fileFolder() + url_str);
        return;
    }

	QMessageBox::warning(this, "Link error", "Could not open link to file: " + url_str);
}

void MainWindow::aboutToClose()
{
	askWetherToStoreFile();
	askForGitCommit();
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
	if (file_=="" || fileIsInNotesFolder())
    {
        ui->browser->show();
        ui->search->show();
		notes_mode_ = true;

		if (file_!="")
		{
			ui->browser->setSelectedFile(file_);
		}
    }
    else
    {
        ui->browser->hide();
        ui->search->hide();
		notes_mode_ = false;
	}

    //apply mode from settings
    QString mode = Settings::string("mode");
    if (mode=="View")
    {
        ui->plain->setVisible(false);
    }
    else if (mode=="Edit")
    {
        ui->plain->setVisible(true);
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
    bool in_notes = fileIsInNotesFolder();

    ui->actionAddLinkGlobal->setEnabled(has_focus && file_open);
    ui->actionAddImage->setEnabled(has_focus && file_open && in_notes);
    ui->actionAddLinkMarkdown->setEnabled(has_focus && file_open && in_notes);
    ui->actionAddLinkAttachment->setEnabled(has_focus && file_open && in_notes);
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
    //general
	QString data_folder = Settings::string("data_folder", true);
    if (data_folder=="")
	{
		data_folder = qApp->applicationDirPath() + "/data/";
		QDir(data_folder).mkpath(".");
	}
    Settings::setString("data_folder", data_folder);
	Settings::setString("mode", Settings::contains("mode") ? Settings::string("mode") : "Edit");

	QString git_exe = Settings::string("git_exe", true);
	if (git_exe=="")
	{
		git_exe = QStandardPaths::findExecutable("git");
	}
	Settings::setString("git_exe", git_exe);

	//editor
    Settings::setString("font", Settings::contains("font") ? Settings::string("font") : "Courier New");
    Settings::setInteger("font_size", Settings::contains("font_size") ? Settings::integer("font_size") : 10);
    Settings::setInteger("tab_width", Settings::contains("tab_width") ? Settings::integer("tab_width") : 4);

	//misc
    Settings::setString("open_folder", Settings::contains("open_folder") ? Settings::string("open_folder") : qApp->applicationDirPath());
}

void MainWindow::applySettings()
{
    //general
    ui->browser->setBaseDirectory(notesFolder());

    //editor
    QFont font(Settings::string("font"), Settings::integer("font_size"));
    ui->plain->setTabStopWidth(Settings::integer("tab_width") * QFontMetrics(font).width(' '));
    ui->plain->setFont(font);
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

void MainWindow::askForGitCommit()
{
	//pre-checks
	if(!notes_mode_) return;
	QString notes_folder = notesFolder();
	if (!Git::isRepo(notes_folder)) return;

	//get Git status
	QHash<QString, GitStatus> status;
	try
	{
		status = Git::status(notes_folder);
	}
	catch (const Exception& e)
	{
		GUIHelper::showException(this, e, "Error in getting git status");
	}
	if (status.isEmpty()) return;

	//determine files
	QStringList files_add_commit;
	QStringList files_delete;
	for (auto it=status.begin(); it!=status.end(); ++it)
	{
		GitStatus status_enum = it.value();

		if (status_enum==GitStatus::MODIFIED || status_enum==GitStatus::ADDED || status_enum==GitStatus::NOT_VERSIONED)
		{
			files_add_commit << it.key();
		}
		if (status_enum==GitStatus::DELETED)
		{
			files_delete << it.key();
		}
	}

	//ask user if the changes should be committed
	QStringList lines;
	lines << "Do you  want to commit your changes to Git?";
	lines << "";
	if (files_add_commit.count()>0)
	{
		lines << "The following files will be added/committed:";
		foreach(QString file, files_add_commit) lines << "  " + file.remove(notes_folder);
		lines << "";
	}
	if (files_delete.count()>0)
	{
		lines << "The following files will be deleted:";
		foreach(QString file, files_delete) lines << "  " + file.remove(notes_folder);
		lines << "";
	}
	if (QMessageBox::question(this, "Git commit", lines.join("\n"))!=QMessageBox::Yes) return;

	//commit and push
	try
	{
		QApplication::setOverrideCursor(Qt::BusyCursor);

		QString git_exe = Settings::string("git_exe", true);
		foreach(QString file, files_add_commit)
		{
			execute(git_exe, QStringList() << "add" << file, notes_folder);
		}
		foreach(QString file, files_delete)
		{
			execute(git_exe, QStringList() << "rm" << file, notes_folder);
		}
		execute(git_exe, QStringList() << "commit" << "-m 'automated commit by Underverse'", notes_folder);
		QByteArray push_result = execute(git_exe, QStringList() << "push", notes_folder);

		QMessageBox::information(this, "Git push performed", push_result);

		QApplication::restoreOverrideCursor();
	}
	catch (const Exception& e)
	{
		GUIHelper::showException(this, e, "Error committing to git");
	}

}

QString MainWindow::markdownToHtml(QString in)
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

    //header
    QString output = "<html>\n";
    output += "<head>\n";
    output += "  <meta charset=\"utf-8\">\n";
    output += "</head>\n";

    //boby
    output += "<body>\n";
    output += QString::fromUtf8(bufcstr(ob));
    output += "</body>\n";
    output += "</html>";

    return output;
}

QString MainWindow::notesFolder()
{
    return QFileInfo(Settings::string("data_folder")).canonicalFilePath() + "/";
}

bool MainWindow::fileIsInNotesFolder()
{
    return fileFolder().startsWith(notesFolder());
}

QString MainWindow::fileFolder()
{
    return QFileInfo(file_).canonicalPath() + "/";
}
