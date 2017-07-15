#include "diagramscene.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>

//! [0]
DiagramScene::DiagramScene(QMenu *itemMenu, QObject *parent)
    : QGraphicsScene(parent)
{

}
