#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace xenon {

enum class CleanerCategory {
    PackageCache,
    CrashReports,
    HistoricalLogs,
    ApplicationCache,
    Trash,
};

struct CleanerItem {
    CleanerCategory category;
    std::string title;
    std::string description;
    std::uintmax_t sizeBytes;
};

class SystemCleaner {
   public:
    void clean(CleanerCategory category) const;
    std::vector<CleanerItem> listItems() const;

    static bool isHistoricalLog(const std::filesystem::path& path);
    static bool isPackageArchive(const std::filesystem::path& path);
};

}  // namespace xenon
