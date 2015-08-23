#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class MarkDownHighlighter
	: public QSyntaxHighlighter
{
public:
	MarkDownHighlighter(QTextDocument* parent);

	 void highlightBlock(const QString& text) override;
};

#endif // MARKDOWNHIGHLIGHTER_H
