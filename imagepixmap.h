#ifndef IMAGEPIXMAP_H
#define IMAGEPIXMAP_H

#include <QGraphicsPixmapItem>

QT_BEGIN_NAMESPACE
class QGraphicsPixmapItem;
class QGraphicsItem;
QT_END_NAMESPACE

class ImagePixMap : public QGraphicsPixmapItem
{
public:
    using QGraphicsPixmapItem::QGraphicsPixmapItem;
    ImagePixMap(QGraphicsItem *parent = 0);
    //void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
private:
    bool rescaling=false;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;



};

#endif // IMAGEPIXMAP_H
