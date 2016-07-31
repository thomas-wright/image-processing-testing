//
//  vtkImageInteractionCallback.hpp
//  ImageSlicing
//
//  Created by Tom on 31/07/2016.
//
//

#ifndef vtkImageInteractionCallback_hpp
#define vtkImageInteractionCallback_hpp

#include "vtkCommand.h"
#include "vtkImageReslice.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkImageMapToColors.h"

// The mouse motion callback, to turn "Slicing" on and off
class vtkImageInteractionCallback : public vtkCommand
{
public:
    
    static vtkImageInteractionCallback *New();
    
    vtkImageInteractionCallback();
    
    void SetImageReslice(vtkImageReslice *reslice) ;
    
    vtkImageReslice *GetImageReslice();
    
    void SetImageColors(vtkImageMapToColors *color) ;
    
    vtkImageMapToColors *GetImageColors();

    void SetInteractor(vtkRenderWindowInteractor *interactor);
    
    vtkRenderWindowInteractor *GetInteractor() ;
    
    virtual void Execute(vtkObject *, unsigned long event, void *);
    
private:
    
    // Actions (slicing only, for now)
    int Slicing;
    
    // Pointer to vtkImageReslice
    vtkImageReslice *ImageReslice;
    
    vtkImageMapToColors *Colors;

    // Pointer to the interactor
    vtkRenderWindowInteractor *Interactor;
};

#endif /* vtkImageInteractionCallback_hpp */
