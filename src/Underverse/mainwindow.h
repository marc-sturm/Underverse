#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	void resizeEvent(QResizeEvent* event);

    static QByteArray fileText(QString filename);
    static QString markdown(QString in);
    void loadFile(QString filename);
    void updateWindowTitle();
    void addRecentFile(QString filename);
    void removeRecentFile(QString filename);

	void initSettings();
    void applySettings();
    void updateRecentFilesMenu();

public slots:
    void on_actionAbout_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionClose_triggered();
	void on_actionSettings_triggered();
    void on_actionMarkdownHelp_triggered();
    void textChanged();
    void openRecentFile();
    void openExternalLink(QUrl url);

private:
    Ui::MainWindow *ui;
    QString file_;
    bool modified_;
};

#endif // MAINWINDOW_H
