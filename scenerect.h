#ifndef SCENERECT_H
#define SCENERECT_H

#include <QGraphicsRectItem>
#include <QPaintDevice>
#include "datawindow.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QGraphicsItem;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QRectF;
class QString;
QT_END_NAMESPACE

class SceneRect : public QGraphicsRectItem
{

public:
    SceneRect(QGraphicsItem *parent = 0);
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    void updateName(QString arg);
    void setUnitScale(float pix, float unit);
    QGraphicsItem *gitem = dynamic_cast<QGraphicsItem *>(this);
    DataWindow *data = new DataWindow(gitem);
    void setShowDim(bool arg);

private:
    QString plotName;
    float height, width;
    bool scaleSet=false;
    float pixScale, unitScale;
    float pixToUnit(float arg);
    bool showDim = true;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

signals:


};

#endif // SCENERECT_H
