#include "CsvExportService.h"

#include <QStringList>

namespace {
QString escapeCsvField(QString field)
{
    const bool needsQuotes = field.contains(QLatin1Char(','))
        || field.contains(QLatin1Char('\n'))
        || field.contains(QLatin1Char('\r'))
        || field.contains(QLatin1Char('"'));

    if (field.contains(QLatin1Char('"'))) {
        field.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    }

    if (needsQuotes) {
        return QStringLiteral("\"%1\"").arg(field);
    }

    return field;
}

QString makeCsvLine(const QStringList &fields)
{
    QStringList escapedFields;
    escapedFields.reserve(fields.size());

    for (const QString &field : fields) {
        escapedFields.append(escapeCsvField(field));
    }

    return escapedFields.join(QLatin1Char(','));
}
}

QString CsvExportService::exportTransactions(const QList<Transaction> &transactions) const
{
    QStringList lines;
    lines.append(makeCsvLine({
        QStringLiteral("编号"),
        QStringLiteral("类型"),
        QStringLiteral("金额"),
        QStringLiteral("分类"),
        QStringLiteral("日期"),
        QStringLiteral("备注"),
        QStringLiteral("创建时间"),
        QStringLiteral("更新时间")
    }));

    for (const Transaction &transaction : transactions) {
        lines.append(makeCsvLine({
            QString::number(transaction.id()),
            Transaction::typeToString(transaction.type()),
            QString::number(transaction.amount(), 'f', 2),
            transaction.category(),
            transaction.date(),
            transaction.note(),
            transaction.createdAt(),
            transaction.updatedAt()
        }));
    }

    return lines.join(QLatin1Char('\n'));
}

