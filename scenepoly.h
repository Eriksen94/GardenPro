#ifndef SCENEPOLY_H
#define SCENEPOLY_H

#include <QGraphicsPolygonItem>
#include "datawindow.h"

QT_BEGIN_NAMESPACE
class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QString;
QT_END_NAMESPACE

class ScenePoly : public QGraphicsPolygonItem
{

public:
    //overload functions aren't found through the derived class
    using QGraphicsPolygonItem::QGraphicsPolygonItem;
    ScenePoly(QGraphicsItem *parent = 0);
    void setupPoly();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void updateName(QString arg);
    void setUnitScale(float pix, float unit);
    QGraphicsItem *gitem = dynamic_cast<QGraphicsItem *>(this);
    DataWindow *data = new DataWindow(gitem);
    void setShowDim(bool arg);

private:
    int contextCount=0;
    double area;
    QString plotName;
    bool scaleSet=false;
    float pixScale, unitScale;
    float pixToUnit(float arg);
    bool dimShow = true;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private slots:

signals:


};

#endif // SCENEPOLY_H
