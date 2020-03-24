#ifndef GAUSSIANSMOOTH_H
#define GAUSSIANSMOOTH_H

#include <QWidget>

namespace Ui {
class GaussianSmooth;
}

class GaussianSmooth : public QWidget
{
    Q_OBJECT

public:
    explicit GaussianSmooth(QWidget *parent = 0);
    ~GaussianSmooth();

signals:
    void gaussian(double);

public slots:
    void inputOK();

private:
    Ui::GaussianSmooth *ui;
};

#endif // GAUSSIANSMOOTH_H
