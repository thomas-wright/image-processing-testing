/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
//  Software Guide : BeginLatex
//
//  Probably the most common representation of datasets in clinical
//  applications is the one that uses sets of DICOM slices in order to compose
//  tridimensional images. This is the case for CT, MRI and PET scanners. It is
//  very common therefore for image analysts to have to process volumetric
//  images that are stored in the form of a set of DICOM files belonging to a
//  common DICOM series.
//
//  The following example illustrates how to use ITK functionalities in order
//  to read a DICOM series into a volume and then save this volume in another
//  file format.
//
//  The example begins by including the appropriate headers. In particular we
//  will need the \doxygen{GDCMImageIO} object in order to have access to the
//  capabilities of the GDCM library for reading DICOM files, and the
//  \doxygen{GDCMSeriesFileNames} object for generating the lists of filenames
//  identifying the slices of a common volumetric dataset.
//
//  \index{itk::ImageSeriesReader!header}
//  \index{itk::GDCMImageIO!header}
//  \index{itk::GDCMSeriesFileNames!header}
//  \index{itk::ImageFileWriter!header}
//
//  Software Guide : EndLatex
// Software Guide : BeginCodeSnippet
#include "itkImage.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"

#include <itkImageToVTKImageFilter.h>

#define USE_BASIC_IMAGE_VIEWER_APPROACH 0
#if USE_BASIC_IMAGE_VIEWER_APPROACH

#include "vtkVersion.h"
#include "vtkImageViewer.h"
#include "vtkImageMapper3D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkImageActor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderer.h"

#else

#include "vtkSmartPointer.h"
#include "vtkImageReader2.h"
#include "vtkMatrix4x4.h"
#include "vtkImageReslice.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToColors.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkCommand.h"
#include "vtkImageData.h"
#include "vtkImageViewer.h"

#include "vtkImageInteractionCallback.hpp"

#endif

// Software Guide : EndCodeSnippet
int main( int argc, char* argv[] )
{
    if( argc < 3 )
    {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0] << " DicomDirectory  outputFileName  [seriesName]"
        << std::endl;
        return EXIT_FAILURE;
    }
    // Software Guide : BeginLatex
    //
    // We define the pixel type and dimension of the image to be read. In this
    // particular case, the dimensionality of the image is 3, and we assume a
    // \code{signed short} pixel type that is commonly used for X-Rays CT scanners.
    //
    // The image orientation information contained in the direction cosines
    // of the DICOM header are read in and passed correctly down the image processing
    // pipeline.
    //
    // Software Guide : EndLatex
    // Software Guide : BeginCodeSnippet
    typedef signed short    PixelType;
    const unsigned int      Dimension = 3;
    typedef itk::Image< PixelType, Dimension >         ImageType;
    // Software Guide : EndCodeSnippet
    // Software Guide : BeginLatex
    //
    // We use the image type for instantiating the type of the series reader and
    // for constructing one object of its type.
    //
    // Software Guide : EndLatex
    // Software Guide : BeginCodeSnippet
    typedef itk::ImageSeriesReader< ImageType >        ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    // Software Guide : EndCodeSnippet
    // Software Guide : BeginLatex
    //
    // A GDCMImageIO object is created and connected to the reader. This object is
    // the one that is aware of the internal intricacies of the DICOM format.
    //
    // Software Guide : EndLatex
    // Software Guide : BeginCodeSnippet
    typedef itk::GDCMImageIO       ImageIOType;
    ImageIOType::Pointer dicomIO = ImageIOType::New();
    reader->SetImageIO( dicomIO );
    // Software Guide : EndCodeSnippet
    // Software Guide : BeginLatex
    //
    // Now we face one of the main challenges of the process of reading a DICOM
    // series. That is, to identify from a given directory the set of filenames
    // that belong together to the same volumetric image. Fortunately for us, GDCM
    // offers functionalities for solving this problem and we just need to invoke
    // those functionalities through an ITK class that encapsulates a communication
    // with GDCM classes. This ITK object is the GDCMSeriesFileNames. Conveniently
    // for us, we only need to pass to this class the name of the directory where
    // the DICOM slices are stored. This is done with the \code{SetDirectory()}
    // method. The GDCMSeriesFileNames object will explore the directory and will
    // generate a sequence of filenames for DICOM files for one study/series.
    // In this example, we also call the \code{SetUseSeriesDetails(true)} function
    // that tells the GDCMSereiesFileNames object to use additional DICOM
    // information to distinguish unique volumes within the directory.  This is
    // useful, for example, if a DICOM device assigns the same SeriesID to
    // a scout scan and its 3D volume; by using additional DICOM information
    // the scout scan will not be included as part of the 3D volume.  Note that
    // \code{SetUseSeriesDetails(true)} must be called prior to calling
    // \code{SetDirectory()}. By default \code{SetUseSeriesDetails(true)} will use
    // the following DICOM tags to sub-refine a set of files into multiple series:
    //
    // \begin{description}
    // \item[0020 0011] Series Number
    // \item[0018 0024] Sequence Name
    // \item[0018 0050] Slice Thickness
    // \item[0028 0010] Rows
    // \item[0028 0011] Columns
    // \end{description}
    //
    // If this is not enough for your specific case you can always add some more
    // restrictions using the \code{AddSeriesRestriction()} method. In this example we will use
    // the DICOM Tag: 0008 0021 DA 1 Series Date, to sub-refine each series. The format
    // for passing the argument is a string containing first the group then the element
    // of the DICOM tag, separed by a pipe (|) sign.
    //
    //
    // \index{itk::GDCMSeriesFileNames!SetDirectory()}
    //
    // Software Guide : EndLatex
    // Software Guide : BeginCodeSnippet
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetUseSeriesDetails( true );
    nameGenerator->AddSeriesRestriction("0008|0021" );
    nameGenerator->SetDirectory( argv[1] );
    // Software Guide : EndCodeSnippet
    try
    {
        std::cout << std::endl << "The directory: " << std::endl;
        std::cout << std::endl << argv[1] << std::endl << std::endl;
        std::cout << "Contains the following DICOM Series: ";
        std::cout << std::endl << std::endl;
        // Software Guide : BeginLatex
        //
        // The GDCMSeriesFileNames object first identifies the list of DICOM series
        // that are present in the given directory. We receive that list in a reference
        // to a container of strings and then we can do things like printing out all
        // the series identifiers that the generator had found. Since the process of
        // finding the series identifiers can potentially throw exceptions, it is
        // wise to put this code inside a try/catch block.
        //
        // Software Guide : EndLatex
        // Software Guide : BeginCodeSnippet
        typedef std::vector< std::string >    SeriesIdContainer;
        const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
        SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
        SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
        while( seriesItr != seriesEnd )
        {
            std::cout << seriesItr->c_str() << std::endl;
            ++seriesItr;
        }
        // Software Guide : EndCodeSnippet
        // Software Guide : BeginLatex
        //
        // Given that it is common to find multiple DICOM series in the same directory,
        // we must tell the GDCM classes what specific series do we want to read. In
        // this example we do this by checking first if the user has provided a series
        // identifier in the command line arguments. If no series identifier has been
        // passed, then we simply use the first series found during the exploration of
        // the directory.
        //
        // Software Guide : EndLatex
        // Software Guide : BeginCodeSnippet
        std::string seriesIdentifier;
        if( argc > 3 ) // If no optional series identifier
        {
            seriesIdentifier = argv[3];
        }
        else
        {
            seriesIdentifier = seriesUID.begin()->c_str();
        }
        // Software Guide : EndCodeSnippet
        std::cout << std::endl << std::endl;
        std::cout << "Now reading series: " << std::endl << std::endl;
        std::cout << seriesIdentifier << std::endl;
        std::cout << std::endl << std::endl;
        // Software Guide : BeginLatex
        //
        // We pass the series identifier to the name generator and ask for all the
        // filenames associated to that series. This list is returned in a container of
        // strings by the \code{GetFileNames()} method.
        //
        // \index{itk::GDCMSeriesFileNames!GetFileNames()}
        //
        // Software Guide : EndLatex
        // Software Guide : BeginCodeSnippet
        typedef std::vector< std::string >   FileNamesContainer;
        FileNamesContainer fileNames;
        fileNames = nameGenerator->GetFileNames( seriesIdentifier );
        // Software Guide : EndCodeSnippet
        // Software Guide : BeginLatex
        //
        //
        // The list of filenames can now be passed to the \doxygen{ImageSeriesReader}
        // using the \code{SetFileNames()} method.
        //
        //  \index{itk::ImageSeriesReader!SetFileNames()}
        //
        // Software Guide : EndLatex
        // Software Guide : BeginCodeSnippet
        reader->SetFileNames( fileNames );
        // Software Guide : EndCodeSnippet
        // Software Guide : BeginLatex
        //
        // Finally we can trigger the reading process by invoking the \code{Update()}
        // method in the reader. This call as usual is placed inside a \code{try/catch}
        // block.
        //
        // Software Guide : EndLatex
        // Software Guide : BeginCodeSnippet
        try
        {
            reader->Update();
        }
        catch (itk::ExceptionObject &ex)
        {
            std::cout << ex << std::endl;
            return EXIT_FAILURE;
        }
        // Software Guide : EndCodeSnippet
        // Software Guide : BeginLatex
        //
        // At this point, we have a volumetric image in memory that we can access by
        // invoking the \code{GetOutput()} method of the reader.
        //
        // Software Guide : EndLatex

    
        // TGW: snip - remove writer code from DicomSeriesReadImageWrite2.cxx and replace with renderer
        typedef itk::ImageToVTKImageFilter<ImageType>       ConnectorType;
        ConnectorType::Pointer connector = ConnectorType::New();
        connector->SetInput(reader->GetOutput());
        connector->Update();

#if USE_BASIC_IMAGE_VIEWER_APPROACH

        vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
        actor->GetMapper()->SetInputData(connector->GetOutput());

        vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
        renderer->AddActor(actor);
        renderer->ResetCamera();
        
        vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
        renderWindow->AddRenderer(renderer);
        
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
        
        renderWindowInteractor->SetInteractorStyle(style);
        
        renderWindowInteractor->SetRenderWindow(renderWindow);
        renderWindowInteractor->Initialize();
        
        renderWindowInteractor->Start();
#else
// TGW yet another method using vtkImageViewer (https://itk.org/CourseWare/Training/GettingStarted-II.pdf)
// But this is a very simple 2D viewer...which can't on its own do coronal/sagittal slices, so probably want something more general
//        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//        vtkSmartPointer<vtkImageViewer> viewer = vtkImageViewer::New();
//        viewer->SetupInteractor(renderWindowInteractor);
//        viewer->SetInputData(connector->GetOutput());
//        viewer->Render();
//        viewer->SetColorWindow(1000);
//        viewer->SetColorLevel(500);
//        renderWindowInteractor->Start();
        
        int extent[6];
        double spacing[3];
        double origin[3];
        connector->GetOutput()->GetExtent(extent);
        connector->GetOutput()->GetSpacing(spacing);
        connector->GetOutput()->GetOrigin(origin);
        
        double center[3];
        center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
        center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
        center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
        
        // Matrices for axial, coronal, sagittal, oblique view orientations
        static double axialElements[16] = {
                 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1 };
        
        static double coronalElements[16] = {
                 1, 0, 0, 0,
                 0, 0, 1, 0,
                 0,-1, 0, 0,
                 0, 0, 0, 1 };
    
        static double sagittalElements[16] = {
            0, 0,-1, 0,
            1, 0, 0, 0,
            0,-1, 0, 0,
            0, 0, 0, 1 };
        
        //static double obliqueElements[16] = {
        //         1, 0, 0, 0,
        //         0, 0.866025, -0.5, 0,
        //         0, 0.5, 0.866025, 0,
        //         0, 0, 0, 1 };
        
        // Set the slice orientation
        vtkSmartPointer<vtkMatrix4x4> resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
        resliceAxes->DeepCopy(axialElements);
        // Set the point through which to slice
        resliceAxes->SetElement(0, 3, center[0]);
        resliceAxes->SetElement(1, 3, center[1]);
        resliceAxes->SetElement(2, 3, center[2]);
        
        // Extract a slice in the desired orientation
        vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
        reslice->SetInputData(connector->GetOutput());
        reslice->SetOutputDimensionality(2);
        reslice->SetResliceAxes(resliceAxes);
        reslice->SetInterpolationModeToLinear();
        reslice->Update();
        
        // Create a greyscale lookup table
        vtkSmartPointer<vtkLookupTable> table = vtkSmartPointer<vtkLookupTable>::New();
        table->SetRange(0, 1000); // image intensity range
        table->SetValueRange(0.0, 1.0); // from black to white
        table->SetSaturationRange(0.0, 0.0); // no color saturation
        table->SetRampToLinear();
        table->Build();
        
        // Map the image through the lookup table
        vtkSmartPointer<vtkImageMapToColors> color = vtkSmartPointer<vtkImageMapToColors>::New();
        color->SetLookupTable(table);
        color->SetInputData(reslice->GetOutput());
        color->Update();
        
        // Display the image
        vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
        actor->SetInputData(color->GetOutput());
        
        vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
        renderer->AddActor(actor);
        
        vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
        window->AddRenderer(renderer);
        
        // Set up the interaction
        vtkSmartPointer<vtkInteractorStyleImage> imageStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
        vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        interactor->SetInteractorStyle(imageStyle);
        window->SetInteractor(interactor);
        window->Render();
        
        vtkSmartPointer<vtkImageInteractionCallback> callback = vtkSmartPointer<vtkImageInteractionCallback>::New();
        callback->SetImageReslice(reslice);
        callback->SetImageColors(color); ///WHY ARE WE HAVING TO SET THIS????
        callback->SetInteractor(interactor);
        
        imageStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);
        imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
        imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback);
        
        // Start interaction
        // The Start() method doesn't return until the window is closed by the user
        interactor->Start();
#endif
    
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
