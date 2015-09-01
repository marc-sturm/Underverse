#include "Editor.h"

#include <QDebug>

Editor::Editor(QWidget* parent)
	: QPlainTextEdit(parent)
{
	highlighter_ = new MarkDownHighlighter(document());
}

void Editor::keyPressEvent(QKeyEvent* e)
{
	if (e->key()==Qt::Key_Tab)
	{
		changeSelectionIndentation(true);
	}
	else if (e->key()==Qt::Key_Backtab)
	{
		changeSelectionIndentation(false);
	}
	else
	{
		QPlainTextEdit::keyPressEvent(e);
	}
}

void Editor::changeSelectionIndentation(bool increase)
{
	QTextCursor curs = textCursor();

	//no selection range => insert/remove tab at cursor
	if(!curs.hasSelection())
	{
		if (increase)
		{
			curs.insertText("\t");
		}
		else
		{
			curs.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
			if (curs.selectedText()=="\t")
			{
				curs.removeSelectedText();
			}
		}
		return;
	}

	//get start and end position
	int spos = curs.anchor();
	int epos = curs.position();
	if(spos > epos) std::swap(spos, epos);

	//get block start and end
	curs.setPosition(spos, QTextCursor::MoveAnchor);
	const int sblock = curs.block().blockNumber();
	curs.setPosition(epos, QTextCursor::MoveAnchor);
	const int eblock = curs.block().blockNumber();

	//indent
	curs.beginEditBlock();
	curs.setPosition(spos, QTextCursor::MoveAnchor);
	for(int i = 0; i <= eblock - sblock; ++i)
	{
		if (increase) //add a tab
		{
			curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
			curs.insertText("\t");
			curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		}
		else //remove a tab
		{
			curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
			curs.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
			if (curs.selectedText()=="\t")
			{
				curs.removeSelectedText();
			}
			curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		}
	}
	curs.endEditBlock();

	//set our cursor's selection to span all of the involved lines.
	curs.setPosition(spos, QTextCursor::MoveAnchor);
	curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
	while(curs.block().blockNumber() < eblock)
	{
		curs.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
	}
	curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	setTextCursor(curs);
}

