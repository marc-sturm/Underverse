#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThreadPool>
#include <QUrl>
#include "GitWorker.h"

namespace Ui {
class MainWindow;
}

class MainWindow
	: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void resizeEvent(QResizeEvent* event);

    //Converts markdown code to HTML
    static QString markdownToHtml(QString in);
    //Returns the notes folder path
    static QString notesFolder();
    //Returns if the current file is in the notes folder
    bool fileIsInNotesFolder();
    //Returns the folder of the current file
    QString fileFolder();

    void initSettings();
    void applySettings();
    void updateWindowTitle();
    void updateRecentFilesMenu();
    void updateWidths();
    void addRecentFile(QString filename);
    void removeRecentFile(QString filename);
	QByteArray execute(QString exe, QStringList args, QString wd="");

public slots:
    void on_actionAbout_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionClose_triggered();
    void on_actionToggleEditing_triggered();
    void on_actionSettings_triggered();
    void on_actionOpenNotesFolder_triggered();
	void on_actionOpenSettingsFiles_triggered();
    void on_actionMarkdownHelp_triggered();
    void on_actionAddImage_triggered();
    void on_actionAddLinkGlobal_triggered();
    void on_actionAddLinkMarkdown_triggered();
    void on_actionAddLinkAttachment_triggered();
    void on_actionSearch_triggered();
    void on_actionOpenNotes_triggered();
	void on_actionDebug_triggered();
	void on_actionGitPull_triggered();
	void on_actionGitPush_triggered();

	void updateGitStatus(GitAction action);
    void loadFile(QString filename);
    void textChanged();
    void updateHTML();
    void updateToolBar();
    void openRecentFile();
    void openExternalLink(QUrl url);
	void askWetherToStoreFile();
	void highlightPullButton(bool data_available);
	void highlightPushButton(bool data_available);
	void showGitError(QString error_message);

private:
	Ui::MainWindow *ui;
	QString file_;
    bool modified_;
	bool notes_mode_;
	QThreadPool pool_;


	void closeEvent(QCloseEvent* event);
};

#endif // MAINWINDOW_H
