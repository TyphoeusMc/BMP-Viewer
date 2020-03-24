#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <QFileDialog>
//#include <Windows.h>
#include <ctime>
#include <string>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include "bmpinfo.h"
#include "colorpalette.h"
#include "inputnewname.h"
#include <QMouseEvent>
#include "newcolor.h"
#include "newcolor8bit.h"
#include "interpolation.h"
#include "gaussiansmooth.h"
#include "bilateralfilter.h"
#include "about.h"
#include <algorithm>
#include <cmath>
#include "bmpimagestructure.h"
#include "addimpulsivenoise.h"
#include "histogram.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void openFileByGivenPath(std::string path);
    void enableActions();
    void disableActions();
    void enable8bitOperation();
    void disable8bitOperation();
    void convert8to24(BMPImage &changedImage,BMPImage &displayedImage);
    void doInterpolation(int type,int factorX,int factorY,BMPImage &image);
    void MedianFiltering3x3(BMPImage &image);
    QVector<double> GaussianMatrix(int windowSize, double MSD);
    void imageUpsideDown(BMPImage &image);
    void imageUpsideDown24bit(BMPImage &image);
    void imageGaussianSmooth(BMPImage image, double MSD);
    void addImpulsiveNoise(BMPImage &image,int type,int portion);
    void histogramEqualization(BMPImage &image);
    void startBilateralFilter(BMPImage &image,int windowSize,double sigma_d,double sigma_r);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);


public slots:
    //void openPath(QString path);
    void openFileClick();
    void openBMPInfo();
    void openColorPalette();
    void getNewFileName();
    void saveAsNewBMPImage(QString name);
    void showAboutInfo();
    void showPixelBar();
    void changeColor(int R, int G, int B);
    void changeColor8bit(int index);
    void interpolationClick();
    void getInterFactor(int type,int factorX, int factorY);
    void doMedianFiltering();
    void doHistogramEqualization();
    void doGaussianSmooth(double MSD);
    void doBilateralFilter(int windowSize,double sigma_d,double sigma_r);
    void gaussianSmoothClick();
    void retrieveOperations();
    void addNoiseSmoothClick();
    void doAddNoise(int type,int portion);
    void showHistoGram();
    void bilateralFilterClick();

private:
    Ui::MainWindow *ui;
    //openFile *pathInput;
    //需初始化为空指针，以防程序中delete野指针
    BMPinfo *info=NULL;
    colorPalette *colorPaletteInfo=NULL;
    inputNewName *newName=NULL;
    newColor *newColorIn=NULL;
    newColor8bit *newColorIn8bit=NULL;
    interpolation *interpol=NULL;
    GaussianSmooth *gaussianSmooth=NULL;
    addimpulsivenoise *addNoise=NULL;
    bilateralFilter *BF=NULL;
    histogram *histo=NULL;
    about *aboutPage=NULL;
    QImage *originImage=NULL;
    QPixmap imageShow;
};

#endif // MAINWINDOW_H
