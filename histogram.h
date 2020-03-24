#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QWidget>
#include "qcustomplot.h"

namespace Ui {
class histogram;
}

class histogram : public QWidget
{
    Q_OBJECT

public:
    explicit histogram(QVector<double> histo,QWidget *parent = 0);
    ~histogram();

private:
    Ui::histogram *ui;
};

#endif // HISTOGRAM_H
