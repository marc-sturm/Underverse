#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static QString markdown(QString in);

public slots:
    void on_actionAbout_triggered(bool checked);
    void updateHtml();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
