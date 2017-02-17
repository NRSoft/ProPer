#pragma once

#include <QHeaderView>
#include <QListWidget>
#include <QMenu>
#include "task.h"


class ProjectHeaderView: public QHeaderView
{
    Q_OBJECT

public:
    ProjectHeaderView(QTreeWidget* tree):QHeaderView(Qt::Horizontal), _tree(tree)
    {
        _history = nullptr;
        _taskLogger = spdlog::get("task");
        setDefaultAlignment(Qt::AlignLeft);
        setSectionResizeMode(QHeaderView::Stretch);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, SIGNAL(sectionClicked(int)), this, SLOT(sectionClicked(int)));
        connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
    }

    inline void setHistoryList(QListWidget* history) {_history = history;}

public slots:
    void sectionClicked(int)
    {
        if(_tree)
            _tree->selectionModel()->clear();
    }

    void customContextMenuRequested(const QPoint& pos)
    {
        QMenu *menu=new QMenu(this);
        menu->addAction(QLatin1String("New Project"), this, SLOT(on_newProject()));
        menu->popup(viewport()->mapToGlobal(pos));
    }

    void on_newProject()
    {
        ProPer::Task* project = new ProPer::Task();
        _tree->addTopLevelItem(project);
        project->setText(0, QLatin1String("[editing...]"));

        if(project->runEditor()){
            _taskLogger->debug("new project \"{}\"", project->name().toUtf8().data());
            if(_history)
                _history->insertItem(0, project->fullPrefix() + QStringLiteral(" was created"));
        }
        else // if cancelled remove newly added item
            _tree->takeTopLevelItem(_tree->indexOfTopLevelItem(project));
    }

private:
    QTreeWidget* _tree;
    QListWidget* _history;
    std::shared_ptr<spdlog::logger> _taskLogger;
};
