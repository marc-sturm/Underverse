#include "Git.h"
#include "Settings.h"
#include "GUIHelper.h"

#include <QProcess>
#include <QFileInfo>

bool Git::isRepo(QString dir)
{
	return QFile::exists(dir + "/.git");
}

QHash<QString, GitStatus> Git::status(QString dir)
{
	//check if git exe is set
	QString git_exe = Settings::string("git_exe", true);
	if (git_exe=="") THROW(InformationMissingException, "Git executable not found in settings!");

	//execute
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(git_exe, QStringList() << "-C"  << dir << "status" << "--porcelain");
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
		else if (status=="A" || status=="AM")
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

		QString filename = dir + "/" + line.mid(pos+1);
		filename = filename.replace("//", "/"); //remove duplicated slashes
		if (status_enum!=GitStatus::DELETED) filename = QFileInfo(filename).canonicalFilePath();
		output[filename] = status_enum;
	}

	return output;
}

bool Git::pullAvailable(QString dir)
{
	//check if git exe is set
	QString git_exe = Settings::string("git_exe", true);
	if (git_exe=="") THROW(InformationMissingException, "Git executable not found in settings!");

	//update remote info
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(git_exe, QStringList() << "-C"  << dir << "remote" << "update");
	bool success = process.waitForFinished(-1);
	QStringList output = QString(process.readAll()).split("\n");
	if (!success) THROW(Exception, "Calling 'git remote update' failed:\n"+output.join("\n"));

	//get status
	process.start(git_exe, QStringList() << "-C"  << dir << "status" << "-u" << "no");
	success = process.waitForFinished(-1);
	output = QString(process.readAll()).split("\n");
	if (!success) THROW(Exception, "Calling 'git status' failed:\n"+output.join("\n"));

	//check status
	foreach(QString line, output)
	{
		if (line.contains("Your branch is behind") && line.contains("can be fast-forwarded")) return true;
	}
	return false;
}
