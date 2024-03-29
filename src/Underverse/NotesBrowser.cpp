#include "NotesBrowser.h"
#include "Settings.h"
#include "Helper.h"
#include "GUIHelper.h"
#include "Git.h"

#include <QFileSystemModel>
#include <QDebug>
#include <QMenu>
#include <QPair>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>

NotesBrowser::NotesBrowser(QWidget* parent)
	: QTreeWidget(parent)
{
	setColumnCount(1);
	setHeaderHidden(true);
	setFrameStyle(QFrame::NoFrame);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setMinimumWidth(350);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenu(QPoint)));
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelected()));
}

void NotesBrowser::setBaseDirectory(QString dir)
{
	base_dir_ = QFileInfo(dir).canonicalFilePath();
	updateView();
}

void NotesBrowser::setSearchTerms(QStringList terms)
{
	search_terms_ = terms;
	updateView();
}

void NotesBrowser::updateView()
{
	clear();

	if (search_terms_.count()==0)
	{
		QHash<QString, GitStatus> status;
		try
		{
			if (Git::isRepo(base_dir_)) status = Git::status(base_dir_);
		}
		catch (const Exception& e)
		{
			GUIHelper::showException(this, e, "Error in getting git status");
		}
		addChildren(nullptr, base_dir_, status);
		expandAll();
	}
	else
	{
		performSearch();
	}
}

void NotesBrowser::performSearch()
{
	QStringList files = Helper::findFiles(base_dir_, "*.md", true);

	//calculate scores
	typedef QPair<QString, int> FileScore;
	QList<FileScore> scores;
	foreach(QString file, files)
	{
		int score = 0;

		QString content = Helper::fileText(file);
		foreach(QString term, search_terms_)
		{
			score += 100 * file.mid(base_dir_.count()).count(term, Qt::CaseInsensitive);
			score += 1 * content.count(term, Qt::CaseInsensitive);
		}

		scores.append(qMakePair(file, score));
	}

	//sort by score DESC
	std::sort(scores.begin(), scores.end(),[](const FileScore& lhs, const FileScore& rhs){return lhs.second > rhs.second;} );

	//display hits with score>0
	foreach(FileScore pair, scores)
	{
		//qDebug() << pair.first.mid(base_dir_.count()) << pair.second;
		if (pair.second==0) break;

		QFileInfo info(pair.first);
		QString text = info.canonicalFilePath().replace(".md", "").replace(base_dir_ + "/", "");
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, text);
		item->setIcon(0, icon_provier_.icon(info));
		item->setData(0, Qt::UserRole, info.canonicalFilePath());

		addTopLevelItem(item);
	}
}

QString NotesBrowser::selectedFile() const
{
	if (selectedItems().count()==0) return "";

	return selectedItems()[0]->data(0, Qt::UserRole).toString();
}

void NotesBrowser::setSelectedFile(QString filename)
{
	QFileInfo info(filename);

	QList<QTreeWidgetItem *> matches = findItems(info.fileName().replace(".md", ""), Qt::MatchExactly | Qt::MatchRecursive);
	foreach(QTreeWidgetItem* item, matches)
	{
		if (item->data(0, Qt::UserRole).toString()==info.canonicalFilePath())
		{
			setCurrentItem(item);
		}
	}
}

void NotesBrowser::onItemSelected()
{
	if (selectedItems().count()==0) return;

	emit fileSelected(selectedFile());
}

void NotesBrowser::onContextMenu(QPoint p)
{
	QTreeWidgetItem* item = itemAt(p);

	QString path;
	if (item==nullptr)
	{
		path = Settings::string("data_folder");
	}
	else
	{
		path = itemAt(p)->data(0, Qt::UserRole).toString();
	}

	//set up menu
	QMenu menu;
	QFileInfo info(path);
	if (info.isDir())
	{
		menu.addAction(icon_provier_.icon(QFileIconProvider::File), "New file");
		menu.addAction(icon_provier_.icon(QFileIconProvider::Folder), "New folder");
	}
	else
	{
		menu.addAction(QIcon(":/Resources/remove.png"), "Delete file");
	}

	//execute menu
	QAction* action = menu.exec(mapToGlobal(p));
	if (action==nullptr) return;

	//perform action
	if(action->text()=="New file")
	{
		QString name = QInputDialog::getText(this, "New file", "File:");
		if (!checkValidFileName(name)) return;

		name = path + "/" + name;
		if (!name.endsWith(".md")) name += ".md";
		Helper::touchFile(name);

		updateView();
		setSelectedFile(name);
		emit fileSelected(selectedFile());
	}
	else if(action->text()=="New folder")
	{
		QString name = QInputDialog::getText(this, "New folder", "Folder:");
		if (!checkValidFileName(name)) return;

		QDir(path).mkdir(name);
		updateView();
	}
	else if(action->text()=="Delete file")
	{
		QFile::remove(item->data(0, Qt::UserRole).toString());
		updateView();
	}
}

void NotesBrowser::addChildren(QTreeWidgetItem* parent_item, QString dir, const QHash<QString, GitStatus>& status)
{
	QFileInfoList infos = QDir(dir).entryInfoList();

	foreach(const QFileInfo& info, infos)
	{
		QString name = info.fileName();
		if (name=="." || name==".." || name=="images" || name=="attachments") continue;
		if (info.isFile() && !name.endsWith(".md")) continue;

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, name.replace(".md", ""));
		QString canonical_name = info.canonicalFilePath();
		item->setData(0, Qt::UserRole, canonical_name);
		QIcon icon = info.isDir() ? icon_provier_.icon(QFileIconProvider::Folder) : QIcon(":/Resources/icon.png");
		if (info.isFile() && status.contains(canonical_name))
		{
			GitStatus git_enum = status[canonical_name];
			if (git_enum==GitStatus::ADDED)
			{
				item->setToolTip(0, "Git status: added");
				icon = QIcon(":/Resources/icon_blue.png");
			}
			else if (git_enum==GitStatus::MODIFIED)
			{
				item->setToolTip(0, "Git status: modified");
				icon = QIcon(":/Resources/icon_blue.png");
			}
			else if (git_enum==GitStatus::NOT_VERSIONED)
			{
				item->setToolTip(0, "Git status: modified");
				icon = QIcon(":/Resources/icon_grey.png");
			}
		}
		item->setIcon(0, icon);

		if(info.isDir())
		{
			addChildren(item ,info.filePath(), status);
			item->setFlags(Qt::ItemIsEnabled); // remove Qt::ItemIsSelectable
		}

		if (parent_item==nullptr)
		{
			addTopLevelItem(item);
		}
		else
		{
			parent_item->addChild(item);
		}
	}
}

bool NotesBrowser::checkValidFileName(QString name)
{
	bool is_invalid = name.contains('*') || name.contains('\\') || name.contains('/')
				   || name.contains(':') || name.contains('"') || name.contains('?')
				   || name.contains('<') || name.contains('>') || name.contains('|');

	if (is_invalid)
	{
		QMessageBox::warning(this, "Invalid name", "Name '" + name + "' is no valid file/folder name!");
	}

	return !is_invalid;
}
