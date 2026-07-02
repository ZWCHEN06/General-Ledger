#include "WeekUtils.h"

namespace {
constexpr const char *DateFormat = "yyyy-MM-dd";
}

namespace WeekUtils {

QDate getWeekStartDate(const QDate &date)
{
    if (!date.isValid()) {
        return QDate();
    }

    return date.addDays(1 - date.dayOfWeek());
}

QDate getWeekEndDate(const QDate &weekStartDate)
{
    if (!weekStartDate.isValid()) {
        return QDate();
    }

    return weekStartDate.addDays(6);
}

bool isWeekStartDate(const QDate &date)
{
    return date.isValid() && date.dayOfWeek() == Qt::Monday;
}

QString formatDate(const QDate &date)
{
    if (!date.isValid()) {
        return QString();
    }

    return date.toString(QString::fromLatin1(DateFormat));
}

QDate parseDate(const QString &date)
{
    const QDate parsedDate = QDate::fromString(date.trimmed(), QString::fromLatin1(DateFormat));
    if (!parsedDate.isValid() || formatDate(parsedDate) != date.trimmed()) {
        return QDate();
    }

    return parsedDate;
}

}
