#include "GitWorker.h"
#include "Git.h"
#include "Exceptions.h"
#include <QFile>

GitWorker::GitWorker(QString folder, GitAction action)
	: QRunnable()
	, folder_(folder)
	, action_(action)
{	
}

void GitWorker::run()
{
	if (!QFile::exists(folder_)) return;
	if (!Git::isRepo(folder_)) return;

	try
	{
		if (action_==GitAction::PUSH)
		{
			if (!Git::status(folder_).isEmpty())
			{
				emit actionPossible(true);
				return;
			}
		}
		else
		{
			if (Git::pullAvailable(folder_))
			{
				emit actionPossible(true);
				return;
			}
		}
	}
	catch(Exception& e)
	{
		emit error(e.message());
		return;
	}
	catch(...)
	{
		emit error("Unknown error");
		return;
	}

	actionPossible(false);
}
