/*=========================================================================
 
 Program:   Visualization Toolkit
 Module:    ImageSlicing.cxx
 
 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.
 
 =========================================================================*/
//
// This example shows how to load a 3D image into VTK and then reformat
// that image into a different orientation for viewing.  It uses
// vtkImageReslice for reformatting the image, and uses vtkImageActor
// and vtkInteractorStyleImage to display the image.  This InteractorStyle
// forces the camera to stay perpendicular to the XY plane.
//
// Thanks to David Gobbi of Atamai Inc. for contributing this example.
//

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
#include "vtkDICOMImageReader.h"

#include "vtkImageInteractionCallback.hpp"

// The program entry point
int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " DATADIR/headsq/quarter" << endl;
        return 1;
    }
    
    // Start by loading some data.
    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(argv[1]);
//    reader->SetDataExtent(0, 63, 0, 63, 1, 93);
//    reader->SetDataSpacing(3.2, 3.2, 1.5);
//    reader->SetDataOrigin(0.0, 0.0, 0.0);
//    reader->SetDataScalarTypeToUnsignedShort();
//    reader->SetDataByteOrderToLittleEndian();
//    reader->UpdateWholeExtent();
    
    // Calculate the center of the volume
    reader->UpdateInformation();
    int extent[6];
    double spacing[3];
    double origin[3];
    reader->GetOutput()->GetExtent(extent);
    reader->GetOutput()->GetSpacing(spacing);
    reader->GetOutput()->GetOrigin(origin);
    
    double center[3];
    center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
    center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
    center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
    
    // Matrices for axial, coronal, sagittal, oblique view orientations
    //static double axialElements[16] = {
    //         1, 0, 0, 0,
    //         0, 1, 0, 0,
    //         0, 0, 1, 0,
    //         0, 0, 0, 1 };
    
    //static double coronalElements[16] = {
    //         1, 0, 0, 0,
    //         0, 0, 1, 0,
    //         0,-1, 0, 0,
    //         0, 0, 0, 1 };
    
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
    resliceAxes->DeepCopy(sagittalElements);
    // Set the point through which to slice
    resliceAxes->SetElement(0, 3, center[0]);
    resliceAxes->SetElement(1, 3, center[1]);
    resliceAxes->SetElement(2, 3, center[2]);
    
    // Extract a slice in the desired orientation
    vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputConnection(reader->GetOutputPort());
    reslice->SetOutputDimensionality(2);
    reslice->SetResliceAxes(resliceAxes);
    reslice->SetInterpolationModeToLinear();
    
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
    color->SetInputConnection(reslice->GetOutputPort());
    
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
    callback->SetInteractor(interactor);
    
    imageStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);
    imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
    imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback);
    
    // Start interaction
    // The Start() method doesn't return until the window is closed by the user
    interactor->Start();
    
    return EXIT_SUCCESS;
}
