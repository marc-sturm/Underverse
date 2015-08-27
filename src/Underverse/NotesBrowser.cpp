#include "NotesBrowser.h"
#include <QFileSystemModel>
#include <QDebug>

NotesBrowser::NotesBrowser(QWidget* parent)
	: QTreeWidget(parent)
{
	setColumnCount(1);
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelected()));
}

void NotesBrowser::setBaseDirectory(QString dir)
{
	base_dir_ = dir;

	clear();
	addChildren(nullptr, base_dir_);
	expandAll();
}

void NotesBrowser::setSelectedFile(QString filename)
{
	QFileInfo info(filename);
	qDebug() << info.fileName().replace(".md", "");
	QList<QTreeWidgetItem *> matches = findItems(info.fileName().replace(".md", ""), Qt::MatchExactly | Qt::MatchRecursive);
	qDebug() << matches;
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

	emit fileSelected(selectedItems()[0]->data(0, Qt::UserRole).toString());
}

void NotesBrowser::addChildren(QTreeWidgetItem* parent_item, QString dir)
{
	QFileInfoList infos = QDir(dir).entryInfoList();

	foreach(QFileInfo info, infos)
	{
		QString name = info.fileName();
		if (name=="." || name==".." || name=="images" || name=="attachments") continue;
		if (info.isFile() && !name.endsWith(".md")) continue;

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, name.replace(".md", ""));
		item->setIcon(0, icon_provier_.icon(info));
		item->setData(0, Qt::UserRole, info.canonicalFilePath());

		if(info.isDir())
		{
			addChildren(item ,info.filePath());
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
