#ifndef BILATERALFILTER_H
#define BILATERALFILTER_H

#include <QWidget>

namespace Ui {
class bilateralFilter;
}

class bilateralFilter : public QWidget
{
    Q_OBJECT

signals:
    void bilateralfilter(int,double,double);
public:
    explicit bilateralFilter(QWidget *parent = 0);
    ~bilateralFilter();
public slots:
    void inputOK();
private:
    Ui::bilateralFilter *ui;
};

#endif // BILATERALFILTER_H
