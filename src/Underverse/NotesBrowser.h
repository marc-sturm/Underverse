#ifndef NOTESBROWSER_H
#define NOTESBROWSER_H

#include "Git.h"
#include <QWidget>
#include <QFileIconProvider>
#include "ui_notesbrowser.h"

class NotesBrowser
	: public QWidget
{
	Q_OBJECT

public:
	NotesBrowser(QWidget* parent = 0);
	void setBaseDirectory(QString dir);

	QString selectedFile() const;
	void setSelectedFile(QString filename);

public slots:
	//Sets search terms (space-separated) and updates the view accordingly. If terms string is empty, search mode is left.
	void setSearchTerms(QStringList terms);

signals:
	//Signal emitted when search terms change
	void searchTermsChanged(QStringList terms);
	//Signal emitted when the file selection changes
	void fileSelected(QString filename);

protected slots:
	void onItemSelected();
	void onContextMenu(QPoint p);
	void clearSearch();
	void searchChanged();
protected:
	void updateView();
	void performSearch();
	void addChildren(QTreeWidgetItem* item, QString dir, const QHash<QString, GitStatus>& status);
	bool checkValidFileName(QString name);

private:
	Ui::NotesBrowser ui_;
	QFileIconProvider icon_provier_;
	QString base_dir_;
	QStringList search_terms_;
};

#endif // NOTESBROWSER_H