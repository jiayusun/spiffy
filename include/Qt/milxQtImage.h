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
#ifndef MILXQTIMAGE_H
#define MILXQTIMAGE_H

#include <vtkSmartPointer.h>
#include "milxQtRenderWindow.h"
#include <vtkImageMapToWindowLevelColors.h>
#include "vtkImageViewer3.h" //smili vtk-ext
#include "milxImage.h"
#include "ui_spiffy.h"
#include <QButtonGroup>



class MILXQT_EXPORT milxQtImage : public milxQtRenderWindow
{
    Q_OBJECT

public:
    /*!
        \fn milxQtImage::milxQtImage(QWidget *parent = 0, bool contextSystem = true)
        \brief The standard constructor
    */
    milxQtImage(QWidget *theParent = 0, bool contextSystem = true);
    /*!
        \fn milxQtImage::~milxQtImage()
        \brief The standard destructor
    */
    virtual ~milxQtImage();
	void setData(vtkSmartPointer<vtkImageData> newImg,int i);
	void generate(int i);
	/*!
	\fn milxQtMain::loadFile(const QString &filename)
	\brief Opens a file for viewing in the current tab.
	*/
	bool loadFile(const QString &filename);
    void setView(int viewMode);
    /*!
        \fn milxQtImage::viewToXYPlane()
        \brief Change view to xy-plane.
    */
    virtual void viewToXYPlane();
    /*!
        \fn milxQtImage::viewToZXPlane()
        \brief Change view to zx-plane.
    */
    virtual void viewToZXPlane();
    /*!
        \fn milxQtImage::viewToZYPlane()
        \brief Change view to zy-plane.
    */
    virtual void viewToZYPlane();

public slots:
	void changeView(QString);
	void open(int i);
	void switchViewer();
	void blend();
	void showSlideWidgetPressed();
	inline vtkImageMapToWindowLevelColors* GetWindowLevel(int i)
	{
		return viewer[i]->GetWindowLevel();
	}

protected:
	Ui_MainWindow ui;
	bool viewerSetup; //!< has the viewer/window been setup (only done initial so is to not disturb users settings)
	bool volume; //!< is the image a volume?
	float opacity = 0.5;
	//Flags
	bool usingVTKImage; //!< using VTK image data?
	bool eightbit; //!< Using eightbit data?
	bool rgb; //!< Using RGB data?
	QString openSupport; //!< Load file extension support list, cats all known extensions.
	bool vectorised; //!< Using Vector image data?
	bool flipped; //!< Flip for display?
	int currentViewer;
	QButtonGroup *btnGroup;
	QWidget *slide;
	vtkSmartPointer<vtkImageViewer3> viewer[3]; //!< VTK Viewer handler, Smart Pointer
	vtkSmartPointer<vtkImageData> imageData[3];
	/*!
	\fn milxQtMain::createConnections()
	\brief Creates the signals and slots connections within the main window.
	*/
	void createConnections();

};

#endif // MILXQTIMAGE_H
