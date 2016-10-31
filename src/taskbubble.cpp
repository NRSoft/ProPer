#include <QPen>
#include <QPainter>
#include <QGraphicsBlurEffect>

#include "taskbubble.h"

int TaskBubble::_origin = 0;
QFont TaskBubble::_font = QFont("Times New Roman", _fontSize);
QFont TaskBubble::_subFont = QFont("Times New Roman", _fontSize-2);
double TaskBubble::_scale = 1.0;


TaskBubble::TaskBubble(QGraphicsItem* parent)
: QGraphicsPathItem(parent)
{
    _distance = 10;
    _bubbleColor = QColor(160, 160, 160);
    _bubbleBrush = nullptr;
    setFlags(QGraphicsItem::ItemIsSelectable);
}


/////////  p a i n t  /////////
void TaskBubble::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option); Q_UNUSED(widget);

    if(_origin > _distance+3) return; // show blurred 3 days back

    painter->setRenderHint(QPainter::Antialiasing);

    if(_origin > _distance) painter->setOpacity(0.2);
    painter->fillRect(_textBounds, _textColor);
    painter->setOpacity(1.0);

    painter->fillPath(path(), *_bubbleBrush);

    QPen originalPen = painter->pen();
    painter->setPen(QPen(Qt::NoPen));
    painter->drawPath(path());
    painter->setPen(originalPen);
}


///////////  s e t  T a s k  /////////
void TaskBubble::setTask(const QString& text, const QString& header, int distance)
{
    _text = text;
    _header = QLatin1String(" [") + header + QStringLiteral("]");
    if(_distance != distance){
        _distance = distance;
        setZValue(-distance);
        _daysLeft = QLatin1String(" (");
        _daysLeft += (_distance == 0)? QStringLiteral("today)"):
                     (_distance == 1)? QStringLiteral("tomorrow)"):
                     (_distance == -1)? QStringLiteral("yesterday)"):
                     (_distance < 0)?  QString::number(-_distance) + QStringLiteral(" days ago)"):
                     QString::number(_distance) + QStringLiteral(" days to go)");
        if(_distance < 0) _distance = 0; // pile all unfinished tasks into today
    }
}


///////   s e t  A t t r i b u t e s  ///////
void TaskBubble::setAttributes(const QPointF pos, const QColor& color)
{
    _pos = pos;
    _bubbleColor = color;
    _textColor = foregroundColor(_bubbleColor);
}


//////  u p d a t e 2 D  ///////
void TaskBubble::update(const QPoint& shift)
{
    _pos += shift;

    QPainterPath path;
    path.addText(_pos,  _subFont, _daysLeft);
    path.addText(_pos.x(), path.boundingRect().top()-2, _font, _text);
    path.addText(_pos.x(), path.boundingRect().top()-2, _subFont, _header);
    _textBounds = path.boundingRect();

    _bubbleRect = _textBounds.marginsAdded(QMargins(_margin, _margin, _margin, _margin));
    path.addRoundedRect(_bubbleRect, _radius, _radius);

    QRectF bounds = path.boundingRect();
    double dx = _pos.x() - (bounds.left() + bounds.width()/2);
    double dy = _pos.y() - (bounds.top() + bounds.height()/2);
    path.translate(dx, dy); // bubble centre is the position
    _textBounds.translate(dx, dy);
    _bubbleRect.translate(dx, dy);

    setPath(path);

    QLinearGradient grad(_bubbleRect.topLeft(), _bubbleRect.bottomLeft());
    grad.setColorAt(0.0, _bubbleColor.lighter());
    grad.setColorAt(1.0, _bubbleColor.darker(200));

    delete _bubbleBrush;
    _bubbleBrush = new QBrush(grad);

    update3D();
}


//////  u p d a t e 3 D  ///////
void TaskBubble::update3D()
{
    int dist = _distance<0? 0: _distance - _origin;
    if(0.02*dist+1.0 > 0)
        setScale(0.15*_scale / (log(0.02*dist+1.0) + 0.1));

    double blurRadius = dist>=0? log(dist+1.0): -dist;//log(1.5-dist);
    if(blurRadius > 5.0) blurRadius = 5.0;
    _blur.setBlurRadius(blurRadius);
    setGraphicsEffect(&_blur);

    if(dist >= 0)
        setOpacity(0.2 / (log(0.02*dist+1.0) + 0.1));
    else
        setOpacity(0.5/(-dist));
}
