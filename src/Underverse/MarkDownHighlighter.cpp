#include "MarkDownHighlighter.h"
#include <QRegularExpression>

MarkDownHighlighter::MarkDownHighlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent)
{
}

void MarkDownHighlighter::highlightBlock(const QString& text)
{
	QTextCharFormat format;
	format.setFontWeight(QFont::Bold);
	format.setForeground(Qt::darkMagenta);

	QRegularExpression expression("^#.*$", QRegularExpression::MultilineOption);
	auto it = expression.globalMatch(text);
	while (it.hasNext())
	{
		auto match = it.next();
		setFormat(match.capturedStart(), match.capturedLength(), format);
	}
}

