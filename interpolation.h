#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include <QWidget>

namespace Ui {
class interpolation;
}

class interpolation : public QWidget
{
    Q_OBJECT
signals:
    void getFactor(int state,int x, int y);
public slots:
    void factorOK();
    void checkBoxStateChange1();
    void checkBoxStateChange2();
    void updateLabel1();
    void updateLabel2();
public:
    explicit interpolation(QWidget *parent = 0);
    ~interpolation();

private:
    Ui::interpolation *ui;
};

#endif // INTERPOLATION_H
