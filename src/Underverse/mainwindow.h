#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>

namespace Ui {
class MainWindow;
}

class MainWindow
	: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	void resizeEvent(QResizeEvent* event);

	static QString fileText(QString filename);
	static QString markdown(QString in);
	static bool fileInNotesFolder(QString filename);

	void initSettings();
    void applySettings();
	void updateWindowTitle();
	void updateToolBar();
	void updateRecentFilesMenu();
	void addRecentFile(QString filename);
	void removeRecentFile(QString filename);

public slots:
    void on_actionAbout_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionClose_triggered();
	void on_actionSettings_triggered();
    void on_actionMarkdownHelp_triggered();
	void on_actionAddImage_triggered();
	void on_actionAddLinkGlobal_triggered();
	void on_actionAddLinkMarkdown_triggered();
	void on_actionAddLinkAttachment_triggered();

	void loadFile(QString filename);
    void textChanged();
	void updateHTML();
    void openRecentFile();
    void openExternalLink(QUrl url);
	void askWetherToStoreFile();

private:
    Ui::MainWindow *ui;
    QString file_;
    bool modified_;
};

#endif // MAINWINDOW_H
