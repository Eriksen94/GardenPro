#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainscene.h"
#include "imagepixmap.h"
#include "textimage.h"
#include "scenerect.h"
#include "sceneline.h"
#include "scenepoly.h"

#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QDialog>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QComboBox>
#include <QLabel>
#include <QGraphicsPixmapItem>
#include <QSizeGrip>
#include <QTextEdit>
#include <QTextStream>
#include <QDate>
#include <QDateTimeEdit>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //initial set up, needs to be larger than furthest out zoom
    //otherwise items created outside of viewable scene when zoomed in
    xPix = 4000;
    yPix = 2000;
    units = "meter";
    cleanSetup();

   // Graphics_view_zoom* z = new Graphics_view_zoom(ui->graphicsView);
   // z->set_modifiers(Qt::NoModifier);
    // https://stackoverflow.com/questions/19113532/qgraphicsview-zooming-in-and-out-under-mouse-position-using-mouse-wheel
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::cleanSetup()
{
    //set scene
    scene = new MainScene(this);
    QRectF exactRect(0, 0, xPix , yPix);
    scene->setSceneRect(exactRect);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->centerOn(exactRect.center());

    //add box for zooming
    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(2);
    connect(sceneScaleCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(sceneScaleChanged(QString)));
    ui->mainToolBar->addWidget(sceneScaleCombo);

    //add unit heading and spacer to right align
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->addWidget(spacer);

    unitLabel = new QLabel(QString("Units: ").append(units).append("    "));
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget(unitLabel);

    scaleLabel = new QLabel(QString(" Scale needs to be set "));
    scaleLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget(scaleLabel);
    connect(scene, SIGNAL(scaleIsSet(bool)), this, SLOT(sceneScaleSet(bool)));

    //adding table to page 2 - sum total table
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(200);

    QTableWidgetItem *newItem = new QTableWidgetItem(tr("Crop heading"));
    ui->tableWidget->setHorizontalHeaderItem(0,newItem);

    QTableWidgetItem *newItem2 = new QTableWidgetItem(tr("Seed Totals"));
    ui->tableWidget->setHorizontalHeaderItem(1,newItem2);
    ui->tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //adding table to page 2 - timeline table
    ui->tableWidget_2->setColumnCount(3);
    ui->tableWidget_2->setRowCount(200);

    newItem = new QTableWidgetItem(tr("Date"));
    ui->tableWidget_2->setHorizontalHeaderItem(2,newItem);
    newItem = new QTableWidgetItem(tr("Plot"));
    ui->tableWidget_2->setHorizontalHeaderItem(0,newItem);
    newItem = new QTableWidgetItem(tr("Task"));
    ui->tableWidget_2->setHorizontalHeaderItem(1,newItem);
    ui->tableWidget_2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->stackedWidget->setCurrentWidget(ui->Page1);
}

//uncheck toolbar buttons so its clear which is in use
void MainWindow::uncheckTool()
{
    ui->actionRect->setChecked(false);
    ui->actionSelect->setChecked(false);
    ui->actionline->setChecked(false);
    ui->actionEline->setChecked(false);
    ui->actionErect->setChecked(false);
    ui->actionsetScale->setChecked(false);
    //ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
    foreach (QGraphicsItem *item, scene->selectedItems()){
        item->setSelected(false);
    }
}

//MENU BAR ACTIONS START***********************************

//clears data to create a fresh starting point
void MainWindow::on_actionNew_triggered()
{
    scene->clear();
    scene->scaleSetFromMem(0,0,false);
    on_actionSelect_triggered();
    scene->update();
    on_actionLayout_Tab_triggered();
    ui->tableWidget->clearContents();
    ui->tableWidget_2->clearContents();
}

//write all user added data to a txt file - images not included
void MainWindow::on_actionSave_triggered()
{

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),
            tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            // error message
        } else {
            QTextStream stream(&file);

            //type: 6 - line, 3 - rect, 5 - poly, 7-pixmap
            foreach (QGraphicsItem *currentItem, scene->items(Qt::DescendingOrder)){

                if(currentItem->type() == 3){
                    SceneRect* result = dynamic_cast<SceneRect*>(currentItem);
                    //get data to rebuild rectangle
                    if(result->rect().height() == 0 || result->rect().width() == 0){
                        continue;
                    }
                    stream << "type:" << currentItem->type() << "\r\n";
                    stream << "pos:" << result->boundingRect().x() << " " << result->boundingRect().y() << "\r\n";
                    stream << "height:" << result->rect().height() << "\r\n";
                    stream << "width:" << result->rect().width() << "\r\n";
                    //get data from associated data window;
                    QVector<QString> data = result->data->collectData();
                    stream << "name:" << data[0] << "\r\n";
                    stream << "crop:" << data[1] << "\r\n";
                    stream << "spacing:" << data[2] << "\r\n";
                    stream << "note:" << data[3] << "\r\n";
                    //log tasks
                    QVector<QString> itemTasks = result->data->taskArea->getTasks();
                    QVector<QDate> itemDates = result->data->taskArea->getDates();
                    int n;
                    if(itemTasks.empty()) n=0;
                    else n=itemTasks.length();
                    stream << "task_count:" << n << "\r\n";
                    for(int i=0;i<n;i++){
                        stream << "task:" << itemTasks[i] << "\r\n";
                        stream << "date:" << itemDates[i].toString() << "\r\n";
                    }
                }
                else if(currentItem->type() == 6){
                    stream << "type:" << currentItem->type() << "\r\n";
                    SceneLine* result = dynamic_cast<SceneLine*>(currentItem);
                    stream << "p1:" << result->line().p1().x() << " " << result->line().p1().y() << "\r\n";
                    stream << "p2:" << result->line().p2().x() << " " << result->line().p2().y() << "\r\n";
                }
                else if(currentItem->type() == 5){
                    stream << "type:" << currentItem->type() << "\r\n";
                    ScenePoly* result = dynamic_cast<ScenePoly*>(currentItem);
                    //get data to rebuild polygon
                    QVector<QPointF> verticies;
                    verticies.clear();
                    verticies.append(result->polygon());
                    int points = verticies.length();
                    stream << "points:" << points << "\r\n";
                    for(int i=0; i<points; i++){
                        stream << "vertex:" << verticies[i].x() << " " << verticies[i].y() << "\r\n";
                    }
                    //collect data from the associated data window
                    QVector<QString> data = result->data->collectData();
                    stream << "name:" << data[0] << "\r\n";
                    stream << "crop:" << data[1] << "\r\n";
                    stream << "spacing:" << data[2] << "\r\n";
                    stream << "note:" << data[3] << "\r\n";
                    //log tasks
                    QVector<QString> itemTasks = result->data->taskArea->getTasks();
                    QVector<QDate> itemDates = result->data->taskArea->getDates();
                    int n;
                    if(itemTasks.empty()) n=0;
                    else n=itemTasks.length();
                    stream << "task_count:" << n << "\r\n";
                    for(int i=0;i<n;i++){
                        stream << "task:" << itemTasks[i] << "\r\n";
                        stream << "date:" << itemDates[i].toString() << "\r\n";
                    }
                }
                /*
                else if (currentItem->type() == 7){
                    stream << "type:" << currentItem->type() << "\r\n";
                    stream.flush();
                    ImagePixMap* result = dynamic_cast<ImagePixMap*>(currentItem);
                    QPixmap img(result->pixmap());
                    QDataStream stream2(&file);
                    stream2 << img << "\r\n";

                }
                */
                stream.flush();
            }
            // scale added last to update items when reading
            int scaleInt;
            if(scene->getScaleState())
                scaleInt = 1;
            else
                scaleInt=0;
            stream << "scale:" << scaleInt << "\r\n";
            stream << "pixScale:" << scene->getPixScale() << "\r\n";
            stream << "unitScale:" << scene->getUnitScale() << "\r\n";
            stream.flush();

            file.close();
        }
    }
}

//reads from a txt file and adds items as a user would
void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
            tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        on_actionNew_triggered();
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if(line.contains("scale:")){
                QString scale = line.mid(6,1);
                bool scl = false;
                if(scale.toInt()) scl =true;
                line = in.readLine();
                QString pix = line.mid(9, line.length()-9);
                line = in.readLine();
                QString unit = line.mid(10, line.length()-10);
                scene->scaleSetFromMem(pix.toDouble(), unit.toDouble(), scl);
            }
            if(line.contains("type:")){
                QString type = line.mid(5,1);
                //load SceneRectangles
                if(type.toInt() == 3){
                    line = in.readLine();
                    int space = line.indexOf(" ");
                    QString posX = line.mid(4,space-4);
                    QString posY = line.mid(space+1,line.length()-space-1);
                    line = in.readLine();
                    QString height = line.mid(7,line.length()-7);
                    line = in.readLine();
                    QString width = line.mid(6,line.length()-6);
                    SceneRect *rect = new SceneRect();
                    QPointF pos_;
                    pos_.setX(posX.toDouble());
                    pos_.setY(posY.toDouble());
                    rect->setRect(pos_.x(),pos_.y(),width.toDouble(),height.toDouble());
                    //load data fields
                    line = in.readLine();
                    QString plotName = line.mid(5,line.length()-5);
                    line = in.readLine();
                    QString crop = line.mid(5,line.length()-5);
                    line = in.readLine();
                    QString spacing = line.mid(8,line.length()-8);
                    line = in.readLine();
                    QString note = line.mid(5,line.length()-5);
                    QVector<QString> dataFill(4);
                    dataFill[0] = plotName;
                    dataFill[1] = crop;
                    dataFill[2] = spacing;
                    dataFill[3] = note;
                    rect->data->setData(dataFill);
                    //add tasks
                    line = in.readLine();
                    QString taskCount = line.mid(11,line.length()-11);
                    int n = taskCount.toInt();
                    if(n!=0){
                        QVector<QString> taskVector;
                        QVector<QDate> dateVector;
                        for(int i=0;i<n;i++){
                            line = in.readLine();
                            QString task = line.mid(5,line.length()-5);
                            taskVector.append(task);
                            line = in.readLine();
                            QString dateS = line.mid(5,line.length()-5);
                            QDate date = QDate::fromString(dateS,"ddd MMM d yyyy");
                            dateVector.append(date);
                        }
                        rect->data->taskArea->setTasks(taskVector,dateVector);
                    }
                    scene->addItem(rect);
                }
                //load SceneLines
                else if(type.toInt() == 6){
                    line = in.readLine();
                    int space = line.indexOf(" ");
                    QString posX1 = line.mid(3,space-3);
                    QString posY1 = line.mid(space+1,line.length()-space-1);
                    line = in.readLine();
                    space = line.indexOf(" ");
                    QString posX2 = line.mid(3,space-3);
                    QString posY2 = line.mid(space+1,line.length()-space-1);
                    QPointF pos_1(posX1.toDouble(),posY1.toDouble());
                    QPointF pos_2(posX2.toDouble(),posY2.toDouble());
                    SceneLine *Sline = new SceneLine();
                    Sline->setLine(QLineF(pos_1,pos_2));
                    Sline->updateFlags();
                    scene->addItem(Sline);
                }
                //load polygons
                else if(type.toInt() == 5){
                    line = in.readLine();
                    QString vertexCount = line.mid(7,line.length()-7);
                    int space;
                    QVector<QPointF> polyVector;
                    polyVector.clear();
                    for(int i=0; i<vertexCount.toInt();i++){
                        line = in.readLine();
                        space = line.indexOf(" ");
                        QString posX = line.mid(7,space-7);
                        QString posY = line.mid(space+1,line.length()-space-1);
                        QPointF pos(posX.toDouble(),posY.toDouble());
                        polyVector.append(pos);
                    }
                    ScenePoly *poly = new ScenePoly();
                    poly->setPolygon(QPolygonF(polyVector));
                    poly->setupPoly();
                    //load data fields
                    line = in.readLine();
                    QString plotName = line.mid(5,line.length()-5);
                    line = in.readLine();
                    QString crop = line.mid(5,line.length()-5);
                    line = in.readLine();
                    QString spacing = line.mid(8,line.length()-8);
                    line = in.readLine();
                    QString note = line.mid(5,line.length()-5);
                    QVector<QString> dataFill(4);
                    dataFill[0] = plotName;
                    dataFill[1] = crop;
                    dataFill[2] = spacing;
                    dataFill[3] = note;
                    poly->data->setData(dataFill);
                    //add tasks
                    line = in.readLine();
                    QString taskCount = line.mid(11,line.length()-11);
                    int n = taskCount.toInt();
                    if(n!=0){
                        QVector<QString> taskVector;
                        QVector<QDate> dateVector;
                        for(int i=0;i<n;i++){
                            line = in.readLine();
                            QString task = line.mid(5,line.length()-5);
                            taskVector.append(task);
                            line = in.readLine();
                            QString dateS = line.mid(5,line.length()-5);
                            QDate date = QDate::fromString(dateS,"ddd MMM d yyyy");
                            dateVector.append(date);
                        }
                        poly->data->taskArea->setTasks(taskVector,dateVector);
                    }
                    scene->addItem(poly);
                }
                //load pixmap
                else if(type.toInt() == 7){

                }
            }
        }
        file.close();
    }
}

void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

//converts image file to pixmap and adds to scene
void MainWindow::on_actionImport_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("Images (*.png *.xpm *.jpg)"));
    if(fileName != NULL){

        ImagePixMap *img = new ImagePixMap(QPixmap(fileName));
        img->setFlag(QGraphicsItem::ItemIsSelectable, true);
        img->setFlag(QGraphicsItem::ItemIsMovable, true);
        //always under items so plots show ontop
        img->setZValue(-1000);
        scene->addItem(img);
    }
}

//removes items from the scene
void MainWindow::on_actionDelete_triggered()
{
    foreach (QGraphicsItem *item, scene->selectedItems()) {
         scene->removeItem(item);
         delete item;
     }
}

//switches to table view, includes filling tables
void MainWindow::on_actionData_Tab_triggered()
{
    //this page is index 1 - ignore button
    if(ui->stackedWidget->currentIndex() == 1){
        return;
    }
    ui->stackedWidget->setCurrentWidget(ui->Page2);
    ui->mainToolBar->hide();

    QVector<QString> allData;
    QTableWidgetItem *newItem = new QTableWidgetItem;
    double sum = 0;
    //index of next entry to be added in data table
    int row = 1;

    //Lists for timeline tables
    QVector<QString> taskHeadings;
    QVector<QDate> dateList;
    QVector<QString> plotNames;

    //initialize area row since name is constant
    newItem = new QTableWidgetItem(QString("area"));
    ui->tableWidget->setItem(0,0,newItem);
    newItem = new QTableWidgetItem(QString("0"));
    ui->tableWidget->setItem(0,1,newItem);
    newItem = new QTableWidgetItem;

    //update table from data windows
    foreach (QGraphicsItem *currentItem, scene->items(Qt::AscendingOrder)){
        if(currentItem->type() == 3){
            SceneRect* testItem = dynamic_cast<SceneRect*>(currentItem);

            //fill in data table with crops and seed counts

            QVector<QString> data = testItem->data->collectData();
            //add to total area
            sum = ui->tableWidget->item(0,1)->text().toDouble() + data[5].toDouble();
            newItem->setText(QString::number(sum));
            ui->tableWidget->setItem(0,1,newItem);
            newItem = new QTableWidgetItem;
            //check table for occurences of the item and either add to sum or add heading
            for(int i=0; i<row; i++){
                if(ui->tableWidget->item(i,0)->text() == data[1]){
                    //heading exists at row i
                    sum = ui->tableWidget->item(i,1)->text().toDouble() + data[4].toDouble();
                    newItem->setText(QString::number(sum));
                    ui->tableWidget->setItem(i,1,newItem);
                    newItem = new QTableWidgetItem;
                    break;
                }
                else if(i == (row-1)){
                    //heading needs to be added
                    newItem->setText(data[1]);
                    ui->tableWidget->setItem(row,0,newItem);
                    newItem = new QTableWidgetItem;
                    newItem->setText(data[4]);
                    ui->tableWidget->setItem(row,1,newItem);
                    newItem = new QTableWidgetItem;
                    row++;
                    break;
                }
            }
            //collect all time line data to be sorted
            QVector<QString> itemTasks = testItem->data->taskArea->getTasks();
            QVector<QDate> itemDates = testItem->data->taskArea->getDates();
            for(int i=0;i<itemTasks.length();i++){
                taskHeadings.append(itemTasks[i]);
                dateList.append(itemDates[i]);
                plotNames.append(data[0]);
            }
        }
        else if(currentItem->type() == 5){
            ScenePoly* testItem = dynamic_cast<ScenePoly*>(currentItem);

            //fill in data table with crops and seed counts

            QVector<QString> data = testItem->data->collectData();
            //add to total area
            sum = ui->tableWidget->item(0,1)->text().toDouble() + data[5].toDouble();
            newItem->setText(QString::number(sum));
            ui->tableWidget->setItem(0,1,newItem);
            newItem = new QTableWidgetItem;
            //check table for occurences of the item and either add to sum or add heading
            for(int i=0; i<row; i++){
                if(ui->tableWidget->item(i,0)->text() == data[1]){
                    //heading exists at row i
                    sum = ui->tableWidget->item(i,1)->text().toDouble() + data[4].toDouble();
                    newItem->setText(QString::number(sum));
                    ui->tableWidget->setItem(i,1,newItem);
                    newItem = new QTableWidgetItem;
                    break;
                }
                else if(i == (row-1)){
                    //heading needs to be added
                    newItem->setText(data[1]);
                    ui->tableWidget->setItem(row,0,newItem);
                    newItem = new QTableWidgetItem;
                    newItem->setText(data[4]);
                    ui->tableWidget->setItem(row,1,newItem);
                    newItem = new QTableWidgetItem;
                    row++;
                    break;
                }
            }

            //collect all time line data to be sorted
            QVector<QString> itemTasks = testItem->data->taskArea->getTasks();
            QVector<QDate> itemDates = testItem->data->taskArea->getDates();
            for(int i=0;i<itemTasks.length();i++){
                taskHeadings.append(itemTasks[i]);
                dateList.append(itemDates[i]);
                plotNames.append(data[0]);
            }
        }
    }

    //sort dates/tasks - modified bubble sort
    for(int i=0; i<dateList.length(); i++){
        int swaps=0;
        for(int j=0; j<dateList.length()-1; j++){
            if(dateList[j]>dateList[j+1]){
                QDate tempD = dateList[j];
                dateList[j] = dateList[j+1];
                dateList[j+1] = tempD;

                QString tempS = taskHeadings[j];
                taskHeadings[j] = taskHeadings[j+1];
                taskHeadings[j+1] = tempS;

                QString tempP = plotNames[j];
                plotNames[j] = plotNames[j+1];
                plotNames[j+1] = tempP;

                swaps++;
            }
        }
        if(swaps==0)
            break;
    }

    //fill in timeline - plot names in first col, tasks in 2nd, dates in 3rd
    for(int i=0; i<dateList.length(); i++){
        newItem = new QTableWidgetItem;
        newItem->setText(plotNames[i]);
        ui->tableWidget_2->setItem(i,0,newItem);
        newItem = new QTableWidgetItem;

        newItem->setText(taskHeadings[i]);
        ui->tableWidget_2->setItem(i,1,newItem);
        newItem = new QTableWidgetItem;

        newItem->setText(dateList[i].toString());
        ui->tableWidget_2->setItem(i,2,newItem);
    }
}

//switches to drawing view
void MainWindow::on_actionLayout_Tab_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->Page1);
    ui->mainToolBar->show();
    //table contents are cleared to avoid stacking when going back to table view
    ui->tableWidget->clearContents();
    ui->tableWidget_2->clearContents();
}

//show some program info
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(
        this,
        tr("About"),
        tr("A simple garden planning tool") );
}

//show some tips
void MainWindow::on_actionTips_triggered()
{
    QMessageBox::information(
        this,
        tr("Tips:"),
        tr("1. draw on the blank space with lines and rectangles \n "
           "2. use the scale set button to set the overall scale \n"
           "3. import images and draw over them to create layouts \n"
           "4. when lines connect to form a closed shape it will turn into a plot area \n"
           "5. the plot only forms when the connecting line is a drawn (not exact) \n"
           "6. right click on a plot to access and edit its data   \n"
           "7. go to the data view (ctr+q) to see a summary of plot data \n"
           "...") );
}

//MENU BAR ACTIONS END***********************************

//TOOL BAR ACTIONS START***********************************
//actions set flags on items and the scene for drawing and GUI usage

void MainWindow::on_actionline_triggered()
{
    scene->setMode(MainScene::InsertLine);
    scene->setExact(false);
    uncheckTool();
    ui->actionline->setChecked(true);
}

void MainWindow::on_actionRect_triggered()
{
    scene->setMode(MainScene::InsertRect);
    scene->setExact(false);
    uncheckTool();
    ui->actionRect->setChecked(true);
}

void MainWindow::on_actionSelect_triggered()
{
    scene->setMode(MainScene::Select);
    uncheckTool();
    ui->actionSelect->setChecked(true);
}

void MainWindow::sceneScaleChanged(const QString &scale)
{
    //can zoom beyond bounds set and create areas only accessable at those scales if scene is too small
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QMatrix oldMatrix = ui->graphicsView->matrix();
    ui->graphicsView->resetMatrix();
    ui->graphicsView->translate(oldMatrix.dx(), oldMatrix.dy());
    ui->graphicsView->scale(newScale, newScale);
}

void MainWindow::on_actionEline_triggered()
{
    if(scene->getScaleState()){
        scene->setMode(MainScene::InsertLine);
        scene->setExact(true);
        uncheckTool();
        ui->actionEline->setChecked(true);
    }
    else{
        uncheckTool();
        QMessageBox::information(
            this,
            tr("scale not set"),
            tr("please set the scale before using this function") );
    }
}

void MainWindow::on_actionErect_triggered()
{
    if(scene->getScaleState()){
        scene->setMode(MainScene::InsertRect);
        scene->setExact(true);
        uncheckTool();
        ui->actionErect->setChecked(true);
    }
    else{
        uncheckTool();
        QMessageBox::information(
            this,
            tr("scale not set"),
            tr("please set the scale before using this function") );
    }
}

void MainWindow::on_actionsetScale_triggered()
{
    scene->setMode(MainScene::Select);
    scene->setExact(false);
    scene->setScale(true);
    uncheckTool();
    ui->actionsetScale->setChecked(true);
    QMessageBox::information(
        this,
        tr("scaleSet"),
        tr("Select a Line \n then set its length") );
}

//TOOL BAR ACTIONS END***********************************

//for implementing multiple units
void MainWindow::saveUnits(QString arg)
{
    units = arg;
}

//graphic to show scale setting
void MainWindow::sceneScaleSet(bool arg)
{
    if(!arg){
        scaleLabel->setText(QString(" Scale needs to be set "));
        scaleLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
    }
    else{
        scaleLabel->setText(QString(" Scale is set "));
        scaleLabel->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
}

//hide or show all dimensions to preent screen congestion
void MainWindow::on_actionShow_Dimensions_toggled(bool arg1)
{
    foreach (QGraphicsItem *currentItem, scene->items(Qt::DescendingOrder)) {
        //update items dimension flag
        if(currentItem->type() == 3){
            SceneRect* result = dynamic_cast<SceneRect*>(currentItem);
            result->setShowDim(arg1);
            result->update();
        }
        else if(currentItem->type() == 6){
            SceneLine* result = dynamic_cast<SceneLine*>(currentItem);
            result->setShowDim(arg1);
            result->update();
        }
        else if(currentItem->type() == 5){
            ScenePoly* result = dynamic_cast<ScenePoly*>(currentItem);
            result->setShowDim(arg1);
            result->update();
        }
    }
}
