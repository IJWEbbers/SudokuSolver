#ifndef NUMBERRECOGNITION_H
#define NUMBERRECOGNITION_H

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

const int MIN_CONTOUR_AREA = 100;

class ContourWithData {
public:
    // member variables
    double fltArea;                              // area of contour
    std::vector<cv::Point> ptContour;           // contour
    cv::Rect boundingRect;                      // bounding rect for contour

    bool checkIfContourIsValid() {                              // obviously in a production grade program
        if (fltArea < MIN_CONTOUR_AREA) return false;           // we would have a much more robust function for
        return true;                                            // identifying if a contour is valid !!
    }

    static bool sortByBoundingRectXPosition(const ContourWithData& cwdLeft, const ContourWithData& cwdRight) {      // this function allows us to sort
        return(cwdLeft.boundingRect.x < cwdRight.boundingRect.x);                                                   // the contours from left to right
    }

};

int numberRecognition(Mat matTestingNumbers);
void imgArrayToIntArray(Mat imgArray[9][9], int intArray[9][9]);
#endif // NUMBERRECOGNITION_H
