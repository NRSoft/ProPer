#include "note.h"

using namespace ProPer;

Note::Note()
{
    _lastUpdate = QDateTime::currentDateTime();
}

void Note::pruneHtml()
{
    int start = _html.indexOf(QLatin1String("<body"));
    if(start < 0) return;
    int end = _html.indexOf(QLatin1String("body>"));
    if(end <= 0) return;
    _html = _html.mid(start, end-start+5);
}
