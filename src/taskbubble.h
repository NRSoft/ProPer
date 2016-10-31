#pragma once

#include <QGraphicsItem>
#include <QGraphicsBlurEffect>


class TaskBubble : public QGraphicsPathItem
{
public:
    TaskBubble(QGraphicsItem* parent=nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setAttributes(const QPointF pos, const QColor& color);
    void setTask(const QString& text, const QString& header, int distance);
//    inline void setPlanePos(const QPointF pos) {_pos = pos;}

    void update(const QPoint &shift = QPoint(0,0)); // update both in 2D & 3D
    void update3D(); // perspective change only

//    inline int distance() const {return _distance;}
//    inline void setDistance(int distance) {_distance = distance;}

    static void setDistanceOrigin(int origin) {_origin = origin;}
    static void incrementDistanceOrigin() {_origin++;}
    static void decrementDistanceOrigin() {if(_origin>0) _origin--;}

    static QColor foregroundColor(const QColor& background)
    {
        int graylevel = (background.red() + background.green() + background.blue()) / 3;
        return (graylevel >= 112)? Qt::black: Qt::white;
    }

private:
    QPointF _pos;

    QColor _textColor;
    QColor _bubbleColor;
    QBrush* _bubbleBrush;

    QRectF _bubbleRect; // to define gradient
    constexpr static int _radius   = 8; // for rounded corners
    constexpr static int _margin   = 8; // margin around text

    static QFont _font;
    static QFont _subFont; // for "days left"
    constexpr static int _fontSize = 9; // main font, sub-font is 2 sizes less
    QRectF _textBounds; // for text background color


    int _distance; // absolute distance to the bubble (days from now)
    static int _origin; // current day in viewing position (in timeline)

    QString _text; // task name
    QString _header;
    QString _daysLeft; // sub-string

    QGraphicsBlurEffect _blur;

    static double _scale; // the scale of the whole bubble
};
