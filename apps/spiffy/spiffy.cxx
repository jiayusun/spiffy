#include <QApplication>
#include <QFile>
#include <iostream>
#include <QMainWindow>
#include "milxQtImage.h"
#include <vtkRenderWindowInteractor.h>
#include "vtkImageViewer3.h"
#include "milxFile.h"
#include "milxImage.h"
#include "milxGlobal.h"
#include <sstream>
#include "ui_spiffy.h"
template<class TImage>
vtkSmartPointer<vtkImageData> openImage(std::string fileName, itk::SmartPointer<TImage> &imageData)
{
	
	///Open image

	if (!milx::File::OpenImage<TImage>(fileName, imageData))
	{
		milx::PrintError("Could not open file.");

	}
	vtkSmartPointer<vtkImageData> imageVTK = milx::Image<TImage>::ConvertITKImageToVTKImage(imageData);
	vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkSmartPointer<vtkImageData> imageVTKReoriented = milx::Image<TImage>::ApplyOrientationToVTKImage(imageVTK, imageData, matrix, true);
	milx::PrintDebug("Open file successfully.");
	return imageVTKReoriented;
}

int main(int argc, char *argv[])
{

	QApplication app(argc, argv);
	QFile qss(":/resources/style.qss");
	qss.open(QFile::ReadOnly);
	qApp->setStyleSheet(qss.readAll());
	qss.close();
	QMainWindow *mainWindow = new QMainWindow;
	if (argc < 2)
	{
		cerr << "View an image with correct orientation." << endl;
		cerr << "Usage: " << argv[0] << " <image>" << endl;
		exit(EXIT_FAILURE);
	}
	std::string fileName = argv[1];

	//create user iterface


	//open image
	typedef itk::Image<float, 3> VisualizingImageType;
	VisualizingImageType::Pointer imageType = VisualizingImageType::New();
	vtkSmartPointer<vtkImageData> imageVTK = openImage<VisualizingImageType>(fileName, imageType);

	//load image 
	milxQtImage *image = new milxQtImage(mainWindow);
	image->setData(imageVTK,0);
	image->generate(0);
	mainWindow->setWindowTitle("SPIFFY");
	mainWindow->show();
	milx::PrintDebug("show show.");
	return app.exec();
}

