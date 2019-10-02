#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "detectgrid.h"
#include "numberrecognition.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <iostream>
#include <sstream>

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

void MainWindow::on_pushButton_File_clicked()
{
    Mat src;
    Mat foundGrid;
    Mat splitSudoku[9][9];
    DetectGrid grid;

    src = imread("../SudokuSolver/Images/sudoku2.jpg",IMREAD_GRAYSCALE);
    if(!src.data) {
        ui->statusBar->showMessage(QString("Could not open image!"),0);
    }
    else {
        grid.splitGrid(grid.removeGridLines(grid.findGrid(src)),splitSudoku);
        numberRecognition(splitSudoku[3][0]);
    }
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
