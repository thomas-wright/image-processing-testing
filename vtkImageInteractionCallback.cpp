//
//  vtkImageInteractionCallback.cpp
//  ImageSlicing
//
//  Created by Tom on 31/07/2016.
//
//

#include "vtkImageInteractionCallback.hpp"

#include "vtkMatrix4x4.h"
#include "vtkInteractorStyleImage.h"
#include "vtkImageData.h"

vtkImageInteractionCallback *vtkImageInteractionCallback::New()
{
    return new vtkImageInteractionCallback;
};
    
vtkImageInteractionCallback::vtkImageInteractionCallback()
{
    this->Slicing = 0;
    this->ImageReslice = 0;
    this->Interactor = 0;
};

void vtkImageInteractionCallback::SetImageReslice(vtkImageReslice *reslice) {
    this->ImageReslice = reslice; };

vtkImageReslice *vtkImageInteractionCallback::GetImageReslice() {
    return this->ImageReslice; };

void vtkImageInteractionCallback::SetInteractor(vtkRenderWindowInteractor *interactor) {
    this->Interactor = interactor; };

vtkRenderWindowInteractor *vtkImageInteractionCallback::GetInteractor() {
    return this->Interactor; };

void vtkImageInteractionCallback::Execute(vtkObject *, unsigned long event, void *)
{
    vtkRenderWindowInteractor *interactor = this->GetInteractor();
    
    int lastPos[2];
    interactor->GetLastEventPosition(lastPos);
    int currPos[2];
    interactor->GetEventPosition(currPos);
    
    if (event == vtkCommand::LeftButtonPressEvent)
    {
        this->Slicing = 1;
    }
    else if (event == vtkCommand::LeftButtonReleaseEvent)
    {
        this->Slicing = 0;
    }
    else if (event == vtkCommand::MouseMoveEvent)
    {
        if (this->Slicing)
        {
            vtkImageReslice *reslice = this->ImageReslice;
            
            // Increment slice position by deltaY of mouse
            int deltaY = lastPos[1] - currPos[1];
            
            reslice->UpdateInformation();
            double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
            vtkMatrix4x4 *matrix = reslice->GetResliceAxes();
            // move the center point that we are slicing through
            double point[4];
            double center[4];
            point[0] = 0.0;
            point[1] = 0.0;
            point[2] = sliceSpacing * deltaY;
            point[3] = 1.0;
            matrix->MultiplyPoint(point, center);
            matrix->SetElement(0, 3, center[0]);
            matrix->SetElement(1, 3, center[1]);
            matrix->SetElement(2, 3, center[2]);
            interactor->Render();
        }
        else
        {
            vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());
            if (style)
            {
                style->OnMouseMove();
            }
        }
    }
};
