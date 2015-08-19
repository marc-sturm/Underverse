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

#include <markdown.h>
#include <html.h>
#include <buffer.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->html->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(ui->html->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(openExternalLink(QUrl)));

    connect(ui->plain, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(on_actionSave_triggered()));
    if (qApp->arguments().count()==2)
    {
        loadFile(qApp->arguments().at(1));
    }

	initSettings();
    applySettings();
    updateRecentFilesMenu();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);
	int w = width()/2;
    ui->splitter->setSizes(QList<int>() << w << w);
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
    QMessageBox::about(this, "About " + QApplication::applicationName(), QApplication::applicationName() + " " + QApplication::applicationVersion() +"\n\nThis program is free software.\n\nThis program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.");
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
    if (!modified_) return;

    Helper::storeTextFile(file_, QStringList() << ui->plain->toPlainText());
    modified_ = false;
    updateWindowTitle();
}

void MainWindow::on_actionClose_triggered()
{
    on_actionSave_triggered();
	loadFile("");
}

void MainWindow::on_actionSettings_triggered()
{
	SettingsDialog dlg(this);
    if (dlg.exec()==QDialog::Accepted)
    {
        applySettings();
        //update HTML in case style changed
        textChanged();
    }
}

void MainWindow::on_actionMarkdownHelp_triggered()
{
    QWebView view;
    view.setHtml(markdown(fileText(":/Resources/MarkdownHelp.md")));
    GUIHelper::showWidgetAsDialog(&view, "Markdown help", false);
}

void MainWindow::textChanged()
{
    //update html
    QString text = ui->plain->toPlainText();
    text = markdown(text);
    text.prepend("<html><head><meta charset=\"utf-8\"><link type=\"text/css\" rel=\"stylesheet\" href=\"qrc:/Resources/" + Settings::string("style") + ".css\"/></head><body>");
    text.append("</body></html>");
    ui->html->setHtml(text);

    //update window title
    modified_ = true;
    updateWindowTitle();
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
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(qApp->applicationName() + " - " + file_ + (modified_ ? "*" : ""));
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
	Settings::setString("data_folder", Settings::string("data_folder", qApp->applicationDirPath() + "\\data\\"));
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
    //general - TODO

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

    return QString::fromUtf8(bufcstr(ob));
}
