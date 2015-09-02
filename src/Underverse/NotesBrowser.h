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

	QString selectedFile() const;
	void setSelectedFile(QString filename);

public slots:
	//Sets search terms (space-separated) and updates the view accordingly. If terms string is empty, search mode is left.
	void setSearchTerms(QString terms);

signals:
	void fileSelected(QString filename);

protected slots:
	void onItemSelected();
	void onContextMenu(QPoint p);

protected:
	void updateView();
	void performSearch();
	void addChildren(QTreeWidgetItem* item, QString dir);
	bool checkValidFileName(QString name);

	QFileIconProvider icon_provier_;
	QString base_dir_;
	QStringList search_terms_;
};

#endif // NOTESBROWSER_H
