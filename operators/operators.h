/******************************************************************************
 * Project    : Embedded Vision Design
 * Copyright  : 2019 HAN Electrical and Electronic Engineering
 * Author     : Hugo Arends
 *
 * Description: Header file for image processing operators
 *
 * Copyright (C) 2019 HAN University of Applied Sciences. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 ******************************************************************************
  Change History:

    Version 1.0 - November 2017
    > Initial revision

    Version 2.0 - September 2019
    > Updated for EVDK3.0

******************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

#ifndef _OPERATORS_H_
#define _OPERATORS_H_

#include "stdint.h"
#include "stdlib.h"
#include "limits.h"
#include "math.h"

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------
#ifdef DBG_PRINTF_ENABLE
#include <stdio.h>
#define dbg_printf(x, ...) {printf((x), ##__VA_ARGS__); fflush(stdout);}
#else
#define dbg_printf(x, ...) //x
#endif

// ----------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------
typedef enum
{
    IMGTYPE_BASIC  = 0,  // Unsigned Character
    IMGTYPE_INT16  = 1,  // Integer
    IMGTYPE_FLOAT  = 2,  // Float
    IMGTYPE_RGB888 = 3,  // RGB 8-bit per pixel
    IMGTYPE_RGB565 = 4,
  
    IMGTYPE_MAX    = 2147483647 // Max 32-bit int value,
                                // forces enum to be 4 bytes
}eImageType;

typedef enum
{
    IMGVIEW_STRETCH = 0,
    IMGVIEW_CLIP    = 1,
    IMGVIEW_BINARY  = 2,
    IMGVIEW_LABELED = 3,
  
    IMGVIEW_MAX     = 2147483647 // Max 32-bit int value,
                                 // forces enum to be 4 bytes
}eImageView;

// Pixel types
typedef uint8_t basic_pixel_t;
typedef int16_t int16_pixel_t;
typedef float float_pixel_t;

typedef struct rgb888_pixel_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    
}rgb888_pixel_t;

typedef uint16_t rgb565_pixel_t;

typedef struct complex_pixel_t
{
    float real;
    float imaginary;
    
}complex_pixel_t;

union pixel
{
    basic_pixel_t  basic_pixel;
    int16_pixel_t  int16_pixel;
    float_pixel_t  float_pixel;
    rgb888_pixel_t rgb888_pixel;
    rgb565_pixel_t rgb565_pixel;
};

typedef union pixel pixel_t;

// Image type
typedef struct
{
    int32_t     cols;
    int32_t     rows;
    eImageView  view;
    eImageType  type;
    uint8_t    *data;
    
}image_t;

// Brightness
typedef enum
{
    BRIGHT = 0,
    DARK
    
}eBrightness;

// Connectivity
typedef enum
{
    FOUR  = 4,
    EIGHT = 8
    
}eConnected;

// Filter operations
typedef enum
{
    AVERAGE = 0,
    HARMONIC,
    MAX,
    MEDIAN,
    MIDPOINT,
    MIN,
    RANGE
    
}eFilterOperation;

// BLOB info structure
typedef struct blobinfo_t
{
    uint16_t height;
    uint16_t width;
    uint16_t nof_pixels;
    float    perimeter;
    
}blobinfo_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Getter and setter functions for individual pixels
// ----------------------------------------------------------------------------
extern void setBasicPixel(image_t *img, int32_t c, int32_t r, basic_pixel_t value);
extern basic_pixel_t getBasicPixel(const image_t *img, int32_t c, int32_t r);
extern void setInt16Pixel(image_t *img, int32_t c, int32_t r, int16_pixel_t value);
extern int16_pixel_t getInt16Pixel(image_t *img, int32_t c, int32_t r);
extern void setFloatPixel(image_t *img, int32_t c, int32_t r, float_pixel_t value);
extern float_pixel_t getFloatPixel(image_t *img, int32_t c, int32_t r);
extern void setRGB888Pixel(image_t *img, int32_t c, int32_t r, rgb888_pixel_t value);
extern rgb888_pixel_t getRGB888Pixel(image_t *img, int32_t c, int32_t r);
extern void setRGB565Pixel(image_t *img, int32_t c, int32_t r, rgb565_pixel_t value);
extern rgb565_pixel_t getRGB565Pixel(image_t *img, int32_t c, int32_t r);

// ----------------------------------------------------------------------------
// Memory (de)allocation
// ----------------------------------------------------------------------------

// These functions can be used for creating new images
// Memory is allocated within these functions
//
// Precondition : -
// Postcondition: User must free allocated memory by calling the deleteImage()
//                function when appropriate
image_t *newBasicImage( const uint32_t cols, const uint32_t rows );
image_t *newInt16Image( const uint32_t cols, const uint32_t rows );
image_t *newFloatImage( const uint32_t cols, const uint32_t rows );
image_t *newRGB888Image( const uint32_t cols, const uint32_t rows );
image_t *newRGB565Image( const uint32_t cols, const uint32_t rows );

// These functions can be used for copying images
// Memory is allocated within these functions
//
// Precondition : -
// Postcondition: User must free allocated memory by calling the deleteImage()
//                function when appropriate
image_t *toBasicImage( image_t *src );
image_t *toInt16Image( image_t *src );
image_t *toFloatImage( image_t *src );
image_t *toRGB888Image( image_t *src );
image_t *toRGB565Image( image_t *src );

// Use the function deleteImage() for freeing memory
// This function will automatically call the appropriate function based on the
// image type
//
// Precondition : -
// Postcondition: -
void deleteImage( image_t *img );
void deleteBasicImage( image_t *img );
void deleteInt16Image( image_t *img );
void deleteFloatImage( image_t *img );
void deleteRGB888Image( image_t *img );
void deleteRGB565Image( image_t *img );

// Use the function convertImage() for converting between image types
//
// Precondition : Memory is already allocated for image and image data
//                The type field is already set for both the 'src' and the
//                'dst' image. All other image parameters are copied from the
//                the 'dst' image to the 'src' image.
//
//                Example: // Convert to basic image
//                         dst->type = IMGTYPE_BASIC;
//                         convertImage(src, dst);
// Postcondition: -
void convertImage( const image_t *src, image_t *dst );
void convertToBasicImage( const image_t *src, image_t *dst );
void convertToInt16Image( const image_t *src, image_t *dst );
void convertToFloatImage( const image_t *src, image_t *dst );
void convertToRGB888Image( const image_t *src, image_t *dst );
void convertToRGB565Image( const image_t *src, image_t *dst );

// ----------------------------------------------------------------------------
// Contrast stretching
// ----------------------------------------------------------------------------

// The contrast of the src image is stretch to 0-255 in the dst image
// This function is required by the Qt PC application. It is used to display
// the source- and destination image. If this function is not implemented, the
// user will see random data.
//
// Precondition : img is a single channel image
// Postcondition: dst is a single channel image
void contrastStretch( const image_t *src
                    ,       image_t *dst
                    , const int32_t bottom
                    , const int32_t top
                    );
// This function does the same as contrastStretch. However, it always
// stretches from 0 to 255 and implements a fast algorithm.
//
// Precondition : img is a single channel image
// Postcondition: dst is a single channel image
void contrastStretchFast( const image_t *src, image_t *dst );

// This function stretches the contrast for the R, G, and B channels
// separately
//
// Precondition : img is a RGB888 image
// Postcondition: dst is a RGB888 image
void contrastStretchRGB888( const image_t *src
                          ,       image_t *dst
                          , const rgb888_pixel_t bottom
                          , const rgb888_pixel_t top
                          );

// This function stretches the contrast for the R, G, and B channels
// separately
//
// Precondition : img is a RGB565 image
// Postcondition: dst is a RGB565 image
void contrastStretchRGB565( const image_t *src
                          ,       image_t *dst
                          , const rgb565_pixel_t bottom
                          , const rgb565_pixel_t top
                          );

// ----------------------------------------------------------------------------
// Rotation
// ----------------------------------------------------------------------------

// Rotates an image 180 degrees
//
// Precondition : -
// Postcondition: -
void rotate180( const image_t *img );

// ----------------------------------------------------------------------------
// Thresholding
// ----------------------------------------------------------------------------

// All pixels in src image with value between 'low' and 'high' are
// set to 1 in dst image. Rest is set to 0
// This function is used in all VisionSets. Without it, initially nothing will
// seem to happen.
//
// Precondition : img is a single channel image
// Postcondition: dst is a binary image
void threshold( const image_t *src
              ,       image_t *dst
              , const int32_t low
              , const int32_t high
              );

// Threshold values are automatically generated based on the 2-means method
//
// Precondition : img is a single channel image
// Postcondition: dst is a binary image
void threshold2Means( const image_t *src
                    ,       image_t *dst
                    , const eBrightness brightness
                    );

// Threshold values are automatically generated based on Otsu's method
//
// Precondition : img is a single channel image
// Postcondition: dst is a binary image
void thresholdOtsu( const image_t *src
                  ,       image_t *dst
                  , const eBrightness brightness
                  );

// ----------------------------------------------------------------------------
// Miscellaneous
// ----------------------------------------------------------------------------

// All pixels are set to the value 0
//
// Precondition : -
// Postcondition: -
void erase( const image_t *img );

// Source image is copied to dst image
//
// Precondition : -
// Postcondition: -
void copy( const image_t *src, image_t *dst );

// First, src image is copied to dst image
// Then, all pixels in dst image with value 'selected' are set to 'value'
//
// Precondition : img is a single channel image
// Postcondition: dst is a single channel image
void setSelectedToValue( const image_t *src
                       ,       image_t *dst
                       , const int32_t selected
                       , const int32_t value
                       );

// This function checks the number of pixels around pixel (c,r) and returns the
// number of pixels that are equal to 'pixel'
//
// Precondition : img is a single channel image
// Postcondition: -
uint32_t neighbourCount( const image_t *img
                       , const int32_t c
                       , const int32_t r
                       , const int32_t pixel
                       , const eConnected connected
                       );

// Make a histogram of the source image and calculate the sum of all pixel
// values
//
// Precondition : img is a single channel image
// Postcondition: -
void histogram( const image_t *img, uint32_t *hist );

// ----------------------------------------------------------------------------
// Arithmetic
// ----------------------------------------------------------------------------

// Pixels are added
//
// Precondition : -
// Postcondition: -
void add( const image_t *src, image_t *dst );

// Pixels are subtracted
//
// Precondition : -
// Postcondition: -
void subtract( const image_t *src, image_t *dst );

// the sum of all pixels is returned
//
// Precondition : -
// Postcondition: -
uint32_t sum( const image_t *img );

// Pixels are multiplied
//
// Precondition : -
// Postcondition: -
void multiply( const image_t *src, image_t *dst );

// src image is binary inverted
//
// Precondition : img is a binary image
// Postcondition: dst is a binary image
void invert( const image_t *src, image_t *dst);


// ----------------------------------------------------------------------------
// Filters
// ----------------------------------------------------------------------------

// A set of nonlinear filters
// n equals the mask size
//
// Precondition : img is a single channel image
//                n must be an odd number
// Postcondition: dst is a single channel image
void nonlinearFilter( const image_t *src
                    ,       image_t *dst
                    , const eFilterOperation fo
                    , const uint8_t n
                    );

// ----------------------------------------------------------------------------
// Binary
// ----------------------------------------------------------------------------

// Remove the blobs connected to one or more borders
//
// Precondition : img is a binary image
// Postcondition: dst is a binary image
void removeBorderBlobs( const image_t *src
                      ,       image_t *dst
                      , const eConnected connected
                      );

// Fill holes
//
// Precondition : img is a binary image
// Postcondition: dst is a binary image
void fillHoles( const image_t *src
              ,       image_t *dst
              , const eConnected connected
              );

// Label all blobs, returns the number of labeled blobs
//
// Precondition : img is a binary image
// Postcondition: dst is a labeled image
uint32_t labelBlobs( const image_t *src
                   ,       image_t *dst
                   , const eConnected connected
                   );

// Find the edges of binary objects
//
// Precondition : img is a binary image
// Postcondition: dst is a binary image
void binaryEdgeDetect( const image_t *src
                     ,       image_t *dst
                     , const eConnected connected
                     );

// ----------------------------------------------------------------------------
// Analysis
// ----------------------------------------------------------------------------

// Analyse blobs
//
// Precondition : img is a labeled image
//                pBlobInfo points to a blobinfo_t struct declared by the
//                calling program
// Postcondition: -
void blobAnalyse( const image_t *img
                , const uint8_t blobnr
                ,       blobinfo_t *blobInfo);

// Calculates the centroid of a blob
//
// Precondition : img is a labeled image
// Postcondition: -
void centroid( const image_t *img
             , const uint8_t blobnr
             ,       int32_t *cc
             ,       int32_t *rc
             );

// Calculates the normalized central moments of a blob specified by p and q
// Ref.: Gonzalez, R. (). 11.3.4 Moment Invariants.
//       In Digital Image Processing. pp. 839-842.
//       New Jersey: Pearson Prentice Hall.
//
// Precondition : img is a binary or labeled image
//                blobnr must be '1' if img is binary
// Postcondition: -
float normalizedCentralMoments( const image_t *img
                              , const uint8_t blobnr
                              , const int32_t p
                              , const int32_t q
                              );


// ----------------------------------------------------------------------------
// Sudoku stuff
// ----------------------------------------------------------------------------

// Copies the rectangle in src and puts it in dst
//
// Precondition : src is a grayscale image
//                dst has the same width and height as the rectangle
// Postcondition: -
void copyRect(image_t *src
              , image_t *dst
              , int32_t topLeftRow
              , int32_t topLeftCol
              , int32_t width
              , int32_t height);

// Interpolates an image
//
// Precondition : src is a grayscale image with the to be interpolated pixels having a value of 255
//
// Postcondition: -
void interpolate(image_t *src);

// Calculates and returns the determinant of the given matrix
//
// Precondition : src is a grayscale image
//
// Postcondition: -
void solve4x4LinearEquation(float_t a11, float_t a12, float_t a13, float_t a14,
        float_t a21, float_t a22, float_t a23, float_t a24,
        float_t a31, float_t a32, float_t a33, float_t a34,
        float_t a41, float_t a42, float_t a43, float_t a44,
        float_t k1,  float_t k2,  float_t k3,  float_t k4, float_t answer[4] );

float_t findDet3x3(
            float_t a11, float_t a12, float_t a13,
            float_t a21, float_t a22, float_t a23,
            float_t a31, float_t a32, float_t a33 );

float_t findDet4x4(
        float_t a11, float_t a12, float_t a13, float_t a14,
        float_t a21, float_t a22, float_t a23, float_t a24,
        float_t a31, float_t a32, float_t a33, float_t a34,
        float_t a41, float_t a42, float_t a43, float_t a44 );

void warpBasic(image_t *src
               , image_t *dst
               , uint32_t pixelPointsSrc[3][2]
               , uint32_t pixelPointsDst[3][2]);
#endif // _OPERATORS_H_

#ifdef __cplusplus
 }
#endif

// ----------------------------------------------------------------------------
// EOF
// ----------------------------------------------------------------------------
