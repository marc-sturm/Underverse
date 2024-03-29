#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog
	: public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QWidget *parent = 0);
	~SettingsDialog();

private slots:
	void changePage();
	void loadSettings();
    void storeSettings();
	void updateDataFolder();
	void selectGitExecutable();

private:
	Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
