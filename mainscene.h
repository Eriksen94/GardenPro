#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <QGraphicsScene>
#include "scenerect.h"
#include "scenepoly.h"
#include "sceneline.h"

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QLineF;
class QDoubleSpinBox;
class QRectF;
class QDialog;
QT_END_NAMESPACE

//! [0]
class MainScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode {noAct, InsertRect, InsertLine, Select};
    explicit MainScene(QObject *parent = 0);
    void setMode(Mode mode);
    void setExact(bool arg);
    void setScale(bool arg);
    bool getScaleState();
    void scaleSetFromMem(double pix, double unit, bool set);
    double getPixScale();
    double getUnitScale();
public slots:
    void popLineClose();
    void popRecClose();
    void popScaleClose();

signals:
    void scaleIsSet(bool arg);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mDrawRect(QGraphicsSceneMouseEvent *mouseEvent);
    double scaleUnitToPix(double arg);
    QPointF mSnapPoint(QPointF testPoint);
    QVector<QPointF> connectingPath(QGraphicsItem *item);
    qreal angleSnap(QGraphicsLineItem *line_);

private:
    SceneLine *line;
    SceneRect *rectangle;
    ScenePoly *sPoly;

    int count;
    int snapBoundP1, snapBoundP2, snapBoundRef;
    qreal startX;
    qreal startY;
    qreal testX;
    qreal testY;
    int lastQuadrant;

    Mode myMode;
    QGraphicsItem *lastItem;

    bool exact;
    QMenu *popLine;
    void createPopLine();
    double eLength, eAngle;
    QLineF eLine;
    QDoubleSpinBox *spinLength;
    QDoubleSpinBox *spinAngle;
    bool safeClick;
    QMenu *popRec;
    void createPopRec();
    QRectF eRec;
    double eWidth, eHeight;
    QDoubleSpinBox *spinWidth;
    QDoubleSpinBox *spinHeight;

    double pixScale =0;
    double unitScale=0;
    bool scaleSet = false;
    bool settingScale = false;
    QDoubleSpinBox *spinUnits;
    QDialog *popScale;
    void createPopScale();

};

#endif // MAINSCENE_H
