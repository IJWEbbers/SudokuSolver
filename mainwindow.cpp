#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "numberrecognition.h"
#include "trainingprogram.h"
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
    void step2(Mat src);
    src = imread("C:/HAN/Semester_7 Vision minor/Project Git/SudokuSolver/Images/Numbers3.png",IMREAD_COLOR);
    if(!src.data) {
        ui->statusBar->showMessage(QString("Could not open image!"),0);
    }
    else {

        trainingNumbers();
        //numberRecognition(src);

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

        // there is no frame
        if (!Cam.data) {
            info.append(QString("Snapshot taken but could not be converted to image!"));
            ui->statusBar->showMessage(info,0);
        }
        else {
            for (int i=0;i<100;i++) {

                // Take snapshot
                Img >> Cam;

                numberRecognition(Cam);
                QCoreApplication::processEvents();
            }
        }
    }
}
