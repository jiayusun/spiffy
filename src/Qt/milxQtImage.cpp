/*=========================================================================
  The Software is copyright (c) Commonwealth Scientific and Industrial Research Organisation (CSIRO)
  ABN 41 687 119 230.
  All rights reserved.

  Licensed under the CSIRO BSD 3-Clause License
  You may not use this file except in compliance with the License.
  You may obtain a copy of the License in the file LICENSE.md or at

  https://stash.csiro.au/projects/SMILI/repos/smili/browse/license.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
=========================================================================*/
//ITK
//#include <itkImageToHistogramFilter.h>
//VTK Libraries
#include "milxQtImage.h"
#include "milxFile.h"
#include "milxImage.h"
#include "milxQtAboutForm.h"
#include "milxGlobal.h"
#include "milxQtAliases.h"
#include "milxColourMap.h"
#include "ui_spiffy.h"
#include <sstream>
#include <QDockWidget>
#include <QApplication>
#include <QInputDialog>
#include <QFile>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplashScreen>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageMapToColors.h>
#include "vtkImageViewer3.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkImageBlend.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageAccumulate.h>
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"

///ITK Imaging
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkPNGImageIOFactory.h>
#include <itkJPEGImageIOFactory.h>
#include <itkBMPImageIOFactory.h>
#include <itkNrrdImageIOFactory.h>
#include <itkRawImageIO.h>
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"
#include <itkObjectFactoryBase.h>

//milxQtImage
milxQtImage::milxQtImage(QWidget *theParent, bool contextSystem) : milxQtRenderWindow(theParent, contextSystem)
{
	usingVTKImage = false;
	eightbit = false;
	rgb = false;
	vectorised = false;
	viewerSetup = false;
	track = false;
	volume = false;
	flipped = true;
	actualNumberOfDimensions = 3;

	QMainWindow *mainWindow = qobject_cast<QMainWindow *>(theParent);
	mainWindow->installEventFilter(this);
	mainWindow->setAcceptDrops(true);
	ui.setupUi(mainWindow);
	vtkRenderWindow* renWin = vtkRenderWindow::New();
	ui.qvtkWidget->SetRenderWindow(renWin);
	renWin->Delete();
	minValue = 0;
	maxValue = 0;
	for (int j = 0; j < 3; j++)
	{
		viewer[j] = vtkSmartPointer<vtkImageViewer3>::New();
		imageData[j] = vtkSmartPointer<vtkImageData>::New();
	}
	openSupport = extensionsOpen.c_str();
	milx::PrintDebug("constructor");
//	ui.pushButton_10->setChecked(false);
	QPoint pos = mainWindow->mapToGlobal(QPoint(0, 0));
	mainWindow->move(pos.x() + 300, pos.y() + 30);
//	mainWindow->resize(611, 654);
	mainWindow->setFixedSize(611, 611);
	mainWindow->setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);//remove maximize button and resize handles from window?
	orientationGroup = new QActionGroup(ui.toolBar);
	orientationGroup->addAction(ui.actionAxial);
	orientationGroup->addAction(ui.actionCoronal);
	orientationGroup->addAction(ui.actionSagittal);

	///Setup Console
	console = new milxQtConsole;
	actionConsole = console->dockWidget()->toggleViewAction();
	actionConsole->setIcon(QIcon(":/resources/toolbar/console.png"));
	ui.toolBar->addAction(actionConsole);
	ui.menuHelp->addAction(actionConsole);
	dockActions.append(actionConsole);
	mainWindow->addDockWidget(console->dockDefaultArea(), console->dockWidget());
	console->dockWidget()->hide();
	setConsole(console);
	ui.toolBar->addSeparator();
	radio1 = new QRadioButton;
	radio2 = new QRadioButton;
	radio3 = new QRadioButton;
	radio1->setText("Image");
	radio2->setText("Label Image");
	radio3->setText("Blend Image");
	ui.toolBar->addWidget(radio1);
	ui.toolBar->addSeparator();
	ui.toolBar->addWidget(radio2);
	ui.toolBar->addSeparator();
	ui.toolBar->addWidget(radio3);
	radioButtonGroup = new QButtonGroup(ui.toolBar);
	radioButtonGroup->addButton(radio1, 0);
	radioButtonGroup->addButton(radio2, 1);
	radioButtonGroup->addButton(radio3, 2);
	radio1->setChecked(true);
	radio2->setDisabled(true);
	radio3->setDisabled(true);
	ui.toolBar->setAllowedAreas(Qt::NoToolBarArea);
	ui.toolBar->setOrientation(Qt::Vertical);
	ui.toolBar->move(mainWindow->pos().x() - 127, mainWindow->pos().y()-3);
	ui.toolBar->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
	ui.toolBar->setFixedHeight(654);
	ui.toolBar->adjustSize();
	ui.toolBar->show();
	actionToolbar = ui.toolBar->toggleViewAction();
	ui.statusbar->setFixedHeight(48);
	expand = new QPushButton();
	expand->setFixedWidth(40);
	expand->setFixedHeight(40);
	expand->setIcon(QIcon(":/resources/toolbar/expand.png"));
	ui.statusbar->addWidget(expand);
	readSettings(mainWindow);
	if (toolbarStatus == 0)
	{
		ui.toolBar->hide();
	}
	cor = new QLabel("");
	ui.statusbar->addPermanentWidget(cor);
	createConnections();
	QSignalMapper* mapper = new QSignalMapper;
	mapper->setMapping(ui.actionExit, mainWindow);
	QObject::connect(ui.actionExit, SIGNAL(triggered()), mapper, SLOT(map()));
	QObject::connect(mapper, SIGNAL(mapped(QWidget*)), this, SLOT(close(QWidget*)));

}

bool milxQtImage::eventFilter(QObject* obj, QEvent* ev){
	QMainWindow* widget = qobject_cast<QMainWindow*>(obj);
	if (ev->type() == QEvent::DragEnter){
		//you can compare widget against the stack of widgets you have
		QDragEnterEvent *k = (QDragEnterEvent *)ev;
		printInfo("de");
		//if you want to stop widget from receiving the event you can return true
		dragEnterEvent(k);
		return true;
	}
	if (ev->type() == QEvent::Drop){
		//you can compare widget against the stack of widgets you have
		QDropEvent *kd = (QDropEvent *)ev;
		printInfo("drop");
		//if you want to stop widget from receiving the event you can return true
		dropEvent(kd);
		return true;
	}
	return false;
}

milxQtImage::~milxQtImage()
{
    ///Smart pointers handle deletion
}

void milxQtImage::setData(vtkSmartPointer<vtkImageData> newImg,int i)
{
	setConsole(console);
	imageData[i]->DeepCopy(newImg);
	usingVTKImage = true;
	eightbit = false;
	if ((newImg->GetNumberOfScalarComponents() == 4 || newImg->GetNumberOfScalarComponents() == 3) && newImg->GetScalarType() == VTK_UNSIGNED_CHAR)
		rgb = true;
	else
		rgb = false;
	loaded = true;
	vectorised = false;
	flipped = false;
	///Setup Connections

	//~ histogram(256, 0, 255, false);
}

void milxQtImage::generate(int i, const bool quietly)
{
	setConsole(console);
	currentViewer = i;
	emit working(-1);
	updateData(true);
	imageData[i]->GetExtent(bounds);
	if (bounds[5] > 1)
		volume = true;
	if (volume == true)
		printInfo("volume" + QString::number(bounds[5]));
	// Visualize
	viewer[i]->SetInputData(imageData[i]);
	//imageViewer->GetRenderWindow()->SetSize(500, 500);
	//
	linkProgressEventOf(viewer[i]);
	milxQtRenderWindow::SetRenderer(viewer[i]->GetRenderer());
	ui.qvtkWidget->SetRenderWindow(viewer[i]->GetRenderWindow());
	viewer[i]->SetupInteractor(ui.qvtkWidget->GetRenderWindow()->GetInteractor());
	SetupWidgets(viewer[i]->GetRenderWindow()->GetInteractor());
	if (volume)
		viewer[i]->SetSlice(bounds[5] / 2); //show middle of volume
	
	viewer[i]->GetInteractorStyle()->InvokeEvent(vtkCommand::ResetWindowLevelEvent);
	viewer[i]->GetRenderer()->ResetCamera();
	viewer[i]->UpdateCursor();
	viewer[i]->Render();
	
	printInfo("Number of Image Components: " + QString::number(imageData[i]->GetNumberOfScalarComponents()));
	if (imageData[i]->GetNumberOfScalarComponents() > 2)
	{
		GetWindowLevel(i)->SetLookupTable(NULL);
		GetWindowLevel(i)->SetWindow(255);
		GetWindowLevel(i)->SetLevel(127.5);
	}
	lookupTable[i] = NULL;

	if (index == 0)
	{
		ui.actionAxial->setChecked(true);
		viewToXYPlane();
	}

	else if (index == 1)
	{
		ui.actionCoronal->setChecked(true);
		viewToZXPlane();

	}
	else if (index == 2)
	{
		ui.actionSagittal->setChecked(true);
		viewToZYPlane();

	}

	emit milxQtRenderWindow::modified(GetImageActor());
	enableUpdates(ui.statusbar);

printDebug("Completed Generating Image");
}


void milxQtImage::viewToXYPlane()
{
	setConsole(console);
	if (ui.actionAxial->isChecked())
	{
		viewer[currentViewer]->SetSliceOrientationToXY();
		if (volume)
			viewer[currentViewer]->SetSlice(bounds[5] / 2); //show middle of volume
		index = 0;
		ui.actionAxial->setChecked(true);
		ui.actionCoronal->setChecked(false);
		ui.actionSagittal->setChecked(false);
	}
    
}



void milxQtImage::viewToZXPlane()
{
	setConsole(console);
	if (ui.actionCoronal->isChecked())
	{
		viewer[currentViewer]->SetSliceOrientationToXZ();
		if (volume)
			viewer[currentViewer]->SetSlice(bounds[5] / 2); //show middle of volume
		index = 1;
		ui.actionAxial->setChecked(false);
		ui.actionCoronal->setChecked(true);
		ui.actionSagittal->setChecked(false);
	}
}

void milxQtImage::viewToZYPlane()
{
	setConsole(console);
	if (ui.actionSagittal->isChecked())
	{
		viewer[currentViewer]->SetSliceOrientationToYZ();
		if (volume)
			viewer[currentViewer]->SetSlice(bounds[5] / 2); //show middle of volume
		index = 2;
		ui.actionSagittal->setChecked(true);
		ui.actionAxial->setChecked(false);
		ui.actionCoronal->setChecked(false);
	}
		    
}

void milxQtImage::updateWindowsWithCursors()
{
	setConsole(console);
	if (ui.actionCrosshair->isChecked())
	{
		viewer[currentViewer]->EnableCursor();
		ui.actionCrosshair->setChecked(true);
		printInfo("enable cursor");
	}
	else
	{
		viewer[currentViewer]->DisableCursor();
		ui.actionCrosshair->setChecked(false);
		printInfo("disable cursor");
	}
}



void milxQtImage::autoLevel(float percentile)
{
	setConsole(console);
	if (currentViewer == 2){
		printInfo("Can not do Auto-level display on blend image");
	}
	else{
		printInfo("Auto Updating Window Level");
		printDebug("Current Window:" + QString::number(GetIntensityWindow()));
		printDebug("Current Level:" + QString::number(GetIntensityLevel()));

		int bins = 256;
		float belowValue = -1000, aboveValue = 4000, lowerPercentile = 1.0 - percentile, upperPercentile = percentile;
		if (maxValue == minValue)
			histogram(bins, belowValue, aboveValue, false); //above and below unused here, uses image min/max automatically
		belowValue = minValue;
		aboveValue = maxValue;

		emit working(-1);

		emit done(-1);

		emit working(-1);
		//Compute the percentile contributions to trim levels for better contrast
		size_t k = 0, kMin = 0, kMax = 0, kMid = 0;
		double binSpacing = (aboveValue - belowValue) / bins, accummulator = 0.0, proportion = 0.0;
		//Upper limit of histgram
		k = 0;
		accummulator = 0.0;
		for (double j = belowValue; j < aboveValue; j += binSpacing, k++)
		{
			proportion = accummulator / hist->GetVoxelCount();
			if (proportion >= lowerPercentile)
				break;
			accummulator += hist->GetOutput()->GetPointData()->GetScalars()->GetTuple1(k); //freq k
		}
		kMin = k - 1;
		double lowLevel = minValue + (kMin)*binSpacing;
		//Lower limit of histgram
		k = bins - 1;
		accummulator = 0.0;
		for (double j = aboveValue; j > belowValue; j -= binSpacing, k--)
		{
			proportion = accummulator / hist->GetVoxelCount();
			if (proportion >= lowerPercentile)
				break;
			accummulator += hist->GetOutput()->GetPointData()->GetScalars()->GetTuple1(k); //freq k
		}
		kMax = k + 1;
		//    printDebug("k high:" + QString::number(kMax));
		//    printDebug("accummulator high:" + QString::number(accummulator));
		double maxLevel = minValue + (kMax)*binSpacing;
		double windowLevel = maxLevel - lowLevel;
		double level = lowLevel + windowLevel / 2; //center the new window from low limit
		emit done(-1);

		printDebug("Histogram Low Level:" + QString::number(lowLevel));
		printDebug("Histogram High Level:" + QString::number(maxLevel));
		printInfo("Window:" + QString::number(windowLevel));
		printInfo("Level:" + QString::number(level));

		viewer[currentViewer]->SetColorWindow(windowLevel);
		viewer[currentViewer]->SetColorLevel(level);
		viewer[currentViewer]->Render();
	}
}


void milxQtImage::open(int i)
{
	setConsole(console);
	QFileDialog *fileOpener = new QFileDialog(this);
	QSettings settings("Shekhar Chandra", "milxQt");

	//Add supported file entry
	QString exts = "Supported Files (" + openSupport + ");;";
	QString path = settings.value("recentPath").toString();

	exts += openExts.c_str();


	QString filenames = fileOpener->getOpenFileName(this,
		tr("Select File to Open"),
		path,
		tr(exts.toStdString().c_str())); //!< \todo Check and validate extensions support at Open in Main class
	if (filenames.length() == 0) {
		QMessageBox::information(NULL, tr("SPIFFY"), tr("You didn't select any files."));
	}
	else {
		typedef itk::Image<float, 3> VisualizingImageType;
		VisualizingImageType::Pointer imageType = VisualizingImageType::New();
		;
		if (!milx::File::OpenImage<VisualizingImageType>(filenames.toStdString(), imageType))
		{
			milx::PrintError("Could not open file.");

		}
		vtkSmartPointer<vtkImageData> imageVTK = milx::Image<VisualizingImageType>::ConvertITKImageToVTKImage(imageType);
		vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
		vtkSmartPointer<vtkImageData> imageVTKOrientation = milx::Image<VisualizingImageType>::ApplyOrientationToVTKImage(imageVTK, imageType, matrix, true);
		setData(imageVTKOrientation,i);
		generate(i);
		if (i = 0)
		{
			radio1->setChecked(true);

		}
		if (i = 1)
		{
			radio2->setDisabled(false);
			radio2->setChecked(true);

		}
	}
}

void milxQtImage::switchViewer()
{
	setConsole(console);
	generate(radioButtonGroup->checkedId());
}

void milxQtImage::updateData(const bool orient)
{
	setConsole(console);
	if (!usingVTKImage)
	{
		printDebug("Updating Data");

		floatImageType::DirectionType direction;
		floatImageType::PointType origin;
		floatImageType::SpacingType spacing;
		vtkSmartPointer<vtkImageData> newImageData = vtkSmartPointer<vtkImageData>::New();
		if (eightbit)
		{
			/// ITK to VTK image (unsigned char)
			newImageData->DeepCopy(milx::Image<charImageType>::ConvertITKImageToVTKImage(imageChar));
			direction = imageChar->GetDirection();
			origin = imageChar->GetOrigin();
			spacing = imageChar->GetSpacing();
			if (orient)
				imageData[currentViewer] = milx::Image<charImageType>::ApplyOrientationToVTKImage(newImageData, imageChar, transformMatrix, true, flipped);
			else
				imageData[currentViewer] = newImageData;
			//Labelled image flag is set as true to avoid artefacts in resampling within the ApplyOrientationToVTKImage member
			printDebug("Updated Internal Char Image Data");
		}
		else if (rgb)
		{
			/// ITK to VTK image (RGB)
			newImageData->DeepCopy(milx::Image<rgbImageType>::ConvertITKImageToVTKImage(imageRGB));
			direction = imageRGB->GetDirection();
			origin = imageRGB->GetOrigin();
			spacing = imageRGB->GetSpacing();
			if (orient)
				imageData[currentViewer] = milx::Image<rgbImageType>::ApplyOrientationToVTKImage(newImageData, imageRGB, transformMatrix, false, flipped);
			else
				imageData[currentViewer] = newImageData;
			//Labelled image flag is set as true to avoid artefacts in resampling within the ApplyOrientationToVTKImage member
			printDebug("Updated Internal RGB Image Data");
		}
		else //if float and/or vector (which also generates float magnitude image)
		{
			/// ITK to VTK image (Float)
			newImageData->DeepCopy(milx::Image<floatImageType>::ConvertITKImageToVTKImage(imageFloat));
			direction = imageFloat->GetDirection();
			origin = imageFloat->GetOrigin();
			spacing = imageFloat->GetSpacing();
			if (orient)
				imageData[currentViewer] = milx::Image<floatImageType>::ApplyOrientationToVTKImage(newImageData, imageFloat, transformMatrix, false, flipped);
			else
				imageData[currentViewer] = newImageData;
			//Labelled image flag is set as true to avoid artefacts in resampling within the ApplyOrientationToVTKImage member
			printDebug("Updated Internal Float Image Data");
		}
	}

	imageData[currentViewer]->Modified();
#if VTK_MAJOR_VERSION <= 5
	imageData->Update();
#endif

	printDebug("Updated Image Data as " + QString(imageData[currentViewer]->GetScalarTypeAsString()));
}

void milxQtImage::histogram(int bins, float belowValue, float aboveValue, bool plotHistogram)
{
	setConsole(console);
	printInfo("Computing Histogram of Image");
	updateData(true);
	double range[2];
	imageData[currentViewer]->GetScalarRange(range);
	int ret = QMessageBox::Yes;
	if (plotHistogram)
	{
		///ask user number of bins
		bool ok1 = false, ok2 = false, ok3 = false;
		bins = QInputDialog::getInt(this, tr("Please Provide the number of bins"),
			tr("Number of Bins:"), 256, 2, 16384, 1, &ok1);
		belowValue = QInputDialog::getDouble(this, tr("Please Provide the lower level"),
			tr("Lower Level:"), range[0], -2147483647, 2147483647, 1, &ok2);
		aboveValue = QInputDialog::getDouble(this, tr("Please Provide the upper level"),
			tr("Upper Level:"), range[1], belowValue, 2147483647, 1, &ok3);

		if (!ok1 || !ok2 || !ok3)
			return;

		QMessageBox msgBox;
		msgBox.setText("Ignore Zero Values in Statistics?");
		msgBox.setInformativeText("Do you wish to ignore zero values in Max/Min/Sum/Mean statistics?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		ret = msgBox.exec();
	}
	else
	{
		belowValue = range[0];
		aboveValue = range[1];
	}

	double binSpacing = (aboveValue - belowValue) / bins;
	printInfo("Bin Spacing: " + QString::number(binSpacing));
	hist = vtkSmartPointer<vtkImageAccumulate>::New(); //!< Histogram of the image
#if VTK_MAJOR_VERSION <= 5
	hist->SetInput(imageData[currentViewer]);
#else
	hist->SetInputData(imageData[currentViewer]);
#endif
	//Only set first values as single component image assumed
	hist->SetComponentExtent(0, bins - 1, 0, 0, 0, 0); //bins
	hist->SetComponentOrigin(belowValue, 0, 0); //offset
	hist->SetComponentSpacing(binSpacing, 0, 0); //spacing
	if (ret == QMessageBox::Yes)
	{
		printWarning("Ignore zero values in summary statistics");
		hist->IgnoreZeroOn();
	}
	else
		hist->IgnoreZeroOff();
	linkProgressEventOf(hist);
	hist->Update();
	hist->Print(cout);

	const coordinate meanTuple(hist->GetMean());
	const coordinate stddevTuple(hist->GetStandardDeviation());
	const coordinate minTuple(hist->GetMin());
	const coordinate maxTuple(hist->GetMax());
	if (imageData[currentViewer]->GetNumberOfScalarComponents() == 1)
	{
		meanValue = meanTuple[0];
		stddevValue = stddevTuple[0];
		minValue = minTuple[0];
		maxValue = maxTuple[0];
	}
	else
	{
		meanValue = meanTuple.mean();
		stddevValue = stddevTuple.mean();
		minValue = minTuple.mean();
		maxValue = maxTuple.mean();
	}
	printInfo("Mean of the data: " + QString::number(meanValue));
	printInfo("Std Deviation of the data: " + QString::number(stddevValue));
	printInfo("Min/Max of the data: " + QString::number(minValue) + "/" + QString::number(maxValue));

	if (plotHistogram)
	{
#if VTK_MAJOR_VERSION <= 5
		hist->GetOutput()->Update();
#endif

		vtkSmartPointer<vtkFloatArray> binsArray = vtkSmartPointer<vtkFloatArray>::New();
		binsArray->SetNumberOfComponents(1);
		binsArray->SetNumberOfTuples(bins);
		binsArray->SetName("Greyscale Bins");
		vtkSmartPointer<vtkIntArray> freqArray = vtkSmartPointer<vtkIntArray>::New();
		freqArray->SetNumberOfComponents(1);
		freqArray->SetNumberOfTuples(bins);
		freqArray->SetName("Frequency");
		freqArray->FillComponent(0, 0);
		int k = 0;
		for (float j = belowValue; j < aboveValue; j += binSpacing, k++)
		{
			binsArray->SetTuple1(k, j);
			freqArray->SetTuple1(k, hist->GetOutput()->GetPointData()->GetScalars()->GetTuple1(k));
		}

		vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
		table->AddColumn(binsArray);
		table->AddColumn(freqArray);

		emit tableToPlot(table, "Histogram");
	}
}

double milxQtImage::GetIntensityWindow()
{
	return viewer[currentViewer]->GetWindowLevel()->GetWindow();
}

double milxQtImage::GetIntensityLevel()
{
	return viewer[currentViewer]->GetWindowLevel()->GetLevel();
}

void milxQtImage::colourMapToHSV(int i, double minRange, double maxRange)
{
	setConsole(console);
	double range[2];
	if (minRange == 0.0 && maxRange == 0.0)
		GetDataSet(i)->GetScalarRange(range); //This will propagate upwards to get the range for images or meshes
	else
	{
		range[0] = minRange;
		range[1] = maxRange;
	}

	milx::ColourMap *colours = new milx::ColourMap;
	colours->toHSV();
	colours->SetRange(range);

	lookupTable[i] = colours->GetOutput();

	logScale = false;
	updateLookupTable(i);
}

void milxQtImage::colourMapToGray(int i,double minRange, double maxRange)
{
	setConsole(console);
	double range[2];
	if (minRange == 0.0 && maxRange == 0.0)
		GetDataSet(i)->GetScalarRange(range); //This will propagate upwards to get the range for images or meshes
	else
	{
		range[0] = minRange;
		range[1] = maxRange;
	}

	milx::ColourMap *colours = new milx::ColourMap;
	colours->toGray();
	colours->SetRange(range);

	lookupTable[i] = colours->GetOutput();

	logScale = false;
	updateLookupTable(i);
}

void milxQtImage::updateLookupTable(int i)
{
	setConsole(console);
	double range[2];
	imageData[i]->GetScalarRange(range);

	lookupTable[i]->SetRange(range[0], range[1]);

	vtkImageMapToWindowLevelColors *filterColorsOverlay = viewer[i]->GetWindowLevel();
	filterColorsOverlay->SetLookupTable(lookupTable[i]);
	filterColorsOverlay->PassAlphaToOutputOn();
	filterColorsOverlay->Update();

	scaleDisplay();
}


void milxQtImage::blend()
{
	setConsole(console);
	if (viewer[1]->GetInput() == NULL)
	{
		printInfo("You need set label Image");
		open(1);
	}
	if (viewer[1]->GetInput() != NULL)
	{

		bool ok1 = true;
		opacity = QInputDialog::getDouble(this, tr("Please Provide the opacity of the blending"),
			tr("Opacity:"), 0.5, 0, 2147483647, 2, &ok1);

		if (!ok1)
			return;
		//	initialiseWindowTraversal();
		if (is8BitImage()) //follow up images expected to be a label
		{
			colourMapToHSV(0);
			colourMapToHSV(1);
		}
		else
		{
			colourMapToGray(0);
			colourMapToGray(1);
		}
		vtkSmartPointer<vtkImageMapToColors> filterColorsImage = vtkSmartPointer<vtkImageMapToColors>::New();
		filterColorsImage->SetLookupTable(GetLookupTable(0));
#if VTK_MAJOR_VERSION <= 5
		filterColorsImage->SetInput(imageData[0]);
#else
		filterColorsImage->SetInputData(imageData[0]);
#endif
		filterColorsImage->PassAlphaToOutputOn();
		filterColorsImage->Update();
		vtkSmartPointer<vtkImageData> ucharData1 = filterColorsImage->GetOutput();

		int initialExtent[6];
		imageData[0]->GetExtent(initialExtent);

		// Combine the images (blend takes multiple connections on the 0th input port)
		emit working(-1);
		vtkSmartPointer<vtkImageBlend> blend = vtkSmartPointer<vtkImageBlend>::New();
		linkProgressEventOf(blend);
		blend->SetOpacity(0, opacity);
#if VTK_MAJOR_VERSION <= 5
		blend->AddInput(ucharData1);
#else
		blend->AddInputData(ucharData1);
#endif
		//        blend->SetBlendModeToCompound();
		blend->SetBlendModeToNormal();
		vtkSmartPointer<vtkImageMapToColors> filterColorsOverlay = vtkSmartPointer<vtkImageMapToColors>::New();
		filterColorsOverlay->SetLookupTable(GetLookupTable(1));
#if VTK_MAJOR_VERSION <= 5
		filterColorsOverlay->SetInput(imageData[1]);
#else
		filterColorsOverlay->SetInputData(imageData[1]);
#endif
		filterColorsOverlay->PassAlphaToOutputOn();
		filterColorsOverlay->Update();
		vtkSmartPointer<vtkImageData> ucharData2 = filterColorsOverlay->GetOutput();

		if (GetLookupTable(1))
			printWarning("Colourmap is not set. Please set a colour map to ensure proper blending.");

		int actualExtent[6];
		imageData[1]->GetExtent(actualExtent);

		if (initialExtent[1] == actualExtent[1] && initialExtent[3] == actualExtent[3] && initialExtent[5] == actualExtent[5])
		{
#if VTK_MAJOR_VERSION <= 5
			blend->AddInput(ucharData2);
#else
			blend->AddInputData(ucharData2);
#endif
			blend->SetOpacity(1, opacity);
	}
		else
			printError("Images are not the same size. Skipping.");


		printInfo("Blending");
		blend->Update();
		printDebug("Number of components: " + QString::number(blend->GetOutput()->GetNumberOfScalarComponents()));
		emit done(-1);
		setData(blend->GetOutput(), 2);
		generate(2);
		radio3->setDisabled(false);
		radio3->setChecked(true);
}
}

void milxQtImage::refresh()
{
	viewer[currentViewer]->UpdateCursor();
	viewer[currentViewer]->GetInteractorStyle()->InvokeEvent(vtkCommand::ResetWindowLevelEvent); //Reset window level as if pressing 'r'
	viewer[currentViewer]->Render();
	milxQtRenderWindow::refresh();
}

void milxQtImage::saveScreen(QString filename)
{
	setConsole(console);
	QFileDialog *fileSaver = new QFileDialog(this);

	QSettings settings("Shekhar Chandra", "milxQt");
	QString path = settings.value("recentPath").toString();
	if (filename.isEmpty())
	{
		filename = fileSaver->getSaveFileName(this,
			tr("Select File Name to Save"),
			path,
			tr(saveExtsForScreens.c_str()));
	}

	vtkSmartPointer<vtkWindowToImageFilter> windowToImage = vtkSmartPointer<vtkWindowToImageFilter>::New();
	viewer[currentViewer]->GetRenderWindow()->Render();
	windowToImage->SetInput(viewer[currentViewer]->GetRenderWindow());
	windowToImage->SetMagnification(magnifyFactor);
	//        windowToImage->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
	windowToImage->ReadFrontBufferOff();
	windowToImage->Update();
	//Save screenshot
	int extent[6];
	windowToImage->GetOutput()->GetExtent(extent);
	printDebug("Screenshot Size: " + QString::number(extent[1] - extent[0]) + ", " + QString::number(extent[3] - extent[2]) + ", " + QString::number(extent[5] - extent[4]));
	bool success = saveImage(filename, windowToImage->GetOutput());

	//        windowVTK->GetRenderWindow()->OffScreenRenderingOff();
	viewer[currentViewer]->GetRenderWindow()->Render(); //Restore rendering

	if (!success)
	{
		printError("Unable to save screenshot. Ignoring.");
		return;
	}

	printInfo("Write Complete.");
}

bool milxQtImage::saveImage(const QString filename, vtkImageData* data)
{
	setConsole(console);
	QFileInfo fileInfo(filename);
	QString extension = fileInfo.suffix().toLower();
	bool charFormat = false, integerFormat = false, medical = true, vtkFormat = false, success = false;
	int bounds[6];

	const QString charStr = "unsigned char";
	const QString typeStr = data->GetScalarTypeAsString();

	if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "bmp")
	{
		charFormat = true;
		medical = false;
	}
	else if (typeStr == charStr || typeStr == "unsigned_char")
	{
		charFormat = true;
		itk::ObjectFactoryBase::RegisterFactory(itk::RawImageIOFactory<unsigned char, 3>::New());
	}
	else if (typeStr == "unsigned" || typeStr == "unsigned_short" || typeStr == "short" || typeStr == "unsigned short" || typeStr == "unsigned_int" || typeStr == "unsigned int" || typeStr == "int")
	{
		integerFormat = true;
	}
	else
		itk::ObjectFactoryBase::RegisterFactory(itk::RawImageIOFactory<float, 3>::New());

	data->GetExtent(bounds);

	if (extension == "vti")
		vtkFormat = true;

	///Flip - ITK and VTK do not have the same orientation
	vtkSmartPointer<vtkErrorWarning> errorObserver = vtkSmartPointer<vtkErrorWarning>::New();
	vtkSmartPointer<vtkImageFlip> imageReorient = vtkSmartPointer<vtkImageFlip>::New();
#if VTK_MAJOR_VERSION <=5
	imageReorient->SetInputConnection(data->GetProducerPort());
#else
	imageReorient->SetInputData(data);
#endif
	imageReorient->SetFilteredAxis(1);
	imageReorient->FlipAboutOriginOn();
	linkProgressEventOf(imageReorient);
	imageReorient->Update();

	if (charFormat)
	{
		if (!medical)
		{
			//Add some default image types
			itk::ObjectFactoryBase::RegisterFactory(itk::RawImageIOFactory<unsigned char, 2>::New());
			itk::ObjectFactoryBase::RegisterFactory(itk::PNGImageIOFactory::New());
			itk::ObjectFactoryBase::RegisterFactory(itk::JPEGImageIOFactory::New());
			itk::ObjectFactoryBase::RegisterFactory(itk::BMPImageIOFactory::New());
			//
			///Export to ITK
			typedef itk::Image<rgbPixelType, 2> rgbImageSliceType;
			rgbImageSliceType::Pointer charImg = milx::Image<rgbImageSliceType>::ConvertVTKImageToITKImage(imageReorient->GetOutput());
			success = milx::File::WriteImageUsingITK<rgbImageSliceType>(filename.toStdString(), charImg);
		}
		else
		{
			///Export to ITK
			rgbImageType::Pointer charImg = milx::Image<rgbImageType>::ConvertVTKImageToITKImage(imageReorient->GetOutput());
			success = milx::File::WriteImageUsingITK<rgbImageType>(filename.toStdString(), charImg);
		}
	}
	else if (vtkFormat)
	{
		vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
#if VTK_MAJOR_VERSION <=5
		writer->SetInput(data);
#else
		writer->SetInputData(data);
#endif
		writer->SetFileName(filename.toStdString().c_str());
		writer->SetDataModeToBinary();
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		linkProgressEventOf(writer);
		writer->Write();

		if (errorObserver->ReportsFailure())
		{
			cerr << "VTI Writer Encountered the following error." << endl;
			cerr << errorObserver->GetMessage() << endl;
		}
		else
			success = true;
	}
	else if (integerFormat)
	{
		///Export to ITK
		intImageType::Pointer intImg = milx::Image<intImageType>::ConvertVTKImageToITKImage(imageReorient->GetOutput());

		success = milx::File::WriteImageUsingITK<intImageType>(filename.toStdString(), intImg);
	}
	else
	{
		///Export to ITK
		floatImageType::Pointer floatImg = milx::Image<floatImageType>::ConvertVTKImageToITKImage(imageReorient->GetOutput());

		success = milx::File::WriteImageUsingITK<floatImageType>(filename.toStdString(), floatImg);
	}

	return success;
}

void milxQtImage::about()
{
	milxQtAboutForm aboutForm(this);

	aboutForm.exec();
}

void milxQtImage::enableUpdates(QStatusBar *bar)
{
	if (!dataPicker)
		dataPicker = vtkSmartPointer<vtkPointPicker>::New();
	Connector->Connect(ui.qvtkWidget->GetRenderWindow()->GetInteractor(),
		vtkCommand::MouseMoveEvent,
		this,
		SLOT(updateCoords(vtkObject*)));

	//updateBar = bar;
}

void milxQtImage::updateCoords(vtkObject *obj)
{
	///Get interactor
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	QString message = "";

	///Get event position
	///Code initial by Mark Wyszomierski 2003-2007 @ devsample
	///Modified by Shekhar Chandra
	///Do the pick. It will return a non-zero value if we intersected the image.
	if (dataPicker->Pick(iren->GetEventPosition()[0],
		iren->GetEventPosition()[1],
		0,  // always zero.
		renderer))
	{
		// Get the mapped position of the mouse using the picker.
		double point[3];
		dataPicker->GetPickPosition(point);

		// Get the volume index within the entire volume now.
		vtkIdType nVolIdx = dataPicker->GetPointId();
		message = "Point " + QString::number(nVolIdx) + ": (" + QString::number(point[0]) + ", " + QString::number(point[1]) + ", " + QString::number(point[2]) + ")";
		
	}

	///Write message to status bar
	cor->setText(message);
}

void milxQtImage::close(QWidget *parent)
{
	printDebug("Closing Main Window");
	QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parent);
	mainWindow->close();
}

void milxQtImage::writeSettings(QWidget *parent)
{
	QSettings settings("Shekhar Chandra", "milxQt");
	settings.beginGroup("milxQtImage");
	if (ui.toolBar->isHidden() == true)
	{
		settings.setValue("toolbar", 0);
	}
	else
	{
		settings.setValue("toolbar", 1);
	}

	settings.setValue("ImageView" , index);
	settings.endGroup();
	printDebug("saveSetting");
}

void milxQtImage::readSettings(QMainWindow *parent)
{
	QSettings settings("Shekhar Chandra", "milxQt");

	settings.beginGroup("milxQtImage");
	toolbarStatus = settings.value("toolbar",0).toInt();

	index = settings.value("ImageView",defaultView).toInt();
	

	///Handle saving dock positions/areas etc.
	settings.endGroup();
	parent->restoreDockWidget(console->dockWidget());
	printDebug("ReadSettings");
	//use value in view to set view in viewer
}

void milxQtImage::dragEnterEvent(QDragEnterEvent *currentEvent)
{
	if (currentEvent->mimeData()->hasFormat("text/uri-list") || currentEvent->mimeData()->hasFormat("text/plain"))
		currentEvent->acceptProposedAction();
}

void milxQtImage::dropEvent(QDropEvent *currentEvent)
{

	QList<QUrl> urlsList = currentEvent->mimeData()->urls();
	QString tmp;

	for (int j = 0; j < urlsList.size(); j++)
	{
		printInfo("j");
		if (urlsList[j].isValid())
		{
			count = count + 1;
#ifdef Q_WS_WIN
			tmp = urlsList[j].path().remove(0, 1); //!< Remove leading forward slash
#else
			tmp = urlsList[j].path().remove(0, 1);
			printInfo(tmp);
#endif
			typedef itk::Image<float, 3> VisualizingImageType;
			VisualizingImageType::Pointer imageType = VisualizingImageType::New();
			;
			if (!milx::File::OpenImage<VisualizingImageType>(tmp.toStdString(), imageType))
			{
				milx::PrintError("Could not open file.");

			}
			vtkSmartPointer<vtkImageData> imageVTK = milx::Image<VisualizingImageType>::ConvertITKImageToVTKImage(imageType);
			vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
			vtkSmartPointer<vtkImageData> imageVTKOrientation = milx::Image<VisualizingImageType>::ApplyOrientationToVTKImage(imageVTK, imageType, matrix, true);
			setData(imageVTKOrientation, count);
			generate(count);
			if (count == 0)
			{
				radio1->setChecked(true);

			}
			if (count == 1)
			{
				radio2->setDisabled(false);
				radio2->setChecked(true);
				blend();

			}
		}
	}

	currentEvent->acceptProposedAction();
}

void milxQtImage::controls()
{
	printDebug("Showing controls available...");
	QPixmap pixmap(":resources/controls_splash.png");
	QSplashScreen *controlsSplash = new QSplashScreen(this);
	controlsSplash->setPixmap(pixmap);
	controlsSplash->setMask(pixmap.mask());
	controlsSplash->show();

	qApp->processEvents();
}


void milxQtImage::createConnections()
{
	QObject::connect(radio1, SIGNAL(clicked()), this, SLOT(switchViewer()));
	QObject::connect(radio2, SIGNAL(clicked()), this, SLOT(switchViewer()));
	QObject::connect(radio3, SIGNAL(clicked()), this, SLOT(switchViewer()));
	QObject::connect(ui.actionBlend, SIGNAL(triggered()), this, SLOT(blend())); 
	QObject::connect(ui.actionCrosshair, SIGNAL(triggered()), this, SLOT(updateWindowsWithCursors()));
	QObject::connect(ui.actionAxial, SIGNAL(triggered()), this, SLOT(viewToXYPlane()));
	QObject::connect(ui.actionCoronal, SIGNAL(triggered()), this, SLOT(viewToZXPlane()));
	QObject::connect(ui.actionSagittal, SIGNAL(triggered()), this, SLOT(viewToZYPlane()));
	QObject::connect(ui.actionAbout_2, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(ui.actionControl_2, SIGNAL(triggered()), this, SLOT(controls()));
	QSignalMapper* mapper = new QSignalMapper;
	mapper->setMapping(ui.actionOpen_2, 0);
	mapper->setMapping(ui.actionOpen, 1);
	QObject::connect(ui.actionIntensity_2, SIGNAL(triggered()), this, SLOT(autoLevel()));
	QObject::connect(ui.actionRefresh_2, SIGNAL(triggered()), this, SLOT(refresh()));
	QObject::connect(ui.actionScreenshot, SIGNAL(triggered()), this, SLOT(saveScreen()));
	QObject::connect(ui.actionOpen_2, SIGNAL(triggered()), mapper, SLOT(map()));
	QObject::connect(ui.actionOpen, SIGNAL(triggered()), mapper, SLOT(map()));
	QObject::connect(mapper, SIGNAL(mapped(int)), this, SLOT(open(int)));
	QObject::connect(expand, SIGNAL(clicked()), actionToolbar, SLOT(trigger()));


}


