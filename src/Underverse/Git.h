#ifndef GIT_H
#define GIT_H

#include <QString>
#include <QHash>

enum class GitStatus
{
	MODIFIED,
	ADDED,
	DELETED,
	NOT_VERSIONED
};

//Git helper
class Git
{

public:
	//Returns the directroy is a git repository.
	static bool isRepo(QString dir);

	//Returns the 'git diff' status of the files in a repository. The returned file name is the canonical file path.
	static QHash<QString, GitStatus> status(QString dir);

	//Returns if there is something to pull
	static bool pullAvailable(QString dir);

protected:
	Git() = delete;
};

#endif // GIT_H
