#include "addimpulsivenoise.h"
#include "ui_addimpulsivenoise.h"

addimpulsivenoise::addimpulsivenoise(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addimpulsivenoise)
{
    ui->setupUi(this);

    ui->conf->setTickInterval(1);
    ui->conf->setMaximum(80);
    ui->conf->setMinimum(1);
    setWindowTitle("冲击噪声");
    connect(ui->conf,SIGNAL(valueChanged(int)),this,SLOT(updateLabel()));
    connect(ui->sp,SIGNAL(clicked(bool)),this,SLOT(checkBoxStateChange1()));
    connect(ui->ri,SIGNAL(clicked(bool)),this,SLOT(checkBoxStateChange2()));
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(factorOK()));
}

addimpulsivenoise::~addimpulsivenoise()
{
    delete ui;
}

void addimpulsivenoise::factorOK()
{
    int type=0;
    if(ui->sp->isChecked())
        type=1;
    else if(ui->ri->isChecked())
        type=2;
    else
        return;
    emit noiseFactorOK(type,ui->conf->value());
}

void addimpulsivenoise::checkBoxStateChange1()
{
    if(ui->sp->isChecked())
        ui->ri->setChecked(false);
}

void addimpulsivenoise::checkBoxStateChange2()
{
    if(ui->ri->isChecked())
        ui->sp->setChecked(false);
}

void addimpulsivenoise::updateLabel()
{
    ui->conf_val->setText(QString::number(ui->conf->value()));
}
