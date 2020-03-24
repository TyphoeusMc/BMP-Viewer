#include "histogram.h"
#include "ui_histogram.h"

QVector <double> x_axis;
QVector <double> baseLine(256,0);
QBrush BGChisto(QColor(90,90,90));
QBrush FillGraph(QColor(255,255,255,100));
QPen GChisto(QColor(255,255,255),1);
QPen GCbaseLine(QColor(255,255,255),0);

histogram::histogram(QVector<double> histo,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::histogram)
{
    ui->setupUi(this);
    setWindowTitle("Histogram");
    x_axis.clear();
    for(int i=0;i<256;i++)
        x_axis.push_back(i);
    double max=histo.at(0);
    for(auto a:histo)
        if(a>max)
            max=a;
    //初始化图表属性
    ui->histo->addGraph();
    ui->histo->addGraph();
    ui->histo->graph(0)->setData(x_axis,histo);
    ui->histo->graph(1)->setData(x_axis,baseLine);
    ui->histo->setBackground(BGChisto);
    ui->histo->graph(0)->setPen(GChisto);
    ui->histo->graph(1)->setPen(GCbaseLine);
    ui->histo->graph(0)->setBrush(FillGraph);
    ui->histo->xAxis->setVisible(false);
    ui->histo->yAxis->setVisible(false);
    ui->histo->axisRect()->setMinimumMargins(QMargins(0,20,0,10));
    ui->histo->xAxis->setRange(0,255);
    ui->histo->yAxis->setRange(0,max);
    ui->histo->xAxis->setPadding(1);
    ui->histo->yAxis->setPadding(1);

    //绘制直方图
    ui->histo->replot();
    ui->histo->graph(0)->setChannelFillGraph(ui->histo->graph(1));
}

histogram::~histogram()
{
    delete ui;
}
