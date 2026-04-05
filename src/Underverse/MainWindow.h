#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThreadPool>
#include <QUrl>

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
    void initSettings();
	void applySettings();
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
    void on_actionOpenNotesFolder_triggered();
	void on_actionOpenSettingsFiles_triggered();
	void on_actionMarkdownHelp_triggered();
	void on_actionSearch_triggered();

    void loadFile(QString filename);
    void openRecentFile();
	void updateWindowTitle();

private:
	Ui::MainWindow* ui;
	bool notes_mode_;

	void closeEvent(QCloseEvent* event);
};

#endif // MAINWINDOW_H
