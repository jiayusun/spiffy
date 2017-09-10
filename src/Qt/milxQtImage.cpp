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
#include "milxQtImage.h"
#include "milxFile.h"
#include <QApplication>
#include <QInputDialog>
#include <QFile>
#include <QMainWindow>
#include "milxQtImage.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkImageBlend.h>
#include <QFileDialog>
#include <QMessageBox>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageMapToColors.h>
#include "vtkImageViewer3.h"
#include "milxFile.h"
#include "milxImage.h"
#include "milxGlobal.h"
#include <sstream>
#include "ui_spiffy.h"
#include "milxQtAliases.h"

//milxQtImage
milxQtImage::milxQtImage(QWidget *theParent, bool contextSystem) : milxQtRenderWindow(theParent, contextSystem)
{
	QMainWindow *mainWindow = qobject_cast<QMainWindow *>(theParent);
	ui.setupUi(mainWindow);
	for (int j = 0; j < 3; j++)
	{
		viewer[j] = vtkSmartPointer<vtkImageViewer3>::New();
		imageData[j] = vtkSmartPointer<vtkImageData>::New();
	}
	openSupport = extensionsOpen.c_str();
	milx::PrintDebug("constructor");
	btnGroup = new QButtonGroup(this);
	btnGroup->addButton(ui.radioButton, 0);
	btnGroup->addButton(ui.radioButton_2, 1);
	btnGroup->addButton(ui.radioButton_3, 2);
	ui.radioButton->setChecked(true);
	createConnections();
}

milxQtImage::~milxQtImage()
{
    ///Smart pointers handle deletion
}

void milxQtImage::setData(vtkSmartPointer<vtkImageData> newImg,int i)
{
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

void milxQtImage::generate(int i)
{
	int bounds[6];
	// Visualize
	viewer[i]->SetInputData(imageData[i]);
	//imageViewer->GetRenderWindow()->SetSize(500, 500);
	//
	setGeometry(QRect(0, 25, 590, 590));
	milxQtRenderWindow::SetRenderer(viewer[i]->GetRenderer());
	ui.qvtkWidget->SetRenderWindow(viewer[i]->GetRenderWindow());
	viewer[i]->SetupInteractor(ui.qvtkWidget->GetRenderWindow()->GetInteractor());
	viewer[i]->SetSlice(bounds[5] / 2); //show middle of volume
	viewer[i]->GetInteractorStyle()->InvokeEvent(vtkCommand::ResetWindowLevelEvent);
	viewer[i]->GetRenderer()->ResetCamera();
	viewer[i]->UpdateCursor();
	viewer[i]->Render();
	viewer[i]->SetSliceOrientationToXZ();
	currentViewer = i;
	update();
}

void milxQtImage::changeView(QString index)
{

	printDebug("Changing view to " + index);
	if (index == "View: Axial")
	{
		viewer[currentViewer]->SetSliceOrientationToXY();

	}
	else if (index == "View: Coronal")
	{
		viewer[currentViewer]->SetSliceOrientationToXZ();

	}
	else if (index == "View: Sagittal")
	{
		viewer[currentViewer]->SetSliceOrientationToYZ();

	}
}


void milxQtImage::setView(int viewMode)
{
    if(!volume)
    {
        printDebug("Volume is 2D. Not changing view to " + QString::number(viewMode));
        viewToXYPlane();
        return;
    }

    milxQtRenderWindow::setView(viewMode);
}

void milxQtImage::viewToXYPlane()
{
    if(viewerSetup)
	{

		viewer[currentViewer]->SetSliceOrientationToXY();

        currentView = AXIAL;
    }
}

void milxQtImage::viewToZXPlane()
{
    if(viewerSetup)
	{

		viewer[currentViewer]->SetSliceOrientationToXZ();
        currentView = CORONAL;
    }
}

void milxQtImage::viewToZYPlane()
{
    if(viewerSetup)
	{
		viewer[currentViewer]->SetSliceOrientationToYZ();
        currentView = SAGITTAL;
    }
}

void milxQtImage::blend()
{
	bool ok1 = true;
	opacity = QInputDialog::getDouble(this, tr("Please Provide the opacity of the blending"),
		tr("Opacity:"), 0.5, 0, 2147483647, 2, &ok1);
	
	if (!ok1)
		return;

	emit working(-1);
	vtkSmartPointer<vtkImageData> ucharData1 = GetWindowLevel(0)->GetOutput();
	vtkSmartPointer<vtkImageData> ucharData2 = GetWindowLevel(1)->GetOutput();
	printDebug("Number of components in Blending Image: " + QString::number(ucharData2->GetNumberOfScalarComponents()));
	// Combine the images (blend takes multiple connections on the 0th input port)
	vtkSmartPointer<vtkImageBlend> blend = vtkSmartPointer<vtkImageBlend>::New();
#if VTK_MAJOR_VERSION <= 5
	blend->AddInput(ucharData1);
	blend->AddInput(ucharData2);
#else
	blend->AddInputData(ucharData1);
	blend->AddInputData(ucharData2);
#endif
	blend->SetOpacity(0, opacity);
	blend->SetOpacity(1, opacity);
	linkProgressEventOf(blend);
	//        blend->SetBlendModeToCompound();
	blend->SetBlendModeToNormal();
	blend->Update();

	printDebug("Number of components in Blended Image: " + QString::number(blend->GetOutput()->GetNumberOfScalarComponents()));
	printDebug("Type of pixel in Blended Image: " + QString(blend->GetOutput()->GetScalarTypeAsString()));
	setData(blend->GetOutput(),2);
	emit done(-1);

	//Force RGB colouring
	GetWindowLevel(2)->SetLookupTable(NULL);
	lookupTable = NULL;
	GetWindowLevel(2)->SetWindow(255);
	GetWindowLevel(2)->SetLevel(127.5);
	generate(2);
	ui.radioButton_3->setChecked(true);
	
	
}

void milxQtImage::showSlideWidgetPressed()
{
	slide = new QWidget(this);
	slide->setGeometry(-slide->width(), 0, slide->width(), slide->height());
	slide->show();
	// then a animation:
	QPropertyAnimation *animation = new QPropertyAnimation(slide, "pos");
	animation->setDuration(10000);
	animation->setStartValue(slide->pos());
	animation->setEndValue(QPoint(0, 0));

	// to slide in call
	animation->start();
}

void milxQtImage::open(int i)
{
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
			ui.radioButton->setChecked(true);
		}
		if (i = 1)
		{
			ui.radioButton_2->setChecked(true);
		}
	}
}

void milxQtImage::switchViewer()
{
	switch (btnGroup->checkedId())
	{
	case 0:
		generate(0);
		break;
	case 1:
		generate(1);
		break;
	case 2:
		generate(2);
		break;
	}
	
}

void milxQtImage::createConnections()
{
	QObject::connect(ui.comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeView(QString)));
	QObject::connect(ui.radioButton, SIGNAL(clicked()), this, SLOT(switchViewer()));
	QObject::connect(ui.radioButton_2, SIGNAL(clicked()), this, SLOT(switchViewer()));
	QObject::connect(ui.radioButton_3, SIGNAL(clicked()), this, SLOT(switchViewer()));
	QObject::connect(ui.pushButton_12, SIGNAL(clicked()), this, SLOT(blend()));
	QObject::connect(ui.pushButton_10, SIGNAL(clicked()), this, SLOT(showSlideWidgetPressed()));
	QSignalMapper* mapper = new QSignalMapper;
	mapper->setMapping(ui.actionOpen_2, 0);
	mapper->setMapping(ui.actionOpen, 1);
	QObject::connect(ui.actionOpen_2, SIGNAL(triggered()), mapper, SLOT(map()));
	QObject::connect(ui.actionOpen, SIGNAL(triggered()), mapper, SLOT(map()));
	QObject::connect(mapper, SIGNAL(mapped(int)), this, SLOT(open(int)));
}


