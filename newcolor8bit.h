#ifndef NEWCOLOR8BIT_H
#define NEWCOLOR8BIT_H

#include <QWidget>

namespace Ui {
class newColor8bit;
}

class newColor8bit : public QWidget
{
    Q_OBJECT
signals:
    void newIndex(int index);
public:
    explicit newColor8bit(int index, QWidget *parent = 0);
    ~newColor8bit();
public slots:
    void indexChange();

private:
    Ui::newColor8bit *ui;
};

#endif // NEWCOLOR8BIT_H
