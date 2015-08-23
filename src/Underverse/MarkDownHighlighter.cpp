#include "MarkDownHighlighter.h"

MarkDownHighlighter::MarkDownHighlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent)
{
}

void MarkDownHighlighter::highlightBlock(const QString& text)
{
	QTextCharFormat format;
	format.setFontWeight(QFont::Bold);
	format.setForeground(Qt::darkMagenta);

	QRegExp expression("^#.*$");
	int index = text.indexOf(expression);
	while (index >= 0)
	{
		int length = expression.matchedLength();
		setFormat(index, length, format);
		index = text.indexOf(expression, index + length);
	}
}

