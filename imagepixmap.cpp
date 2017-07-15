#include "imagepixmap.h"
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QEvent>
#include <QDebug>

ImagePixMap::ImagePixMap(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
{

}

//dont do anything unless the image is being scaled. dragging still works from scene
void ImagePixMap::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(rescaling){
        QGraphicsItem::mousePressEvent(event);
    }
    else{

    }
}

void ImagePixMap::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    //on context menu, select and show resizable corner stretchers
    setSelected(true);
    rescaling = true;
}

