#include "detectgrid.h"

using namespace cv;
using namespace std;

//Function to find the Sudoku grid and separate it from the full image
Mat DetectGrid::findGrid(Mat grayScaleSrc)
{
    //clone src
    Mat srcb = grayScaleSrc.clone();

    //smooth the image
    Mat smooth;
    Mat thresholded;
    GaussianBlur(srcb, smooth, Size(11, 11), 0, 0); //removing noises
    adaptiveThreshold(smooth, thresholded, 255, ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY_INV, 15, 5);

    //find contours
    vector<vector<Point>>contours;
    findContours(thresholded, contours,RETR_TREE,CHAIN_APPROX_SIMPLE);

    //find the biggest contour
    for (int i = 0; i < static_cast<int>(contours.size()); i++)
    {
        area = contourArea(contours[static_cast<unsigned long long>(i)], false);
        if (area > 50 )
        {
            if (area > maxArea)
            {
                maxArea = area;
                maxAreaBlob = i;
            }
        }
    }
    //get only the corners of the contour
    double perimeter = arcLength(contours[static_cast<unsigned long long>(maxAreaBlob)], true);
    approxPolyDP(contours[static_cast<unsigned long long>(maxAreaBlob)], contours[static_cast<unsigned long long>(maxAreaBlob)], 0.1*perimeter, true);

    //decide in which corner the point belongs
    double sum = 0; //minimum sum = bottom left, maximum sum = top right
    double diff; //maximum diff (x - y) = bottom right, minimum diff = top left

    double maxDiff = 0;
    double minDiff = 0;
    double maxSum = 0;
    double minSum = contours[static_cast<unsigned long long>(maxAreaBlob)][0].x + contours[static_cast<unsigned long long>(maxAreaBlob)][0].y;

    for (int i = 0; i < 4; i++)
    {
        sum = contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(i)].x + contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(i)].y;
        diff = contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(i)].x - contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(i)].y;

        if (diff >= maxDiff)
        {
            maxDiff = diff;
            bottomRight = i;
        }
        if (diff <= minDiff)
        {
            minDiff = diff;
            bottomLeft = i;
        }
        if (sum >= maxSum)
        {
            maxSum = sum;
            topLeft = i;
        }
        if (sum <= minSum)
        {
            minSum = sum;
            topRight = i;
        }
    }

    //create the warp arrays
    Point2f warpIn[4];
    Point2f warpOut[4];
    warpIn[0] = contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(topLeft)];
    warpIn[1] = contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(topRight)];
    warpIn[2] = contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(bottomRight)];
    warpIn[3] = contours[static_cast<unsigned long long>(maxAreaBlob)][static_cast<unsigned long long>(bottomLeft)];
    warpOut[0] = Point2f(450, 450);
    warpOut[1] = Point2f(0, 0);
    warpOut[2] = Point2f(450, 0);
    warpOut[3] = Point2f(0, 450);

    //warp the image
    Mat wrap; Mat mat;
    mat = Mat::zeros(grayScaleSrc.size(), grayScaleSrc.type());
    wrap = getPerspectiveTransform(warpIn, warpOut);
    warpPerspective(srcb, mat, wrap, Size(450, 450));
    return mat;
}

Mat DetectGrid::removeGridLines(Mat grid)
{
    //apply adaptiveThreshold at the bitwise_not of gray, notice the ~ symbol
    Mat bw;
    adaptiveThreshold(~grid, bw, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

    //create the images that will use to extract the horizontal and vertical lines
    Mat horizontal = bw.clone();
    Mat vertical = bw.clone();

    //specify size on axis
    int horizontalsize = horizontal.cols / 10;
    int verticalsize = vertical.rows / 10;

    //apply morphology operations
    erode(horizontal, horizontal, getStructuringElement(MORPH_RECT, Size(horizontalsize,1)), Point(-1, -1));
    dilate(horizontal, horizontal, getStructuringElement(MORPH_RECT, Size(horizontalsize,1)), Point(-1, -1));
    erode(vertical, vertical, getStructuringElement(MORPH_RECT, Size( 1,verticalsize)), Point(-1, -1));
    dilate(vertical, vertical, getStructuringElement(MORPH_RECT, Size( 1,verticalsize)), Point(-1, -1));

    bw = bw - (horizontal + vertical);
    morphologyEx(bw,bw,MORPH_OPEN,getStructuringElement(MORPH_RECT, Size( 3,3)));

    return bw;
}



//Function to assign every box in the grid to a position in an array
void DetectGrid::splitGrid(Mat grayscaleGridSrc, Mat gridArray[9][9])
{
    //find the grid and remove the lines
    Mat grid = removeGridLines(findGrid(grayscaleGridSrc));

    cvtColor(grid,grid,COLOR_GRAY2BGR);
    Mat smallimage;

    //split the full grid into smaller images each with the size of 50x50 pixels
    for (int m=0; m < 450; m += 50)
    {
        for (int n = 0; n < 450; n += 50)
        {
            smallimage = Mat(grid, Rect(n, m, 50, 50));
            gridArray[n/50][m/50] = smallimage;
        }
    }
}
