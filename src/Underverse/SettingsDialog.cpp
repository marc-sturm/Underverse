#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "Settings.h"

#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
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

	connect(ui->change_data_folder, SIGNAL(clicked(bool)), this, SLOT(updateDataFolder()));

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

	//editor
	ui->font->setCurrentText(Settings::string("font"));
	ui->font_size->setValue(Settings::integer("font_size"));
	ui->tab_width->setValue(Settings::integer("tab_width"));

	//view
    ui->style->setCurrentText(Settings::string("style"));
}

void SettingsDialog::storeSettings()
{
    //general
    Settings::setString("data_folder", ui->data_folder->text());

    //editor
    Settings::setString("font", ui->font->currentText());
    Settings::setInteger("font_size", ui->font_size->value());
    Settings::setInteger("tab_width", ui->tab_width->value());

    //view
	Settings::setString("style", ui->style->currentText());
}

void SettingsDialog::updateDataFolder()
{
	QString dir = QFileDialog::getExistingDirectory(this, "Select data folder", Settings::string("data_folder"));
	if (dir=="") return;

	ui->data_folder->setText(QFileInfo(dir).canonicalFilePath());
}
