#include "SearchBox.h"
#include "ui_SearchBox.h"
#include <QDebug>

SearchBox::SearchBox(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SearchBox)
{
	ui->setupUi(this);

	connect(ui->button, SIGNAL(clicked(bool)), this, SLOT(clearText()));
	connect(ui->edit, SIGNAL(editingFinished()), this, SLOT(onTextChanged()));

	setFocusProxy(ui->edit);
}

SearchBox::~SearchBox()
{
	delete ui;
}

void SearchBox::setText(QString text)
{
	ui->edit->setText(text);
	onTextChanged();
}

QStringList SearchBox::terms()
{
	return terms_;
}

void SearchBox::clearText()
{
	ui->edit->clear();
	terms_.clear();
	emit textEdited(terms_);
}

void SearchBox::onTextChanged()
{
	terms_ = ui->edit->text().split(' ', QString::SkipEmptyParts);
	emit textEdited(terms_);
}
