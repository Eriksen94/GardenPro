#include "scenerect.h"
#include "datawindow.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QtWidgets>
#include <QDebug>
//! [0]

SceneRect::SceneRect(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    plotName.clear();

}

//black edges, green fill, red when selected
void SceneRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem *testRect = dynamic_cast<QGraphicsRectItem *>(this);
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);
    QBrush greenBrush(Qt::green);
    QPen dottedPen(Qt::red);
    QPen redPen(Qt::red);
    redPen.setWidth(3);
    dottedPen.setWidth(2);
    dottedPen.setStyle(Qt::DashLine);

    //basic paint set up
    QRectF rec = QRectF(rect());
    Q_UNUSED(widget);
    painter->setPen(blackPen);
    painter->setBrush(greenBrush);
    painter->drawRect(rec);

    //add measurements if scale set and dim set to show
    if(scaleSet && showDim){
        height = rec.height();
        width = rec.width();
        if(height > 25 && width > 30){
            height = pixToUnit(height);
            width = pixToUnit(width);
            QPointF posH = QPointF(testRect->rect().right() - 30, testRect->boundingRect().center().y());
            QPointF posW = QPointF(testRect->boundingRect().center().x()-10, testRect->rect().top() + 20);
            painter->setPen(redPen);
            float h = height;
            h = roundf(h * 100) / 100;
            float w = width;
            w = roundf(w * 100) / 100;
            painter->drawText(posH, QString("%1").arg(h));
            painter->drawText(posW, QString("%1").arg(w));
        }
    }

    //handle states/cases
    if (option->state & QStyle::State_Selected)
    {
      painter->setPen(dottedPen);
      painter->drawRect(rec);
    }
    if(!plotName.isEmpty()){
        QPointF pos = QPointF(testRect->boundingRect().center().x()-10, testRect->boundingRect().center().y());
        painter->setPen(blackPen);
        painter->drawText(pos, plotName);
    }
}

void SceneRect::updateName(QString arg)
{
    plotName = arg;
    update();
}

//sets scale
void SceneRect::setUnitScale(float pix, float unit)
{
    pixScale = pix;
    unitScale = unit;
    scaleSet = true;
    update();
    //update area in data window as well, needed for when loading settings
    double w,h,a;
    QGraphicsRectItem *testRect = dynamic_cast<QGraphicsRectItem *>(this);
    QRectF *rec = new QRectF(testRect->rect());
    h = pixToUnit(rec->height());
    w = pixToUnit(rec->width());
    a = h*w;
    data->setArea(a);
}

void SceneRect::setShowDim(bool arg)
{
    showDim = arg;
}

//converts scene scale to user set
float SceneRect::pixToUnit(float arg)
{
    float ret;
    ret = arg * unitScale / pixScale;
    return ret;
}

//opens data window on right click and sets area
void SceneRect::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    if(scaleSet){
        double w,h,a;
        QGraphicsRectItem *testRect = dynamic_cast<QGraphicsRectItem *>(this);
        QRectF *rec = new QRectF(testRect->rect());
        h = pixToUnit(rec->height());
        w = pixToUnit(rec->width());
        a = h*w;
        data->setArea(a);
    }

    data->show();
    //data->setFocus();
    setSelected(true);
}




