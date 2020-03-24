#ifndef BMPINFO_H
#define BMPINFO_H

#include <QWidget>
//#include <Windows.h>
#include "bmpimagestructure.h"

namespace Ui {
class BMPinfo;
}

class BMPinfo : public QWidget
{
    Q_OBJECT

public:
    explicit BMPinfo(QWidget *parent = 0);
    ~BMPinfo();
    void initializeTable();

private:
    Ui::BMPinfo *ui;
};

#endif // BMPINFO_H
