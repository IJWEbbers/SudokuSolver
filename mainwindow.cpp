#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "numberrecognition.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <iostream>
#include <sstream>


// global variables


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

using namespace cv;
using namespace std;

Mat src_gray;
int thresh = 10;
RNG rng(12345);

void thresh_callback(int, void* );



void MainWindow::on_pushButton_File_clicked()
{
    Mat src;
    void step2(Mat src);
    src = imread("C:/HAN/Semester_7 Vision minor/Project Git/SudokuSolver/Images/crackynumbers.png",IMREAD_COLOR);
    if(!src.data) {
        ui->statusBar->showMessage(QString("Could not open image!"),0);
    }
    else {

        int height = src.rows, width = src.cols;
        QString info;

        // Get the image data
        info.append(QString("Image info: "));
        info.append(QString("height=%1 ").arg(height));
        info.append(QString("width=%1 ").arg(width));

        ui->statusBar->showMessage(info);

        // Create a window
        namedWindow("Original image", WINDOW_AUTOSIZE);
        moveWindow("Original image", 100, 100);

        // Show the image
        imshow("Original image",src);

        numberRecognition(src);

    }
}

void thresh_callback(int, void* )
{
    Mat canny_output;
    Canny( src_gray, canny_output, thresh, thresh*2 );
    vector<vector<Point> > contours;
    findContours( canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE );
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );
    vector<Point2f>centers( contours.size() );
    // vector<float>radius( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( contours[i], contours_poly[i], 3, true );
        boundRect[i] = boundingRect( contours_poly[i] );
        //minEnclosingCircle( contours_poly[i], centers[i], radius[i] );
    }
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        drawContours( drawing, contours_poly, static_cast<int>(i), color );
        //rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 1 );
        //circle( drawing, centers[i], static_cast<int>(radius[i]), color, 2 );
    }
    imshow( "Contours", drawing );
}

void step2(Mat src)
{
    Mat gray;
    cvtColor(src,gray,COLOR_BGR2GRAY);
    threshold(gray,gray,200,255,THRESH_BINARY_INV);

    vector< vector <Point> > contours;
    vector< Vec4i > hierarchy;

    findContours( gray, contours, hierarchy,RETR_CCOMP, CHAIN_APPROX_SIMPLE );

    for( size_t i = 0; i < contours.size(); i=static_cast<size_t>( hierarchy[i][0]) )
    {
        Rect r= boundingRect(contours[i]);
        rectangle(src,Point(r.x,r.y), Point(r.x+r.width,r.y+r.height), Scalar(0,0,255),1,8,0);
    }

    imwrite("result2.jpg",src);
    imshow("step 2 result",src);
    //waitKey();
}

void MainWindow::on_pushButton_Webcam_clicked()
{
    QString info;
    VideoCapture Img;
    Img.open(0);

    if (!Img.isOpened()) {
        info.append(QString("Could not take a snapshot, probably no camera connected!"));
        ui->statusBar->showMessage(info,0);
    }
    else {

        Mat Cam;
        // Take first snapshot
        Img >> Cam;
        int height = Cam.rows, width = Cam.cols;
        // there is no frame
        if (!Cam.data) {
            info.append(QString("Snapshot taken but could not be converted to image!"));
            ui->statusBar->showMessage(info,0);
        }
        else {
            for (int i=0;i<100;i++) {
                // Create images
                Mat src(width,height,CV_8UC1,1);
                Mat dst(width,height,CV_8UC1,1);
                Mat Edge(width,height,CV_8UC1,1);

                // Take snapshot
                Img >> Cam;
                // Convert to grayscale
                cvtColor(Cam,src,COLOR_BGR2GRAY);
                // Threshold
                threshold(src,dst,130,255,THRESH_BINARY);
                // Edge detection
                Canny(src,Edge,150,255,3);

                // Create Windows
                namedWindow("Snapshot",WINDOW_AUTOSIZE);
                moveWindow("Snapshot",100,5);
                namedWindow("GrayScale",WINDOW_AUTOSIZE);
                moveWindow("GrayScale",650,5);
                namedWindow("Threshold",WINDOW_AUTOSIZE);
                moveWindow("Threshold",100,500);
                namedWindow("Edge",WINDOW_AUTOSIZE);
                moveWindow("Edge",650,500);

                // Show images
                imshow("Snapshot",Cam);
                imshow("GrayScale", src);
                imshow("Threshold",dst);
                imshow("Edge",Edge);
                QCoreApplication::processEvents();
            }
        }
    }

}
