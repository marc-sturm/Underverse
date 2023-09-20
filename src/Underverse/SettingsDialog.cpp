#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "Settings.h"

#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
    ui->stack->setCurrentIndex(0);
    connect(this, SIGNAL(accepted()), this, SLOT(storeSettings()));

	//connect page buttons
	foreach(QPushButton* button, findChildren<QPushButton*>())
	{
        if (button->isFlat())
		{
			connect(button, SIGNAL(clicked(bool)), this, SLOT(changePage()));
		}
	}

	connect(ui->data_folder, SIGNAL(clicked(QPoint)), this, SLOT(updateDataFolder()));
	connect(ui->git_exe, SIGNAL(clicked(QPoint)), this, SLOT(selectGitExecutable()));

	loadSettings();
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::changePage()
{
    ui->title->setText("<i><font size=+1>" + qobject_cast<QPushButton*>(sender())->text() + "</font></i>");

	QString page_name = sender()->objectName().replace("_button", "");
	for (int i=0; i<ui->stack->count(); ++i)
	{
		if (ui->stack->widget(i)->objectName()==page_name)
		{
			ui->stack->setCurrentIndex(i);
		}
	}
}

void SettingsDialog::loadSettings()
{
	//general
	ui->data_folder->setText(Settings::string("data_folder"));
	ui->mode->setCurrentText(Settings::string("mode"));
	ui->git_exe->setText(Settings::string("git_exe"));

	//editor
	ui->font->setCurrentText(Settings::string("font"));
	ui->font_size->setValue(Settings::integer("font_size"));
	ui->tab_width->setValue(Settings::integer("tab_width"));
}

void SettingsDialog::storeSettings()
{
    //general
	Settings::setString("data_folder", ui->data_folder->text());
	Settings::setString("mode", ui->mode->currentText());
	Settings::setString("git_exe", ui->git_exe->text());

    //editor
    Settings::setString("font", ui->font->currentText());
    Settings::setInteger("font_size", ui->font_size->value());
	Settings::setInteger("tab_width", ui->tab_width->value());
}

void SettingsDialog::updateDataFolder()
{
	QString dir = QFileDialog::getExistingDirectory(this, "Select data folder", Settings::string("data_folder"));
	if (dir=="") return;

	ui->data_folder->setText(QFileInfo(dir).canonicalFilePath());
}

void SettingsDialog::selectGitExecutable()
{
	QString exe = QFileDialog::getOpenFileName(this, "Select git executable");
	if (exe=="") return;

	ui->git_exe->setText(QFileInfo(exe).canonicalFilePath());
}
