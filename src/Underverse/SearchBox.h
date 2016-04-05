#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QWidget>

namespace Ui {
class SearchBox;
}

class SearchBox
	: public QWidget
{
	Q_OBJECT

public:
	explicit SearchBox(QWidget *parent = 0);
	~SearchBox();
	void setText(QString text);
	QStringList terms();

signals:
	void textEdited(QStringList);

private slots:
	void clearText();
	void onTextChanged();

private:
	Ui::SearchBox *ui;
	QStringList terms_;
};

#endif // SEARCHBOX_H
