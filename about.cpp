#include "about.h"
#include "ui_about.h"

about::about(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::about)
{
    ui->setupUi(this);
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(okClick()));
    setWindowTitle("关于");
}

about::~about()
{
    delete ui;
}

void about::okClick()
{
    this->close();
}
