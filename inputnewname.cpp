#include "inputnewname.h"
#include "ui_inputnewname.h"

inputNewName::inputNewName(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::inputNewName)
{
    ui->setupUi(this);
    setWindowTitle("Save file");
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(confirmNameInput()));
}

inputNewName::~inputNewName()
{
    delete ui;
}

void inputNewName::confirmNameInput()
{
    std::cout<<ui->lineEdit->text().toStdString()<<std::endl;
    emit nameGot(ui->lineEdit->text());
}
