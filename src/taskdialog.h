#pragma once

#include <QDialog>

#include "spdlog/spdlog.h"
#include "task.h"

namespace Ui {
class TaskDialog;
}

class TaskDialog : public QDialog
{
    Q_OBJECT
public:
    enum Type{
        PROJECT, // no parent task
        COMPONENT, // everything in between
        TASK // no children tasks
    };

public:
    explicit TaskDialog(QWidget *parent = 0);
    ~TaskDialog();

    bool initNew(QTreeWidget* tree, ProPer::Task* parent);
    bool initEdit(QTreeWidget* tree, ProPer::Task* task);

    inline const ProPer::Task& task() const {return _task;}
    void setTask(const ProPer::Task* task);

private:
    void _initCommon();
    void _updateNote();
    void _updateCategories();

private slots:
    void on_buttonBox_accepted();

    void on_editNoteButton_clicked();
    void on_prevNoteButton_clicked();
    void on_nextNoteButton_clicked();
    void on_newNoteButton_clicked();
    void on_deleteNoteButton_clicked();

    void on_deadlineDate_dateChanged(const QDate &date);

    void on_categoriesButton_clicked();

    void ctrlB_clicked(); // bold
    void ctrlI_clicked(); // italic
    void ctrlK_clicked(); // hyperlink

private:
    Ui::TaskDialog *ui;

    bool _noteEditing;

    Type _type;

    QTreeWidget* _tree;
    ProPer::Task _task;

    int _currentNote;

//    QList<ProPer::Task*> _siblings;
//    int _taskPosition; // among siblings

//    QHash<QString, QColor> _localCategories;
//    QColor _currentColor;

    std::shared_ptr<spdlog::logger> _logger;
};
