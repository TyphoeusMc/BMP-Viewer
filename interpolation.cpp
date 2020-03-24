#include "interpolation.h"
#include "ui_interpolation.h"

interpolation::interpolation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::interpolation)
{
    ui->setupUi(this);
    ui->factorx->setTickInterval(1);
    ui->factorx->setMaximum(10);
    ui->factorx->setMinimum(1);
    ui->factory->setTickInterval(1);
    ui->factory->setMaximum(10);
    ui->factory->setMinimum(1);
    setWindowTitle("缩放");
    connect(ui->factorx,SIGNAL(valueChanged(int)),this,SLOT(updateLabel1()));
    connect(ui->factory,SIGNAL(valueChanged(int)),this,SLOT(updateLabel2()));
    connect(ui->checkBox,SIGNAL(clicked(bool)),this,SLOT(checkBoxStateChange1()));
    connect(ui->checkBox_2,SIGNAL(clicked(bool)),this,SLOT(checkBoxStateChange2()));
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(factorOK()));
}

void interpolation::factorOK()
{
    int state;
    if(ui->checkBox->isChecked())
        state=1;
    else if(ui->checkBox_2->isChecked())
        state=2;
    else
        return;
    emit getFactor(state,ui->factorx->value(),ui->factory->value());
}

void interpolation::checkBoxStateChange1()
{
    if(ui->checkBox_2->isChecked())
        ui->checkBox_2->setChecked(false);
}

void interpolation::checkBoxStateChange2()
{
    if(ui->checkBox->isChecked())
        ui->checkBox->setChecked(false);
}

void interpolation::updateLabel1()
{
    ui->x_val->setText(QString::number(ui->factorx->value()));
}

void interpolation::updateLabel2()
{
    ui->y_val->setText(QString::number(ui->factory->value()));
}

interpolation::~interpolation()
{
    delete ui;
}
