#include <iostream>

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QSet>
#include <QString>
#include <QStringList>

#include "afickReport.h"

#include <nlohmann/json.hpp>

using namespace afick_report;
enum
{
    TODAY,
    YESTERDAY,
};
QString       ReportFolder("/var/log/afick");
QString       ReportName("afick_history");
const QString Extension(".log");
void          printUsage()
{
    std::cout << "Usage: [path/to/report_folder] [file_name] [force_new_report:true/false(default)] (DO NOT specify the file extension, it will be .log)\n";
    std::cout << "No parameters provided - the defaults used:\n";
    std::cout << QString(ReportFolder + ' ' + ReportName + Extension + '\n').toStdString();
};

int main(int argc, char* argv[])
{
    bool forceNewDayReport = false; // Сгенерировать новый отчёт афика перед созданием json
    if (argc >= 3)
    {
        ReportFolder = QString(argv[1]);
        ReportName   = QString(argv[2]);
        if (argc == 4)
        {
            forceNewDayReport = QString(argv[3]) == "true";
        }
    }
    else
        printUsage();

    ReportFolder.append('/');

    auto quitWithMessage = [](int errCode, QString message) -> void {
        std::cerr << QObject::tr("AfickMonitoring finished with error (%1): %2")
                         .arg(errCode)
                         .arg(message)
                         .toStdString()
                  << std::endl;
        exit(errCode);
    };

    auto writeToFile = [](const QString& file, std::string&& data) -> void {
        QFile jsonReport;
        jsonReport.setFileName(file);
        jsonReport.open(QIODevice::WriteOnly);
        jsonReport.write(data.c_str(), data.size());
        jsonReport.close();
    };

    QString todaysReportPath = findDayReport(TODAY);
    if (forceNewDayReport)
    {
        QProcess afick;
        afick.start(afick_report::AfickMakeReport);
        if (not afick.waitForFinished(300000)) // 6 minutes cap;
            quitWithMessage(-1, QObject::tr("Afick execution timeout"));
        auto errCode = afick.exitCode();
        if (afick.exitStatus() == QProcess::ExitStatus::CrashExit)
            quitWithMessage(errCode, afick.errorString());
        todaysReportPath = findDayReport(TODAY);
        if (todaysReportPath.isEmpty())
            quitWithMessage(-1, QObject::tr("No todays report was found (at: %1) "
                                            "after afick execution")
                                    .arg(afick_report::LogFolder));
    }

    auto todayReport = afick_report::makeReport(todaysReportPath);

    // Write json report to file
    const QString jsonReportPath(ReportFolder + ReportName + Extension);
    writeToFile(jsonReportPath, todayReport.toJson());

    // rebase report to yesterday report result (if it exists)
    auto yesterdayReportPath = findDayReport(YESTERDAY);
    if (not yesterdayReportPath.isEmpty())
    {
        auto yesterdayReport = afick_report::makeReport(yesterdayReportPath);
        todayReport.rebase(yesterdayReport);
        writeToFile(ReportFolder + ReportName.append("_dayDiff") + Extension, todayReport.toJson());
    }
    else
        std::cout << "No 'day before' report was found. DayDiff report wasn't created\n";

    return 0;
}
