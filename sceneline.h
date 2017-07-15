#ifndef SCENELINE_H
#define SCENELINE_H

#include <QGraphicsLineItem>

class SceneLine : public QGraphicsLineItem
{
public:
    using QGraphicsLineItem::QGraphicsLineItem;
    SceneLine(QGraphicsItem *parent = 0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void updateFlags();
    QRectF boundingRect() const;
    void setUnitScale(float pix, float unit);
    void setShowDim(bool arg);

private:
    float pixToUnit(float arg);
    bool scaleSet=false;
    float pixScale, unitScale;
    bool showDim = true;


protected:

};

#endif // SCENELINE_H
