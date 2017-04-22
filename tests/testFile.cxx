#include "milxFile.h"
#include "milxGlobal.h"
#include <sstream>

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "View an image with correct orientation." << endl;
		cerr << "Usage: " << argv[0] << " <image>" << endl;
		exit(EXIT_FAILURE);
	}
	std::string fileName = argv[1];

	typedef itk::Image<float, 3> VisualizingImageType;

	///Open image
	VisualizingImageType::Pointer image = VisualizingImageType::New();
	if (!milx::File::OpenImage<VisualizingImageType>(fileName, image))
	{
		milx::PrintError("Could not open file.");
		return EXIT_FAILURE;
	}
	vtkSmartPointer<vtkImageData> imageVTK = milx::File::ConvertITKImageToVTKImage(image);

	std::string savedFileName = argv[2];
	if (!milx::File::SaveImage<VisualizingImageType>(savedFileName,imageVTK))
	{ 
		milx::PrintError("Could not save file.");
		return EXIT_FAILURE;
	}

	//milxFileTest save the image to another name argv[2]
	//repeat for milxQtWindow create a test : show a window.

	return EXIT_SUCCESS;
}


