#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
FILE *BMPin;
FILE *BMPout;
#define PI 3.1415926

/************************************************************
 * 保存BMP文件信息的结构体有三个全局变量实例
 * 其分别作用为：
 * 1.originalImage:
 * 用于存放打开的BMP文件的原始数据，在程序运行过程中
 * 数据不会作出任何改变
 * 2.changedImage:
 * 原始BMP数据的一个副本，所有对图像的修改都将对此
 * 实例的数据进行，24bit图像中无用
 * 3.displayedImage:
 * 实际显示到屏幕上的图像的数据，8bit图的情况下数据通过changedImage
 * 中的数据相应处理得来，24bit图像的情况下中对图像的操作直接作用于
 * displayedImage
 * *********************************************************
 * Note:
 * 1.设置originalImage的原因：
 * 所有的更改可以一键撤销，该操作通过保留原始图像数据完成
 * 2.设置displayedImage的原因：
 * 对于24bit图像而言，BMP存储格式中像素从左下至右上，RGB值排列为BGR，
 * Qt中通过位图数据输出图像需要构造QImage对象，而QImage对象的扫描顺序为
 * 坐上至右下，并以RGB顺序扫描位图数据，因此需要将数据重排。
 * 对于8bit数据而言，同样有像素顺序的问题(重排在changedImage中完成)，并且，
 * 由于8bit图的位图数据全是调色板索引值，显示时需要转换，
 * 因此对于8bit图而言，displayedImage存放的是根据索引值与调色板的值转换成的
 * 24bit图像
 * *********************************************************/
BMPImage originalImage;
BMPImage changedImage;
BMPImage displayedImage;

bool imageOpened=false;
string filePath;
string fileName;
bool is24bit=false;
bool pickColor=false;
bool changeColor=false;
unsigned long totalSizeinByte=0;
QPoint mousePos;
QPoint mousePos1;
uchar R;
uchar G;
uchar B;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("TyphoeusBrowser BMP");
    ui->widget->hide();
    disableActions();
    //选取颜色时的色块实际上是TableWidget，大小为1*1，此处进行初始化
    //使用tableItem显示色块是无奈之举，因为对label或widget等控件的palette设置无效
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setColumnCount(1);
    ui->tableWidget->setColumnWidth(0,20);
    ui->tableWidget->setRowHeight(0,20);
    ui->tableWidget->setItem(0,0,new QTableWidgetItem(" "));
    //连接所有信号
    connect(ui->actionOpen_BMP_file,SIGNAL(triggered(bool)),this,SLOT(openFileClick()));
    connect(ui->actionDisplayBMPInfo,SIGNAL(triggered(bool)),this,SLOT(openBMPInfo()));
    connect(ui->actionDisplay_color_palatte,SIGNAL(triggered(bool)),this,SLOT(openColorPalette()));
    connect(ui->actionSave_to_new_BMP_file,SIGNAL(triggered(bool)),this,SLOT(getNewFileName()));
    connect(ui->actionAbout,SIGNAL(triggered(bool)),this,SLOT(showAboutInfo()));
    connect(ui->actionPixelPickBar,SIGNAL(triggered(bool)),this,SLOT(showPixelBar()));
    connect(ui->actionImage_interpolation_2,SIGNAL(triggered(bool)),this,SLOT(interpolationClick()));
    connect(ui->actionMedian_filtering,SIGNAL(triggered(bool)),this,SLOT(doMedianFiltering()));
    connect(ui->actionGaussian_smoothing,SIGNAL(triggered(bool)),this,SLOT(gaussianSmoothClick()));
    connect(ui->actionAbout,SIGNAL(triggered(bool)),this,SLOT(showAboutInfo()));
    connect(ui->actionBack_to_Original_Picture,SIGNAL(triggered(bool)),this,SLOT(retrieveOperations()));
    connect(ui->actionAdd_Impulsive_Noise,SIGNAL(triggered(bool)),this,SLOT(addNoiseSmoothClick()));
    connect(ui->actionShowHistogram,SIGNAL(triggered(bool)),this,SLOT(showHistoGram()));
    connect(ui->actionHistogram_Equalization,SIGNAL(triggered(bool)),this,SLOT(doHistogramEqualization()));
    connect(ui->actionBilateralFilter,SIGNAL(triggered(bool)),this,SLOT(bilateralFilterClick()));
}

MainWindow::~MainWindow()
{
    //释放三个结构体内的空间
    free(originalImage.imageBuffer);
    free(originalImage.colorTable);
    free(displayedImage.imageBuffer);
    free(displayedImage.colorTable);
    free(changedImage.imageBuffer);
    free(changedImage.colorTable);
    delete ui;
}

/*******************************
 * description:
 * this function will open a file browser
 * that allow user to pick a BMP file.
 * noted that when call open file function,
 * the file path conveyed should be transfered
 * to stdstring fisrt
 * ****************************/
void MainWindow::openFileClick()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle(tr("打开图片"));
    fileDialog->setDirectory(".");
    fileDialog->setNameFilter(tr("Images(*.bmp)"));
    fileDialog->setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if(fileDialog->exec())
    {
        fileNames=fileDialog->selectedFiles();
        //call open function
        fileName=QString(fileNames.at(0)).toStdString();
        openFileByGivenPath(QString(fileNames.at(0)).toStdString());
        //处理路径，保存打开文件的目录
        string filePath_temp=QString(fileNames.at(0)).toStdString();
        string::size_type iter=filePath_temp.find_last_of('/');
        filePath=filePath_temp.substr(0,iter+1);
    }
}

/*********************************
 * description:
 * this function will open the BMP image
 * from given path
 * ******************************/
void MainWindow::openFileByGivenPath(string path)
{
    if(imageOpened)  //当前已经打开了图片，释放已打开文件的空间
    {
        free(originalImage.imageBuffer);
        free(originalImage.colorTable);
        free(displayedImage.imageBuffer);
        free(displayedImage.colorTable);
        free(changedImage.imageBuffer);
        free(changedImage.colorTable);
        originalImage.imageBuffer=originalImage.colorTable=NULL;
        displayedImage.imageBuffer=displayedImage.colorTable=NULL;
        changedImage.imageBuffer=changedImage.colorTable=NULL;
    }
    if((BMPin=fopen(path.c_str(),"rb"))==NULL)
        QMessageBox::warning(NULL,"WARNING","Fail to open BMP file");
    fread((void *)&originalImage.fileHeader,1,14,BMPin);
    fread((void *)&originalImage.infoHeader,1,40,BMPin);
    imageOpened=true;
    enableActions();
    int imageWidth=originalImage.infoHeader.biWidth;      //待修改
    int imageHeight=originalImage.infoHeader.biHeight;
    setMaximumSize(imageWidth,imageHeight+17);   //修改窗口大小以适应图像
    repaint();  
    if(originalImage.infoHeader.biBitCount==24)//24位真彩色图像
    {
        disable8bitOperation();
        ui->actionImage_interpolation_2->setEnabled(false);
        ui->actionGaussian_smoothing->setEnabled(false);
        ui->actionMedian_filtering->setEnabled(false);
        is24bit=true;
        originalImage.bytePerLine=(imageWidth*3+3)/4*4;            //计算每行的字节数，BMP中用于4B对齐
        displayedImage.bytePerLine=originalImage.bytePerLine;
        displayedImage.fileHeader=originalImage.fileHeader;
        displayedImage.infoHeader=originalImage.infoHeader;
        changedImage.fileHeader=originalImage.fileHeader;          //用于显示文件信息
        changedImage.infoHeader=originalImage.infoHeader;
        fseek(BMPin,originalImage.fileHeader.bfOffBits,0);         //设置指针偏移量，不设置也可以
        totalSizeinByte=originalImage.infoHeader.biSizeImage;      //总大小(位图数据)
        originalImage.imageBuffer=(uchar *)malloc(sizeof(uchar)*totalSizeinByte); //申请缓冲区
        displayedImage.imageBuffer=(uchar *)malloc(sizeof(uchar)*totalSizeinByte); //申请缓冲区
        memset(originalImage.imageBuffer,0,sizeof(uchar)*totalSizeinByte);     //缓冲区清0
        memset(displayedImage.imageBuffer,0,sizeof(uchar)*totalSizeinByte);
        fread((void *)originalImage.imageBuffer,1,totalSizeinByte,BMPin);
        memcpy(displayedImage.imageBuffer,originalImage.imageBuffer,sizeof(uchar)*totalSizeinByte);
        imageUpsideDown24bit(displayedImage);
        //contruct the original QImage
        originImage=new QImage(displayedImage.imageBuffer,imageWidth,imageHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    }
    else if(originalImage.infoHeader.biBitCount==8)     //8bit图像
    {
        enable8bitOperation();
        is24bit=false;
        changedImage.fileHeader=originalImage.fileHeader;
        changedImage.infoHeader=originalImage.infoHeader;
        originalImage.colorTable=(uchar *)malloc(sizeof(uchar)*1024);
        changedImage.colorTable=(uchar *)malloc(sizeof(uchar)*1024);  //创建调色板空间并读取
        fread((void *)originalImage.colorTable,1,1024,BMPin);
        memcpy(changedImage.colorTable,originalImage.colorTable,sizeof(uchar)*1024); //将数据拷贝至修改缓冲区
        ui->actionDisplay_color_palatte->setEnabled(true);
        changedImage.bytePerLine=originalImage.bytePerLine=(imageWidth+3)/4*4;                           //计算每行的字节数，BMP中用于4B对齐
        fseek(BMPin,originalImage.fileHeader.bfOffBits,0);              //设置指针偏移量，不设置也可以
        /*************************************************
         * 此处使用的方法为将8bit的每个像素通过调色板扩充为24bit
         * 每个像素，
         * **********************************************/
        originalImage.infoHeader.biSizeImage=changedImage.infoHeader.biSizeImage=originalImage.infoHeader.biHeight*originalImage.bytePerLine;
        totalSizeinByte=originalImage.infoHeader.biSizeImage;
        cout<<totalSizeinByte<<endl;
        originalImage.imageBuffer=(uchar *)malloc(sizeof(uchar)*totalSizeinByte); //申请缓冲区
        changedImage.imageBuffer=(uchar *)malloc(sizeof(uchar)*totalSizeinByte);
        memset(originalImage.imageBuffer,0,sizeof(uchar)*totalSizeinByte);     //缓冲区清0
        memset(changedImage.imageBuffer,0,sizeof(uchar)*totalSizeinByte);
        if(originalImage.imageBuffer==NULL)
            cout<<"malloc wrong"<<endl;
        fread((void *)originalImage.imageBuffer,1,totalSizeinByte,BMPin);
        memcpy(changedImage.imageBuffer,originalImage.imageBuffer,sizeof(uchar)*totalSizeinByte);
        //将原图像翻转至正常顺序
        imageUpsideDown(changedImage);
        //以读入的原始8bit位图，使用调色板扩充为24bit图
        convert8to24(changedImage,displayedImage);
        //construct the original QImage
        originImage=new QImage(displayedImage.imageBuffer,imageWidth,imageHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    }
    else
    {
        QMessageBox::warning(NULL,"WARNING","该格式不受支持(24bit,8bit only)");
        return;
    }
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
    fclose(BMPin);
}

/*********************************
 * 显示BMP信息
 * 若图片已经打开，将显示BMP信息
 * ******************************/
void MainWindow::openBMPInfo()
{
    if(!imageOpened)
        return;
    info=new BMPinfo;
    info->setAttribute(Qt::WA_DeleteOnClose);
    info->show();
}

/******************************
 * 显示调色板
 * 若图片已经打开，将显示调色板数据
 * 当图片为24bit图片时无法调用(界面控制)
 * ***************************/
void MainWindow::openColorPalette()
{
    if(!imageOpened)
        return;
    colorPaletteInfo=new colorPalette;
    //info->setWindowModality(Qt::WindowModal);
    colorPaletteInfo->setAttribute(Qt::WA_DeleteOnClose);
    colorPaletteInfo->show();
}

/******************************
 * this function disable all the actions
 * in menubar when no image loaded
 * ***************************/
void MainWindow::disableActions()
{
    ui->actionback->setEnabled(false);
    ui->actionBack_to_Original_Picture->setEnabled(false);
    ui->actionDisplayBMPInfo->setEnabled(false);
    ui->actionDisplay_color_palatte->setEnabled(false);
    ui->actionGaussian_smoothing->setEnabled(false);
    ui->actionImage_interpolation_2->setEnabled(false);
    ui->actionMedian_filtering->setEnabled(false);
    ui->actionSave_to_new_BMP_file->setEnabled(false);
    ui->actionPixelPickBar->setEnabled(false);
    ui->actionShowHistogram->setEnabled(false);
    ui->actionAdd_Impulsive_Noise->setEnabled(false);
    ui->actionHistogram_Equalization->setEnabled(false);
    ui->actionBilateralFilter->setEnabled(false);
}

void MainWindow::enableActions()
{
    ui->actionback->setEnabled(true);
    ui->actionBack_to_Original_Picture->setEnabled(true);
    ui->actionDisplayBMPInfo->setEnabled(true);
    ui->actionSave_to_new_BMP_file->setEnabled(true);
    ui->actionPixelPickBar->setEnabled(true);
}

void MainWindow::enable8bitOperation()
{
    ui->actionDisplay_color_palatte->setEnabled(true);
    ui->actionGaussian_smoothing->setEnabled(true);
    ui->actionImage_interpolation_2->setEnabled(true);
    ui->actionMedian_filtering->setEnabled(true);
    ui->actionAdd_Impulsive_Noise->setEnabled(true);
    ui->actionShowHistogram->setEnabled(true);
    ui->actionHistogram_Equalization->setEnabled(true);
    ui->actionBilateralFilter->setEnabled(true);
}

void MainWindow::disable8bitOperation()
{
    ui->actionDisplay_color_palatte->setEnabled(false);
    ui->actionGaussian_smoothing->setEnabled(false);
    ui->actionImage_interpolation_2->setEnabled(false);
    ui->actionMedian_filtering->setEnabled(false);
    ui->actionAdd_Impulsive_Noise->setEnabled(false);
    ui->actionShowHistogram->setEnabled(false);
    ui->actionHistogram_Equalization->setEnabled(false);
    ui->actionBilateralFilter->setEnabled(false);
}

void MainWindow::getNewFileName()
{
    newName=new inputNewName;
    newName->setWindowModality(Qt::WindowModal);
    newName->setAttribute(Qt::WA_DeleteOnClose);
    connect(newName,SIGNAL(nameGot(QString)),this,SLOT(saveAsNewBMPImage(QString)));
    newName->show();
}

/****************************
 * 保存为新的BMP文件
 * 新文件所在目录为打开的原始BMP文件所在目录
 * *************************/
void MainWindow::saveAsNewBMPImage(QString name)
{
    newName->close();
    string finalPath=filePath+name.toStdString()+".bmp";
    if((BMPout=fopen(finalPath.c_str(),"wb+"))==NULL)
    {
        QMessageBox::warning(NULL,"WARNING","Fail to create file");
        return;
    }
    if(is24bit)
    {
        fwrite((void *)(&displayedImage.fileHeader),1,14,BMPout); //写文件头
        fwrite((void *)(&displayedImage.infoHeader),1,40,BMPout); //写信息头
        imageUpsideDown24bit(displayedImage);
        fwrite((void *)displayedImage.imageBuffer,1,sizeof(uchar)*displayedImage.infoHeader.biSizeImage,BMPout);
        imageUpsideDown24bit(displayedImage);
    }
    else
    {
        fwrite((void *)(&changedImage.fileHeader),1,14,BMPout); //写文件头
        fwrite((void *)(&changedImage.infoHeader),1,40,BMPout); //写信息头
        fwrite((void *)changedImage.colorTable,1,1024,BMPout); //写调色板
        imageUpsideDown(changedImage);
        fwrite((void *)changedImage.imageBuffer,1,sizeof(uchar)*changedImage.infoHeader.biSizeImage,BMPout); //写位图信息
        imageUpsideDown(changedImage);
    }
    fclose(BMPout);
}

/******************************
 * 显示像素选取区
 * 该函数控制像素选取区域的显示与收回
 * ***************************/
void MainWindow::showPixelBar()
{
    if(imageOpened)
    {
        if(!pickColor)
        {
            ui->widget->show();
            pickColor=true;
            ui->centralWidget->setMouseTracking(true); //顺序很关键
            this->setMouseTracking(true);
            ui->label->setMouseTracking(true);
        }
        else
        {
            ui->widget->hide();
            pickColor=false;
            //this->resize(ui->label->width(),ui->label->height()+ui->menuBar->height());
            //this->adjustSize();
            setMaximumHeight(ui->label->height()+ui->menuBar->height());  //不起作用。。。
            setMaximumWidth(ui->label->width());
            this->repaint();
            //在不需要时关闭鼠标移动监测，以免浪费资源
            ui->centralWidget->setMouseTracking(false); //顺序很关键
            this->setMouseTracking(false);
            ui->label->setMouseTracking(false);
            ui->checkBox->setChecked(false);
        }
    }
    return;
}

/*******************************
 * 显示软件声明
 * ****************************/
void MainWindow::showAboutInfo()
{
    delete aboutPage;
    aboutPage=new about;
    aboutPage->show();
}

//检测鼠标移动的事件函数
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(pickColor)
    {
        mousePos=event->globalPos();
        mousePos=ui->label->mapFromGlobal(mousePos);
        if(mousePos.x()<0||mousePos.y()<0||mousePos.x()>displayedImage.infoHeader.biWidth-1)           //防止越界
            return;
        else
        {
            ui->coord_x->setText(QString::number(mousePos.x()));
            ui->coord_y->setText(QString::number(mousePos.y()));
            //由于8bit与24bit模式都使用24bit的方法显示（见上文注释）,直接从displayedImage.imageBuffer空间读三个字节
            R=*(displayedImage.imageBuffer+mousePos.y()*displayedImage.bytePerLine+mousePos.x()*3);
            G=*(displayedImage.imageBuffer+mousePos.y()*displayedImage.bytePerLine+mousePos.x()*3+1);
            B=*(displayedImage.imageBuffer+mousePos.y()*displayedImage.bytePerLine+mousePos.x()*3+2);
            ui->val_r->setText(QString::number((int)R));
            ui->val_g->setText(QString::number((int)G));
            ui->val_b->setText(QString::number((int)B));
            ui->tableWidget->item(0,0)->setBackgroundColor(QColor((int)R,(int)G,(int)B));
        }
    }
}

//检测鼠标左键点击的事件函数
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(ui->checkBox->isChecked())
    {
        if(event->button()==Qt::LeftButton)
        {
            mousePos1=event->globalPos();
            mousePos1=ui->label->mapFromGlobal(mousePos1);
            if(mousePos1.x()<0||mousePos1.y()<0||mousePos1.x()>displayedImage.infoHeader.biWidth-1)           //防止越界
                return;
            else
            {
                /**********************************
                 * 此处假设的是一种理想情况：鼠标先移动后点击，
                 * 移动事件在点击事件前先被处理，因此RGB值与坐标已经获得
                 * *******************************/
                if(is24bit)
                {
                    delete newColorIn;
                    newColorIn=new newColor((int)R,(int)G,(int)B);
                    newColorIn->setWindowModality(Qt::WindowModal);
                    connect(newColorIn,SIGNAL(colorChange(int,int,int)),this,SLOT(changeColor(int,int,int)));
                    newColorIn->show();
                }
                else
                {
                    //首先获得索引值
                    int index=(int)*(changedImage.imageBuffer+(changedImage.infoHeader.biHeight-1-mousePos1.y())*changedImage.bytePerLine+mousePos1.x());
                    delete newColorIn8bit;
                    newColorIn8bit=new newColor8bit(index);
                    newColorIn8bit->setWindowModality(Qt::WindowModal);
                    connect(newColorIn8bit,SIGNAL(newIndex(int)),this,SLOT(changeColor8bit(int)));
                    newColorIn8bit->show();
                }
            }
        }
    }
}

void MainWindow::changeColor(int R, int G, int B)
{
    *(displayedImage.imageBuffer+mousePos1.y()*displayedImage.bytePerLine+mousePos1.x()*3)=uchar(R);
    *(displayedImage.imageBuffer+mousePos1.y()*displayedImage.bytePerLine+mousePos1.x()*3+1)=uchar(G);
    *(displayedImage.imageBuffer+mousePos1.y()*displayedImage.bytePerLine+mousePos1.x()*3+2)=uchar(B);
    originImage=new QImage(displayedImage.imageBuffer,displayedImage.infoHeader.biWidth,displayedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
    newColorIn->close();
}

void MainWindow::changeColor8bit(int index)
{
    *(changedImage.imageBuffer+mousePos1.y()*changedImage.bytePerLine+mousePos1.x())=index;
    convert8to24(changedImage,displayedImage);
    originImage=new QImage(displayedImage.imageBuffer,changedImage.infoHeader.biWidth,changedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
    newColorIn8bit->close();
}

void MainWindow::interpolationClick()
{
    delete interpol;
    interpol=new interpolation;
    interpol->setWindowModality(Qt::WindowModal);
    //newColorIn->setAttribute(Qt::WA_DeleteOnClose);
    connect(interpol,SIGNAL(getFactor(int,int,int)),this,SLOT(getInterFactor(int,int,int)));
    interpol->show();
}

void MainWindow::getInterFactor(int type,int factorX, int factorY)
{
    //call actual interpolation function
    doInterpolation(type,factorX,factorY,changedImage);
    convert8to24(changedImage,displayedImage);
    originImage=new QImage(displayedImage.imageBuffer,changedImage.infoHeader.biWidth,changedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
    interpol->close();
}

/******************************
 * 该函数将位图数据上下对称颠倒，
 * 用于8bit图像的处理与保存
 * ***************************/
void MainWindow::imageUpsideDown(BMPImage &image)
{
    //申请一块新空间存放翻转后的数据
    uchar *imageAfterReverse=(uchar *)malloc(sizeof(uchar)*image.infoHeader.biSizeImage);
    int height=image.infoHeader.biHeight;
    for(int counter=0;counter<height;counter++)
        memcpy(imageAfterReverse+counter*image.bytePerLine,image.imageBuffer+(height-counter-1)*image.bytePerLine,image.bytePerLine);
    free(image.imageBuffer);
    image.imageBuffer=imageAfterReverse;
    return;
}

/******************************
 * 该函数将位图数据上下对称颠倒，
 * 用于24bit图像，与8bit的图像相比，
 * 24bit图像的RGB值也要进行颠倒
 * ***************************/
void MainWindow::imageUpsideDown24bit(BMPImage &image)
{
    //申请一块新空间存放翻转后的数据
    uchar *imageAfterReverse=(uchar *)malloc(sizeof(uchar)*image.infoHeader.biSizeImage);
    int height=image.infoHeader.biHeight;
    int width=image.infoHeader.biWidth;
    int row_bias=0,column_bias=0,row_bias_d=0,row_bias_b=0;
    int total_bias_d=0,total_bias_b=0;
    for(row_bias=height-1;row_bias>=0;row_bias--)
    {
        row_bias_d=(height-1-row_bias)*image.bytePerLine;
        row_bias_b=row_bias*image.bytePerLine;
        for(column_bias=0;column_bias<width;column_bias++)
        {
            total_bias_d=row_bias_d+column_bias*3;
            total_bias_b=row_bias_b+column_bias*3;
            *(imageAfterReverse+total_bias_d)=*(image.imageBuffer+total_bias_b+2);
            *(imageAfterReverse+total_bias_d+1)=*(image.imageBuffer+total_bias_b+1);
            *(imageAfterReverse+total_bias_d+2)=*(image.imageBuffer+total_bias_b);
        }
        //读入冗余字节部分已经初始化为0
    }
    free(image.imageBuffer);
    image.imageBuffer=imageAfterReverse;
    return;
}

/******************************
 * 该函数通过调色盘将8bit图像扩充至24bit以方便显示
 * imageData是存储最终显示的图片的数据的缓冲区
 * ***************************/
void MainWindow::convert8to24(BMPImage &changedImage,BMPImage &displayedImage)
{
    if(displayedImage.imageBuffer!=NULL)
        free(displayedImage.imageBuffer);
    displayedImage.infoHeader=changedImage.infoHeader;
    displayedImage.bytePerLine=(changedImage.infoHeader.biWidth*3+3)/4*4;
    unsigned long totalSizeinByte_trans=displayedImage.bytePerLine*changedImage.infoHeader.biHeight;     //扩充后的大小
    displayedImage.imageBuffer=(uchar *)malloc(sizeof(uchar)*totalSizeinByte_trans);   //申请缓冲区2
    memset(displayedImage.imageBuffer,0,sizeof(uchar)*totalSizeinByte_trans);
    int row_bias=0,column_bias=0,row_bias_d=0,row_bias_b=0;
    int total_bias_d=0,total_bias_b=0;
    int width=changedImage.infoHeader.biWidth;
    int height=changedImage.infoHeader.biHeight;
    for(row_bias=0;row_bias<height;row_bias++)
    {
        row_bias_b=row_bias*changedImage.bytePerLine;
        row_bias_d=row_bias*displayedImage.bytePerLine;
        for(column_bias=0;column_bias<width;column_bias++)
        {
            total_bias_d=row_bias_d+column_bias*3;
            total_bias_b=row_bias_b+column_bias;
            *(displayedImage.imageBuffer+total_bias_d)=*(changedImage.colorTable+(int(*(changedImage.imageBuffer+total_bias_b)))*4+2);
            *(displayedImage.imageBuffer+total_bias_d+1)=*(changedImage.colorTable+(int(*(changedImage.imageBuffer+total_bias_b)))*4+1);
            *(displayedImage.imageBuffer+total_bias_d+2)=*(changedImage.colorTable+(int(*(changedImage.imageBuffer+total_bias_b)))*4);
        }
    }
    return;
}

/************************
 * 中值滤波函数调用函数
 * 该函数用于响应菜单中中值滤波功能的点击
 * *********************/
void MainWindow::doMedianFiltering()
{
    MedianFiltering3x3(changedImage);
    convert8to24(changedImage,displayedImage);
    originImage=new QImage(displayedImage.imageBuffer,changedImage.infoHeader.biWidth,changedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
}

/***********************
 * 直方图均衡化调用函数
 * 该函数响应菜单中直方图均衡化功能的点击
 * ********************/
void MainWindow::doHistogramEqualization()
{
    delete histo;
    histo=NULL;
    histogramEqualization(changedImage);
    convert8to24(changedImage,displayedImage);
    originImage=new QImage(displayedImage.imageBuffer,changedImage.infoHeader.biWidth,changedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
}

/***********************
 * 高斯平滑函数调用函数
 * 该函数用于响应菜单中高斯平滑功能的点击
 * 执行结果为弹出参数输入对话框
 * ********************/
void MainWindow::gaussianSmoothClick()
{
    delete gaussianSmooth;
    gaussianSmooth=new GaussianSmooth;
    gaussianSmooth->setWindowModality(Qt::WindowModal);
    connect(gaussianSmooth,SIGNAL(gaussian(double)),this,SLOT(doGaussianSmooth(double)));
    gaussianSmooth->show();
}

/*********************
 * 执行高斯平滑的实际函数
 * ******************/
void MainWindow::doGaussianSmooth(double MSD)
{
    gaussianSmooth->close();
    imageGaussianSmooth(changedImage,MSD);
    convert8to24(changedImage,displayedImage);
    originImage=new QImage(displayedImage.imageBuffer,changedImage.infoHeader.biWidth,changedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
}

/*********************
 * 添加噪声函数调用函数
 * 用于响应菜单中添加冲击噪声选项的点击
 * 调出噪声参数输入窗口
 * ******************/
void MainWindow::addNoiseSmoothClick()
{
    delete addNoise;
    addNoise=new addimpulsivenoise;
    addNoise->setWindowModality(Qt::WindowModal);
    connect(addNoise,SIGNAL(noiseFactorOK(int,int)),this,SLOT(doAddNoise(int,int)));
    addNoise->show();
}

/*********************
 * 噪声添加调用实际函数
 * ******************/
void MainWindow::doAddNoise(int type,int portion)
{
    addNoise->close();
    addImpulsiveNoise(changedImage,type,portion);
    convert8to24(changedImage,displayedImage);
    originImage=new QImage(displayedImage.imageBuffer,changedImage.infoHeader.biWidth,changedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
}

/*********************
 * 双边滤波函数调用函数
 * 用于响应菜单中双边滤波选项的点击
 * 调出参数输入窗口
 * ******************/
void MainWindow::bilateralFilterClick()
{
    delete BF;
    BF=new bilateralFilter();
    BF->setWindowModality(Qt::WindowModal);
    connect(BF,SIGNAL(bilateralfilter(int,double,double)),this,SLOT(doBilateralFilter(int,double,double)));
    BF->show();
}

/*********************
 * 双边滤波调用实际函数
 * ******************/
void MainWindow::doBilateralFilter(int windowSize,double sigma_d,double sigma_r)
{
    BF->close();
    startBilateralFilter(changedImage,windowSize,sigma_d,sigma_r);
    convert8to24(changedImage,displayedImage);
    originImage=new QImage(displayedImage.imageBuffer,changedImage.infoHeader.biWidth,changedImage.infoHeader.biHeight,displayedImage.bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
}

/***********************************
 * 撤回操作函数
 * 将所有操作撤回值原始图片
 * 其实就是重新读了一遍
 * ********************************/
void MainWindow::retrieveOperations()
{
    openFileByGivenPath(fileName);
}

/*****************************
 * 显示图像直方图函数
 * **************************/
void MainWindow::showHistoGram()
{
    //统计图像直方图数据
    QVector<double> histoData(256,0);
    int row,column;
    for(row=0;row<changedImage.infoHeader.biHeight;row++)
        for(column=0;column<changedImage.infoHeader.biWidth;column++)
            histoData[*(changedImage.imageBuffer+row*changedImage.bytePerLine+column)]++;
    long double totalPixel=changedImage.infoHeader.biWidth*changedImage.infoHeader.biHeight;
    //计算灰度频率
    for(int itor=0;itor<256;itor++)
        histoData[itor]/=totalPixel;
    //调出直方图窗口
    delete histo;
    histo=new histogram(histoData);
    histo->show();
    return;
}

/***********************************
 * 以下为图像的插值、平滑、滤波、添加噪声、
 * 直方图均衡化、双边滤波的五个函数
 * *********************************/

/***********************************
 * 插值函数
 * ********************************/
void MainWindow::doInterpolation(int type,int factorX,int factorY,BMPImage &image)
{
    cout<<type<<endl;
    //重新计算放大后的图像大小并申请缓冲区
    int height=image.infoHeader.biHeight;
    int width=image.infoHeader.biWidth;
    int newWidth=image.infoHeader.biWidth*factorX;
    int newHeight=image.infoHeader.biHeight*factorY;
    int newBytePerLine=(newWidth+3)/4*4;
    int oldBytePerLine=image.bytePerLine;
    unsigned long sizeinByte=newBytePerLine*newHeight;
    uchar *newBuffer;
    if((newBuffer=(uchar *)malloc(sizeinByte*sizeof(uchar)))==NULL)
    {
        QMessageBox::warning(NULL,"warning","无法创建新缓冲区");
        return;
    }
    else
    {
        memset(newBuffer,0,sizeinByte);
        int rowCounter,columnCounter=10;
        //最邻近法
        if(type==1)
        {
            int oriX,oriY;
            for(rowCounter=0;rowCounter<newHeight;rowCounter++)
                for(columnCounter=0;columnCounter<newWidth;columnCounter++)
                {
                    oriX=min(int((double(columnCounter))/factorX+0.5),width-1);    //获取最邻近x
                    oriY=min(int((double(rowCounter))/factorY+0.5),height-1);      //获取最邻近y
                    *(newBuffer+rowCounter*newBytePerLine+columnCounter)=*(image.imageBuffer+oriY*oldBytePerLine+oriX);   //获取对应像素值
                }
        }
        //双线性法
        else if(type==2)
        {
            double Ga=0,Gb=0;
            int x,y1,y2;
            double dy,dx;
            for(rowCounter=0;rowCounter<newHeight;rowCounter++)
                for(columnCounter=0;columnCounter<newWidth;columnCounter++)
                {
                    //左侧线性值
                    x=min(int((double(columnCounter))/factorX),width-1);
                    dx=(double(columnCounter))/factorX-x;
                    y1=min(int((double(rowCounter))/factorY),height-1);
                    y2=min(y1+1,height-1);
                    dy=(double(rowCounter))/factorY-y1;
                    Ga=double(*(image.imageBuffer+y1*oldBytePerLine+x))+double(dy*(int(*(image.imageBuffer+y2*oldBytePerLine+x))-int(*(image.imageBuffer+y1*oldBytePerLine+x))));
                    //右侧线性值
                    x=min(x+1,width-1);
                    Gb=double(*(image.imageBuffer+y1*oldBytePerLine+x))+double(dy*(int(*(image.imageBuffer+y2*oldBytePerLine+x))-int(*(image.imageBuffer+y1*oldBytePerLine+x))));
                    //计算对应像素值
                    *(newBuffer+rowCounter*newBytePerLine+columnCounter)=uchar(int(Ga+dx*(Gb-Ga)));
                }
        }
        else
            return;  //暂无
        //计算修改image中的图像数据
        image.infoHeader.biWidth=newWidth;
        image.infoHeader.biHeight=newHeight;
        image.infoHeader.biSizeImage=newHeight*newBytePerLine;
        image.bytePerLine=newBytePerLine;
        image.fileHeader.bfSize=image.fileHeader.bfOffBits+image.infoHeader.biSizeImage;
        //新图像数据赋值
        free(image.imageBuffer);
        image.imageBuffer=newBuffer;
        return;
    }
}

/***********************************
 * 3*3中值滤波
 * ********************************/
//::MedianFiltering3x3(uchar *buffer,int width,int height)
void MainWindow::MedianFiltering3x3(BMPImage &image)
{
    QVector<int> pixelSort;
    int row,column,windowRow,windowColumn;
    uchar *newBuffer=(uchar *)malloc(image.infoHeader.biSizeImage*sizeof(uchar));
    if(newBuffer==NULL)
        return;
    memset(newBuffer,0,image.infoHeader.biSizeImage);
    for(row=0;row<image.infoHeader.biHeight;row++)
        for(column=0;column<image.infoHeader.biWidth;column++)
        {
            pixelSort.clear();
            //获得需要排序的像素
            for(windowRow=row-1;windowRow<=row+1;windowRow++)
                for(windowColumn=column-1;windowColumn<=column+1;windowColumn++)
                {
                    if(windowColumn<0||windowRow<0||windowColumn>=image.infoHeader.biWidth||windowRow>=image.infoHeader.biHeight) //边缘像素
                        continue;
                    pixelSort.push_back(int(*(image.imageBuffer+image.bytePerLine*windowRow+windowColumn)));
                }
            //对像素值进行排序
            sort(pixelSort.begin(),pixelSort.end());
            *(newBuffer+image.bytePerLine*row+column)=uchar(pixelSort.at(int(pixelSort.length()/2)+1));
        }
    free(image.imageBuffer);
    image.imageBuffer=newBuffer;
    return;
}

/*********************************
 * 高斯平滑
 * ******************************/
void MainWindow::imageGaussianSmooth(BMPImage image,double MSD)
{
    //计算窗口大小
    int windowSize=int(3*MSD);
    if(windowSize%2==0)
        windowSize++;
    if(windowSize<3)
        windowSize=3;
    //不需要申请新空间
    int row,column,windowRow,windowColumn;
    //计算权值矩阵以用于计算
    QVector<double> weightMatrix=GaussianMatrix(windowSize,MSD);
    //计算矩阵权重和
    long double weightSum=0;
    for(double itor:weightMatrix)
        weightSum+=itor;
    //开始计算，图像边缘的部分像素未考虑
    long double sum=0;
    int currentPixelBias,currentMatrixBias;
    for(row=int(windowSize/2);row<image.infoHeader.biHeight-int(windowSize/2);row++)
        for(column=int(windowSize/2);column<image.infoHeader.biWidth-int(windowSize/2);column++)
        {
            sum=0;
            for(windowRow=row-int(windowSize/2);windowRow<=row+int(windowSize/2);windowRow++)
                for(windowColumn=column-int(windowSize/2);windowColumn<=column+int(windowSize/2);windowColumn++)
                {
                    //if(windowRow<0||windowColumn<0||windowRow>image.infoHeader.biHeight-1||windowColumn>image.infoHeader.biWidth)
                        //continue;
                    currentMatrixBias=(windowRow-(row-int(windowSize/2)))*windowSize+windowColumn-(column-int(windowSize/2));
                    currentPixelBias=windowRow*image.bytePerLine+windowColumn;
                    sum+=double((weightMatrix[currentMatrixBias]/weightSum)*double(*(image.imageBuffer+currentPixelBias)));
                }
            *(image.imageBuffer+row*image.bytePerLine+column)=uchar(int(sum+0.5/*/(windowSize*windowSize)*/));
        }
    return;
}

/*****************************
 * 计算高斯权值矩阵
 * **************************/
QVector<double> MainWindow::GaussianMatrix(int windowSize, double MSD)
{
    QVector<double> GaussianVector(windowSize*windowSize,0);
    //根据窗口大小设置中点，用于计算权值
    int median=int(windowSize/2);
    int row,column;
    //计算各个距离的点的权值
    for(row=0;row<windowSize;row++)
        for(column=0;column<windowSize;column++)
            GaussianVector[row*windowSize+column]=exp(-((pow(column-median,2)+pow(row-median,2))/(2*pow(MSD,2))))/(2*PI*pow(MSD,2));
    return GaussianVector;
}

/**********************************
 * 添加冲击噪声
 * type=1 椒盐噪声
 * type=2 随机冲击噪声
 * *******************************/
void MainWindow::addImpulsiveNoise(BMPImage &image,int type,int portion)
{

    double dirtyPoint=0;

    if(type!=1&&type!=2)
        return;
    int column;
    int randomNum;
    //int threshold=portion*10;
    int threshold=portion;
    srand(time(0));
    for(int row=0;row<image.infoHeader.biHeight;row++)
        for(column=0;column<image.infoHeader.biWidth;column++)
        {
            randomNum=rand()%100;
            if(randomNum>=threshold)
                continue;//不添加噪声
            else
            {
                dirtyPoint++;
                if(type==1) //椒盐噪声
                {
                    randomNum=rand()%100;
                    *(image.imageBuffer+row*image.bytePerLine+column)=(randomNum>=50?255:0);
                }
                else   //随机冲击噪声
                    *(image.imageBuffer+row*image.bytePerLine+column)=rand()%255;
            }
        }
    double noiseRate=double(dirtyPoint/(image.infoHeader.biHeight*image.infoHeader.biWidth));
    cout<<noiseRate<<endl;
    return;
}

/***********************
 * 直方图均衡化
 * ********************/
void MainWindow::histogramEqualization(BMPImage &image)
{
    QVector<double> histogram(256,0);            //原始直方图,初始化很重要
    QVector<double> cumulatedHistogram(256,0);   //累积直方图
    QMap<int,int> greyLevelTransfer;             //灰度转换关系
    histogram.clear();
    //统计原始灰度数据
    int row,column;
    for(row=0;row<image.infoHeader.biHeight;row++)
        for(column=0;column<image.infoHeader.biWidth;column++)
            histogram[*(image.imageBuffer+row*image.bytePerLine+column)]++;
    long double totalPixel=image.infoHeader.biWidth*image.infoHeader.biHeight;
    //计算灰度频率
    for(int itor=0;itor<256;itor++)
        histogram[itor]/=totalPixel;
    //计算累积直方图
    cumulatedHistogram[0]=histogram[0];
    for(int itor=1;itor<256;itor++)
        cumulatedHistogram[itor]=cumulatedHistogram[itor-1]+histogram[itor];
    //计算变换后灰度值
    for(int itor=0;itor<256;itor++)
        greyLevelTransfer[itor]=int(255*cumulatedHistogram[itor]+0.5);
    //修改像素灰度
    for(row=0;row<image.infoHeader.biHeight;row++)
        for(column=0;column<image.infoHeader.biWidth;column++)
            *(image.imageBuffer+row*image.bytePerLine+column)=greyLevelTransfer[*(image.imageBuffer+row*image.bytePerLine+column)];
    return;
}

/*************
 * 双边滤波函数
 * ***********/
void MainWindow::startBilateralFilter(BMPImage &image,int windowSize,double sigma_d,double sigma_r)
{
    int row,column,windowRow,windowColumn;
    long double sum;
    double rd,rr,k;
    int m,n,fmn,fMN;
    for(row=int(windowSize/2);row<image.infoHeader.biHeight-int(windowSize/2);row++)
        for(column=int(windowSize/2);column<image.infoHeader.biWidth-int(windowSize/2);column++,sum=0,k=0)
        {
            for(windowRow=row-int(windowSize/2);windowRow<=row+int(windowSize/2);windowRow++)
                for(windowColumn=column-int(windowSize/2);windowColumn<=column+int(windowSize/2);windowColumn++)
                {
                    //if(windowRow<0||windowColumn<0||windowRow>image.infoHeader.biHeight-1||windowColumn>image.infoHeader.biWidth)
                        //continue;
                    m=windowRow-row;
                    n=windowColumn-column;
                    fmn=*(image.imageBuffer+windowRow*image.bytePerLine+windowColumn);
                    fMN=*(image.imageBuffer+row*image.bytePerLine+column);
                    rd=exp((pow(m,2)+pow(n,2))/(2*pow(sigma_d,2)));
                    rr=exp(pow((fmn-fMN),2)/(2*pow(sigma_r,2)));
                    k+=rd*rr;
                    sum+=fmn*rd*rr;
                }
            *(image.imageBuffer+row*image.bytePerLine+column)=uchar(int(sum/k+0.5));
        }
    return;
}







