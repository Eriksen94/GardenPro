#include "datawindow.h"
#include "ui_datawindow.h"
#include "scenerect.h"
#include "scenepoly.h"
#include "taskdata.h"

#include <QTextCharFormat>
#include <QRectF>
#include <QDebug>
#include <QStyle>
#include <QDesktopWidget>
#include <QScrollArea>
#include <QDate>

DataWindow::DataWindow(QGraphicsItem *item) :
    QWidget(),
    ui(new Ui::DataWindow)
{
    parentItem = item;

    setWindowFlag(Qt::Popup);
    //set to appear in middle of screen
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            qApp->desktop()->availableGeometry()
        )
    );

    //initial set up
    ui->setupUi(this);
    ui->plotArea->setEnabled(false);
    ui->plotName->setMaxLength(15);
    ui->seeds->setValidator(new QIntValidator());
    ui->seeds->setText(QString("0"));
    ui->seedSpace->setMaximum(1000);

    //scroll area for user added buttons
    taskArea = new TaskData(this);
    ui->horizontalLayout_2->addWidget(taskArea);
    connect(ui->addTask, SIGNAL(clicked()), taskArea, SLOT(OnAddButtonPressed()));

}

void DataWindow::setArea(double a)
{
    darea = a;

    QString area = QString(" %1 m").arg(darea);
    QString units = QString("2");

    //add superscript for units - needs to be in a QTextEdit
    QTextCharFormat formatSup;
    formatSup.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    QTextCharFormat formatNorm;
    formatNorm.setVerticalAlignment(QTextCharFormat::AlignNormal);

    ui->plotArea->setEnabled(true);
    ui->plotArea->clear();
    ui->plotArea->mergeCurrentCharFormat(formatNorm);
    ui->plotArea->insertPlainText(area);
    ui->plotArea->mergeCurrentCharFormat(formatSup);
    ui->plotArea->insertPlainText(units);
    ui->plotArea->setEnabled(false);

    //update seed spacing and seed amounts incase they were input before area was set
    if(ui->seedSpace->value() != 0){
        on_seedSpace_valueChanged(ui->seedSpace->value());
    }
    else if(ui->seeds->text().toDouble() != 0){
        on_seeds_textEdited(ui->seeds->text());
    }
}

const QVector<QString> DataWindow::collectData()
{
    QVector<QString> data(6," ");
    data[0] = ui->plotName->text();
    data[1] = ui->crop->text(); 
    data[2] = QString::number(ui->seedSpace->value());
    data[3] = ui->notes->toPlainText();
    data[4] = ui->seeds->text();
    data[5] = QString::number(darea);
    return data;
}

void DataWindow::setData(QVector<QString> dataFill)
{
    ui->plotName->setText(dataFill[0]);
    ui->crop->setText(dataFill[1]);
    ui->seedSpace->setValue(dataFill[2].toDouble());
    ui->notes->insertPlainText(dataFill[3]);
}

DataWindow::~DataWindow()
{
    delete ui;
}

void DataWindow::on_seedSpace_valueChanged(double arg1)
{
    //ignore to avoid rounding errors and loop calling between seeds and seed space
    if(ignore){
        ignore = false;
        return;
    }
    double value = arg1;
    //assuming radial spacing around the seed
    if(arg1 != 0){
        arg1 = arg1/100;
        value = darea/(arg1*3.14*arg1);
    }
    QString numSeed = QString(" %1 ").arg((int)value);
    ui->seeds->setText(numSeed);
}

void DataWindow::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    parentItem->setSelected(false);
    close();
}

void DataWindow::on_plotName_textChanged(const QString &arg1)
{
    if(parentItem->type() == 3){
        SceneRect *rec = dynamic_cast<SceneRect *>(parentItem);
        rec->updateName(arg1);
    }
    if(parentItem->type() == 5){
        ScenePoly *pol = dynamic_cast<ScenePoly *>(parentItem);
        pol->updateName(arg1);
    }
}

void DataWindow::on_seeds_textEdited(const QString &arg1)
{
    double numSeed = arg1.toDouble();
    double space = 0;
    ignore = true;
    if(darea != 0){
        //area per seed (m ^2)
        space = darea/numSeed;
        //radius, cm
        space = sqrt(space/3.141592) * 100;
        //ignore set to avoid looping updates
        ui->seedSpace->setValue(space);
    }
}
