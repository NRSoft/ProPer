#include "taskpipeview.h"


TaskPipeView::TaskPipeView(QWidget* parent)
{
    Q_UNUSED(parent);
    _pan = false;
}


void TaskPipeView::wheelEvent(QWheelEvent* event)
{
    if(event->angleDelta().y() > 0)
        TaskBubble::incrementDistanceOrigin();
    else
        TaskBubble::decrementDistanceOrigin();
    TaskPipe* pipe = dynamic_cast<TaskPipe*>(scene());
    pipe->updateZoom();
    event->ignore();
}


void TaskPipeView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        _pan = true;
        _panStart = event->pos();
        event->accept();
        return;
    }
    event->ignore();
}


void TaskPipeView::mouseMoveEvent(QMouseEvent *event)
{
    if(_pan){
        TaskPipe* pipe = dynamic_cast<TaskPipe*>(scene());
        pipe->updatePan(event->pos() - _panStart);
        _panStart = event->pos();
        event->accept();
        return;
    }
    event->ignore();
}


void TaskPipeView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        _pan = false;
        event->accept();
        return;
    }
    event->ignore();
}
