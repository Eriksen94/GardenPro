#ifndef DATAWINDOW_H
#define DATAWINDOW_H

#include <QWidget>
#include <QGraphicsItem>
#include "taskdata.h"

namespace Ui {
class DataWindow;
class QString;
class QGraphicsItem;
class QVector;
}

class DataWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DataWindow(QGraphicsItem *item);
    ~DataWindow();
    void setArea(double a);
    const QVector<QString> collectData();
    void setData(QVector<QString> dataFill);
    TaskData *taskArea;

private slots:
    void on_seedSpace_valueChanged(double arg1);
    void on_plotName_textChanged(const QString &arg1);
    void on_seeds_textEdited(const QString &arg1);

protected:
    void focusOutEvent(QFocusEvent *event);

private:
    Ui::DataWindow *ui;
    double darea=0;
    QString plotN;
    QGraphicsItem *parentItem;
    bool ignore=false;

signals:

};

#endif // DATAWINDOW_H
