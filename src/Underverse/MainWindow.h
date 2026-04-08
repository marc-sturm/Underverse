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
    void initSettings();
	void applySettings();
	void updateRecentFilesMenu();
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
	void on_actionOpenSettingsFiles_triggered();
	void on_actionMarkdownHelp_triggered();
	void on_actionSearch_triggered();

    void loadFile(QString filename);
    void openRecentFile();
	void updateWindowTitle();
	void searchTermsChanged();

private:
	Ui::MainWindow* ui;

	void closeEvent(QCloseEvent* event);
	bool eventFilter(QObject* obj, QEvent* event); //event filter for search
};

#endif // MAINWINDOW_H
