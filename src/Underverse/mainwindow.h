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
    void loadFile(QString filename);
    void updateWindowTitle();
    void addRecentFile(QString filename);
    void updateRecentFilesMenu();

public slots:
    void on_actionAbout_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionClose_triggered();
    void textChanged();
    void openRecentFile();

private:
    Ui::MainWindow *ui;
    QString file_;
    bool modified_;
};

#endif // MAINWINDOW_H
