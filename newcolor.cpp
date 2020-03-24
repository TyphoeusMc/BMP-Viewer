#include "newcolor.h"
#include "ui_newcolor.h"

//自定义带参构造函数
newColor::newColor(int R, int G, int B, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::newColor)
{
    ui->setupUi(this);
    ui->label_2->setText(QString::number(R));
    ui->label_5->setText(QString::number(G));
    ui->label_8->setText(QString::number(B));
    setWindowTitle("NewColor(24bit)");
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(okPress()));
    connect(ui->pushButton_2,SIGNAL(clicked(bool)),this,SLOT(clearPress()));
}

void newColor::clearPress()
{
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
}

void newColor::okPress()
{
    if(ui->lineEdit->text().isEmpty()||ui->lineEdit_2->text().isEmpty()||ui->lineEdit_3->text().isEmpty())
        return;
    emit colorChange(ui->lineEdit->text().toInt(),
                     ui->lineEdit_2->text().toInt(),
                     ui->lineEdit_3->text().toInt());
}

newColor::~newColor()
{
    delete ui;
}
