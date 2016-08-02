//
//  BoxCarSmoothFilter.hpp
//  ImageSlicing
//
//  Created by Tom on 01/08/2016.
//
//

#ifndef BoxCarSmoothFilter_hpp
#define BoxCarSmoothFilter_hpp

#include <itkImageToImageFilter.h>

/**
 * Simple box-car filter implementation using a 3x3x3 voxel kernel.
 */
template< typename TImage >
class BoxCarSmoothFilter : public itk::ImageToImageFilter< TImage, TImage >
{
public:
    
    typedef BoxCarSmoothFilter Self;
    typedef itk::ImageToImageFilter<TImage,TImage> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    
    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    
    /** Run-time type information (and related methods). */
    itkTypeMacro(BoxCarSmoothFilter, itk::ImageToImageFilter);
    
    // Optional method for output
    void PrintSelf( std::ostream& os, itk::Indent indent ) const;

    // Because we need neighbouring pixels to do the processing, we'll create our own implementation
    // of this method.
    virtual void GenerateData() ITK_OVERRIDE;

protected:
    
    BoxCarSmoothFilter() {};
    virtual ~BoxCarSmoothFilter() {};
    
private:
    
    BoxCarSmoothFilter(const Self &) ITK_DELETE_FUNCTION;
    void operator=(const Self &) ITK_DELETE_FUNCTION;
    
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "BoxCarSmoothFilter.hxx"
#endif

#endif /* BoxCarSmoothFilter_hpp */
