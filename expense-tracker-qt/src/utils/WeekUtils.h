#pragma once

#include <QDate>
#include <QString>

namespace WeekUtils {

QDate getWeekStartDate(const QDate &date);
QDate getWeekEndDate(const QDate &weekStartDate);
bool isWeekStartDate(const QDate &date);
QString formatDate(const QDate &date);
QDate parseDate(const QString &date);

}
