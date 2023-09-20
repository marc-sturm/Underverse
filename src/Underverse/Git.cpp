#include "Git.h"
#include "Settings.h"
#include "GUIHelper.h"

#include <QProcess>
#include <QFileInfo>

QHash<QString, GitStatus> Git::status(QString repo_base_dir)
{
	//check if git exe is set
	QString git_exe = Settings::string("git_exe", true);
	if (git_exe=="") THROW(InformationMissingException, "Git executable not found in settings!");

	//execute scipGitStatust
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(git_exe, QStringList() << "-C"  << repo_base_dir << "status" << "--porcelain");
	bool success = process.waitForFinished(-1);
	QStringList text = QString(process.readAll()).split("\n");
	if (!success) THROW(Exception, "Calling 'git status' failed:\n"+text.join("\n"));

	//convert git status lines to hash
	QHash<QString, GitStatus> output;
	foreach(QString line, text)
	{
		line = line.trimmed();
		if (line.isEmpty()) continue;

		line = line.replace("  ", " ").replace("\"", ""); //handle escaping of files with spaces
		int pos = line.indexOf(' ');
		if (pos==-1) continue;

		//convert status to enum
		QString status = line.left(pos);
		GitStatus status_enum = GitStatus::NOT_VERSIONED;
		if (status=="M")
		{
			status_enum = GitStatus::MODIFIED;
		}
		else if (status=="D")
		{
			status_enum = GitStatus::DELETED;
		}
		else if (status=="A")
		{
			status_enum = GitStatus::ADDED;
		}
		else if (status=="??")
		{
			status_enum = GitStatus::NOT_VERSIONED;
		}
		else
		{
			THROW(FileParseException, "Git status '" + status + "' not known!");
		}

		QString filename = repo_base_dir + "/" + line.mid(pos+1);
		if (status_enum!=GitStatus::DELETED) filename = QFileInfo(filename).canonicalFilePath();
		output[filename] = status_enum;
	}

	return output;
}
