#pragma once

#include <QTreeWidgetItem>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringRef>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "spdlog/spdlog.h"
#include "note.h"

namespace ProPer {

class Task: public QTreeWidgetItem
{
public:
    enum Status{
        ICEBOX,
        BACKLOG,
//        PROGRESS,
//        REVIEW,
        DONE
    };

public:
    Task();
    Task(const Task& task);

    const Task& operator=(const Task& t);

    void clear();

    // classification
    inline bool isProject() const {return parent() == nullptr;}
    inline bool isTask() const {return childCount() == 0;}
    inline bool isComponent() const {return !(isProject() || isTask());}

    //operations
    void saveToXml(QXmlStreamWriter& writer) const;
    void loadFromXml(QXmlStreamReader& reader);

    // ui-related
    void hideCompleted(); // if DONE tasks have to be hidden, hide this task and children in UI
    bool runEditor();

    // model-related
    Task* project() {return parentTask()? parentTask()->project(): this;}
    Task* parentTask() {return dynamic_cast<Task*>(parent());}

    // properties
    inline const QString& name() const {return _name;}
    inline void setName(const QString& name) {_name = name;}

    inline const QString& description() const {return _description;}
    inline void setDescription(const QString& description) {_description = description;}

    inline const QDate& deadline() const {return _deadline;}
    inline void setDeadline(const QDate& deadline) {if(isTask()) _deadline = deadline;}
    void updateDeadline(const QDate& latest); // recursively push upper tasks' deadline

    double priority();

    inline const int noteCount() const {return _notes.size();}
    inline void appendNote(const Note& note) {_notes.push_back(note);}
    inline void updateNote(int pos, const Note& note) {_notes[pos] = note;}
    inline void deleteNote(int pos) {if(0<=pos && pos<_notes.size()) _notes.removeAt(pos);}
    inline const Note note(int pos) {return pos<_notes.size()? _notes.at(pos): Note();}

    inline Status status() const {return _status;}
    void setStatus(Status s);

    const QString& category() const {return _category;}
    void assignCategory(const QString& category) {_category = category;}

    // persistent static properties
    static inline QHash<QString, QColor>& categories() {return _categories;}
    static inline QColor categoryColor(const QString& category) {return _categories.value(category, QColor(128,128,128));}
    static inline void setCategoryColor(const QString& category, const QColor& color) {_categories[category] = color;}
//    static inline bool isCategoryPresent(const QString& category) {return _categories.find(category) > 0;}
    static inline void removeCategory(const QString& category) {_categories.remove(category);}

    static inline bool hiddenIfCompleted() {return _hiddenIfCompleted;}
    static inline void setHiddenIfCompleted(bool hidden=true) {_hiddenIfCompleted = hidden;}

    static inline bool isModified() {return _modified;}
    static inline void setModified(bool modified) {_modified = modified;}

    static void saveCategoriesToXml(QXmlStreamWriter& writer);
    static bool loadCategoriesFromXml(QXmlStreamReader& reader);

    const QString statusString() const;
    void assignStatus(const QString status);

private:



private:
    QString _name; // short name
    QString _description; // long description of the problem

    Status _status;

    double _priotity; // [0-1], unique between all siblings
    QDate _deadline; // when done, keeps the finished date

    QList<Note> _notes;

    static QHash<QString, QColor> _categories;
    QString _category;

    static QString _dateShortFormat;

    static bool _hiddenIfCompleted;
    static bool _modified; // flag raised if any task was changed

    std::shared_ptr<spdlog::logger> _logger;
};

} // namespace
