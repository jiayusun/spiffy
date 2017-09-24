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
#include <vtkImageAccumulate.h>
#include <vtkSmartPointer.h>
#include "milxQtRenderWindow.h"
#include <vtkImageMapToWindowLevelColors.h>
#include "vtkImageViewer3.h" //smili vtk-ext
#include "milxImage.h"
#include "ui_spiffy.h"
#include <QButtonGroup>

typedef unsigned char charPixelType;
typedef float floatPixelType;
typedef int intPixelType;
typedef itk::RGBPixel<unsigned char> rgbPixelType;
typedef itk::Image<rgbPixelType, milx::imgDimension> rgbImageType;
typedef itk::Image<charPixelType, milx::imgDimension> charImageType;
typedef itk::Image<floatPixelType, milx::imgDimension> floatImageType;
typedef itk::Image<intPixelType, milx::imgDimension> intImageType;

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
	void generate(int i, const bool quietly = false);
	/*!
	\fn milxQtMain::loadFile(const QString &filename)
	\brief Opens a file for viewing in the current tab.
	*/
	bool loadFile(const QString &filename);
	/*!
	\brief Get the data, return the vtkImageData object downcast to vtkDataSet, useful for getting scalar range etc.
	*/
	virtual vtkDataSet* GetDataSet(int i)
	{
		return imageData[i];
	}
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
	inline bool is8BitImage()
	{
		return eightbit;
	}

public slots:
	void changeView(QString);
	void updateWindowsWithCursors();
	void open(int i);
	void switchViewer();
	void blend();
	inline vtkImageMapToWindowLevelColors* GetWindowLevel(int i)
	{
		return viewer[i]->GetWindowLevel();
	}
	void autoLevel(float percentile = 0.99);
	/*!
	\fn milxQtImage::GetIntensityWindow()
	\brief Get the window for the image intensity tranfer function
	*/
	double GetIntensityWindow();
	/*!
	\fn milxQtImage::GetIntensityLevel()
	\brief Get the level for the image intensity tranfer function
	*/
	double GetIntensityLevel();
	/*!
	\fn milxQtImage::updateLookupTable(int i)
	\brief Sets the necessary LUTs to model view.
	*/
	virtual void updateLookupTable(int i);
	/**
	\fn milxQtImage::histogram(int bins = 256, float belowValue = 0, float aboveValue = 255, bool plotHistogram = true)
	\brief Computes the histogram of the image and prints info found.
	*/
	void histogram(int bins = 256, float belowValue = 0, float aboveValue = 255, bool plotHistogram = true);
	/*!
	\fn milxQtRenderWindow::colourMapToGray(int i, double minRange = 0.0, double maxRange = 0.0)
	\brief Change the colour map to Gray
	*/
	virtual void colourMapToGray(int i,double minRange = 0.0, double maxRange = 0.0);
	/*!
	\fn milxQtRenderWindow::colourMapToHSV(double minRange = 0.0, double maxRange = 0.0)
	\brief Change the colour map to HSV
	*/
	virtual void colourMapToHSV(int i, double minRange = 0.0, double maxRange = 0.0);
	void saveScreen(QString filename = "");
	inline vtkLookupTable* GetLookupTable(int i)
	{
		return lookupTable[i];
	}
	/*!
	\fn milxQtFile::saveImage(const QString filename, vtkImageData* data)
	\brief Saves data as an image file, which is any of the following: JPEG, PNG, DICOM, TIFF, NIFTI, HDR etc.

	Returns true if successful. ImageData is allocated within this member, so pass a NULL pointer.
	Image is also flipped since the ITK reader orientation is different to VTK images.
	*/
	bool saveImage(const QString filename, vtkImageData* data);

protected:
	Ui_MainWindow ui;
	bool viewerSetup; //!< has the viewer/window been setup (only done initial so is to not disturb users settings)
	bool volume; //!< is the image a volume?
	float opacity = 0.5;
	vtkSmartPointer<vtkImageAccumulate> hist; //!< Histogram filter, allocated on histogram() call
	bool track; //!< track the coordinates during user interaction
	///Other Variables
	double meanValue; //!< Average data value currently held
	double stddevValue; //!< Std deviation data value currently held
	double minValue; //!< min value in image
	double maxValue; //!< max value in image
	size_t actualNumberOfDimensions; //!< All images loaded as 3D images or 3D vector image
	//Flags
	bool usingVTKImage; //!< using VTK image data?
	bool eightbit; //!< Using eightbit data?
	bool rgb; //!< Using RGB data?
	QString openSupport; //!< Load file extension support list, cats all known extensions.
	bool vectorised; //!< Using Vector image data?
	bool flipped; //!< Flip for display?
	int currentViewer;
	QButtonGroup *btnGroup;
	vtkSmartPointer<vtkImageViewer3> viewer[3]; //!< VTK Viewer handler, Smart Pointer
	vtkSmartPointer<vtkImageData> imageData[3];
	vtkSmartPointer<vtkLookupTable> lookupTable[3]; //!< Lookup table for the shapes/images, base class is used to allow references to different look up table types
	bool timestamping; //!< Prefer showing timestamp?
	QList< QAction* > dockActions; //!< List of dock actions of dock widgets loaded succesfully.
	QPointer<milxQtConsole> console; //!< console docked window
	QAction* actionConsole; //!< toggle action for console
	QAction* actionToolbar;//!< toggle action for toolbar
	rgbImageType::Pointer imageRGB; //!< Up to date 32-bit image data (used only internally atm)
	charImageType::Pointer imageChar; //!< Up to date 8-bit greyscale image data
	floatImageType::Pointer imageFloat; //!< Up to date floating point image data
	int magnifyFactor = 2;
	/*!
	\fn milxQtImage::updateData(const bool orient = true)
	\brief Ensures the internal visualisation data is up to date.
	*/
	void updateData(const bool orient = true);
	/*!
	\fn milxQtMain::createConnections()
	\brief Creates the signals and slots connections within the main window.
	*/
	void createConnections();


};

#endif // MILXQTIMAGE_H
