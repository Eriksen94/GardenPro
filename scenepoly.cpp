#include "scenepoly.h"
#include "scenerect.h"
#include "datawindow.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsPolygonItem>
#include <QPainter>
#include <QtWidgets>
#include <QDebug>
#include <QRectF>

ScenePoly::ScenePoly(QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent)
{
    plotName.clear();
}

void ScenePoly::setupPoly()
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//black edges, green interior, red when selected, dimensions taken care of by sceneLines left around the edges
void ScenePoly::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    QGraphicsPolygonItem *testPoly = dynamic_cast<QGraphicsPolygonItem *>(this);
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);
    QBrush greenBrush(Qt::green);
    QPen dottedPen(Qt::red);
    dottedPen.setWidth(2);
    dottedPen.setStyle(Qt::DashLine);
    QPen redPen(Qt::red);
    redPen.setWidth(3);

    QPolygonF poly = QPolygonF(polygon());

    painter->setPen(blackPen);
    painter->setBrush(greenBrush);
    painter->drawPolygon(poly, Qt::OddEvenFill);

    //dimension lines
    if(scaleSet && dimShow){
        QVector<QPointF> verticies = this->polygon();
        int lines = verticies.length()-1;
        for(int i=0;i<lines;i++){
            QPointF p1 = verticies[i];
            QPointF p2 = verticies[i+1];
            qreal x, y;
            //calculate length between verticies
            float deltX = p1.rx() - p2.rx();
            float deltY = p1.ry() - p2.ry();
            float len = sqrt(pow(deltX,2)+pow(deltY,2));
            //find midpoint to insert dimension
             x = (p1.rx()+p2.rx())/2;
             y = (p1.ry()+p2.ry())/2;
             QPointF pos = QPointF(x,y);
             len = pixToUnit(len);
             len = roundf(len * 100) / 100;
             QString dim = QString("%1").arg(len);
             painter->setPen(redPen);
             painter->drawText(pos, dim);
        }
    }

    if (option->state & QStyle::State_Selected){
        painter->setPen(dottedPen);
        painter->drawPolygon(poly, Qt::OddEvenFill);
    }
    if(!plotName.isEmpty()){
        QPointF pos = QPointF(testPoly->boundingRect().center().x()-10, testPoly->boundingRect().center().y());
        painter->setPen(blackPen);
        painter->drawText(pos, plotName);
    }
}

void ScenePoly::updateName(QString arg)
{
    plotName = arg;
    update();
}

void ScenePoly::setUnitScale(float pix, float unit)
{
    pixScale = pix;
    unitScale = unit;
    scaleSet = true;
    update();
    //update area as well for when loading settings and scale is set but standard context even not called
    int count=0;
    if(contextCount == 0){
        //create 1 1x1 polygon and cascade throughout the polygons bounding rectangle counting area in 1x1 blocks
        for (int x = boundingRect().x() - 1; x < boundingRect().x()+boundingRect().width(); x++)
        {
            for (int y = boundingRect().y() - 1; y < boundingRect().y()+boundingRect().height(); y++)
            {
                QRectF rect(x, y, 1, 1);
                QPolygonF cell(rect);
                QPolygonF intersection = polygon().intersected(cell);
                if (!intersection.empty())
                {
                    // Cell is fully contained
                    count++;
                }
            }
        }
        contextCount++;
        //count is # of 1x1 boxes in the polygon, modify to units
        area = pow(pixToUnit(sqrt(count)),2);
        data->setArea(area);
    }
}

void ScenePoly::setShowDim(bool arg)
{
    dimShow = arg;
}

//converts scene scale to user set scale
float ScenePoly::pixToUnit(float arg)
{
    float ret;
    ret = arg * unitScale / pixScale;
    return ret;
}

//open data window on right click and sets the area
void ScenePoly::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    if(scaleSet){
        int count=0;
        if(contextCount == 0){
            //create 1 1x1 polygon and cascade throughout the polygons bounding rectangle counting area in 1x1 blocks
            for (int x = boundingRect().x() - 1; x < boundingRect().x()+boundingRect().width(); x++)
            {
                for (int y = boundingRect().y() - 1; y < boundingRect().y()+boundingRect().height(); y++)
                {
                    QRectF rect(x, y, 1, 1);
                    QPolygonF cell(rect);
                    QPolygonF intersection = polygon().intersected(cell);
                    if (!intersection.empty())
                    {
                        // Cell is fully contained
                        count++;
                    }
                }
            }
            contextCount++;
            //count is # of 1x1 boxes in the polygon, modify to units
            area = pow(pixToUnit(sqrt(count)),2);
            data->setArea(area);
        }
    }
    data->show();
    setSelected(true);
}
