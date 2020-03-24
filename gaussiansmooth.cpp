#include "gaussiansmooth.h"
#include "ui_gaussiansmooth.h"

GaussianSmooth::GaussianSmooth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GaussianSmooth)
{
    ui->setupUi(this);
    //初始化高斯平滑输入界面
    setWindowTitle("GaussianSmooth");
    connect(ui->ok,SIGNAL(clicked(bool)),this,SLOT(inputOK()));
}

GaussianSmooth::~GaussianSmooth()
{
    delete ui;
}

void GaussianSmooth::inputOK()
{
    if(ui->msd->text().isEmpty())
        return;
    emit gaussian(ui->msd->text().toDouble());
    return;
}
