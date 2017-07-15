#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "mainscene.h"

QT_BEGIN_NAMESPACE
class QAction;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QGraphicsTextItem;
class QLabel;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    double xBound, yBound;
    MainScene *scene;

signals:

public slots:
    //initial settings
    void saveUnits(QString arg);
    void sceneScaleSet(bool arg);
private slots:
    //menu and toolbar Actions
    void on_actionSave_triggered();

    void on_actionOpen_triggered();

    void on_actionNew_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_actionline_triggered();

    void on_actionRect_triggered();

    void on_actionSelect_triggered();

    void on_actionDelete_triggered();
    void sceneScaleChanged(const QString &scale);

    void on_actionEline_triggered();

    void on_actionErect_triggered();

    void on_actionsetScale_triggered();

    void on_actionImport_triggered();

    void on_actionTips_triggered();

    void on_actionData_Tab_triggered();

    void on_actionLayout_Tab_triggered();

    void on_actionShow_Dimensions_toggled(bool arg1);

private:
    void cleanSetup();
    void uncheckTool();
    double xPix, yPix;
    Ui::MainWindow *ui;
    QGraphicsRectItem *rectangle;
    QGraphicsLineItem *line;
    QComboBox *sceneScaleCombo;
    QString units;
    QLabel *unitLabel;
    QLabel *scaleLabel;

protected:

};

#endif // MAINWINDOW_H
