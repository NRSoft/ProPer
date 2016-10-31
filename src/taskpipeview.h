#pragma once

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>


#include "taskpipe.h"

class TaskPipeView : public QGraphicsView
{
public:
    TaskPipeView(QWidget* parent = Q_NULLPTR);

    inline void init(TaskPipe* pipe) {setScene(pipe);}

    void test();

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
//    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent* event);

private:
//    TaskPipe* _pipe;
    bool _pan;
    QPoint _panStart;
};
