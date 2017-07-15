#ifndef TASKDATA_H
#define TASKDATA_H


#include <QScrollArea>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QPushButton;
class QLineEdit;
class QDateEdit;
QT_END_NAMESPACE

class TaskData : public QScrollArea
{
    Q_OBJECT

public:
    explicit TaskData(QWidget *parent = 0);
    void AddRow();
    const QVector<QString> getTasks();
    const QVector<QDate> getDates();
    void setTasks(QVector<QString> taskVector, QVector<QDate> dateVector);
public slots:
    void OnAddButtonPressed(void);
private slots:
    void removeButtonPressed();
private:
    QVBoxLayout *m_LayoutFirstRow;
    QVBoxLayout *m_LayoutSecondRow;
    QVBoxLayout *m_LayoutThirdRow;

    QList<QPushButton*> m_Buttons;
    QList<QLineEdit*> m_lines;
    QList<QDateEdit*> m_date;

};

#endif // TASKDATA_H
