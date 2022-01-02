#include "afickReport.h"

namespace afick_report
{
    std::string report::toJson()
    {
        nlohmann::json json(*this);
        return std::string(date + " " + json.dump()) + "\n";
    }
    report makeReport(const QStringList& rows)
    {
        report report;

        auto parseHeader = [&](const QString& str) -> void {
            enum HeaderWordsPlace
            {
                STATUS,
                TYPE,
                NO_USE,
                PATH,
                COUNT
            };
            auto splitResult = str.splitRef(" ", QString::SkipEmptyParts);
            if (splitResult.size() < COUNT)
                return;
            change change;
            change.status = splitResult[STATUS].toString().toStdString();
            change.type   = splitResult[TYPE].toString().toStdString();
            change.path   = splitResult[PATH].toString().toStdString();
            report.changes.emplace_back(change);
        };
        auto parseDetails = [&](const QString& str) -> void {
            afick_report::attr_change atttribute;
            auto                      typeAndChanges = str.splitRef(" : ", QString::SkipEmptyParts);
            auto                      modifications  = typeAndChanges.back().split('\t');

            atttribute.after = modifications.back().toString().toStdString();
            if (modifications.size() > 1) // both 'before' and 'after' modifications available
                atttribute.before = modifications.front().toString().toStdString();

            atttribute.name = typeAndChanges.front().toString().remove('\t').trimmed().toStdString();
            report.changes.back().attributes.emplace_back(atttribute);
        };

        for (const auto& string : rows)
        {
            if (string.isEmpty())
                continue;

            if (string.front().isLetter())
                parseHeader(string);
            else if (string.front() == '\t')
                parseDetails(string);
            else if (string.front().isNumber())
                report.date = string.toStdString();
        }
        report.changesCount = report.changes.size();
        return report;
    }

    report makeReport(const QString& report)
    {
        auto details = getReportDetails(report);
        if (details.isEmpty())
            return {};
        return makeReport(details);
    }

    QString findDayReport(uint16_t daysBack)
    {
        static QDir archive(LogFolder);
        auto        date      = QDateTime::currentDateTime().date().toString(Qt::DateFormat::ISODate).remove('-');
        auto        dayBefore = QDateTime::currentDateTime().date().addDays(-1).toString(Qt::DateFormat::ISODate).remove('-');

        archive.setSorting(QDir::SortFlag::Time);
        auto entries = archive.entryList({ LogPrefix + QString::number(date.toUInt() - daysBack) + "*" });
        if (entries.isEmpty())
            return {};
        else
            return LogFolder + "/" + entries.first();
    };

    QStringList getReportDetails(const QString& report)
    {
        QFile file(report);
        if (not file.open(QFile::ReadOnly))
            return {};

        auto list = QString::fromStdString(file.readAll().toStdString()).split('\n');
        file.close();

        QString date("");
        while (list.front() != TagDetails)
        {
            if (date.isEmpty() && list.front().contains(" at "))
            {
                date = list.first().section(" ", 5, 6); // is the position of date value
            }
            list.pop_front();
            if (list.isEmpty())
                break;
        }
        list.push_back(date);
        return list;
    };
}; //namespace afick_report
