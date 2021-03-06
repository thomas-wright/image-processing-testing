//
//  BoxCarSmoothFilter.hxx
//  ImageSlicing
//
//  Created by Tom on 01/08/2016.
//
//

#ifndef BoxCarSmoothFilter_hxx
#define BoxCarSmoothFilter_hxx

#include "BoxCarSmoothFilter.h"

#include "itkNeighborhoodIterator.h"
#include "itkImageIterator.h"
#include "itkTimeProbe.h"
#include "itkConstantBoundaryCondition.h"
#include "itkNeighborhoodAlgorithm.h"

template< typename TImage >
void BoxCarSmoothFilter< TImage >::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
    os << "I am a box-car filter using a 3x3x3 voxel kernel, ignoring the outer edge of voxels" << std::endl;
}

#if !USE_THREADED_IMPLEMENTATION
template< typename TImage >
void BoxCarSmoothFilter< TImage >::GenerateData()
{
    itk::TimeProbe clock;
    
    clock.Start();
    
    typename TImage::ConstPointer input = this->GetInput();
    typename TImage::Pointer output = this->GetOutput();
    
    this->AllocateOutputs();

#define USE_NEIGHBOURHOOD_ITERATOR 1
#if USE_NEIGHBOURHOOD_ITERATOR
    
    // 3x3x3 kernel, so radius is 1 around our central voxel
    const int radiusSize = 1;
    typedef itk::ConstNeighborhoodIterator< TImage > NeighborhoodIteratorType;
    typename NeighborhoodIteratorType::RadiusType radius;
    radius.Fill(radiusSize);

    // To properly handle the boundaries, we need to separate the faces (i.e., boundaries) of the volume from
    // the middle. Then we can process the middle without worrying about bounds checking.
    typedef itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< TImage > FaceCalculatorType;
    FaceCalculatorType faceCalculator;
    typename FaceCalculatorType::FaceListType faceList;
    faceList = faceCalculator(input, output->GetRequestedRegion(), radius);
    typename FaceCalculatorType::FaceListType::iterator fit;
    
    // Normal iterator for iterating over the output
    typedef itk::ImageRegionIterator< TImage > IteratorType;
    
    // Now loop! First over the list of regions we got by splitting our volume into the different faces
    for ( fit=faceList.begin(); fit != faceList.end(); ++fit)
    {
        // Now for each of those face regions, do the same processing
        NeighborhoodIteratorType inputIt(radius, input, *fit);
        IteratorType outputIt( output, *fit);
        for (inputIt.GoToBegin(), outputIt.GoToBegin(); ! inputIt.IsAtEnd(); ++inputIt, ++outputIt)
        {
            float accumulator = 0;
            for ( int kk = 0; kk < inputIt.Size(); ++kk )
            {
                accumulator += inputIt.GetPixel( kk );
            }
            const typename TImage::PixelType filteredValue = static_cast< typename TImage::PixelType >( accumulator /= inputIt.Size() );
            outputIt.Set( filteredValue );
        }
    }
    
#else
    const typename TImage::RegionType & outputRegion = output->GetRequestedRegion();
    typename TImage::IndexType start = outputRegion.GetIndex();
    typename TImage::SizeType size = outputRegion.GetSize();
    for ( int k = start[2] + 1; k < start[2] + size[2] - 1; ++k )
    {
        for ( int j = start[1] + 1; j < start[1] + size[1] - 1; ++j )
        {
            for ( int i = start[0] + 1; i < start[0] + size[0] - 1; ++i )
            {
                // Loop over the kernel on our input image
                float accumulator = 0;
                for ( int kk = k - 1; kk <= k + 1; ++kk )
                {
                    for ( int jj = j - 1; jj <= j + 1; ++jj )
                    {
                        for ( int ii = i - 1; ii <= i + 1; ++ii )
                        {
                            typename TImage::IndexType inPos;
                            inPos[0] = ii;
                            inPos[1] = jj;
                            inPos[2] = kk;
                            const float inputValue = input->GetPixel( inPos );
                            accumulator += inputValue;
                        }
                    }
                }
                
                // Output voxel index
                typename TImage::IndexType outPos;
                outPos[0] = i;
                outPos[1] = j;
                outPos[2] = k;
                
                // Average the accumulator and store
                const typename TImage::PixelType filteredValue = static_cast< typename TImage::PixelType >( accumulator /= 27.0f );
                output->SetPixel( outPos, filteredValue );
            }
        }
    }
#endif
    
    clock.Stop();
    std::cout << "Total time for box car filtering: " << clock.GetTotal() << std::endl;
}

#else

template< typename TImage >
void BoxCarSmoothFilter< TImage >::BeforeThreadedGenerateData()
{
    this->clock = new itk::TimeProbe();
    this->clock->Start();
}

template< typename TImage >
void BoxCarSmoothFilter< TImage >::AfterThreadedGenerateData()
{
    this->clock->Stop();
    std::cout << "Total time for box car filtering (threaded): " << this->clock->GetTotal() << std::endl;
    delete this->clock;
    this->clock = nullptr;
}

template< typename TImage >
void BoxCarSmoothFilter< TImage >::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId )
{
    typename TImage::ConstPointer input = this->GetInput();
    typename TImage::Pointer output = this->GetOutput();
    
    this->AllocateOutputs();
    
    //// TGW massive assumption that input region and output region are the same
    
    // Set up neighbourhood iterator for iterating over the input
    typedef itk::ConstNeighborhoodIterator< TImage > NeighborhoodIteratorType;
    typename NeighborhoodIteratorType::RadiusType radius;
    radius.Fill(1);
    NeighborhoodIteratorType inputIt(radius, input, outputRegionForThread);
  
    // If we know that we don't need to check any boundary conditions, then this gives a bit of extra performance
    // (but not as much as I expected (10-15%?).
    inputIt.NeedToUseBoundaryConditionOff();
    
    // Normal iterator for iterating over the output
    typedef itk::ImageRegionIterator< TImage > IteratorType;
    IteratorType outputIt( output, outputRegionForThread);

    // Now loop!
    for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt )
    {
        float accumulator = 0;
        for ( int kk = 0; kk < inputIt.Size(); ++kk )
        {
            accumulator += inputIt.GetPixel( kk );
        }
        const typename TImage::PixelType filteredValue = static_cast< typename TImage::PixelType >( accumulator /= inputIt.Size() );
        outputIt.Set( filteredValue );
    }
}

#endif

#endif /* BoxCarSmoothFilter_h */
