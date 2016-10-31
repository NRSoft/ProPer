#pragma once

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QString>

namespace ProPer {

class Note
{
public:
    Note();

    inline const QDate date() const {return _lastUpdate.date();}
    inline const QTime time() const {return _lastUpdate.time();}
    inline void setDateTime(const QDateTime& datetime) {_lastUpdate = datetime;}

    inline const QString& html() const {return _html;}
    inline void setHtml(const QString& html) {_html = html;}

     void pruneHtml(); // strip of extra tags, leave body only

private:
    QDateTime _lastUpdate;
    QString _html;
};

} // namespace
