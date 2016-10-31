#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "task.h"
#include "taskbubble.h"


class TaskPipe: public QGraphicsScene
{
public:
    TaskPipe(QObject *parent = Q_NULLPTR);

    inline void assignTaskTree(QTreeWidget* tree) {_tree = tree;}
//    void setProjectCount(size_t projectCount) {_projectCount = projectCount;}
    void build(QTreeWidget *tree);

    void addBubble(ProPer::Task *task);

//    void updateBubble(ProPer::Task* task);

    // call it before deleting the task, otherwise bubble may dangle and cause crash
    void removeBubble(const ProPer::Task* task);

    void updateZoom();

    void updatePan(const QPoint &shift);

//    const ProPer::Task* task(TaskBubble& bubble) const {_bubbles.key(bubble, nullptr);}
//    void Populate(const QList<Task*>& tasks);

//    void draw(QGraphicsScene* scene);

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);


private:
    size_t _projectCount;
    QHash<ProPer::Task*, TaskBubble*> _bubbles;

    void _buildBubble(ProPer::Task* task);

    QTreeWidget* _tree;
};
