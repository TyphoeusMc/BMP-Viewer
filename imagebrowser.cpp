#include "imagebrowser.h"
#include "ui_imagebrowser.h"

imageBrowser::imageBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::imageBrowser)
{
    ui->setupUi(this);
    setWindowTitle("imageBrowser");
}

imageBrowser::~imageBrowser()
{
    delete ui;
}
