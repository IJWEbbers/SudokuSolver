#ifndef NUMBERRECOGNITION_H
#define NUMBERRECOGNITION_H

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"

const int MIN_CONTOUR_AREA = 100;

class ContourWithData {
public:
    // member variables
    double fltArea;                             // area of contour
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

void numberRecognition(cv::Mat matTestingNumbers);

#endif // NUMBERRECOGNITION_H
