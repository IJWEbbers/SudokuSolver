#ifndef DETECTGRID_H
#define DETECTGRID_H

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;
using LineTestFn = function<bool(Rect&, Mat&)>;
using ExpandRectFn = function<Rect(Rect&, Mat&)>;

class DetectGrid
{
private:
    const int DIGIT_PADDING = 3;
    const int MIN_DIGIT_PIXELS = 20;
    const int CANNY_THRESHOLD = 65;
    const double MIN_GRID_PCT = 0.3;
    const int MIN_PUZZLE_SIZE = 325;
    const int MAX_PUZZLE_SIZE = 900;
public:
    DetectGrid();
    void extractGrid(const Mat& img, Mat& dst, vector<float>& gridPoints, float& scale);
    Size findCorners(const vector<Point> largestContour, Point2f corners[]);
    vector<Rect> FindDigitRects(const Mat& raw, Mat& cleaned, vector<float>& gridPoints, float &scale);
    vector<Rect> findDigits(const Mat& img);
};

#endif // DETECTGRID_H
