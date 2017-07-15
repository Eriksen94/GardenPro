#include "sceneline.h"
#include <QGraphicsLineItem>
#include <QPen>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

SceneLine::SceneLine(QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
{

}

//draw black, set red when selected, paint dimensions as well
void SceneLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsLineItem *testLine = dynamic_cast<QGraphicsLineItem *>(this);
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);
    QPen redPen(Qt::red);
    redPen.setWidth(2);
    QPen dottedPen(Qt::red);
    dottedPen.setWidth(3);
    dottedPen.setStyle(Qt::DashLine);

    QLineF lin = QLineF(line());
    Q_UNUSED(widget);
    painter->setPen(blackPen);
    painter->drawLine(lin);

    //add dimensions if scale is set and dimensions set to show
    if(scaleSet && showDim){
        float len = pixToUnit(lin.length());
        len = roundf(len * 100) / 100;

        if(len>0){
            //position tet, bounding rect has been modified below to provide a minimum size to fit text
            QString dim = QString("%1").arg(len);
            QPointF pos = QPointF(testLine->boundingRect().center().x()-15, testLine->boundingRect().center().y()+10);
            painter->setPen(redPen);
            painter->drawText(pos, dim);
        }
    }

    if (option->state & QStyle::State_Selected)
    {
      painter->setPen(dottedPen);
      painter->drawLine(lin);
    }

}

void SceneLine::updateFlags()
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setSelected(false);
}

//set a minimum size for vertical/horizontal lines that wouldn't fit dim text
QRectF SceneLine::boundingRect() const
{
    QRectF baseRec = shape().controlPointRect();
    QRectF newRec = baseRec;
    float width, height;
    width = newRec.width();
    height = newRec.height();
    //minimum bounding rect size needed to paint text above line
    if(width < 30){
        newRec.setRight(newRec.right()+40);

    }
    if(height < 20){
        newRec.setTop(newRec.top()-25);
    }
    return newRec;
}

//sets the scale, used by all items, updates to repaint dimensions
void SceneLine::setUnitScale(float pix, float unit)
{
    pixScale = pix;
    unitScale = unit;
    scaleSet = true;
    update();
}

//toggle dimensions on/off via painter update
void SceneLine::setShowDim(bool arg)
{
    showDim = arg;
}

//converts scene scale being drawn on to user defined scale
float SceneLine::pixToUnit(float arg)
{
    float ret;
    ret = arg * unitScale / pixScale;
    return ret;
}

