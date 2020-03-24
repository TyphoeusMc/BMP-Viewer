#ifndef OPENFILE_H
#define OPENFILE_H

#include <QWidget>

namespace Ui {
class openFile;
}

class openFile : public QWidget
{
    Q_OBJECT

signals:
    void addPath(QString);

public:
    explicit openFile(QWidget *parent = 0);
    ~openFile();

public slots:
    void openButtonClick();

private:
    Ui::openFile *ui;
};

#endif // OPENFILE_H
