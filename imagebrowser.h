#ifndef IMAGEBROWSER_H
#define IMAGEBROWSER_H

#include <QWidget>

namespace Ui {
class imageBrowser;
}

class imageBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit imageBrowser(QWidget *parent = 0);
    ~imageBrowser();

private:
    Ui::imageBrowser *ui;
};

#endif // IMAGEBROWSER_H
