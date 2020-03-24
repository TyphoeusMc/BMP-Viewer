#include "bmpinfo.h"
#include "ui_bmpinfo.h"

extern BMPImage changedImage;
#define infoCount 14

BMPinfo::BMPinfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BMPinfo)
{
    ui->setupUi(this);
    setWindowTitle("BMP信息");
    initializeTable();
}

BMPinfo::~BMPinfo()
{
    delete ui;
}

void BMPinfo::initializeTable()
{
    //get the list prepared
    ui->tableWidget->setColumnCount(2);
    QStringList header;
    header<<tr("参数")<<tr("数据");
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setColumnWidth(0,150);
    //ui->tableWidget->setColumnWidth(1,400);
    //fill the table
    ui->tableWidget->setRowCount(infoCount);  //14 lines of data (if no addition)
    ui->tableWidget->setItem(0,0,new QTableWidgetItem("bfType"));
    ui->tableWidget->setItem(0,1,new QTableWidgetItem(QString::number(changedImage.fileHeader.bfType,16)));
    ui->tableWidget->setItem(1,0,new QTableWidgetItem("bfSize"));
    ui->tableWidget->setItem(1,1,new QTableWidgetItem(QString::number(changedImage.fileHeader.bfSize)));
    ui->tableWidget->setItem(2,0,new QTableWidgetItem("bfOffBits"));
    ui->tableWidget->setItem(2,1,new QTableWidgetItem(QString::number(changedImage.fileHeader.bfOffBits)));
    ui->tableWidget->setItem(3,0,new QTableWidgetItem("biSize"));
    ui->tableWidget->setItem(3,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biSize)));
    ui->tableWidget->setItem(4,0,new QTableWidgetItem("biWidth"));
    ui->tableWidget->setItem(4,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biWidth)));
    ui->tableWidget->setItem(5,0,new QTableWidgetItem("biHeight"));
    ui->tableWidget->setItem(5,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biHeight)));
    ui->tableWidget->setItem(6,0,new QTableWidgetItem("biPlanes"));
    ui->tableWidget->setItem(6,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biPlanes)));
    ui->tableWidget->setItem(7,0,new QTableWidgetItem("biBitCount"));
    ui->tableWidget->setItem(7,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biBitCount)));
    ui->tableWidget->setItem(8,0,new QTableWidgetItem("biCompression"));
    ui->tableWidget->setItem(8,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biCompression)));
    ui->tableWidget->setItem(9,0,new QTableWidgetItem("biSizeImage"));
    ui->tableWidget->setItem(9,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biSizeImage)));
    ui->tableWidget->setItem(10,0,new QTableWidgetItem("biXPelsPerMeter"));
    ui->tableWidget->setItem(10,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biXPelsPerMeter)));
    ui->tableWidget->setItem(11,0,new QTableWidgetItem("biYPelsPerMeter"));
    ui->tableWidget->setItem(11,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biYPelsPerMeter)));
    ui->tableWidget->setItem(12,0,new QTableWidgetItem("biClrUsed"));
    ui->tableWidget->setItem(12,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biClrUsed)));
    ui->tableWidget->setItem(13,0,new QTableWidgetItem("biClrImportant"));
    ui->tableWidget->setItem(13,1,new QTableWidgetItem(QString::number(changedImage.infoHeader.biClrImportant)));
    //set table format
    int row,column;
    for(row=0;row<infoCount;row++)
        for(column=0;column<2;column++)
        {
            ui->tableWidget->item(row,column)->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->item(row,column)->setTextColor(QColor(255,255,255));
            if(row%2==0)
                ui->tableWidget->item(row,column)->setBackgroundColor(QColor(160,160,160));
        }
}







