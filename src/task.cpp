#include <QXmlStreamAttributes>
#include "task.h"
#include "taskdialog.h"

using namespace ProPer;

QHash<QString, QColor> Task::_categories = {std::pair<QString, QColor>("undefined", QColor(128,128,0))};
QString Task::_dateShortFormat = QLatin1String("yyyyMMdd");
bool Task::_hiddenIfCompleted = false;
bool Task::_modified = false;


Task::Task():
    QTreeWidgetItem(QTreeWidgetItem::UserType)
{
    _logger = spdlog::get("task");
    _logger->debug("constructing new task");

    clear();
}


Task::Task(const Task& task):
    QTreeWidgetItem(QTreeWidgetItem::UserType)
{
    _logger = spdlog::get("task");
    _logger->debug("copy-constructing new task");

    *this = task;
}


////////  o p e r a t o r  =  ///////
const Task& Task::operator=(const Task& t)
{
    _name = t._name;
    _description = t._description;
    _category = t._category;
    _deadline = t._deadline;
    _status = t._status;
    _notes = t._notes;
    return *this;
}


///////  c l e a r  ////////
void Task::clear()
{
    _name.clear();
    _description.clear();
    _status = BACKLOG;
    _notes.clear();
    _category = QLatin1String("undefined");
    _deadline = QDate::currentDate(); // deafult is today
}


//////  h i d e  C o m p l e t e d  //////
void Task::hideCompleted()
{
    if(_status == DONE)
        setHidden(_hiddenIfCompleted);

    for(int i=0; i<childCount(); ++i) // apply recursively
        dynamic_cast<Task*>(child(i))->hideCompleted();
}


////////  r u n  E d i t o r  ///////
bool Task::runEditor()
{
    TaskDialog dlg;
    dlg.setWindowFlags(dlg.windowFlags()&(~Qt::WindowContextHelpButtonHint)); // no questions!

    QString header = isProject()? "New Project": project()->name();
    if(!isProject() && !parentTask()->isProject())
        header += QString(": ") + parentTask()->name();
    dlg.setWindowTitle(header);

    dlg.initEdit(treeWidget(), this);
    if(!dlg.exec())
        return false;

    *this = dlg.task();
    setText(0, name());
    setHidden(_status == DONE? _hiddenIfCompleted: false);
    updateDeadline(_deadline);
    _modified = true;

    return true;
}


///////  u p d a t e  D e a d l i n e  ///////
void Task::updateDeadline(const QDate &latest)
{
    if(_deadline < latest){
        _deadline = latest;
        if(!isProject())
            parentTask()->updateDeadline(latest);
    }
}


////////  s e t  S t a t u s  /////////
void Task::setStatus(Status s)
{
    _status = s;
    QFont f = font(0);
    if(_status == DONE){
        f.setStrikeOut(true);
        setForeground(0, QBrush(Qt::gray));
    }
    else{
        f.setStrikeOut(false);
        setForeground(0, QBrush(Qt::black));
    }
    setFont(0, f);
}


const QString Task::statusString() const
{
    if(_status == DONE)
        return "done";
    if(_status == BACKLOG)
        return "to do";
//    if(_status == PROGRESS)
//        return "in progress";
//    if(_status == REVIEW)
//        return "in review";
    return "frozen";
}


void Task::assignStatus(const QString status)
{
    status.toLower();
    if(status == "done")
        setStatus(DONE);
    else if(status == "to do")
        setStatus(BACKLOG);
//    else if(status == "in progress")
//        setStatus(PROGRESS);
//    else if(status == "in review")
//        setStatus(REVIEW);
    else
        setStatus(ICEBOX);
}


double Task::priority()
{
    Task* p = dynamic_cast<Task*>(parent());
    if(p == nullptr) return 1.0; // is it right? should projects be prioritised as well? perhaps not..

    double n = p->childCount();
    double i = n - p->indexOfChild(this); // top-down
    return (i / n) * p->priority(); // recursively calculate global priority
}


///////  s a v e T o X m l  ///////
void Task::saveToXml(QXmlStreamWriter& writer) const
{
    writer.writeStartElement((parent() == nullptr)? "project": "task");
    writer.writeAttribute("name", text(0));
    writer.writeAttribute("status", statusString());
    writer.writeAttribute("category", _category);
    writer.writeAttribute("deadline", _deadline.toString(_dateShortFormat));

    if(!_description.isEmpty())
        writer.writeTextElement("description", _description);

    for(int i=0; i<_notes.size(); ++i){
        writer.writeStartElement("note");
        writer.writeAttribute("date", _notes.at(i).date().toString());
        writer.writeAttribute("time", _notes.at(i).time().toString());
        writer.writeCharacters(_notes.at(i).html());
        writer.writeEndElement();
    }

    for(int i=0; i<childCount(); ++i)
        dynamic_cast<Task*>(child(i))->saveToXml(writer);

    writer.writeEndElement();
}


///////  l o a d F r o m X m l  //////
void Task::loadFromXml(QXmlStreamReader& reader)
{
    if(!reader.isStartElement())
        return;
    if(reader.name() != QLatin1String("project") && reader.name() != QLatin1String("task"))
        return;

    // read attributes
    const QXmlStreamAttributes& attr = reader.attributes();
    QStringRef text = attr.value(QStringLiteral("name"));
    if(text.isEmpty())
        return;

    _name = text.toString();
    setText(0, _name);

    assignStatus(attr.value(QStringLiteral("status")).toString());
    _category = attr.value(QStringLiteral("category")).toString();
    _deadline = QDate::fromString(attr.value(QStringLiteral("deadline")).toString(), _dateShortFormat);

    for(;;){
        // read children
        reader.readNext();

        if(reader.isStartElement()){
            if(reader.name() == QLatin1String("task")){ // sub-task
                Task* item = new Task();
                item->loadFromXml(reader);
                addChild(item);
            }
            if(reader.name() == QLatin1String("description")){
                _description = reader.readElementText();
            }
            if(reader.name() == QLatin1String("note")){
                QDateTime dt(QDate::fromString(reader.attributes().value(QStringLiteral("date")).toString()),
                             QTime::fromString(reader.attributes().value(QStringLiteral("time")).toString()));
                Note note;
                note.setDateTime(dt);
                note.setHtml(reader.readElementText());
                _notes.push_back(note);
            }
        }
        else if(reader.isEndElement()){
            if(reader.name() == QLatin1String("project") || reader.name() == QLatin1String("task"))
                break;
        }
        else if(reader.atEnd()){
            _logger->error("ill-formed XML file");
            return;
        }        
    }
}


//////  s a v e  C a t e g o r i e s  T o  X m l  //////
void Task::saveCategoriesToXml(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("categories"));
    QHash<QString,QColor>::const_iterator cat;
    for(cat=_categories.cbegin(); cat!=_categories.cend(); ++cat){
        writer.writeStartElement(QStringLiteral("category"));
        writer.writeAttribute(QStringLiteral("name"), cat.key());
        writer.writeAttribute(QStringLiteral("color"), cat.value().name());
        writer.writeEndElement();
    }
    writer.writeEndElement();
}


///////  l o a d  C a t e g o r i e s  F r o m  X m l  //////
bool Task::loadCategoriesFromXml(QXmlStreamReader& reader)
{
    if(!reader.isStartElement() && reader.name() != QLatin1String("categories"))
        return false;

    _categories.clear();

    for(;;){
        reader.readNext();

        if(reader.isStartElement()){
            if(reader.name() == QLatin1String("category")){
                const QXmlStreamAttributes& attr = reader.attributes();
                QStringRef name = attr.value(QStringLiteral("name"));
                QColor color(attr.value(QStringLiteral("color")).toString());
                _categories[name.toString()] = color;
            }
        }
        else if(reader.isEndElement()){
            if(reader.name() == QLatin1String("categories"))
                break;
        }
        else if(reader.atEnd()){
            return false;
        }
    }
    return true;
}
