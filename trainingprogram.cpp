#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "trainingprogram.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <dirent.h>
#include <vector>

//============================= global variables ===================================
const int MIN_CONTOUR_AREA = 100;

const int RESIZED_IMAGE_WIDTH = 20;
const int RESIZED_IMAGE_HEIGHT = 30;

//==================================================================================
using namespace cv;
using namespace std;
void trainingNumbers() {

    int num = 797;
    int size = 16 * 16;
    Mat trainData = Mat(Size(size, num), CV_32FC1);
    Mat responces = Mat(Size(1, num), CV_32FC1);
    int counter = 0;
    for(int i=0;i<=9;i++)
    {
        // reading the images from the folder of tarining samples
        DIR *dir;
        struct dirent *ent;
        char pathToImages[]="./digits3"; // name of the folder containing images
        char path[255];
        sprintf(path, "%s/%d", pathToImages, i);
        if ((dir = opendir(path)) != NULL)
        {
            while ((ent = readdir (dir)) != NULL)
            {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 )
                {
                    char text[255];
                    sprintf(text,"/%s",ent->d_name);
                    string digit(text);
                    digit=path+digit;
                    Mat mat=imread(digit,1); //loading the image
                    cvtColor(mat,mat,COLOR_BGR2GRAY);  //converting into grayscale
                    threshold(mat , mat , 200, 255 ,THRESH_OTSU); // preprocessing
                    mat.convertTo(mat,CV_32FC1,1.0/255.0); //necessary to convert images to CV_32FC1 for using K nearest neighbour algorithm.
                    resize(mat, mat, Size(16,16 ),0,0,INTER_NEAREST); // same size as our testing samples
                    mat.reshape(1,1);
                    for (int k=0; k<size;k++)
                    {
                        trainData.at<float>(counter*size+k) = mat.at<float>(k); // storing the pixels of the image
                    }
                    responces.at<float>(counter) = i; // stroing the responce corresponding to image
                    counter++;
                }
            }
        }
        // ========================== save classifications to file =============================

        cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::WRITE);           // open the classifications file

        if (fsClassifications.isOpened() == false) {                                                        // if the file was not opened successfully
            std::cout << "error, unable to open training classifications file, exiting program\n\n";        // show error message
            return;                                                                                      // and exit program
        }

        fsClassifications << "classifications" << responces;        // write classifications into classifications section of classifications file
        fsClassifications.release();
        //============================ save training images to file ========================

        cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::WRITE);         // open the training images file

        if (fsTrainingImages.isOpened() == false) {                                                 // if the file was not opened successfully
            std::cout << "error, unable to open training images file, exiting program\n\n";         // show error message
            return;                                                                              // and exit program
        }

        fsTrainingImages << "images" << trainData;         // write training images into images section of images file
        fsTrainingImages.release();
        closedir(dir);
    }

   // KNearest knearest(trainData,responces  );
   // knearest.train(trainData,responces);

    /* cv::Mat imgTrainingNumbers;         // input image
    cv::Mat imgGrayscale;               //
    cv::Mat imgBlurred;                 // declare various images
    cv::Mat imgThresh;                  //
    cv::Mat imgThreshCopy;              //

    std::vector<std::vector<cv::Point> > ptContours;        // declare contours vector
    std::vector<cv::Vec4i> v4iHierarchy;                    // declare contours hierarchy

    cv::Mat matClassificationInts;      // these are our training classifications, note we will have to perform some conversions before writing to file later

    // these are our training images, due to the data types that the KNN object KNearest requires, we have to declare a single Mat,
    // then append to it as though it's a vector, also we will have to perform some conversions before writing to file later
    cv::Mat matTrainingImagesAsFlattenedFloats;

    // possible chars we are interested in are digits 0 through 9 and capital letters A through Z, put these in vector intValidChars
    std::vector<int> intValidChars = {'0','1', '2', '3', '4', '5', '6', '7', '8', '9'};

    imgTrainingNumbers = cv::imread("C:/HAN/Semester_7 Vision minor/Project Git/SudokuSolver/Images/digits.png");          // read in training numbers image

    if (imgTrainingNumbers.empty()) {                               // if unable to open image
        std::cout << "error: image not read from file\n\n";         // show error message on command line
        return;                                                     // and exit program
    }

    cv::cvtColor(imgTrainingNumbers, imgGrayscale, cv::COLOR_BGR2GRAY);        // convert to grayscale

    cv::GaussianBlur(imgGrayscale,              // input image
                     imgBlurred,                             // output image
                     cv::Size(5, 5),                         // smoothing window width and height in pixels
                     0);                                     // sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

    // filter image from grayscale to black and white
    cv::adaptiveThreshold(~imgBlurred,                            // input image
                          imgThresh,                              // output image
                          255,                                    // make pixels that pass the threshold full white
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C,         // use gaussian rather than mean, seems to give better results
                          cv::THRESH_BINARY_INV,                  // invert so foreground will be white, background will be black
                          11,                                     // size of a pixel neighborhood used to calculate threshold value
                          2);                                     // constant subtracted from the mean or weighted mean

    cv::imshow("imgThresh", imgThresh);                     // show threshold image for reference

    imgThreshCopy = imgThresh.clone();                      // make a copy of the thresh image, this in necessary b/c findContours modifies the image

    cv::findContours(imgThreshCopy,                         // input image, make sure to use a copy since the function will modify this image in the course of finding contours
                     ptContours,                            // output contours
                     v4iHierarchy,                          // output hierarchy
                     cv::RETR_EXTERNAL,                     // retrieve the outermost contours only
                     cv::CHAIN_APPROX_SIMPLE);              // compress horizontal, vertical, and diagonal segments and leave only their end points

    cv::Mat drawing;
    for (size_t i = 0; i < ptContours.size(); i++) {                            // for each contour
        if (cv::contourArea(ptContours[i]) > MIN_CONTOUR_AREA) {                // if contour is big enough to consider
            cv::Rect boundingRect = cv::boundingRect(ptContours[i]);            // get the bounding rect

            cv::rectangle(imgTrainingNumbers, boundingRect, cv::Scalar(0, 0, 255), 2);      // draw red rectangle around each contour as we ask user for input

            cv::Mat matROI = imgThresh(boundingRect);                                       // get ROI image of bounding rect

            cv::Mat matROIResized;
            cv::resize(matROI, matROIResized, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));     // resize image, this will be more consistent for recognition and storage

            cv::imshow("matROI", matROI);                               // show ROI image for reference
            cv::imshow("matROIResized", matROIResized);                 // show resized ROI image for reference
            cv::imshow("imgTrainingNumbers", imgTrainingNumbers);       // show training numbers image, this will now have red rectangles drawn on it

            int intChar = cv::waitKey(0);           // get key press

            if (intChar == 27) {                    // if esc key was pressed
                return;                             // exit program
            }
            else if (std::find(intValidChars.begin(), intValidChars.end(), intChar) != intValidChars.end()) {     // else if the char is in the list of chars we are looking for . . .

                matClassificationInts.push_back(intChar);       // append classification char to integer list of chars

                cv::Mat matImageFloat;                          // now add the training image (some conversion is necessary first) . . .
                matROIResized.convertTo(matImageFloat, CV_32FC1);       // convert Mat to float

                cv::Mat matImageFlattenedFloat = matImageFloat.reshape(1, 1);       // flatten

                matTrainingImagesAsFlattenedFloats.push_back(matImageFlattenedFloat);       // add to Mat as though it was a vector, this is necessary due to the
                // data types that KNearest.train accepts
            }   // end if
        }   // end if
    }   // end for

    std::cout << "training complete\n\n";

    // ========================== save classifications to file =============================

    cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::WRITE);           // open the classifications file

    if (fsClassifications.isOpened() == false) {                                                        // if the file was not opened successfully
        std::cout << "error, unable to open training classifications file, exiting program\n\n";        // show error message
        return;                                                                                      // and exit program
    }

    fsClassifications << "classifications" << matClassificationInts;        // write classifications into classifications section of classifications file
    fsClassifications.release();                                            // close the classifications file

    //============================ save training images to file ========================

    cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::WRITE);         // open the training images file

    if (fsTrainingImages.isOpened() == false) {                                                 // if the file was not opened successfully
        std::cout << "error, unable to open training images file, exiting program\n\n";         // show error message
        return;                                                                              // and exit program
    }

    fsTrainingImages << "images" << matTrainingImagesAsFlattenedFloats;         // write training images into images section of images file
    fsTrainingImages.release(); */                                                // close the training images file

    return;
}
