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

// Forward declarations
namespace itk
{
    class TimeProbe;
}

#define USE_THREADED_IMPLEMENTATION 1

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
    
    /** Superclass typedefs. */
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    
    /** Run-time type information (and related methods). */
    itkTypeMacro(BoxCarSmoothFilter, itk::ImageToImageFilter);
    
    // Optional method for output
    void PrintSelf( std::ostream& os, itk::Indent indent ) const;

#if !USE_THREADED_IMPLEMENTATION
    // Because we need neighbouring pixels to do the processing, we'll create our own implementation
    // of this method.
    virtual void GenerateData() ITK_OVERRIDE;
#else
    virtual void BeforeThreadedGenerateData() ITK_OVERRIDE;
    virtual void AfterThreadedGenerateData() ITK_OVERRIDE;
    virtual void ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId ) ITK_OVERRIDE;
#endif
    
protected:
    
    BoxCarSmoothFilter() {};
    virtual ~BoxCarSmoothFilter() {};
    
private:
    
    BoxCarSmoothFilter(const Self &) ITK_DELETE_FUNCTION;
    void operator=(const Self &) ITK_DELETE_FUNCTION;
    
    itk::TimeProbe* clock;
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "BoxCarSmoothFilter.hxx"
#endif

#endif /* BoxCarSmoothFilter_hpp */
