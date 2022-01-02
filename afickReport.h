#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QSet>
#include <QString>
#include <QStringList>

#include <nlohmann/json.hpp>

namespace afick_report
{
    static const QString AfickMakeReport("afick -k");
    static const QString TagDetails("# detailed changes");
    static const QString TagHash("# Hash database");
    static const QString LogFolder("/var/lib/afick/archive");
    static const QString LogPrefix("afick.");

    struct attr_change
    {
        std::string name;
        std::string before;
        std::string after;
        friend void to_json(nlohmann::json& j, const attr_change& o)
        {
            j["attribute"] = o.name;
            j["before"]    = o.before;
            j["after"]     = o.after;
        }
        friend bool operator==(const attr_change& l, const attr_change& r)
        {
            return l.name == r.name && l.before == r.before && l.after == r.after;
        }
    };
    struct change
    {
        std::string              status;
        std::string              type;
        std::string              path;
        std::vector<attr_change> attributes;

        friend void to_json(nlohmann::json& j, const change& o)
        {
            j["status"]     = o.status;
            j["type"]       = o.type;
            j["path"]       = o.path;
            j["attributes"] = nlohmann::json(o.attributes);
        }
        friend bool operator==(const change& l, const change& r)
        {
            bool base = l.status == r.status &&
                        l.type == r.type &&
                        l.path == r.path &&
                        l.attributes.size() == r.attributes.size();
            if (not base)
                return false;
            for (const auto& attr : l.attributes)
            {
                auto rAttributeIt = std::find_if(r.attributes.begin(), r.attributes.end(), [&](const auto& el) {
                    return attr.name == el.name;
                });
                if (rAttributeIt == r.attributes.end())
                    return false;
                if (attr.after != rAttributeIt->after || attr.before != rAttributeIt->before)
                    return false;
            }
            return true;
        }
    };
    struct report
    {
        std::string         date;
        uint64_t            changesCount;
        std::vector<change> changes;
        friend void to_json(nlohmann::json& j, const report& o)
        {
            j["date"]         = o.date;
            j["changes"]      = o.changes;
            j["changesCount"] = o.changesCount;
        }
        friend report operator-(const report& ethalon, const report& r)
        {
            report diff;
            diff.date    = "diff";
            diff.changes = r.changes;
            return diff;
        }
        void rebase(const report& newBase)
        {
            auto changeIt = changes.begin();
            while (changeIt != changes.end())
            {
                auto& change        = *changeIt;
                auto  otherChangeIt = std::find_if(newBase.changes.cbegin(), newBase.changes.cend(), [&](const auto& el) { return change.path == el.path; });

                if (otherChangeIt != newBase.changes.end())
                {
                    const auto& otherChange = *otherChangeIt;
                    // remove fully same(duplicates) change record
                    if (*otherChangeIt == change)
                    {
                        changeIt = changes.erase(changeIt);
                        continue;
                    }
                    else
                    {
                        auto attrIt = change.attributes.begin();
                        // find same attribute changes and remove duplicates
                        while (attrIt != change.attributes.end())
                        {
                            auto& attr         = *attrIt;
                            auto  rAttributeIt = std::find_if(otherChange.attributes.begin(), otherChange.attributes.end(), [&](const auto& el) { return attr.name == el.name; });
                            if (rAttributeIt != otherChange.attributes.end()) // if the same attr exists in base report
                            {
                                if (attr == *rAttributeIt) // remove same attributes change in 'change' record
                                {
                                    attrIt = change.attributes.erase(attrIt);
                                    continue;
                                }
                                else if (attr.after != rAttributeIt->after) // if attribute has change (different to base)
                                    attr.before = rAttributeIt->after;
                            }
                            ++attrIt;
                        }
                    }
                }
                ++changeIt;
            }
        }
        std::string toJson();
    };

    report      makeReport(const QStringList& rows);
    report      makeReport(const QString& report);
    QString     findDayReport(uint16_t daysBack = 0);
    QStringList getReportDetails(const QString& report);
};
