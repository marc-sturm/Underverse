#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>

class Editor
	: public QPlainTextEdit
{
	Q_OBJECT

public:
	Editor(QWidget* parent = 0);

protected:
	void keyPressEvent(QKeyEvent* e) override;
	void changeSelectionIndentation(bool increase);
};

#endif // EDITOR_H
