#ifndef DETECTGRID_H
#define DETECTGRID_H
#include <stdio.h>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"

#define UNASSIGNED 0 // UNASSIGNED is used for empty cells in sudoku
#define N 9 // N is used for size of Sudoku grid. Size will be NxN

using namespace cv;
using namespace std;
using LineTestFn = function<bool(Rect&, Mat&)>;
using ExpandRectFn = function<Rect(Rect&, Mat&)>;

class DetectGrid
{
private:
    double area;
    double maxArea = 0;
    int maxAreaBlob;
    int bottomRight = 0;
    int bottomLeft = 0;
    int topLeft = 0;
    int topRight = 0;
public:
    Mat findGrid(Mat grayScaleSrc);
    Mat removeGridLines(Mat grid);
    void splitGrid(Mat grayscaleGridSrc, Mat gridArray[9][9]);
};

#endif // DETECTGRID_H
