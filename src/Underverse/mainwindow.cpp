#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

#include "MainWindow.h"
#include "Settings.h"
#include "Helper.h"
#include "Exceptions.h"

#include <markdown.h>
#include <html.h>
#include <buffer.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->plain, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(on_actionSave_triggered()));
    if (qApp->arguments().count()==2)
    {
        loadFile(qApp->arguments().at(1));
    }

    updateRecentFilesMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About " + QApplication::applicationName(), QApplication::applicationName() + " " + QApplication::applicationVersion() +"\n\nThis program is free software.\n\nThis program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.");
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file", Settings::string("open_folder", QCoreApplication::applicationDirPath()),"MarkDown files (*.md);;All files (*.*)");
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

void MainWindow::textChanged()
{
    //update html
    QString text = ui->plain->toPlainText();
    text = markdown(text);
    text.prepend("<html><head><meta charset=\"utf-8\"><link type=\"text/css\" rel=\"stylesheet\" href=\"qrc:/Resources/github.css\"/></head><body>");
    text.append("</body></html>");
    ui->html->setHtml(text);

    //update window title
    modified_ = true;
    updateWindowTitle();
}

void MainWindow::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());
    loadFile(action->text());
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
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            THROW(FileAccessException, "Could not open file for reading: '" + filename + "'!");
        }
        ui->plain->setPlainText(file.readAll());

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
    struct buf *ib, *ob;
    struct sd_callbacks cbs;
    struct html_renderopt opts;
    struct sd_markdown *mkd;

    if(in.size() > 0)
    {
        QByteArray qba = in.toUtf8();
        const char *txt = qba.constData();
        if(NULL == txt)
        {
            qDebug() << "txt was null!";
        }
        if(0 < qba.size())
        {
            ib = bufnew(qba.size());
            bufputs(ib,txt);
            ob = bufnew(64);
            sdhtml_renderer(&cbs,&opts,0);
            mkd = sd_markdown_new(0,16,&cbs,&opts);
            sd_markdown_render(ob,ib->data,ib->size,mkd);
            sd_markdown_free(mkd);
            return QString::fromUtf8(bufcstr(ob));
        }
        else
        {
            qDebug() <<"qstrlen was null";
        }
    }
    return "";
}
