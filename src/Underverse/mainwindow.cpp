#include "ui_mainwindow.h"

#include <QMessageBox>

#include "MainWindow.h"

#include <markdown.h>
#include <html.h>
#include <buffer.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->plain, SIGNAL(textChanged()), this, SLOT(updateHtml()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered(bool)
{
    QMessageBox::about(this, "About " + QApplication::applicationName(), QApplication::applicationName() + " " + QApplication::applicationVersion() +"\n\nThis program is free software.\n\nThis program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.");
}

void MainWindow::updateHtml()
{
    QString text = ui->plain->toPlainText();
    text = markdown(text);
    text.prepend("<html><head><meta charset=\"utf-8\"><link type=\"text/css\" rel=\"stylesheet\" href=\"qrc:/Resources/github.css\"/></head><body>");
    text.append("</body></html>");
    ui->html->setHtml(text);
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
