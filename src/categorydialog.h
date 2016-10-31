#pragma once

#include <QDialog>
#include <QTreeWidget>

#include "spdlog/spdlog.h"
#include "task.h"

namespace Ui {
class CategoryDialog;
}

class CategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoryDialog(QTreeWidget* tree, QWidget *parent = 0);
    ~CategoryDialog();

private slots:
    void on_buttonBox_accepted();

    void on_colorButton_clicked();

    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_categoryCombo_currentTextChanged(const QString &arg1);

private:
    size_t _taskCount(const QString& category);
    size_t _taskCountPerItem(const ProPer::Task *task, const QString& category);

private:
    Ui::CategoryDialog *ui;

    QTreeWidget* _tree;

    QHash<QString, QColor> _localCategories;
    QColor _currentColor;

    std::shared_ptr<spdlog::logger> _logger;
};
