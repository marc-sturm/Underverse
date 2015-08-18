#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "Settings.h"

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

	loadSettings();
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::changePage()
{
    ui->title->setText("<i>" + qobject_cast<QPushButton*>(sender())->text() + "</i>");

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
