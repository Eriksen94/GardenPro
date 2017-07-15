#include "mainscene.h"
#include "scenerect.h"
#include "scenepoly.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QRectF>
#include <QDebug>
#include <QGraphicsItem>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QStyle>
#include <QLabel>
#include <QApplication>
#include <QPushButton>
#include <QDesktopWidget>
#include <QMenu>
#include <QDialog>
#include <QMessageBox>


MainScene::MainScene(QObject *parent)
    : QGraphicsScene(parent)
{
    //initial set up
    myMode = noAct;
    exact = false;
    safeClick = false;
    createPopLine();
    createPopRec();
    createPopScale();
}

void MainScene::setMode(MainScene::Mode mode)
{
    myMode = mode;
}

void MainScene::setExact(bool arg)
{
    exact = arg;
}

//clears selections and prepared for setting the scale via selecting an existing line
void MainScene::setScale(bool arg)
{
    settingScale = arg;
     foreach (QGraphicsItem *item, items(Qt::DescendingOrder)){
         item->setSelected(false);
     }
}

//returns state of the scale
bool MainScene::getScaleState()
{
    return scaleSet;
}

//set the scale and update objects when loading a saved file
void MainScene::scaleSetFromMem(double pix, double unit, bool set)
{
    pixScale = pix;
    unitScale = unit;
    scaleSet = set;

    emit scaleIsSet(scaleSet);
    if(scaleSet){
        foreach (QGraphicsItem *item, items(Qt::DescendingOrder)){
            item->setSelected(false);
            if(item->type() == 6){
                SceneLine *lineTest = dynamic_cast<SceneLine *>(item);
                lineTest->setUnitScale(pixScale, unitScale);
            }
            if(item->type()== 3){
                SceneRect *recTest = dynamic_cast<SceneRect *>(item);
                recTest->setUnitScale(pixScale,unitScale);
            }
            if(item->type() == 5){
                ScenePoly *pol = dynamic_cast<ScenePoly *>(item);
                pol->setUnitScale(pixScale, unitScale);
            }
        }
    }
}

double MainScene::getPixScale()
{
    return pixScale;
}

double MainScene::getUnitScale()
{
    return unitScale;
}

//sets global sceneLine to specifications when the pop up is closed
void MainScene::popLineClose()
{
    popLine->close();
    safeClick = true;
    eLength = scaleUnitToPix(spinLength->value());
    eAngle = spinAngle->value();
    if(eLength > 1){
        //use point setting since line position is always 0 when created
        qreal x = eLine.p1().x() + eLength * cos(eAngle * 3.141592 / 180);
        qreal y = eLine.p1().y() - eLength * sin(eAngle * 3.141592 / 180);
        QPointF pos = QPointF(x,y);
        eLine.setP2(pos);
        line->setLine(eLine);
        line->updateFlags();
    }
    else
        delete line;
}

//sets global sceneRect item to specifications for ERect pop up
void MainScene::popRecClose()
{
    popRec->close();
    safeClick = true;
    eHeight = scaleUnitToPix(spinHeight->value());
    eWidth = scaleUnitToPix(spinWidth->value());
    if(eHeight > 1 && eWidth > 1){
        //use point setting since rect position is always 0 when created
        qreal x = eRec.left() + eWidth;
        qreal y = eRec.top() + eHeight;
        eRec.setRight(x);
        eRec.setBottom(y);
        rectangle->setRect(eRec);
    }
}

//updates the number of pixels which represent the user set distance, sets scale bool for painting
void MainScene::popScaleClose()
{
    if(spinUnits->value() != 0){
        unitScale = spinUnits->value();
        settingScale = false;
        //update all existing items
        foreach (QGraphicsItem *item, items(Qt::DescendingOrder)){
            item->setSelected(false);
            if(item->type() == 6){
                SceneLine *lineTest = dynamic_cast<SceneLine *>(item);
                lineTest->setUnitScale(pixScale, unitScale);
            }
            if(item->type()== 3){
                SceneRect *recTest = dynamic_cast<SceneRect *>(item);
                recTest->setUnitScale(pixScale,unitScale);
            }
            if(item->type() == 5){
                ScenePoly *pol = dynamic_cast<ScenePoly *>(item);
                pol->setUnitScale(pixScale, unitScale);
            }
        }
        scaleSet = true;
        //emit signal to main window to update scale label
        emit scaleIsSet(scaleSet);
        popScale->close();
    }
    else
        //avoid scale not being set or being set to create invalid div 0 issues
        QMessageBox::information(
            popScale,
            tr("Setting scale"),
            tr("Select a non 0 value") );

}

void MainScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() != Qt::LeftButton)
        return;

    switch (myMode){
        case InsertLine:
        {
            //avoids 1 mouse event
            if(safeClick){
                return;
            }
            //set up line item, globalally available
            line = new SceneLine(QLineF(mouseEvent->scenePos(),mouseEvent->scenePos()));
            addItem(line);
            lastItem = line;
            //snap starting point
            QPointF point_ = mSnapPoint(line->line().p1());
            QLineF newLine(point_, mouseEvent->scenePos());
            line->setLine(newLine);

            //still use snapped point for eLine and open input window
            if(exact){
                eLine.setP1(newLine.p1());
                popLine->show();
            }
            //update scale info
            if(scaleSet){
                line->setUnitScale(pixScale, unitScale);
            }
            break;
        }
        case InsertRect:
        {
            //avoids 1 mouse event
            if(safeClick){
                return;
            }
             //set up rect item, globalally available
            rectangle = new SceneRect();
            rectangle->setRect(QRectF(mouseEvent->scenePos(),mouseEvent->scenePos()));
            addItem(rectangle);
            lastItem = rectangle;
            //snap start corner
            QPointF point_ = mSnapPoint(rectangle->rect().topLeft());
            //start points saved for the drawing function to keep track of which quadrant the mouse moved to
            startY = point_.ry();
            startX = point_.rx();
            lastQuadrant = 0;

            //exact always goes from top left corner
            if(exact){
                eRec.setTopLeft(point_);
                popRec->show();
            }
            //updates scale info
            if(scaleSet){
                rectangle->setUnitScale(pixScale, unitScale);
            }
            break;
        }
        case noAct:
        {
            break;
        }
        case Select:
        {
            //avoid other mouse click actions except when in select state
            QGraphicsScene::mousePressEvent(mouseEvent);
            break;
        }
    }
}

void MainScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    switch (myMode){
        case InsertLine:
        {
            //avoids 1 mouse event
            if(safeClick){
                return;
            }
            if(!exact){
                //update line to mouse position
                QLineF newLine(line->line().p1(), mouseEvent->scenePos());
                line->setLine(newLine);
            }
            break;
        }
        case InsertRect:
        {
            //avoids 1 mouse event
            if(safeClick){
                return;
            }
            //exact rect uses pop up info so no drawing beyond first click
            if(!exact){
                mDrawRect(mouseEvent);
            }
            break;
        }
        case noAct:
        {
            break;
        }
        case Select:
        {
            QGraphicsScene::mouseMoveEvent(mouseEvent);
        }

    }
}

void MainScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() != Qt::LeftButton)
        return;

    QPointF point_;

    switch (myMode){
        case InsertLine:
        {
            //1 event finished reset ignore flag
            if(safeClick){
                safeClick = false;
                return;
            }
            if(!exact){
                //snap end point and give precidence to point over line snapping
                QPointF lastP2 = line->line().p2();
                point_ = mSnapPoint(line->line().p2());
                if(lastP2 != point_){
                    QLineF newLine(line->line().p1(), point_);
                    line->setLine(newLine);
                }
                else{
                    //snap angles
                    qreal angle = angleSnap(line);
                    QLineF angleLine(line->line().p1(), line->line().p2());
                    angleLine.setAngle(angle);
                    line->setLine(angleLine);
                }
                //remove point clicks
                if(line->line().length() < 2){
                    delete line;
                }
                //add flags to line
                line->updateFlags();
                //connect lines into polygon, removes lines in connectingPath to avoid duplicating polygons
                QVector<QPointF> testCall = connectingPath(lastItem);
                if(!testCall.empty()){
                    sPoly = new ScenePoly(QPolygonF(testCall));
                    sPoly->setupPoly();
                    if(scaleSet){
                        sPoly->setUnitScale(pixScale, unitScale);
                    }
                    sPoly->setZValue(-1);
                    addItem(sPoly);
                }
            }
            break;
        }
        case InsertRect:
        {
            //1 event complete, reset flag
            if(safeClick){
                safeClick = false;
                return;
            }
            if(!exact){
                QRectF newRect;
                //snap ending corner, depends on how the drawing ends with reference to the first click
                //fix starting (opposite) corner and then set corner where the mouse ends
                switch(lastQuadrant)
                {
                case 1: //mouse ends top right
                    point_ = mSnapPoint(rectangle->rect().topRight());
                    newRect.setBottomLeft(rectangle->rect().bottomLeft());
                    newRect.setRight(point_.rx());
                    newRect.setTop(point_.ry());
                    break;
                case 2: //mouse ends top left
                    point_ = mSnapPoint(rectangle->rect().topLeft());
                    newRect.setBottomRight(rectangle->rect().bottomRight());
                    newRect.setLeft(point_.rx());
                    newRect.setTop(point_.ry());
                    break;
                case 3: //mouse ends bottom left
                    point_ = mSnapPoint(rectangle->rect().bottomLeft());
                    newRect.setTopRight(rectangle->rect().topRight());
                    newRect.setLeft(point_.rx());
                    newRect.setBottom(point_.ry());
                    break;
                case 4: //mouse ends bottom right
                    point_ = mSnapPoint(rectangle->rect().bottomRight());
                    newRect.setTopLeft(rectangle->rect().topLeft());
                    newRect.setRight(point_.rx());
                    newRect.setBottom(point_.ry());
                    break;
                default: break;
                }
                rectangle->setRect(newRect);
            }
            break;
        }
        case Select:
        {
          if(settingScale){
            foreach (QGraphicsItem *item, selectedItems()){
                if(item->type()==6){
                    //set scale to line length
                    QGraphicsLineItem *lineTest = dynamic_cast<QGraphicsLineItem *>(item);
                    pixScale = lineTest->line().length();
                    popScale->show();
                }
                else
                    item->setSelected(false);
            }
            QGraphicsScene::mouseReleaseEvent(mouseEvent);
          }
         break;
        }
        case noAct:
        {
            break;
        }
        default:
        {
            ;
        }
    }
}

//get a path of all connected points from the last item placed
//empty unless first and last point are the same, thus a complete path
QVector<QPointF> MainScene::connectingPath(QGraphicsItem *item)
{
    if(items(Qt::DescendingOrder).empty() || item->type() != 6){
        QVector<QPointF> path;
        path.clear();
        return path;
    }

    QList<QGraphicsItem *> itemList;
    itemList.append(items(Qt::DescendingOrder));
    QVector<QPointF> path;
    QVector<QPointF> linePoints;
    QPointF p1_,p2_;
    QVector<int> linkArray;
    int j = 0;
    int index = 0;

    //store all line items into a vector of their end points
    for(int i=0; i<itemList.length(); i++)
    {
        if(itemList[i]->type()==6){
            QGraphicsLineItem *testLine = dynamic_cast<QGraphicsLineItem *>(itemList[i]);
            linePoints.append(testLine->line().p1());
            linePoints.append(testLine->line().p2());
            linkArray.append(i);
        }
    }
    //remove any point sets that have a unique point since they can't be part of a closed shape
    //run until nothing is removed to avoid multi line branches that have liens with shared points
    for(int i=0; i<linePoints.length();i++)
    {
        //only 1 point in vector
        if(linePoints.count(linePoints[i]) == 1)
        {
            //p1, remove with next
            if(i%2 == 0){
                linkArray.remove(i/2);
                linePoints.remove(i,2);
                i += -1;
                j++;
            }
            //p2, remove with previous
            else{
                linkArray.remove(i/2);
                linePoints.remove(i-1,2);
                i += -2;
                j++;
            }
        }
        //got to end of the list and some poitns have been removed
        //run again to remove dead end branches
        if(i == linePoints.length()-1 && j!=0){
            i=0;
            j=0;
        }
    }
    //stop if the last item was removed since no polygon has been closed
    if(!linkArray.contains(0)) return path;

    //last line as start of path, remove from the linePoints as matches found to avoid repeats
    path.append(linePoints[0]);
    path.append(linePoints[1]);
    linePoints.remove(0,2);

    //find a point that matches the lasts path point and add that lines points to the path
    //all remaining points will be part of connected lines, just need to sort out the order
    j=1;
    while(linePoints.length() > 0)
    {
        //returns where in the point list a match to the point in the path list is
        index = linePoints.indexOf(path[j]);
        //found a loop around
        if(path.first() == path.last()) break;
        //invalid index, no more matches exist
        if(index == -1){
            break;
        }
        //matching p1, add p2 to path and remove from points
        else if(index%2 == 0){
            path.append(linePoints[index+1]);
            linePoints.remove(index,2);
        }
        //matching p2, add p1 to path and remove from points
        else{
            path.append(linePoints[index-1]);
            linePoints.remove(index-1,2);
        }
        j++;
    }

    //polygon failed if first and last point aren't the same
    if(path.first() != path.last()) path.clear();

    //delete lines so they dont become part of other polygons
    if(!path.empty()){
        for(int i=0; i<linkArray.length();i++){
            removeItem(itemList[linkArray[i]]);
            delete itemList[linkArray[i]];
        }
    }
    return path;
}

//draws a rectangle on the scene during mouse move events
void MainScene::mDrawRect(QGraphicsSceneMouseEvent *mouseEvent)
{
    QRectF newRect;
    //mouse moves right side of start point
    if(mouseEvent->scenePos().rx() > startX){
        //mouse moves above start: y inc down, x inc right
        if(mouseEvent->scenePos().ry() < startY){
            newRect.setTopRight(mouseEvent->scenePos());
            newRect.setLeft(startX);
            newRect.setBottom(startY);
            lastQuadrant = 1;
        }
        //mouse is below start point
        else{
            newRect.setLeft(startX);
            newRect.setTop(startY);
            newRect.setBottomRight(mouseEvent->scenePos());
            lastQuadrant = 4;
        }
    }
    //mouse moves left of start
    else{
        //mouse moves above start point
        if(mouseEvent->scenePos().ry() < startY){
            newRect.setTopLeft(mouseEvent->scenePos());
            newRect.setRight(startX);
            newRect.setBottom(startY);
            lastQuadrant = 2;
        }
        //mouse is below start point
        else{
            newRect.setRight(startX);
            newRect.setTop(startY);
            newRect.setBottomLeft(mouseEvent->scenePos());
            lastQuadrant = 3;
        }
    }
    //update
    rectangle->setRect(newRect);
}

double MainScene::scaleUnitToPix(double arg)
{
    double scaled;
    if(scaleSet){
        scaled = arg * pixScale / unitScale;
    }
    else
        scaled = arg;
    return scaled;
}

//returns the closest end point, within snapBoundRef distance, of an item to the point being tested,
QPointF MainScene::mSnapPoint(QPointF testPoint)
{
    //initialize snap point as the starting point
    QPointF snapPoint_ = testPoint;
    double deltX, deltY;
    QVector<QPointF> pointArr;
    QLineF newLine;
    QRectF newRect;
    snapBoundRef = 10;

    //copy the last item placed so it doesn't try to snap to itself when being added
    if(lastItem->type() == 6){
        newLine.setP1(line->line().p1());
        newLine.setP2(line->line().p2());
    }
    if(lastItem->type() == 3){
        newRect.setTopLeft(rectangle->rect().topLeft());
        newRect.setTopRight(rectangle->rect().topRight());
        newRect.setBottomLeft(rectangle->rect().bottomLeft());
        newRect.setBottomRight(rectangle->rect().bottomRight());
    }

    if(!items(Qt::DescendingOrder).empty())
    {
        foreach (QGraphicsItem *item, items(Qt::DescendingOrder))
        { //lists from most recently placed to first placed
          //line type 6, Rectangle type 3, Polygon type 5
          if(item->type() == 6)
          {
            QGraphicsLineItem *lineTest = dynamic_cast<QGraphicsLineItem *>(item);
            //don't consider the line being placed
            if(newLine.p1() != lineTest->line().p1() && newLine.p2() != lineTest->line().p2())
            {
                //calc distance from p1 to other lines p1 & p2
                deltX = testPoint.rx() - lineTest->line().p1().rx();
                deltY = testPoint.ry() - lineTest->line().p1().ry();
                snapBoundP1 = sqrt(pow(deltX,2)+pow(deltY,2));

                deltX = testPoint.rx() - lineTest->line().p2().rx();
                deltY = testPoint.ry() - lineTest->line().p2().ry();
                snapBoundP2 = sqrt(pow(deltX,2)+pow(deltY,2));

                //set snapPoint to closest end point and update reference so only closer points are found
                if(snapBoundP1 < snapBoundRef || snapBoundP2 < snapBoundRef)
                {
                    if(snapBoundP1 < snapBoundP2)
                    {
                        snapPoint_ = lineTest->line().p1();
                        snapBoundRef = snapBoundP1;
                    }
                    else
                    {
                        snapPoint_ = lineTest->line().p2();
                        snapBoundRef = snapBoundP2;
                    }
                }
            }
         }
          if(item->type()==3)
          {
              QGraphicsRectItem *rectTest = dynamic_cast<QGraphicsRectItem *>(item);
              if(newRect != rectTest->rect())
              {
                  //find rectangle snap points
                  pointArr.clear();
                  pointArr.append(rectTest->rect().bottomLeft());
                  pointArr.append(rectTest->rect().bottomRight());
                  pointArr.append(rectTest->rect().topLeft());
                  pointArr.append(rectTest->rect().topRight());
                  foreach(QPointF point, pointArr)
                  {
                      deltX = testPoint.rx() - point.rx();
                      deltY = testPoint.ry() - point.ry();
                      snapBoundP1 = sqrt(pow(deltX,2)+pow(deltY,2));
                      if(snapBoundP1 < snapBoundRef)
                      {
                          snapPoint_ = point;
                          snapBoundRef = snapBoundP1;
                      }
                  }
              }
          }
          if(item->type()==5)
          {
              //find polygon snap points
              pointArr.clear();
              QGraphicsPolygonItem *polyTest = dynamic_cast<QGraphicsPolygonItem *>(item);
              //polygon class is a vector or QPointF's
              pointArr = polyTest->polygon();
              foreach(QPointF point, pointArr)
              {
                  deltX = testPoint.rx() - point.rx();
                  deltY = testPoint.ry() - point.ry();
                  snapBoundP1 = sqrt(pow(deltX,2)+pow(deltY,2));
                  if(snapBoundP1 < snapBoundRef)
                  {
                      snapPoint_ = point;
                      snapBoundRef = snapBoundP1;
                  }
              }
          }
       }
    }

    return snapPoint_;
}

//return cardinal direction if the angle of a line is within the angleRange
qreal MainScene::angleSnap(QGraphicsLineItem *line_)
{
    qreal angle = line_->line().angle();
    int angleRange=10;
    qreal retAngle = angle;
    if(angle > 360-angleRange || angle < angleRange){
        retAngle = 0;
    }
    if(angle > 90-angleRange && angle < 90+angleRange){
        retAngle = 90;
    }
    if(angle > 180-angleRange && angle < 180+angleRange){
        retAngle = 180;
    }
    if(angle > 270-angleRange && angle < 270+angleRange){
        retAngle = 270;
    }
    return retAngle;
}

//popup used to get data when setting exact lines, created on closing function
void MainScene::createPopLine()
{
    popLine = new QMenu;
    popLine->setWindowFlag(Qt::Popup);
    //set to appear in middle of screen
    popLine->setFixedSize(250,100);
    popLine->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            popLine->size(),
            qApp->desktop()->availableGeometry()
        )
    );
    QFormLayout *layout = new QFormLayout;
    popLine->setLayout(layout);

    //length set
    QLabel *label1 = new QLabel("Length:", popLine);
    spinLength = new QDoubleSpinBox(popLine);
    spinLength->setButtonSymbols( QAbstractSpinBox::NoButtons);
    spinLength->setMaximum(1000);
    spinLength->setValue(0);

    //angle set
    QLabel *label2 = new QLabel("Angle (0-359.99):", popLine);
    spinAngle = new QDoubleSpinBox(popLine);
    spinAngle->setButtonSymbols( QAbstractSpinBox::NoButtons);
    spinAngle->setMaximum(359.99);

    //complete
    QLabel *label3 = new QLabel("complete?", popLine);
    QPushButton *confirm = new QPushButton("confirm", popLine);
    confirm->setCheckable(true);

    layout->addRow(label1, spinLength);
    layout->addRow(label2, spinAngle);
    layout->addRow(label3, confirm);

    connect(confirm, SIGNAL (clicked()), this, SLOT (popLineClose()));
    connect(popLine, SIGNAL (aboutToHide()), this, SLOT (popLineClose()));

}

//popup used to get data when setting exact rectangle, created on closing function
void MainScene::createPopRec()
{
    popRec = new QMenu;
    popRec->setWindowFlag(Qt::Popup);
    //set to appear in middle of screen
    popRec->setFixedSize(250,100);
    popRec->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            popLine->size(),
            qApp->desktop()->availableGeometry()
        )
    );
    QFormLayout *layout = new QFormLayout;
    popRec->setLayout(layout);

    //length set
    QLabel *label1 = new QLabel("Width: ", popRec);
    spinWidth = new QDoubleSpinBox(popRec);
    spinWidth->setButtonSymbols( QAbstractSpinBox::NoButtons);
    spinWidth->setMaximum(1000);
    spinWidth->setValue(0);

    //angle set
    QLabel *label2 = new QLabel("Height: ", popRec);
    spinHeight = new QDoubleSpinBox(popRec);
    spinHeight->setButtonSymbols( QAbstractSpinBox::NoButtons);
    spinHeight->setMaximum(1000);

    //complete
    QLabel *label3 = new QLabel("complete?", popRec);
    QPushButton *confirm = new QPushButton("confirm", popRec);
    confirm->setCheckable(true);

    layout->addRow(label1, spinWidth);
    layout->addRow(label2, spinHeight);
    layout->addRow(label3, confirm);

    connect(confirm, SIGNAL (clicked()), this, SLOT (popRecClose()));
    connect(popRec, SIGNAL (aboutToHide()), this, SLOT (popRecClose()));

}

//popup used when setting scale, scale set on closing function
void MainScene::createPopScale()
{
    popScale = new QDialog;
    popScale->setWindowFlag(Qt::Popup);
    //set to appear in middle of screen
    popScale->setFixedSize(300,100);
    popScale->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            popLine->size(),
            qApp->desktop()->availableGeometry()
        )
    );
    popScale->setModal(true);
    QFormLayout *layout = new QFormLayout;
    popScale->setLayout(layout);

    QLabel *label1 = new QLabel("How many units \n is this line: ", popScale);
    spinUnits = new QDoubleSpinBox(popScale);
    spinUnits->setButtonSymbols( QAbstractSpinBox::NoButtons);
    spinUnits->setMaximum(1000);
    spinUnits->setValue(0);

    //complete
    QLabel *label3 = new QLabel("complete?", popScale);
    QPushButton *confirm = new QPushButton("confirm", popScale);
    confirm->setCheckable(true);

    layout->addRow(label1, spinUnits);
    layout->addRow(label3, confirm);

    connect(confirm, SIGNAL (clicked()), this, SLOT (popScaleClose()));
}


