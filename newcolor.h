#ifndef NEWCOLOR_H
#define NEWCOLOR_H

#include <QWidget>

namespace Ui {
class newColor;
}

class newColor : public QWidget
{
    Q_OBJECT

public:
    explicit newColor(int R, int G, int B, QWidget *parent = 0);
    ~newColor();
signals:
    void colorChange(int R, int G, int B);

public slots:
    void okPress();
    void clearPress();
private:
    Ui::newColor *ui;
};

#endif // NEWCOLOR_H
