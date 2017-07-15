#include "taskdata.h"
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QDateEdit>
#include <QDebug>

TaskData::TaskData(QWidget *parent) :
    QScrollArea(parent)
{

    setWidget(new QWidget);
    setWidgetResizable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    //scroll area has 3 columns for widgets
    m_LayoutFirstRow    = new QVBoxLayout();
    m_LayoutSecondRow   = new QVBoxLayout();
    m_LayoutThirdRow    = new QVBoxLayout();

    widget()->setLayout(mainLayout);

    mainLayout->addLayout(m_LayoutFirstRow);
    mainLayout->addLayout(m_LayoutSecondRow);
    mainLayout->addLayout(m_LayoutThirdRow);
}

void TaskData::OnAddButtonPressed(void)
{
    AddRow();
}

//deletes a row of added widgets
void TaskData::removeButtonPressed()
{
    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender()); // retrieve the button you have clicked
    int index = m_Buttons.indexOf(buttonSender);
    m_LayoutThirdRow->removeWidget(m_Buttons[index]);
    m_Buttons[index]->deleteLater();
    m_Buttons.removeAt(index);

    m_LayoutSecondRow->removeWidget(m_date[index]);
    m_date[index]->deleteLater();
    m_date.removeAt(index);

    m_LayoutFirstRow->removeWidget(m_lines[index]);
    m_lines[index]->deleteLater();
    m_lines.removeAt(index);
}

//adds a line edit, date box and remove row button
void TaskData::AddRow()
{
    QLineEdit *heading = new QLineEdit;
    QDateEdit *date = new QDateEdit;
    date->setMinimumDate(QDate::currentDate().addDays(-365));
    date->setMaximumDate(QDate::currentDate().addDays(365));
    date->setCalendarPopup(true);
    date->setDate(QDate::currentDate());
    date->setDisplayFormat("yyyy.MM.dd");
    QPushButton *remove = new QPushButton(this);
    remove->setText(QString("remove task"));
    connect(remove, SIGNAL(clicked()), this, SLOT(removeButtonPressed()));

    m_LayoutFirstRow->addWidget(heading);
    m_LayoutSecondRow->addWidget(date);
    m_LayoutThirdRow->addWidget(remove);

    m_lines.append(heading);
    m_date.append(date);
    m_Buttons.append(remove);

}

//returns all taks that the user has added
const QVector<QString> TaskData::getTasks()
{
    QVector<QString> tasks;
    foreach(QLineEdit *line, m_lines){
        tasks.append(line->text());
    }
    return tasks;
}

//returns all dates that the user has added
const QVector<QDate> TaskData::getDates()
{
    QVector<QDate> dates;
    foreach(QDateEdit *line, m_date){
        dates.append(line->date());
    }
    return dates;
}

//creates rows and sets tasks and dates for loading files
void TaskData::setTasks(QVector<QString> taskVector, QVector<QDate> dateVector)
{
    for(int i=0;i<taskVector.length();i++){
        AddRow();
        m_lines[i]->setText(taskVector[i]);
        m_date[i]->setDate(dateVector[i]);
    }
    return;
}

