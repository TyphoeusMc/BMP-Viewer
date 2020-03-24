#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
FILE *BMPin;
FILE *BMPout;
extern BITMAPFILEHEADER fileHeader;
extern BITMAPINFOHEADER infoHeader;
bool imageOpened=false;
int bytePerLine=0;
int bytePerLine_trans=0;
int imageWidth;
int imageHeight;
int imageCurrentWidth;
int imageCurrentHeight;
uchar *imageBuffer;
uchar *imageData;
uchar *imageAfterProcess;
extern uchar *colorTable;
string filePath;
bool is24bit=false;
bool pickColor=false;
bool changeColor=false;
unsigned long totalSizeinByte=0;
QPoint mousePos;
QPoint mousePos1;
uchar R;
uchar G;
uchar B;
//QPalette exampleColor;  //获取颜色时的颜色块颜色


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("TyphoeusBrowser BMP");
    ui->widget->hide();
    disableActions();
    newColorIn=new newColor(0,0,0);
    newColorIn8bit=new newColor8bit(1);
    interpol=new interpolation;
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
}

MainWindow::~MainWindow()
{
    free(imageBuffer);
    free(imageData);
    free(colorTable);
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
        fileNames = fileDialog->selectedFiles();
        //call open function
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
 * CS1509 U201514728
 * ******************************/
void MainWindow::openFileByGivenPath(string path)
{
    if(imageOpened)  //当前已经打开了图片，释放已打开文件的空间
    {
        free(imageBuffer);
        free(imageData);
        free(colorTable);
        free(originalImage);
        imageBuffer=imageData=colorTable=NULL;
    }
    if((BMPin=fopen(path.c_str(),"rb"))==NULL)
        QMessageBox::warning(NULL,"WARNING","Fail to open BMP file");
    fread((void *)&fileHeader,1,14,BMPin);
    fread((void *)&infoHeader,1,40,BMPin);
    imageOpened=true;
    enableActions();
    imageCurrentWidth=imageWidth=infoHeader.biWidth;
    imageCurrentHeight=imageHeight=infoHeader.biHeight;
    setMaximumSize(imageWidth,imageHeight+17);
    repaint();
    //resize(imageWidth,imageHeight+17);   //修改窗口大小以适应图像
    if(infoHeader.biBitCount==24)//24位真彩色图像
    {
        disable8bitOperation();
        ui->actionImage_interpolation_2->setEnabled(false);
        ui->actionGaussian_smoothing->setEnabled(false);
        ui->actionMedian_filtering->setEnabled(false);
        is24bit=true;
        bytePerLine=(imageWidth*3+3)/4*4;            //计算每行的字节数，BMP中用于4B对齐
        fseek(BMPin,fileHeader.bfOffBits,0);         //设置指针偏移量，不设置也可以
        totalSizeinByte=bytePerLine*imageHeight;     //总大小，为全局变量
        /*************************************************
         * BMP存储格式中像素从下至上，RGB值排列为BGR，
         * 输入构造QImage前需要重新排列像素顺序与RGB顺序。
         * 在读取时，可根据排列的顺序按规律跳跃式地读取文件，
         * 但由于fread带来文件指针的移动，若不每次都复位指针，算法可能
         * 比较复杂；而每次都复位会带来一定开销。
         * 此处简化的方法为先顺序读取入一个缓冲区，再将其按规律赋值给另一个
         * 保存最终结果的缓冲区
         * 此方法效率较为低下
         * **********************************************/
        imageBuffer=(uchar *)malloc(sizeof(uchar)*totalSizeinByte); //申请缓冲区1
        imageData=(uchar *)malloc(sizeof(uchar)*totalSizeinByte);   //申请缓冲区2
        memset(imageBuffer,0,sizeof(uchar)*totalSizeinByte);     //缓冲区清0
        memset(imageData,0,sizeof(uchar)*totalSizeinByte);
        if(imageData==NULL||imageBuffer==NULL)
            cout<<"malloc wrong"<<endl;
        fread((void *)imageBuffer,1,totalSizeinByte,BMPin);
        int row_bias=0,column_bias=0,row_bias_d=0,row_bias_b=0;
        int total_bias_d=0,total_bias_b=0;
        for(row_bias=imageHeight-1;row_bias>=0;row_bias--)
        {
            row_bias_d=(imageHeight-1-row_bias)*bytePerLine;
            row_bias_b=row_bias*bytePerLine;
            for(column_bias=0;column_bias<imageWidth;column_bias++)
            {
                total_bias_d=row_bias_d+column_bias*3;
                total_bias_b=row_bias_b+column_bias*3;
                *(imageData+total_bias_d)=*(imageBuffer+total_bias_b+2);
                *(imageData+total_bias_d+1)=*(imageBuffer+total_bias_b+1);
                *(imageData+total_bias_d+2)=*(imageBuffer+total_bias_b);
            }
            //读入冗余字节部分已经初始化为0
        }
        //contruct the original QImage
        originalImage=new QImage(imageData,imageWidth,imageHeight,bytePerLine,QImage::Format_RGB888);
    }
    else if(infoHeader.biBitCount==8)     //8bit图像
    {
        enable8bitOperation();
        is24bit=false;
        colorTable=(uchar *)malloc(sizeof(uchar)*1024);
        fread((void *)colorTable,1,1024,BMPin);             //创建调色板空间并读取
        ui->actionDisplay_color_palatte->setEnabled(true);
        bytePerLine=(imageWidth+3)/4*4;            //计算每行的字节数，BMP中用于4B对齐
        fseek(BMPin,fileHeader.bfOffBits,0);       //设置指针偏移量，不设置也可以
        /*************************************************
         * 此处使用的方法为将8bit的每个像素通过调色板扩充为24bit
         * 每个像素，
         * **********************************************/
        totalSizeinByte=bytePerLine*imageHeight;
        //bytePerLine_trans=(imageWidth*3+3)/4*4;   //扩充成24bit后的bpl
        //unsigned long totalSizeinByte_trans=bytePerLine_trans*imageHeight; //扩充后的大小
        imageBuffer=(uchar *)malloc(sizeof(uchar)*totalSizeinByte); //申请缓冲区1
        //imageData=(uchar *)malloc(sizeof(uchar)*totalSizeinByte_trans);   //申请缓冲区2
        memset(imageBuffer,0,sizeof(uchar)*totalSizeinByte);     //缓冲区清0
        //memset(imageData,0,sizeof(uchar)*totalSizeinByte_trans);
        if(imageBuffer==NULL)
            cout<<"malloc wrong"<<endl;
        fread((void *)imageBuffer,1,totalSizeinByte,BMPin);
        //以读入的原始8bit位图，使用调色板扩充为24bit图
        bytePerLine_trans=convert8to24(imageHeight,imageWidth,bytePerLine,imageData,imageBuffer,colorTable);
        /*
        int row_bias=0,column_bias=0,row_bias_d=0,row_bias_b=0;
        int total_bias_d=0,total_bias_b=0;
        for(row_bias=imageHeight-1;row_bias>=0;row_bias--)
        {
            row_bias_d=(imageHeight-1-row_bias)*bytePerLine_trans;
            row_bias_b=row_bias*bytePerLine;
            for(column_bias=0;column_bias<imageWidth;column_bias++)
            {
                total_bias_d=row_bias_d+column_bias*3;
                total_bias_b=row_bias_b+column_bias;
                *(imageData+total_bias_d)=*(colorTable+(int(*(imageBuffer+total_bias_b)))*4+2);
                *(imageData+total_bias_d+1)=*(colorTable+(int(*(imageBuffer+total_bias_b)))*4+1);
                *(imageData+total_bias_d+2)=*(colorTable+(int(*(imageBuffer+total_bias_b)))*4);
            }
        }
        */
        //contruct the original QImage
        originalImage=new QImage(imageData,imageWidth,imageHeight,bytePerLine_trans,QImage::Format_RGB888);
    }
    else
    {
        QMessageBox::warning(NULL,"WARNING","该格式不受支持(24bit,8bit only)");
        return;
    }
    imageShow=imageShow.fromImage(*originalImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
    fclose(BMPin);
}

/*********************************
 * description:
 * this function will show the BMP info of the
 * BMP image that has already opened
 * ******************************/
void MainWindow::openBMPInfo()
{
    if(!imageOpened)
        return;
    info=new BMPinfo;
    //info->setWindowModality(Qt::WindowModal);
    info->setAttribute(Qt::WA_DeleteOnClose);
    info->show();
}

/******************************
 * description:
 * this function will show the color palette
 * read by open function.
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
    ui->actionGaussian_smoothing->setEnabled(true);
    ui->actionImage_interpolation_2->setEnabled(true);
    ui->actionMedian_filtering->setEnabled(true);
}

void MainWindow::disable8bitOperation()
{
    ui->actionGaussian_smoothing->setEnabled(false);
    ui->actionImage_interpolation_2->setEnabled(false);
    ui->actionMedian_filtering->setEnabled(false);
}

void MainWindow::getNewFileName()
{
    newName=new inputNewName;
    newName->setWindowModality(Qt::WindowModal);
    newName->setAttribute(Qt::WA_DeleteOnClose);
    connect(newName,SIGNAL(nameGot(QString)),this,SLOT(saveAsNewBMPImage(QString)));
    newName->show();
}

void MainWindow::saveAsNewBMPImage(QString name)
{
    newName->close();
    string finalPath=filePath+name.toStdString()+".bmp";
    if((BMPout=fopen(finalPath.c_str(),"wb+"))==NULL)
    {
        QMessageBox::warning(NULL,"WARNING","Fail to create file");
        return;
    }
    fwrite((void *)(&fileHeader),1,14,BMPout); //写文件头
    fwrite((void *)(&infoHeader),1,40,BMPout); //写信息头
    if(!is24bit)
        fwrite((void *)colorTable,1,1024,BMPout); //若为8bit文件，写调色板
    fwrite((void *)imageBuffer,1,sizeof(uchar)*totalSizeinByte,BMPout); //写位图信息
    fclose(BMPout);
}

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

void MainWindow::showAboutInfo()
{
    ;
}

//检测鼠标移动的事件函数
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(pickColor)
    {
        mousePos=event->globalPos();
        mousePos=ui->label->mapFromGlobal(mousePos);
        if(mousePos.x()<0||mousePos.y()<0||mousePos.x()>imageWidth-1)           //防止越界
            return;
        else
        {
            ui->coord_x->setText(QString::number(mousePos.x()));
            ui->coord_y->setText(QString::number(mousePos.y()));
            //由于8bit与24bit模式都使用24bit的方法显示（见上文注释）,直接从imageData空间读三个字节
            if(is24bit)
            {
                R=*(imageData+mousePos.y()*bytePerLine+mousePos.x()*3);
                G=*(imageData+mousePos.y()*bytePerLine+mousePos.x()*3+1);
                B=*(imageData+mousePos.y()*bytePerLine+mousePos.x()*3+2);
            }
            else
            {
                R=*(imageData+mousePos.y()*bytePerLine_trans+mousePos.x()*3);
                G=*(imageData+mousePos.y()*bytePerLine_trans+mousePos.x()*3+1);
                B=*(imageData+mousePos.y()*bytePerLine_trans+mousePos.x()*3+2);
            }
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
            if(mousePos1.x()<0||mousePos1.y()<0||mousePos1.x()>imageWidth)           //防止越界
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
                    //newColorIn->setAttribute(Qt::WA_DeleteOnClose);
                    connect(newColorIn,SIGNAL(colorChange(int,int,int)),this,SLOT(changeColor(int,int,int)));
                    newColorIn->show();
                }
                else
                {
                    //首先获得索引值
                    cout<<"1"<<endl;
                    int index=(int)*(imageBuffer+(imageHeight-1-mousePos1.y())*bytePerLine+mousePos1.x());
                    cout<<"2"<<endl;
                    delete newColorIn8bit;
                    cout<<"4"<<endl;
                    newColorIn8bit=new newColor8bit(index);
                    cout<<"3"<<endl;
                    newColorIn8bit->setWindowModality(Qt::WindowModal);
                    //newColorIn->setAttribute(Qt::WA_DeleteOnClose);
                    connect(newColorIn8bit,SIGNAL(newIndex(int)),this,SLOT(changeColor8bit(int)));
                    newColorIn8bit->show();
                }
            }
        }
    }
}

void MainWindow::changeColor(int R, int G, int B)
{
    *(imageData+mousePos1.y()*bytePerLine+mousePos1.x()*3)=uchar(R);
    *(imageData+mousePos1.y()*bytePerLine+mousePos1.x()*3+1)=uchar(G);
    *(imageData+mousePos1.y()*bytePerLine+mousePos1.x()*3+2)=uchar(B);
    originalImage=new QImage(imageData,imageWidth,imageHeight,bytePerLine,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originalImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
    newColorIn->close();
}

void MainWindow::changeColor8bit(int index)
{
    *(imageData+mousePos1.y()*bytePerLine_trans+mousePos1.x()*3)=*(colorTable+index*4);
    *(imageData+mousePos1.y()*bytePerLine_trans+mousePos1.x()*3+1)=*(colorTable+index*4+1);
    *(imageData+mousePos1.y()*bytePerLine_trans+mousePos1.x()*3+2)=*(colorTable+index*4+2);
    originalImage=new QImage(imageData,imageWidth,imageHeight,bytePerLine_trans,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originalImage);
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
    free(imageAfterProcess);
    imageAfterProcess=doInterpolation(type,factorX,factorY,imageBuffer,imageWidth,imageHeight);
    cout<<"s"<<endl;
    imageCurrentWidth=imageWidth*factorX;
    imageCurrentHeight=imageHeight*factorY;
    bytePerLine_trans=convert8to24(imageCurrentHeight,imageCurrentWidth,(imageCurrentWidth+3)/4*4,imageData,imageAfterProcess,colorTable);
    originalImage=new QImage(imageData,imageCurrentWidth,imageCurrentHeight,bytePerLine_trans,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originalImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
    interpol->close();
}

/******************************
 * 该函数通过调色盘将8bit图像扩充至24bit以方便显示
 * 全局变量imageData是存储最终显示的图片的数据的缓冲区
 * ***************************/
int MainWindow::convert8to24(int height,int width,int oldBytePerLine,uchar *&imageData,uchar *imageBuffer,uchar *colorTable)
{
    if(imageData!=NULL)
        free(imageData);
    int newBytePerLine=(width*3+3)/4*4;
    unsigned long totalSizeinByte_trans=newBytePerLine*height;     //扩充后的大小
    imageData=(uchar *)malloc(sizeof(uchar)*totalSizeinByte_trans);   //申请缓冲区2
    memset(imageData,0,sizeof(uchar)*totalSizeinByte_trans);
    int row_bias=0,column_bias=0,row_bias_d=0,row_bias_b=0;
    int total_bias_d=0,total_bias_b=0;
    for(row_bias=height-1;row_bias>=0;row_bias--)
    {
        row_bias_d=(height-1-row_bias)*newBytePerLine;
        row_bias_b=row_bias*oldBytePerLine;
        for(column_bias=0;column_bias<width;column_bias++)
        {
            total_bias_d=row_bias_d+column_bias*3;
            total_bias_b=row_bias_b+column_bias;
            *(imageData+total_bias_d)=*(colorTable+(int(*(imageBuffer+total_bias_b)))*4+2);
            *(imageData+total_bias_d+1)=*(colorTable+(int(*(imageBuffer+total_bias_b)))*4+1);
            *(imageData+total_bias_d+2)=*(colorTable+(int(*(imageBuffer+total_bias_b)))*4);
        }
    }
    return newBytePerLine;
}

/************************
 * 中值滤波函数调用函数
 * 该函数用于响应菜单中中值滤波功能的点击
 * *********************/
void MainWindow::doMedianFiltering()
{
    free(imageAfterProcess);
    imageAfterProcess=MedianFiltering3x3(imageBuffer,imageWidth,imageHeight);
    convert8to24(imageHeight,imageWidth,bytePerLine,imageData,imageAfterProcess,colorTable);
    originalImage=new QImage(imageData,imageCurrentWidth,imageCurrentHeight,bytePerLine_trans,QImage::Format_RGB888);
    imageShow=imageShow.fromImage(*originalImage);
    ui->label->setPixmap(imageShow);
    ui->label->show();
}

/***********************************
 * 以下为图像的插值、平滑、滤波的三个函数
 * 在本程序中，处理后的图像以一个新的缓冲区单独存放
 * 图像的处理过程不具有叠加性，即每次处理都基于原图像
 * *********************************/

/***********************************
 * 插值函数
 * 实际上在本程序中位图缓冲区、长宽参数等是全局变量，
 * 为了达到通用性，将参数作为输入
 * 若变换成功，将返回新的缓冲区地址
 * 若失败，返回空指针
 * ********************************/
uchar *MainWindow::doInterpolation(int type,int factorX,int factorY,uchar *buffer,int width,int height)
{
    //重新计算放大后的图像大小并申请缓冲区
    int newWidth=width*factorX;
    int newHeight=height*factorY;
    int newBytePerLine=(newWidth+3)/4*4;
    int oldBytePerLine=(width+3)/4*4;
    unsigned long sizeinByte=newBytePerLine*newHeight;
    uchar *newBuffer;
    if((newBuffer=(uchar *)malloc(sizeinByte*sizeof(uchar)))==NULL)
    {
        QMessageBox::warning(NULL,"warning","无法创建新缓冲区");
        return NULL;
    }
    else
    {
        memset(newBuffer,0,sizeinByte);
        int rowCounter,columnCounter;
        //最邻近法
        if(type==1)
        {
            int oriX,oriY;
            for(rowCounter=0;rowCounter<newHeight;rowCounter++)
                for(columnCounter=0;columnCounter<newWidth;columnCounter++)
                {
                    oriX=min(int(double(columnCounter/factorX)+0.5),width-1);    //获取最邻近x
                    oriY=min(int(double(rowCounter/factorY)+0.5),height-1);      //获取最邻近y
                    *(newBuffer+rowCounter*newBytePerLine+columnCounter)=*(buffer+oriY*oldBytePerLine+oriX);   //获取对应像素值
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
                    x=min(int(double(columnCounter/factorX)),width-1);
                    dx=double(columnCounter/factorX)-x;
                    y1=min(int(double(rowCounter/factorY)),height-1);
                    y2=min(y1+1,height-1);
                    dy=double(rowCounter/factorY)-y1;
                    Ga=double(*(buffer+y1*oldBytePerLine+x))+double(dy*(int(*(buffer+y2*oldBytePerLine+x))-int(*(buffer+y1*oldBytePerLine+x))));
                    //右侧线性值
                    x=min(x+1,width-1);
                    Gb=double(*(buffer+y1*oldBytePerLine+x))+double(dy*(int(*(buffer+y2*oldBytePerLine+x))-int(*(buffer+y1*oldBytePerLine+x))));
                    //计算对应像素值
                    *(newBuffer+rowCounter*newBytePerLine+columnCounter)=uchar(int(Ga+dx*(Gb-Ga)));
                }
        }
        else
            return NULL;  //暂无
        return newBuffer;
    }
}

/***********************************
 * 3*3中值滤波
 * 若成功，将返回新的缓冲区地址
 * 若失败，返回空指针
 * ********************************/
uchar *MainWindow::MedianFiltering3x3(uchar *buffer,int width,int height)
{
    QVector<int> pixelSort;
    int row,column,windowRow,windowColumn;
    int bpl=(width+3)/4*4;
    uchar *newBuffer=(uchar *)malloc(bpl*height*sizeof(uchar));
    if(newBuffer==NULL)
        return NULL;
    memset(newBuffer,0,bpl*height);
    for(row=0;row<height;row++)
        for(column=0;column<width;column++)
        {
            pixelSort.clear();
            //获得需要排序的像素
            for(windowRow=row-1;windowRow<=row+1;windowRow++)
                for(windowColumn=column-1;windowColumn<=column+1;windowColumn++)
                {
                    if(windowColumn<0||windowRow<0||windowColumn>=width||windowRow>=height) //边缘像素
                        continue;
                    pixelSort.push_back(int(*(buffer+bpl*windowRow+windowColumn)));
                }
            //对像素值进行排序
            sort(pixelSort.begin(),pixelSort.end());
            *(newBuffer+bpl*row+column)=uchar(pixelSort.at(int(pixelSort.length()/2)+1));
        }
    return newBuffer;
}

/*********************************
 * 高斯平滑
 * 正常情况返回新的图像存储空间，否则返
 * 回空指针
 * ******************************/
/*
uchar *MainWindow::GaussianSmooth(uchar *buffer,int windowSize,int MSD,int width,int height)
{
    //新空间申请
    int row,column,windowRow,windowColumn;
    int bpl=(width+3)/4*4;
    uchar *newBuffer=(uchar *)malloc(bpl*height*sizeof(uchar));
    if(newBuffer==NULL)
        return NULL;
    memset(newBuffer,0,bpl*height);
    //计算权值矩阵以用于计算
    QVector<double> weightMatrix=GaussianMatrix(windowSize,MSD);
    //开始计算，图像边缘未考虑
    for(row=0+int(windowSize/2);row<height-int(windowSize/2);row++)
        for(column=0+int(windowSize/2);column<width-int(windowSize/2);column++)
        {
            //
            for(windowRow=row-1;windowRow<=row+1;windowRow++)
                for(windowColumn=column-1;windowColumn<=column+1;windowColumn++)
                {
                    if(windowColumn<0||windowRow<0||windowColumn>=width||windowRow>=height) //边缘像素
                        continue;
                    pixelSort.push_back(int(*(buffer+bpl*windowRow+windowColumn)));
                }
            //对像素值进行排序
            sort(pixelSort.begin(),pixelSort.end());
            *(newBuffer+bpl*row+column)=uchar(pixelSort.at(int(pixelSort.length()/2)+1));
        }
    return newBuffer;
}
*/
/*****************************
 * 计算高斯权值矩阵
 * **************************/
QVector<double> MainWindow::GaussianMatrix(int windowSize, int MSD)
{
    QVector<double> GaussianVector(windowSize*windowSize,0);
    //根据窗口大小设置中点，用于计算权值
    int median=int(windowSize/2);
    int row,column;
    //计算各个距离的点的权值
    for(row=0;row<windowSize;row++)
        for(column=0;column<windowSize;column++)
            ;//GaussianVector[row*windowSize+column]=exp(-((pow(column-median,2)+pow(row-median,2))/2*pow(MSD,2)))/(2*M_PI*pow(MSD,2));
    return GaussianVector;
}








