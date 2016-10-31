#include "taskpipe.h"

using namespace ProPer;


TaskPipe::TaskPipe(QObject *parent)
{
    Q_UNUSED(parent);
    _projectCount = 1;
    setBackgroundBrush(Qt::lightGray);//QColor(80, 128, 255));
}


void TaskPipe::build(QTreeWidget* tree)
{
    _tree = tree;
    clear();
    _bubbles.clear();
    for(int i=tree->topLevelItemCount()-1; i>=0; --i){ // reverse order to keep less important projects behind
        Task* project = dynamic_cast<Task*>(_tree->topLevelItem(i));
        _buildBubble(project); // recursively go down to each task
    }
}


void TaskPipe::updateZoom()
{
    QList<TaskBubble*> bubbles = _bubbles.values();
    for(int i=0; i<bubbles.size(); ++i)
        bubbles.at(i)->update3D();
    update();
}

void TaskPipe::updatePan(const QPoint& shift)
{
    QList<TaskBubble*> bubbles = _bubbles.values();
    for(int i=0; i<bubbles.size(); ++i)
        bubbles.at(i)->update(shift);
    update();
}


void TaskPipe::addBubble(Task *task)
{
    if(task->isProject() || !task->isTask()) return; // only lower level tasks
    if(task->status() == Task::DONE) return; // do not display finished tasks

    TaskBubble* bubble = new TaskBubble;
    QString header = task->project()->name();
    if(!task->parentTask()->isProject())
        header += QString(": ") + task->parentTask()->name();
    bubble->setTask(task->name(), header, QDate::currentDate().daysTo(task->deadline()));

    int index = _tree->indexOfTopLevelItem(task->project()); // index of the tasks' project
    double dx = sceneRect().width() / _tree->topLevelItemCount(); // horz spacing between projects
    double x = sceneRect().left() + dx * (index + 0.5);
    double y = sceneRect().top() + height()*(1.0 - task->priority());
    bubble->setAttributes(QPointF(x, y), task->categoryColor(task->category()));

    bubble->update();
    _bubbles.insert(task, bubble);
    addItem(bubble);
}


void TaskPipe::_buildBubble(Task* task)
{
    for(int i=task->childCount()-1; i>=0; --i){ // reverse order to keep less important tasks behind
        Task* t = dynamic_cast<Task*>(task->child(i));
        _buildBubble(t); // recursively go down to each task
    }
    addBubble(task);
}


void TaskPipe::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
    if(item){
        Task* task = _bubbles.key(dynamic_cast<TaskBubble*>(item));
        if(task->runEditor())
            build(_tree);
    }
}
