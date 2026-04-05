#include "Application.h"
#include "mainwindow.h"
#include <QStyleFactory>

int main(int argc, char *argv[])
{
	Application a(argc, argv);

    MainWindow w;
	w.setStyle(QStyleFactory::create("windowsvista"));
	w.showMaximized();
	return a.exec();
}
