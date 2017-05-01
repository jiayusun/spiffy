#include <QApplication>
#include <QMainWindow>
#include "milxQtWindow.h"
#include <sstream>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QMainWindow mainWindow;
	milxQtWindow *image = new milxQtWindow(&mainWindow);
	mainWindow.setWindowTitle("milxQtWindowTest");
	mainWindow.setCentralWidget(image);
	mainWindow.show();
	return app.exec();
}


