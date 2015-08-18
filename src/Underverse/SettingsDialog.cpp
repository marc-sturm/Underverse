#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "Settings.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	//connect page buttons
	foreach(QPushButton* button, findChildren<QPushButton*>())
	{
		if (button->objectName().endsWith("_button"))
		{
			connect(button, SIGNAL(clicked(bool)), this, SLOT(changePage()));
		}
	}

	loadSettings();
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::changePage()
{
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
