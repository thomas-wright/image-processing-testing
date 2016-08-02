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

template< typename TImage >
void BoxCarSmoothFilter< TImage >::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
    os << "I am a box-car filter using a 3x3x3 voxel kernel, ignoring the outer edge of voxels" << std::endl;
}

template< typename TImage >
void BoxCarSmoothFilter< TImage >::GenerateData()
{
    itk::TimeProbe clock;
    
    clock.Start();
    
    typename TImage::ConstPointer input = this->GetInput();
    typename TImage::Pointer output = this->GetOutput();
    
    this->AllocateOutputs();

    // Set up neighbourhood iterator for iterating over the input
    typedef itk::ConstNeighborhoodIterator< TImage > NeighborhoodIteratorType;
    typename NeighborhoodIteratorType::RadiusType radius;
    radius.Fill(1);
    NeighborhoodIteratorType inputIt(radius, input, input->GetRequestedRegion());
    
    // Normal iterator for iterating over the output
    typedef itk::ImageRegionIterator< TImage > IteratorType;
    IteratorType outputIt( output, output->GetRequestedRegion());
    
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
//    const typename TImage::RegionType & outputRegion = output->GetRequestedRegion();
//    typename TImage::IndexType start = outputRegion.GetIndex();
//    typename TImage::SizeType size = outputRegion.GetSize();
//    for ( int k = start[2] + 1; k < start[2] + size[2] - 1; ++k )
//    {
//        for ( int j = start[1] + 1; j < start[1] + size[1] - 1; ++j )
//        {
//            for ( int i = start[0] + 1; i < start[0] + size[0] - 1; ++i )
//            {
//                // Loop over the kernel on our input image
//                float accumulator = 0;
//                for ( int kk = k - 1; kk <= k + 1; ++kk )
//                {
//                    for ( int jj = j - 1; jj <= j + 1; ++jj )
//                    {
//                        for ( int ii = i - 1; ii <= i + 1; ++ii )
//                        {
//                            typename TImage::IndexType inPos;
//                            inPos[0] = ii;
//                            inPos[1] = jj;
//                            inPos[2] = kk;
//                            const float inputValue = input->GetPixel( inPos );
//                            accumulator += inputValue;
//                        }
//                    }
//                }
//                
//                // Output voxel index
//                typename TImage::IndexType outPos;
//                outPos[0] = i;
//                outPos[1] = j;
//                outPos[2] = k;
//                
//                // Average the accumulator and store
//                const typename TImage::PixelType filteredValue = static_cast< typename TImage::PixelType >( accumulator /= 27.0f );
//                output->SetPixel( outPos, filteredValue );
//            }
//        }
//    }
    
    clock.Stop();
    std::cout << "Total: " << clock.GetTotal() << std::endl;
}

#endif /* BoxCarSmoothFilter_h */
