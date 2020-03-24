#include "openfile.h"
#include "ui_openfile.h"

openFile::openFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::openFile)
{
    ui->setupUi(this);
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(openButtonClick()));
}

openFile::~openFile()
{
    delete ui;
}

void openFile::openButtonClick()
{
    emit addPath(ui->lineEdit->text());
}
