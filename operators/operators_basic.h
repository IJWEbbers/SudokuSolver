/******************************************************************************
 * Project    : Embedded Vision Design
 * Copyright  : 2019 HAN Electrical and Electronic Engineering
 * Author     : Hugo Arends
 *
 * Description: Header file for basic image processing operators
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
#ifndef _OPERATORS_BASIC_H_
#define _OPERATORS_BASIC_H_

#include "operators.h"

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------

void contrastStretch_basic( const image_t *src
                          ,       image_t *dst
                          , const basic_pixel_t bottom
                          , const basic_pixel_t top
                          );

void contrastStretchFast_basic( const image_t *src
                              ,       image_t *dst
                              );

// ----------------------------------------------------------------------------
// Rotation
// ----------------------------------------------------------------------------

void rotate180_basic(const image_t *img );

// ----------------------------------------------------------------------------
// Thresholding
// ----------------------------------------------------------------------------


void threshold_basic( const image_t *src
                    ,       image_t *dst
                    , const basic_pixel_t low
                    , const basic_pixel_t high
                    );

void threshold2Means_basic( const image_t *src
                          ,       image_t *dst
                          , const eBrightness brightness
                          );

void thresholdOtsu_basic( const image_t *src
                        ,       image_t *dst
                        , const eBrightness brightness
                        );

// ----------------------------------------------------------------------------
// Miscellaneous
// ----------------------------------------------------------------------------

void erase_basic( const image_t *img );

void copy_basic( const image_t *src, image_t *dst );

void setSelectedToValue_basic( const image_t *src
                             ,       image_t *dst
                             , const basic_pixel_t selected
                             , const basic_pixel_t value
                             );

uint32_t neighbourCount_basic( const image_t *img
                             , const int32_t c
                             , const int32_t r
                             , const basic_pixel_t pixel
                             , const eConnected connected
                             );

uint32_t getLowestneighbour_basic(const image_t *img,
                              const int32_t c,
                              const int32_t r,
                              const eConnected connected);

void histogram_basic( const image_t *img, uint32_t *hist );

// ----------------------------------------------------------------------------
// Arithmetic
// ----------------------------------------------------------------------------

void add_basic( const image_t *src, image_t *dst );

void subtract_basic( const image_t *src, image_t *dst );

uint32_t sum_basic( const image_t *img );

void multiply_basic( const image_t *src, image_t *dst );

void invert_basic( const image_t *src, image_t *dst);


// ----------------------------------------------------------------------------
// Filters
// ----------------------------------------------------------------------------

void nonlinearFilter_basic( const image_t *src
                          ,       image_t *dst
                          , const eFilterOperation fo
                          , const uint8_t n
                          );

// ----------------------------------------------------------------------------
// Binary
// ----------------------------------------------------------------------------

void removeBorderBlobs_basic( const image_t *src
                            ,       image_t *dst
                            , const eConnected connected
                            );

void fillHoles_basic( const image_t *src
                    ,       image_t *dst
                    , const eConnected connected
                    );

uint32_t labelBlobs_basic( const image_t *src
                         ,       image_t *dst
                         , const eConnected connected);

void binaryEdgeDetect_basic( const image_t *src
                           ,       image_t *dst
                           , const eConnected connected
                           );

// ----------------------------------------------------------------------------
// Analysis
// ----------------------------------------------------------------------------

void blobAnalyse_basic( const image_t *img
                      , const uint8_t blobnr
                      ,       blobinfo_t *blobInfo);

void centroid_basic( const image_t *img
                   , const uint8_t blobnr
                   ,       int32_t *cc
                   ,       int32_t *rc
                   );

float normalizedCentralMoments_basic( const image_t *img
                                    , const uint8_t blobnr
                                    , const int32_t p
                                    , const int32_t q
                                    );

#endif // _OPERATORS_BASIC_H_
// ----------------------------------------------------------------------------
// EOF
// ----------------------------------------------------------------------------
