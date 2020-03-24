#ifndef INPUTNEWNAME_H
#define INPUTNEWNAME_H

#include <QWidget>
#include <iostream>

namespace Ui {
class inputNewName;
}

class inputNewName : public QWidget
{
    Q_OBJECT

public:
    explicit inputNewName(QWidget *parent = 0);
    ~inputNewName();

signals:
    void nameGot(QString name);

public slots:
    void confirmNameInput();

private:
    Ui::inputNewName *ui;
};

#endif // INPUTNEWNAME_H
