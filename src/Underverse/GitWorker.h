#ifndef GITWORKER_H
#define GITWORKER_H

#include<QObject>
#include<QRunnable>

enum class GitAction
{
	PULL,
	PUSH
};

//Worker class that checks if a Git action can be preformed (i.e. if there is something to pull/push)
class GitWorker
	: public QObject
	, public QRunnable
{
	Q_OBJECT

public:
	GitWorker(QString folder, GitAction action);
	void run();

signals:
	void actionPossible(bool possible);
	void error(QString error_message);

private:
	QString folder_;
	GitAction action_;
};

#endif // GITWORKER_H
