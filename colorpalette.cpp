#include "colorpalette.h"
#include "ui_colorpalette.h"

extern BMPImage changedImage;

colorPalette::colorPalette(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::colorPalette)
{
    ui->setupUi(this);
    setWindowTitle("color palette");
    initializeTable();
}

colorPalette::~colorPalette()
{
    delete ui;
}

void colorPalette::initializeTable()
{
    //get the list prepared
    ui->tableWidget->setColumnCount(5);
    QStringList header;
    header<<tr("索引")<<tr("B")<<tr("G")<<tr("R")<<tr("ALPHA");
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setColumnWidth(0,60);
    ui->tableWidget->setColumnWidth(1,110);
    ui->tableWidget->setColumnWidth(2,110);
    ui->tableWidget->setColumnWidth(3,110);
    //ui->tableWidget->setColumnWidth(1,400);
    //fill the table
    ui->tableWidget->setRowCount(256);  //256 lines of data
    int row,column;
    for(row=0;row<256;row++)
    {
        ui->tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(row)));
        for(column=1;column<5;column++)
            ui->tableWidget->setItem(row,column,new QTableWidgetItem(QString::number(int(*(changedImage.colorTable+row*4+column-1)))));
    }
    //set table format
    for(row=0;row<256;row++)
        for(column=0;column<5;column++)
        {
            ui->tableWidget->item(row,column)->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->item(row,column)->setTextColor(QColor(255,255,255));
        }
}
