#include "bilateralfilter.h"
#include "ui_bilateralfilter.h"

bilateralFilter::bilateralFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::bilateralFilter)
{
    ui->setupUi(this);
    ui->windowsize->addItems(QStringList(QString("3,5,7,9,11,13,15").split(",")));
    ui->windowsize->setCurrentIndex(0);
    setWindowTitle("BilateralFilter");
    connect(ui->ok,SIGNAL(clicked(bool)),this,SLOT(inputOK()));
}

bilateralFilter::~bilateralFilter()
{
    delete ui;
}

void bilateralFilter::inputOK()
{
    if(ui->sigd->text().isEmpty()||ui->sigr->text().isEmpty())
        return;
    emit bilateralfilter(ui->windowsize->currentText().toInt(),ui->sigd->text().toDouble(),ui->sigr->text().toDouble());
    return;
}
