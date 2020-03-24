#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QWidget>
#include "bmpimagestructure.h"

namespace Ui {
class colorPalette;
}

class colorPalette : public QWidget
{
    Q_OBJECT

public:
    explicit colorPalette(QWidget *parent = 0);
    ~colorPalette();
    void initializeTable();

private:
    Ui::colorPalette *ui;
};

#endif // COLORPALETTE_H
