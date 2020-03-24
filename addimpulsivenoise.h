#ifndef ADDIMPULSIVENOISE_H
#define ADDIMPULSIVENOISE_H

#include <QWidget>

namespace Ui {
class addimpulsivenoise;
}

class addimpulsivenoise : public QWidget
{
    Q_OBJECT
signals:
    void noiseFactorOK(int,int);

public:
    explicit addimpulsivenoise(QWidget *parent = 0);
    ~addimpulsivenoise();

public slots:
    void factorOK();
    void checkBoxStateChange1();
    void checkBoxStateChange2();
    void updateLabel();

private:
    Ui::addimpulsivenoise *ui;
};

#endif // ADDIMPULSIVENOISE_H
