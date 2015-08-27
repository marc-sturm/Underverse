#ifndef NOTESBROWSER_H
#define NOTESBROWSER_H

#include <QTreeWidget>
#include <QFileSystemModel>
#include <QFileIconProvider>

class NotesBrowser
	: public QTreeWidget
{
	Q_OBJECT

public:
	NotesBrowser(QWidget* parent = 0);
	void setBaseDirectory(QString dir);
	void setSelectedFile(QString filename);

signals:
	void fileSelected(QString filename);

protected slots:
	void onItemSelected();

protected:
	void addChildren(QTreeWidgetItem* item, QString dir);

	QFileIconProvider icon_provier_;
	QString base_dir_;
};

#endif // NOTESBROWSER_H
