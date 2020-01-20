/******************************************************************************
 * Project    : Embedded Vision Design
 * Copyright  : 2019 HAN Electrical and Electronic Engineering
 * Author     : Hugo Arends
 *
 * Description: Implementation file for basic image processing operators
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

#include "operators_basic.h"
#include "math.h"

#ifdef STM32F746xx
#include "mem_manager.h"
#endif

int compare( const void* a, const void* b)
{
    int int_a = * ( (int*) a );
    int int_b = * ( (int*) b );

    if ( int_a == int_b ) return 0;
    else if ( int_a < int_b ) return -1;
    else return 1;
}

// ----------------------------------------------------------------------------
// Function implementations
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
image_t *newBasicImage(const uint32_t cols, const uint32_t rows)
{
    image_t *img = (image_t *)malloc(sizeof(image_t));
    if(img == NULL)
    {
        // Unable to allocate memory for new image
        return NULL;
    }

#ifdef STM32F746xx
    img->data = mem_manager_alloc();
#else
    img->data = (uint8_t *)malloc((rows * cols) * sizeof(basic_pixel_t));
#endif

    if(img->data == NULL)
    {
        // Unable to allocate memory for data
        free(img);
        return NULL;
    }

    img->cols = cols;
    img->rows = rows;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_BASIC;
    return(img);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
image_t *toBasicImage(image_t *src)
{
    image_t *dst = newBasicImage(src->cols, src->rows);
    if(dst == NULL)
        return NULL;

    convertToBasicImage(src, dst);

    return dst;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void deleteBasicImage(image_t *img)
{
#ifdef STM32F746xx
    mem_manager_free(img->data);
#else
    free(img->data);
#endif

    free(img);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void convertToBasicImage( const image_t *src, image_t *dst )
{
    register long int i = src->rows * src->cols;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    dst->view = src->view;
    dst->cols = src->cols;
    dst->rows = src->rows;
    dst->view = src->view;

    switch(src->type)
    {
    case IMGTYPE_BASIC:
    {
        copy_basic(src, dst);

    }break;
    case IMGTYPE_INT16:
    {
        int16_pixel_t *s = (int16_pixel_t *)src->data;
        // Loop all pixels and copy
        while(i-- > 0)
            *d++ = (basic_pixel_t)(*s++);

    }break;
    case IMGTYPE_FLOAT:
    {
        float_pixel_t *s = (float_pixel_t *)src->data;
        // Loop all pixels and copy
        while(i-- > 0)
            *d++ = (basic_pixel_t)(*s++);

    }break;
    case IMGTYPE_RGB888:
    {
        rgb888_pixel_t *s = (rgb888_pixel_t *)src->data;
        // Loop all pixels, convert and copy
        while(i-- > 0)
        {
            unsigned char r = s->r;
            unsigned char g = s->g;
            unsigned char b = s->b;

            *d++ = (basic_pixel_t)(0.212671f * r + 0.715160f * g + 0.072169f * b);
            s++;
        }
        
    }break;
    case IMGTYPE_RGB565:
    {
        rgb565_pixel_t *s = (rgb565_pixel_t *)src->data;
        // Loop all pixels, convert and copy
        while(i-- > 0)
        {
            unsigned char r = *s >> 11;
            unsigned char g = (*s >>  5) & (rgb565_pixel_t)0x003F;
            unsigned char b = (*s)       & (rgb565_pixel_t)0x001F;

            *d++ = (basic_pixel_t)(0.212671f * r + 0.715160f * g + 0.072169f * b);
            s++;
        }

    }break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------
// Benchmark: 0.061s
// ----------------------------------------------------------------------------
void contrastStretch_basic( const image_t *src
                            ,       image_t *dst
                            , const basic_pixel_t bottom
                            , const basic_pixel_t top)
{
    register long int totalPixels = src->rows * src->cols;
    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;
    uint8_t lpixel = 255;
    uint8_t hpixel = 0;

    // Loops through the pixels to check for the lowest and higest values
    while(totalPixels-- > 0)
    {
        if(*s < lpixel)
        {
            lpixel = *s;
        }
        if(*s > hpixel)
        {
            hpixel = *s;
        }
        s++;
    }

    // If the highest value and lowest value are the same, just copy the src into the dst
    if(hpixel == lpixel){
        copy_basic(src,dst);
    }

    // Else use the formula
    else{
        totalPixels = src->rows * src->cols;
        s = (basic_pixel_t *)src->data;
        while(totalPixels-- > 0)
        {
            *s = ((*s - lpixel) * (((float_t)top-(float_t)bottom)/((float_t)hpixel-(float_t)lpixel)) + 0.5 + bottom);
            *d++ = *s++;
        }
    }

}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void contrastStretchFast_basic(const image_t *src, image_t *dst)
{
    register uint32_t hpixel = 0;
    register uint32_t lpixel = 255;
    uint32_t LUT[256];

    register float_t SF;
    register uint32_t i = (src->rows * src->cols);

    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    // Loops through the pixels to check for the lowest and higest values
    while(i-- > 0)
    {
        if(*s > hpixel) {hpixel = *s;}
        if( *s < lpixel) {lpixel = *s;}
        s++;
    }

    // Calculate spreading factor
    SF = (255.0f) / (hpixel - lpixel);

    // Create lookup table
    i = lpixel;
    while (i <= hpixel)
    {
        LUT[i] = (basic_pixel_t)(((i - lpixel)* SF) + 0.5f);
        i++;
    }

    // Set pixels
    i = src->rows * src->cols;
    s = src->data;
    while(i-- > 0)
    {
        *d = LUT[*s];
        d++;
        s++;

    }
    return;

}

// ----------------------------------------------------------------------------
// Rotation
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Benchmark: 0.0056s
// ----------------------------------------------------------------------------
void rotate180_basic(const image_t *img)
{

#ifdef STM32F746xx
    register uint32_t *first_ptr = (uint32_t *)img->data;
    register uint32_t *last_ptr = (uint32_t *)(img->data + (img->rows * img->cols * sizeof(basic_pixel_t)));
    register uint32_t src_pixels, dst_pixels;
    while(first_ptr != last_ptr)
    {
        last_ptr--;
        src_pixels = *first_ptr;
        dst_pixels = *last_ptr;

        __asm__ ("REV %[result], %[value]" : [result] "=r" (src_pixels) : [value] "r" (src_pixels));
        __asm__ ("REV %[result], %[value]" : [result] "=r" (dst_pixels) : [value] "r" (dst_pixels));

        *first_ptr = dst_pixels;
        *last_ptr = src_pixels;
        first_ptr++;
    }
#else
    register long int totalPixels = img->rows * img->cols;

    // create temporary image
    image_t *temp = newBasicImage(img->cols, img->rows);
    copy_basic(img, temp);

    register basic_pixel_t *s = (basic_pixel_t *)img->data;
    register basic_pixel_t *d = (basic_pixel_t *)temp->data;

    // set the destination pixel to the last pixel
    d = d + (totalPixels - 1);

    // loop through the pixels
    while(totalPixels-- > 0)
    {
        *(d--) = *(s++);
    }

    d = (basic_pixel_t *)temp->data;
    s = (basic_pixel_t *)img->data;
    totalPixels = img->rows * img->cols;

    // Loop all pixels and copy
    while(totalPixels-- > 0)
        *s++ = *d++;

    deleteImage(temp);
#endif


}

// ----------------------------------------------------------------------------
// Thresholding
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Benchmark: 0.026s
// ----------------------------------------------------------------------------
void threshold_basic( const image_t *src
                      ,       image_t *dst
                      , const basic_pixel_t low
                      , const basic_pixel_t high)
{
    register long int totalPixels = src->rows * src->cols;

    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    // loop through the pixels
    while(totalPixels-- > 0)
    {
        if(*s >= low && *s <= high)
        {
            *d = 1;
        }
        else {
            *d = 0;
        }
        s++;
        d++;
    }
    dst->cols = src->cols;
    dst->rows = src->rows;
    dst->view = IMGVIEW_BINARY;
}

// ----------------------------------------------------------------------------
// Benchmark: 0.032s
// ----------------------------------------------------------------------------
void threshold2Means_basic( const image_t *src
                            ,       image_t *dst
                            , const eBrightness brightness)
{
    register uint32_t meanMean, meanMeanPrev, meanLeft, meanRight, amountLeft, amountRight = 0;
    uint32_t hist[256];
    basic_pixel_t lPixel, hPixel = 0;

    histogram_basic(src,hist);
    register int i = 0;

    //find the lowest pixel with a value above 0
    while(i <= 255)
    {
        if(hist[i] != 0)
        {
            lPixel = i;
            break;
        }
        i++;
    }

    //find the highest pixel with a value above 0
    i = 255;
    while(i >= 0)
    {
        if(hist[i] != 0)
        {
            hPixel = i;
            break;
        }
        i--;
    }

    // Mean the lowest and highest pixel to get the starting pixel for finding the threshold
    meanMean = (((float_t)lPixel + (float_t)hPixel) / 2);


    while((basic_pixel_t)meanMean != (basic_pixel_t)meanMeanPrev)
    {
        // Reset values
        i = 0;
        meanLeft = 0;
        meanRight = 0;
        amountLeft = 0;
        amountRight = 0;
        meanMeanPrev = meanMean;

        // Loop through all values
        while(i <= 255)
        {
            // Calculate the meanLeft
            if(i <= (basic_pixel_t)meanMean)
            {
                meanLeft += hist[i] * i;
                amountLeft += hist[i];
            }
            // Calculate the meanRight
            else
            {
                meanRight += hist[i] * i;
                amountRight += hist[i];
            }
            i++;
        }
        // Calculate the mean of the mean
        meanMean = (uint32_t)(((meanLeft / amountLeft) + (meanRight / amountRight)) / 2);
    }

    if(brightness == DARK){
        threshold_basic(src,dst,0,(basic_pixel_t)meanMean);
    }
    else
    {
        threshold_basic(src,dst,(basic_pixel_t)meanMean,255);
    }
    dst->cols = src->cols;
    dst->rows = src->rows;
    dst->view = IMGVIEW_BINARY;
}


// ----------------------------------------------------------------------------
// Benchmark: 0.033s
// ----------------------------------------------------------------------------
void thresholdOtsu_basic( const image_t *src
                          ,       image_t *dst
                          , const eBrightness brightness)
{

    uint32_t hist[256];
    uint32_t totalPixels = 0, sumPixels = 0;
    uint32_t objectPixels = 0, objectSum = 0;
    uint32_t backPixels = 0, backSum = 0;
    float_t objectMean = 0, backMean = 0;
    float_t BCV = 0, BCVmax = 0;
    int T = 0;

    histogram_basic(src,hist);
    register int i = 0;

    for(i = 0; i < 255; i++)
    {
        totalPixels += hist[i];
        sumPixels += hist[i] * i;
    }

    for(i = 0; i < 255; i++)
    {
        objectPixels += hist[i];
        objectSum += hist[i] * i;
        objectMean = (float_t)objectSum / objectPixels;

        backPixels = totalPixels - objectPixels;
        backSum = sumPixels - objectSum;
        backMean = (float_t)backSum / backPixels;

        BCV = ((float_t)backPixels * (float_t)objectPixels) * pow(((double)backMean - (double)objectMean),2);
        if(BCV > BCVmax)
        {
            BCVmax = BCV;
            T = i;
        }
    }
    if(brightness == DARK){
        threshold_basic(src,dst,0,(basic_pixel_t)T);
    }
    else
    {
        threshold_basic(src,dst,(basic_pixel_t)T,255);
    }

    dst->cols = src->cols;
    dst->rows = src->rows;
    dst->view = IMGVIEW_BINARY;
}

// ----------------------------------------------------------------------------
// Miscellaneous
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void erase_basic(const image_t *img)
{
    register long int totalPixels = img->rows * img->cols;
    register basic_pixel_t *s = (basic_pixel_t *)img->data;

    // Loop all pixels and set to 0
    while(totalPixels-- > 0)
        *s++ = 0;
}

// ----------------------------------------------------------------------------
// Benchmark: 0.133s
// ----------------------------------------------------------------------------
void copy_basic(const image_t *src, image_t *dst)
{
    register long int i = src->rows * src->cols;
    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    dst->rows = src->rows;
    dst->cols = src->cols;
    dst->type = src->type;
    dst->view = src->view;

    // Loop all pixels and copy
    while(i-- > 0)
        *d++ = *s++;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void setSelectedToValue_basic(const image_t *src,
                              image_t *dst,
                              const basic_pixel_t selected,
                              const basic_pixel_t value)
{
    register long int totalPixels = src->rows * src->cols;
    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    // Loop all pixels and copy or set to value
    while(totalPixels-- > 0)
    {
        if(*s == selected){
            *d = value;
        }
        else {
            *d = *s;
        }
        s++;
        d++;
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
uint32_t neighbourCount_basic(const image_t *img,
                              const int32_t c,
                              const int32_t r,
                              const basic_pixel_t pixel,
                              const eConnected connected)
{
    register int32_t i,j;
    uint8_t neighbors = 0;
    // Go through rows of window
    for(i = -1; i <= 1; i++)
    {
        // Go through column of window
        for(j = -1; j <= 1; j++)
        {
            // Check if its not the pixel itself
            if(!(i == 0 && j == 0))
            {
                // Check if neighbor pixel is inside the image
                if((c - i >= 0) && (r - j >= 0) && (c + i < img->cols) && (r - j < img->rows))
                {
                    // Check if neighbor pixel is equal to asked value
                    if(getBasicPixel(img,c-i,r-j) == pixel)
                    {
                        //Check if it is connected
                        if( ( (i == 0) != (j == 0) ) || connected == EIGHT)
                        {
                            neighbors++;
                        }
                    }
                }

            }
        }
    }
    return neighbors;
}

uint32_t getLowestneighbour_basic(const image_t *img,
                                  const int32_t c,
                                  const int32_t r,
                                  const eConnected connected)
{
    register int32_t i,j;
    uint8_t lowestNeighbor = 255;
    // Go through rows of window
    for(i = -1; i <= 1; i++)
    {
        // Go through column of window
        for(j = -1; j <= 1; j++)
        {
            // Check if its not the pixel itself
            if(!(i == 0 && j == 0))
            {
                // Check if neighbor pixel is inside the image
                if((c - i >= 0) && (r - j >= 0) && (c + i < img->cols) && (r - j < img->rows))
                {
                    // Check if neighbor pixel is lower than the lowest neighbor
                    if(getBasicPixel(img,c-i,r-j) < lowestNeighbor && getBasicPixel(img,c-i,r-j) != 0)
                    {
                        //Check if it is connected
                        if( ( (i == 0) != (j == 0) ) || connected == EIGHT)
                        {
                            lowestNeighbor = getBasicPixel(img,c-i,r-j);
                        }
                    }
                }

            }
        }
    }
    return lowestNeighbor;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void histogram_basic( const image_t *img, uint32_t *hist )
{
    int32_t i;
    basic_pixel_t *s = (basic_pixel_t *)img->data;
    i = 255;
    while(i >= 0)
    {
        hist[i--] = 0;
    }
    i = img->cols * img->rows;
    while(i-- > 0)
    {
        hist[*s++]++;
    }
}

// ----------------------------------------------------------------------------
// Arithmetic
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void add_basic( const image_t *src, image_t *dst )
{
    register long int totalPixels = src->rows * src->cols;

    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    while(totalPixels-- > 0)
    {
        if(*s + *d > 255)
        {
            *d = 255;
        }
        else
        {
            *d += *s;
        }
        s++;
        d++;
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void subtract_basic( const image_t *src, image_t *dst )
{
    register long int totalPixels = src->rows * src->cols;

    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    while(totalPixels-- > 0)
    {
        if(*d - *s < 0)
        {
            *d = 0;
        }
        else
        {
            *d -= *s;
        }
        s++;
        d++;
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
uint32_t sum_basic( const image_t *img )
{
    register long int totalPixels = img->rows * img->cols;
    register uint32_t sum = 0;
    register basic_pixel_t *s = (basic_pixel_t *)img->data;

    while(totalPixels-- > 0)
    {
        sum += *s;
        s++;
    }
    return sum;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void multiply_basic( const image_t *src, image_t *dst )
{

    register long int totalPixels = src->rows * src->cols;

    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    while(totalPixels-- > 0)
    {
        if(*s * *d > 255)
        {
            *d = 255;
        }
        else
        {
            *d = *d * *s;
        }
        s++;
        d++;
    }

}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void invert_basic( const image_t *src, image_t *dst )
{

    register long int totalPixels = src->rows * src->cols;

    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;

    while(totalPixels-- > 0)
    {
        if(*s == 1)
        {
            *d = 0;
        }
        else
        {
            *d = 1;
        }
        s++;
        d++;
    }
}


// ----------------------------------------------------------------------------
// Filters
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void nonlinearFilter_basic( const image_t *src
                            ,       image_t *dst
                            , const eFilterOperation fo
                            , const uint8_t n)
{
    int16_t i= 0, j = 0, c, r;
    switch(fo)
    {
    // Average will just blur everything
    case AVERAGE:
        // Loop all pixels in the image (rows)
        for(r = 0; r < dst->rows; r++)
        {
            // Loop all pixels in the image (cols)
            for(c = 0; c < dst->cols; c++)
            {
                // Reset variables
                uint32_t sum = 0;
                uint32_t pixelCount = 0;

                // Apply kernel
                // Loop all pixels in the kernel (rows)
                for(i=- (n/2); i <= n/2; i++)
                {
                    // Loop all pixels in the kernel (cols)
                    for(j =- (n/2); j <= n/2; j++)
                    {
                        // Ignore pixels not in the image
                        if((r+i) >= 0 && (c+j) >= 0 && (r+i) < src->rows && (c+j) < src->cols)
                        {
                            ++pixelCount;
                            sum += getBasicPixel(src, (c+j), (r+i));
                        }
                    }
                }
                // Set destination pixel to the average of the kernel
                setBasicPixel(dst, c, r, ((float)sum/(float)pixelCount)+0.5);
            }
        }

        break;

    //Same as average except high value outliers don't count as much as low values
    case HARMONIC:
        // Loop all pixels in the image (rows)
        for(r = 0; r < dst->rows; r++)
        {
            // Loop all pixels in the image (cols)
            for(c = 0; c < dst->cols; c++)
            {
                // Reset variables
                float_t sum = 0;
                uint32_t pixelCount = 0;
                uint32_t hasZero = 0;

                // Apply kernel
                // Loop all pixels in the kernel (rows)
                for(i=- (n/2); i <= n/2; i++)
                {
                    // Loop all pixels in the kernel (cols)
                    for(j =- (n/2); j <=n/2; j++)
                    {
                        // Ignore pixels not in the image
                        if((r+i) >= 0 && (c+j) >= 0 && (r+i) < src->rows && (c+j) < src->cols)
                        {
                            // If a pixel has a value of 0 break the kernel loop because dividing by 0 is not possible
                            if(getBasicPixel(src, (c+j), (r+i)) == 0)
                            {
                                hasZero = 1;
                                break;
                            }
                            ++pixelCount;
                            sum += (float_t)1 / (getBasicPixel(src, (c+j), (r+i)));
                        }
                    }
                }
                // Set pixel to 0 if there was a 0 in the kernel window
                if(hasZero == 1)
                {
                    setBasicPixel(dst, c, r, 0);
                }
                // Set destination pixel to the harmonic of the kernel
                else
                {
                    setBasicPixel(dst, c, r, ((float)pixelCount/(float)sum)+0.5f);
                }
            }
        }
        break;
    //uses the maximum value so the image will strengthen high value outliers
    case MAX:
        // Loop all pixels in the image (rows)
        for(r = 0; r < dst->rows; r++)
        {
            // Loop all pixels in the image (cols)
            for(c = 0; c < dst->cols; c++)
            {
                // Reset variables
                basic_pixel_t max = 0;

                // Apply kernel
                // Loop all pixels in the kernel (rows)
                for(i=- (n/2); i <= n/2; i++)
                {
                    // Loop all pixels in the kernel (cols)
                    for(j =- (n/2); j <=n/2; j++)
                    {
                        // Ignore pixels not in the image
                        if((r+i) >= 0 && (c+j) >= 0 && (r+i) < src->rows && (c+j) < src->cols)
                        {
                            if(max < getBasicPixel(src, (c+j), (r+i)))
                            {
                                max = getBasicPixel(src, (c+j), (r+i));
                            }
                        }
                    }
                }
                // Set destination pixel to the max of the kernel
                setBasicPixel(dst, c, r, (max)+0.5);
            }
        }
        break;
    //sorts the pixels in the filter and then picks the middle value, good for filtering out salt and pepper noise
    case MEDIAN:
        // Loop all pixels in the image (rows)
        for(r = 0; r < dst->rows; r++)
        {
            // Loop all pixels in the image (cols)
            for(c = 0; c < dst->cols; c++)
            {
                // Reset variables
                uint8_t kernelPixels = 0;
                basic_pixel_t medianArr[n*n];

                // Apply kernel
                // Loop all pixels in the kernel (rows)
                for(i=- (n/2); i <= n/2; i++)
                {
                    // Loop all pixels in the kernel (cols)
                    for(j =- (n/2); j <=n/2; j++)
                    {
                        // Ignore pixels not in the image
                        if((r+i) >= 0 && (c+j) >= 0 && (r+i) < src->rows && (c+j) < src->cols)
                        {
                            medianArr[kernelPixels] = getBasicPixel(src, (c+j), (r+i));
                            kernelPixels++;
                        }
                    }
                }
                // Calculate median
                int q, p;

                // Loop so one less bit in the array gets checked after every full swap loop
                for (q = 0; q < kernelPixels; q++)
                {
                    // Loop to check if something needs swapped
                    for (p = 0; p < kernelPixels-1-q; p++)
                    {
                        if (medianArr[p] > medianArr[p+1])
                        {
                            // Swap position
                            basic_pixel_t temp = medianArr[p];
                            medianArr[p] = medianArr[p+1];
                            medianArr[p+1] = temp;
                        }
                    }
                }
                // Set destination pixel to the median of the kernel
                if(kernelPixels % 2 == 0)
                {
                    kernelPixels++;
                }
                setBasicPixel(dst, c, r, (medianArr[(int32_t)(kernelPixels/2)]));
            }
        }
        break;
    // Mix of min and max
    case MIDPOINT:
        // Loop all pixels in the image (rows)
        for(r = 0; r < dst->rows; r++)
        {
            // Loop all pixels in the image (cols)
            for(c = 0; c < dst->cols; c++)
            {
                // Reset variables
                basic_pixel_t min = 255;
                basic_pixel_t max = 0;

                // Apply kernel
                // Loop all pixels in the kernel (rows)
                for(i=- (n/2); i <= n/2; i++)
                {
                    // Loop all pixels in the kernel (cols)
                    for(j =- (n/2); j <=n/2; j++)
                    {
                        // Ignore pixels not in the image
                        if((r+i) >= 0 && (c+j) >= 0 && (r+i) < src->rows && (c+j) < src->cols)
                        {
                            if(min > getBasicPixel(src, (c+j), (r+i)))
                            {
                                min = getBasicPixel(src, (c+j), (r+i));
                            }
                            if(max < getBasicPixel(src, (c+j), (r+i)))
                            {
                                max = getBasicPixel(src, (c+j), (r+i));
                            }
                        }
                    }
                }
                // Set destination pixel to the midpoint of the kernel
                setBasicPixel(dst, c, r, ((min + max) / 2));
            }
        }
        break;
    //uses the minimum value so the image will strengthen low value outliers
    case MIN:
        // Loop all pixels in the image (rows)
        for(r = 0; r < dst->rows; r++)
        {
            // Loop all pixels in the image (cols)
            for(c = 0; c < dst->cols; c++)
            {
                // Reset variables
                basic_pixel_t min = 255;

                // Apply kernel
                // Loop all pixels in the kernel (rows)
                for(i=- (n/2); i <= n/2; i++)
                {
                    // Loop all pixels in the kernel (cols)
                    for(j =- (n/2); j <=n/2; j++)
                    {
                        // Ignore pixels not in the image
                        if((r+i) >= 0 && (c+j) >= 0 && (r+i) < src->rows && (c+j) < src->cols)
                        {
                            if(min > getBasicPixel(src, (c+j), (r+i)))
                            {
                                min = getBasicPixel(src, (c+j), (r+i));
                            }
                        }
                    }
                }
                // Set destination pixel to the min of the kernel
                setBasicPixel(dst, c, r, (min)+0.5);
            }
        }
        break;

    // Uses the range between min and max, noise will be amplified
    case RANGE:
        // Loop all pixels in the image (rows)
        for(r = 0; r < dst->rows; r++)
        {
            // Loop all pixels in the image (cols)
            for(c = 0; c < dst->cols; c++)
            {
                // Reset variables
                basic_pixel_t min = 255;
                basic_pixel_t max = 0;

                // Apply kernel
                // Loop all pixels in the kernel (rows)
                for(i=- (n/2); i <= n/2; i++)
                {
                    // Loop all pixels in the kernel (cols)
                    for(j =- (n/2); j <=n/2; j++)
                    {
                        // Ignore pixels not in the image
                        if((r+i) >= 0 && (c+j) >= 0 && (r+i) < src->rows && (c+j) < src->cols)
                        {
                            if(min > getBasicPixel(src, (c+j), (r+i)))
                            {
                                min = getBasicPixel(src, (c+j), (r+i));
                            }
                            if(max < getBasicPixel(src, (c+j), (r+i)))
                            {
                                max = getBasicPixel(src, (c+j), (r+i));
                            }
                        }
                    }
                }
                // Set destination pixel to the range of the kernel
                setBasicPixel(dst, c, r, (max - min)+0.5);
            }
        }
        break;
    }
}

// ----------------------------------------------------------------------------
// Binary
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void removeBorderBlobs_basic( const image_t *src
                              ,       image_t *dst
                              , const eConnected connected)
{
    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;
    int32_t i,j;
    uint8_t changed = 1;
    // Loop all pixels in the image (rows)
    for(i = 0; i < src->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < src->cols; j++)
        {
            // Set pixel to 2 if it is connected to the border of the image and it is a 1
            if((i == 0 || j == 0 || i == src->rows - 1 || j == src->cols - 1) && *s == 1)
            {
                *d = 2;
            }
            // Else copy source
            else
            {
                *d = *s;
            }
            s++;
            d++;
        }
    }

    // Continue as long as a pixel has changed
    while(changed == 1)
    {
        changed = 0;
        // Loop all pixels in the image (rows)
        for(i = 0; i < dst->rows; i++)
        {
            // Loop all pixels in the image (cols)
            for(j = 0; j < dst->cols; j++)
            {
                if(getBasicPixel(dst,j,i) == 1)
                {
                    if(neighbourCount_basic(dst,j,i,2,connected) > 0)
                    {
                        changed = 1;
                        setBasicPixel(dst,j,i,2);
                    }
                }
            }
        }
        for(i = dst->rows - 1; i > 0; i--)
        {
            // Loop all pixels in the image (cols)
            for(j = dst->cols; j > 0; j--)
            {
                if(getBasicPixel(dst,j,i) == 1){
                    if(neighbourCount_basic(dst,j,i,2,connected) > 0)
                    {
                        changed = 1;
                        setBasicPixel(dst,j,i,2);
                    }
                }
            }
        }
    }
    setSelectedToValue_basic(dst,dst,2,0);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void fillHoles_basic( const image_t *src
                      ,       image_t *dst
                      , const eConnected connected)
{
    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;
    int32_t i,j;
    uint8_t changed = 1;
    // Loop all pixels in the image (rows)
    for(i = 0; i < src->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < src->cols; j++)
        {
            // Set pixel to 2 if it is connected to the border of the image and it is not a 1
            if((i == 0 || j == 0 || i == src->rows - 1 || j == src->cols - 1) && *s != 1)
            {
                *d = 2;
            }
            // Else copy source
            else
            {
                *d = *s;
            }
            s++;
            d++;
        }
    }
    // Continue as long as a pixel has changed
    while(changed == 1)
    {
        changed = 0;
        // Loop all pixels in the image (rows)
        for(i = 0; i < dst->rows; i++)
        {
            // Loop all pixels in the image (cols)
            for(j = 0; j < dst->cols; j++)
            {
                if(getBasicPixel(dst,j,i) == 0)
                {
                    if(neighbourCount_basic(dst,j,i,2,connected) > 0)
                    {
                        changed = 1;
                        setBasicPixel(dst,j,i,2);
                    }
                }
            }
        }
        for(i = dst->rows - 1; i > 0; i--)
        {
            // Loop all pixels in the image (cols)
            for(j = dst->cols; j > 0; j--)
            {
                if(getBasicPixel(dst,j,i) == 0){
                    if(neighbourCount_basic(dst,j,i,2,connected) > 0)
                    {
                        changed = 1;
                        setBasicPixel(dst,j,i,2);
                    }
                }
            }
        }
    }
    // Fill leftover pixels
    for(i = 0; i < dst->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < dst->cols; j++)
        {
            // Set pixel to 1 if it is not border or 1 already
            if(getBasicPixel(dst,j,i) == 0)
            {
                setBasicPixel(dst,j,i,1);
            }
        }
    }
    setSelectedToValue_basic(dst,dst,2,0);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
uint32_t labelBlobs_basic( const image_t *src
                           ,       image_t *dst
                           , const eConnected connected)
{
    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;
    register uint32_t totalPixels = src->cols * src->rows;

    register int32_t i,j;
    uint8_t changed = 1;
    int32_t blobCount = 1;

    while(totalPixels-- > 0)
    {
        // Set all object pixels to 255
        if(*s == 1)
        {
            *d = 255;
        }
        s++;
        d++;
    }

    while(changed == 1)
    {
        changed = 0;

        //LT -> RB
        // Loop all pixels in the image (rows)
        for(i = 0; i < dst->rows; i++)
        {
            // Loop all pixels in the image (cols)
            for(j = 0; j < dst->cols; j++)
            {
                if(getBasicPixel(dst,j,i) != 0 && getLowestneighbour_basic(dst,j,i,connected) < 255 && getBasicPixel(dst,j,i) != getLowestneighbour_basic(dst,j,i,connected))
                {
                    setBasicPixel(dst,j,i,getLowestneighbour_basic(dst,j,i,connected));
                    changed = 1;
                }
                else if(getBasicPixel(dst,j,i) == 255)
                {
                    setBasicPixel(dst,j,i,blobCount);
                    blobCount++;
                    changed = 1;
                }
            }
        }

        // RB -> LT
        // Loop all pixels in the image (rows)
        for(i = dst->rows - 1; i > 0; i--)
        {
            // Loop all pixels in the image (cols)
            for(j = dst->cols; j > 0; j--)
            {
                if(getBasicPixel(dst,j,i) != 0 && getLowestneighbour_basic(dst,j,i,connected) < 255 && getBasicPixel(dst,j,i) != getLowestneighbour_basic(dst,j,i,connected))
                {
                    setBasicPixel(dst,j,i,getLowestneighbour_basic(dst,j,i,connected));
                    changed = 1;
                }
                else if(getBasicPixel(dst,j,i) == 255)
                {
                    setBasicPixel(dst,j,i,blobCount);
                    blobCount++;
                    changed = 1;
                }
            }
        }
    }

    blobCount = 1;
    //LT -> RB
    // Loop all pixels in the image (rows)
    for(i = 0; i < dst->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < dst->cols; j++)
        {
            if(getBasicPixel(dst,j,i) != 0 && getBasicPixel(dst,j,i) >= blobCount)
            {
                setSelectedToValue_basic(dst,dst,getBasicPixel(dst,j,i),blobCount);
                blobCount++;
            }
        }
    }

    dst->cols = src->cols;
    dst->rows = src->rows;
    dst->view = IMGVIEW_LABELED;
    return blobCount - 1;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void binaryEdgeDetect_basic( const image_t *src
                             ,       image_t *dst
                             , const eConnected connected)
{
    register basic_pixel_t *s = (basic_pixel_t *)src->data;
    register basic_pixel_t *d = (basic_pixel_t *)dst->data;
    int32_t i,j;

    // Loop all pixels in the image (rows)
    for(i = 0; i < src->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < src->cols; j++)
        {
            if(connected == FOUR)
            {
                // Set pixel to 2 if it is connected to the border of the image and it is not a 1
                if((*s == 1) && (neighbourCount_basic(src,j,i,0,EIGHT) == 0))
                {
                    *d = 2;
                }
                // Else copy source
                else
                {
                    *d = *s;
                }
            }
            else
            {
                // Set pixel to 2 if it is connected to the border of the image and it is not a 1
                if((*s == 1) && (neighbourCount_basic(src,j,i,0,FOUR) == 0))
                {
                    *d = 2;
                }
                // Else copy source
                else
                {
                    *d = *s;
                }
            }

            s++;
            d++;
        }
    }
    setSelectedToValue_basic(dst,dst,2,0);
    dst->cols = src->cols;
    dst->rows = src->rows;
    dst->view = IMGVIEW_BINARY;

}

// ----------------------------------------------------------------------------
// Analysis
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void blobAnalyse_basic( const image_t *img
                        , const uint8_t blobnr
                        ,       blobinfo_t *blobInfo)
{
    int32_t i,j;
    register basic_pixel_t *s = (basic_pixel_t *)img->data;
    int32_t maxRow = 0;
    int32_t minRow = img->rows;
    int32_t maxColumn = 0;
    int32_t minColumn = img->cols;

    blobInfo->height = 0;
    blobInfo->width = 0;
    blobInfo->nof_pixels = 0;
    blobInfo->perimeter = 0;

    // Loop all pixels in the image (rows)
    for(i = 0; i < img->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < img->cols; j++)
        {
            if(*s == blobnr)
            {
                blobInfo->nof_pixels++;
                if(maxRow < i)
                {
                    maxRow = i;
                }
                if(minRow > i)
                {
                    minRow = i;
                }
                if(maxColumn < j)
                {
                    maxColumn = j;
                }
                if(minColumn > j)
                {
                    minColumn = j;
                }
                if(neighbourCount_basic(img,j,i,0,FOUR) == 1)
                {
                    blobInfo->perimeter += 1.0f;
                }
                else if(neighbourCount_basic(img,j,i,0,FOUR) == 2)
                {
                    blobInfo->perimeter += sqrt(2.0f);
                }
                else if(neighbourCount_basic(img,j,i,0,FOUR) == 3)
                {
                    blobInfo->perimeter += sqrt(5.0f);
                }
            }
            s++;
        }
    }
    blobInfo->height = maxRow - minRow + 1;
    blobInfo->width = maxColumn - minColumn + 1;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void centroid_basic( const image_t *img
                     , const uint8_t blobnr
                     ,       int32_t *cc
                     ,       int32_t *rc)
{
    float m00 = 0;
    int32_t m10 = 0;
    int32_t m01 = 0;

    // Calculate moments
    register int32_t i,j;
    // Loop all pixels in the image (rows)
    for(i = 0; i < img->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < img->cols; j++)
        {
            if(getBasicPixel(img,j,i) == blobnr)
            {
                m00++;
                m01+= i;
                m10+= j;
            }
        }
    }

    // Calculate centroid
    *cc = (int32_t)((float_t)m10/(float_t)m00);
    *rc = (int32_t)((float_t)m01/(float_t)m00);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
float normalizedCentralMoments_basic( const image_t *img
                                      , const uint8_t blobnr
                                      , const int32_t p
                                      , const int32_t q)
{
    if((p == 0 && q == 1) || (p == 1 && q == 0))
    {
        return 0.0f;
    }
    if(p == 0 && q == 0)
    {
        return 1.0f;
    }

    // Calculate moments
    int32_t m00 = 0;
    int32_t m10 = 0;
    int32_t m01 = 0;
    register int32_t i,j;
    // Loop all pixels in the image (rows)
    for(i = 0; i < img->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < img->cols; j++)
        {
            if(getBasicPixel(img,j,i) == blobnr)
            {
                m00++;
                m01+= i;
                m10+= j;
            }
        }
    }

    // Calculate central moments
    float_t cc = ((float_t)m10/(float_t)m00);
    float_t rc = ((float_t)m01/(float_t)m00);;

    // Calculate normalized central moments
    float_t upq = 0.0f;
    float_t u00 = 0.0f;
    // Loop all pixels in the image (rows)
    for(i = 0; i < img->rows; i++)
    {
        // Loop all pixels in the image (cols)
        for(j = 0; j < img->cols; j++)
        {
            if(getBasicPixel(img,j,i) == blobnr)
            {
                u00++;
                upq += pow((double_t)(j-cc), p) * pow((double_t)(i-rc), q);
            }
        }
    }
    float_t y = (((float_t)p + (float_t)q)/2.0f)+1;
    return ((upq)/(pow((double_t)u00,y)));
}

// ----------------------------------------------------------------------------
// EOF
// ----------------------------------------------------------------------------
