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
#ifndef __MILXIMAGE_H
#define __MILXIMAGE_H

//ITK
#include <itkVectorImage.h>
#include <itkImageDuplicator.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkAdaptiveHistogramEqualizationImageFilter.h>
#if (ITK_REVIEW || ITK_VERSION_MAJOR > 3) //Review only members
  #include <itkLabelImageToLabelMapFilter.h>
  #include <itkLabelMapToLabelImageFilter.h>
  #include <itkAutoCropLabelMapFilter.h>
  #include <itkBinaryContourImageFilter.h>
  #include <itkLabelContourImageFilter.h>
  #include <itkLabelOverlayImageFilter.h>
  #include <itkMergeLabelMapFilter.h>
#endif // (ITK_REVIEW || ITK_VERSION_MAJOR > 3)
#include <itkImportImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkNormalizeImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkHistogramMatchingImageFilter.h>
#include <itkCheckerBoardImageFilter.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkApproximateSignedDistanceMapImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkFlipImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkBilateralImageFilter.h>
#include <itkSmoothingRecursiveGaussianImageFilter.h>
#include <itkSobelEdgeDetectionImageFilter.h>
#include <itkMedianImageFilter.h>
#include <itkLaplacianRecursiveGaussianImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkIdentityTransform.h>
#include <itkExtractImageFilter.h>
#include <itkVectorIndexSelectionCastImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkAffineTransform.h>
#include <itkVersorRigid3DTransform.h>
#include <itkCenteredEuler3DTransform.h>
#include <itkEuler3DTransform.h>
#include <itkTransformFileWriter.h>
#include <itkShrinkImageFilter.h>
#include <itkAddImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkJoinSeriesImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
//ITK4 Compatibility
#if (ITK_VERSION_MAJOR > 3)
  #include <itkv3Rigid3DTransform.h> //ITK4 (4.3.1) version has New() member issues
  #include <itkFFTConvolutionImageFilter.h>
  #include <itkVectorMagnitudeImageFilter.h> //vector images
#endif
//Exporter/Importer
#include "itkImageToVTKImageFilter.h"//¡Ù
#include "itkVTKImageToImageFilter.h"
#ifndef ITK_ONLY
  //VTK
  #include <vtkMatrix4x4.h>
  #include <vtkTransform.h>
  #include <vtkImageReslice.h>
#endif
//ITK Extensions
#include "milxGlobal.h"

const double CoordinateTolerance = 1e-3;
const double DirectionTolerance = 1e-3;

namespace milx
{

class SMILI_EXPORT ImageBase
{
public:
  /*!
  	\fn Image::ImageBase()
  	\brief Standard constructor
  */
  ImageBase();
  /*!
  	\fn Image::~ImageBase()
  	\brief Standard Destructor
  */
  virtual ~ImageBase() {};
};
/**
  \class Image
  \brief Represents an image (i.e. an regular rectangular array with scalar values) and their common operations using the Insight Toolkit (ITK).

  Due to the templated nature of itk::Image objects, most of the members in this class are static and the current state is not tracked.
  This class is used extensively throughout the milxQtImage class.
  Use the milx::File object to load and save images.
  See the usage examples given below:

  Converting
  \code
  typedef unsigned char InputPixelType;
  typedef itk::Image<InputPixelType, 3> LabelImageType;
  LabelImageType::SpacingType labelSpacing = m_LabelledImages[index]->GetSpacing();

*/
template<class TImage>
class SMILI_EXPORT Image : public ImageBase
{
public:
  /*!
  	\fn Image::Image()
  	\brief Standard constructor
  */
  Image();
  /*!
  	\fn Image::Image(itk::SmartPointer<TImage> image)
  	\brief Constructor that copies the input model.
  */
//  Image(itk::SmartPointer<TImage> image);
  /*!
  	\fn Image::~Image()
  	\brief Standard Destructor
  */
  virtual ~Image() {};
  static vtkSmartPointer<vtkImageData> ConvertITKImageToVTKImage(itk::SmartPointer<TImage> img);
  /*!
  \fn Image::ConvertVTKImageToITKImage(vtkSmartPointer<vtkImageData> img)
  \brief Converts a VTK image object to an ITK image object.
  */
  static itk::SmartPointer<TImage> ConvertVTKImageToITKImage(vtkSmartPointer<vtkImageData> img);
  /*!
  \fn Image::DuplicateImage(itk::SmartPointer<TImage> img)
  \brief Duplicates the image into a new image.
  */
  static itk::SmartPointer<TImage> DuplicateImage(itk::SmartPointer<TImage> img);
#ifndef ITK_ONLY //Requires VTK
  /*!
  \fn Image:: ApplyOrientationToVTKImage(vtkSmartPointer<vtkImageData> img, itk::SmartPointer<TImage> refImage, vtkSmartPointer<vtkMatrix4x4> &transformMatrix, const bool labelledImage, const bool flipY = true)
  \brief Applies orientation/direction and origin to a VTK image from a reference image.

  Flip y-axis to comply with VTK coordinate system? The transformMatrix, where the extraneous transforms (from the image orientation, such as flipping) will be stored, need not be pre-allocated.

  \code
  vtkSmartPointer<vtkImageData> newImageData = vtkSmartPointer<vtkImageData>::New();
  newImageData->DeepCopy( milx::Image<floatImageType>::ConvertITKImageToVTKImage(imageFloat) );
  imageData = milx::Image<floatImageType>::ApplyOrientationToVTKImage(newImageData, imageFloat, transformMatrix, true, true);
  \endcode
  */
  static vtkSmartPointer<vtkImageData> ApplyOrientationToVTKImage(vtkSmartPointer<vtkImageData> img, itk::SmartPointer<TImage> refImage, vtkSmartPointer<vtkMatrix4x4> &transformMatrix, const bool labelledImage, const bool flipY = true);
#endif
};

template<class TImage>
Image<TImage>::Image()
{

}


//Arithmetic
template<class TImage>
vtkSmartPointer<vtkImageData> Image<TImage>::ConvertITKImageToVTKImage(itk::SmartPointer<TImage> img)
{
  typedef itk::ImageToVTKImageFilter<TImage> ConvertImageType;

  typename ConvertImageType::Pointer convertFilter = ConvertImageType::New();
  convertFilter->SetInput(img);
  convertFilter->AddObserver(itk::ProgressEvent(), ProgressUpdates);
  try
  {
    convertFilter->Update();
  }
  catch (itk::ExceptionObject & ex )
  {
    PrintError("Failed Converting ITK Image to VTK Image");
    PrintError(ex.GetDescription());
  }

  return convertFilter->GetOutput();
}

template<class TImage>
itk::SmartPointer<TImage> Image<TImage>::ConvertVTKImageToITKImage(vtkSmartPointer<vtkImageData> img)
{
	typedef itk::VTKImageToImageFilter<TImage> ConvertImageType;

	typename ConvertImageType::Pointer convertFilter = ConvertImageType::New();
	convertFilter->SetInput(img);
	convertFilter->AddObserver(itk::ProgressEvent(), ProgressUpdates);
	try
	{
		convertFilter->Update();
	}
	catch (itk::ExceptionObject & ex)
	{
		PrintError("Failed Converting VTK Image to ITK Image");
		PrintError(ex.GetDescription());
	}

	itk::SmartPointer<TImage> tmpImage = DuplicateImage(convertFilter->GetOutput());

	return tmpImage;
}

template<class TImage>
itk::SmartPointer<TImage> Image<TImage>::DuplicateImage(itk::SmartPointer<TImage> img)
{
	typedef itk::ImageDuplicator<TImage> DuplicateType;

	typename DuplicateType::Pointer duplicator = DuplicateType::New();
	duplicator->SetInputImage(img);
	duplicator->AddObserver(itk::ProgressEvent(), ProgressUpdates);
	try
	{
		duplicator->Update();
	}
	catch (itk::ExceptionObject & ex)
	{
		PrintError("Failed Generating Duplicate Image");
		PrintError(ex.GetDescription());
	}

	return duplicator->GetOutput();
}

#ifndef ITK_ONLY //Requires VTK
template<class TImage>
vtkSmartPointer<vtkImageData> Image<TImage>::ApplyOrientationToVTKImage(vtkSmartPointer<vtkImageData> img, itk::SmartPointer<TImage> refImage, vtkSmartPointer<vtkMatrix4x4> &transformMatrix, const bool labelledImage, const bool flipY)
{
	typename TImage::DirectionType direction = refImage->GetDirection();
	typename TImage::PointType origin = refImage->GetOrigin();

	vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New(); //start with identity matrix
	matrix->Identity();
	for (int i = 0; i < 3; i++)
		for (int k = 0; k < 3; k++)
			matrix->SetElement(i, k, direction(i, k));
	matrix->Transpose(); //img volume to correct space, comment to go from space to img volume

	if (!transformMatrix)
		transformMatrix = vtkSmartPointer<vtkMatrix4x4>::New(); //start with identity matrix
	transformMatrix->Identity();
	//Flip the image for VTK coordinate system
	if (flipY)
		transformMatrix->SetElement(1, 1, -1); //flip

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->Identity();
	transform->PostMultiply();
	transform->Concatenate(transformMatrix); //flip
	transform->Translate(-origin[0], -origin[1], -origin[2]); //remove origin
	transform->Concatenate(matrix); //direction
	transform->Translate(origin.GetDataPointer()); //add origin displacement

	vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
#if VTK_MAJOR_VERSION <= 5
	reslice->SetInput(img);
#else
	reslice->SetInputData(img);
#endif
	reslice->AddObserver(vtkCommand::ProgressEvent, VTKProgressUpdates);
	reslice->AutoCropOutputOn();
	reslice->TransformInputSamplingOn();
	reslice->SetOutputDimensionality(milx::imgDimension);
	reslice->SetResliceAxes(transform->GetMatrix());

	if (!labelledImage)
		reslice->SetInterpolationModeToLinear(); //reduce artefacts for normal images
	else
	{
		reslice->SetInterpolationModeToNearestNeighbor(); //reduce artefacts for labels
		PrintDebug("Using NN Interpolator for Image Orientation.");
		//  #if VTK_MAJOR_VERSION > 5
		//      reslice->SetOutputScalarType(VTK_CHAR);
		//  #endif
	}
	reslice->Update();

	std::string resliceType = reslice->GetOutput()->GetScalarTypeAsString();
	PrintDebug("Orientated Image Type as " + resliceType);

	return reslice->GetOutput();
}
#endif
} //end namespace milx

#endif //__MILXIMAGE_H
