#include "newcolor8bit.h"
#include "ui_newcolor8bit.h"

newColor8bit::newColor8bit(int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::newColor8bit)
{
    ui->setupUi(this);
    ui->label_2->setText(QString::number(index));
    setWindowTitle("NewColor(8bit)");
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(indexChange()));
}

void newColor8bit::indexChange()
{
    if(!ui->lineEdit->text().isEmpty())
        emit newIndex(ui->lineEdit->text().toInt());
}

newColor8bit::~newColor8bit()
{
    delete ui;
}
