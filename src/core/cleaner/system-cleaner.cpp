#include "core/cleaner/system-cleaner.hpp"

#include <gio/gio.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

constexpr char APT_ARCHIVES_PATH[] = "/var/cache/apt/archives";
constexpr char CRASH_REPORTS_PATH[] = "/var/crash";
constexpr char SYSTEM_LOGS_PATH[] = "/var/log";

std::filesystem::path userCachePath() { return g_get_user_cache_dir(); }

std::filesystem::path userTrashPath() {
    return std::filesystem::path(g_get_user_data_dir()) / "Trash";
}

std::uintmax_t directorySize(
    const std::filesystem::path& path,
    const std::function<bool(const std::filesystem::path&)>& shouldInclude) {
    std::uintmax_t totalSize = 0;
    std::error_code error;
    const std::filesystem::recursive_directory_iterator iterator(
        path, std::filesystem::directory_options::skip_permission_denied, error);

    for (auto entry = iterator; entry != std::filesystem::recursive_directory_iterator();
         entry.increment(error)) {
        if (error || !entry->is_regular_file(error) || !shouldInclude(entry->path())) {
            error.clear();
            continue;
        }

        totalSize += entry->file_size(error);
        error.clear();
    }

    return totalSize;
}

void removeDirectoryContents(const std::filesystem::path& path) {
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(
             path, std::filesystem::directory_options::skip_permission_denied, error)) {
        std::filesystem::remove_all(entry.path(), error);
        if (error) {
            throw std::runtime_error("Unable to delete " + entry.path().string() + ": " +
                                     error.message());
        }
    }
}

void runPrivilegedCommand(std::initializer_list<const char*> command) {
    std::vector<const char*> arguments{"pkexec"};
    arguments.insert(arguments.end(), command.begin(), command.end());
    arguments.push_back(nullptr);
    GError* error = nullptr;
    auto* process = g_subprocess_newv(arguments.data(), G_SUBPROCESS_FLAGS_STDERR_PIPE, &error);
    if (process == nullptr) {
        const std::string message = error == nullptr ? "unknown error" : error->message;
        if (error != nullptr) {
            g_error_free(error);
        }
        throw std::runtime_error("Unable to authorize cleanup: " + message);
    }

    auto processCleanup =
        std::unique_ptr<GSubprocess, decltype(&g_object_unref)>(process, &g_object_unref);
    gchar* errorText = nullptr;
    if (!g_subprocess_communicate_utf8(process, nullptr, nullptr, nullptr, &errorText, &error)) {
        const std::string message = error == nullptr ? "unknown error" : error->message;
        if (error != nullptr) {
            g_error_free(error);
        }
        g_free(errorText);
        throw std::runtime_error("Cleanup failed: " + message);
    }

    if (!g_subprocess_get_successful(process)) {
        const std::string message = errorText == nullptr ? "authorization denied" : errorText;
        g_free(errorText);
        throw std::runtime_error("Cleanup failed: " + message);
    }

    g_free(errorText);
}

}  // namespace

namespace xenon {

bool SystemCleaner::isHistoricalLog(const std::filesystem::path& path) {
    const std::string filename = path.filename().string();
    const std::string extension = path.extension().string();
    if (extension == ".gz" || extension == ".old") {
        return true;
    }

    const std::size_t dotPosition = filename.find_last_of('.');
    if (dotPosition == std::string::npos || dotPosition + 1 == filename.size()) {
        return false;
    }

    const std::string_view suffix(filename.c_str() + dotPosition + 1);
    return std::all_of(suffix.begin(), suffix.end(),
                       [](unsigned char character) { return std::isdigit(character) != 0; });
}

bool SystemCleaner::isPackageArchive(const std::filesystem::path& path) {
    const std::string extension = path.extension().string();
    return extension == ".deb" || extension == ".ddeb" || extension == ".udeb";
}

void SystemCleaner::clean(CleanerCategory category) const {
    switch (category) {
        case CleanerCategory::PackageCache:
            runPrivilegedCommand({"/usr/bin/apt-get", "clean"});
            break;
        case CleanerCategory::CrashReports:
            runPrivilegedCommand({"/usr/bin/find", CRASH_REPORTS_PATH, "-mindepth", "1",
                                  "-maxdepth", "1", "-type", "f", "-delete"});
            break;
        case CleanerCategory::HistoricalLogs:
            runPrivilegedCommand({"/usr/bin/find", SYSTEM_LOGS_PATH, "-type", "f", "(", "-name",
                                  "*.gz", "-o", "-name", "*.old", "-o", "-regextype",
                                  "posix-extended", "-regex", ".*\\.[0-9]+", ")", "-delete"});
            break;
        case CleanerCategory::ApplicationCache:
            removeDirectoryContents(userCachePath());
            break;
        case CleanerCategory::Trash:
            removeDirectoryContents(userTrashPath());
            break;
    }
}

std::vector<CleanerItem> SystemCleaner::listItems() const {
    return {
        {.category = CleanerCategory::PackageCache,
         .title = "Package Cache",
         .description = "Downloaded APT package archives",
         .sizeBytes = directorySize(APT_ARCHIVES_PATH, isPackageArchive)},
        {.category = CleanerCategory::CrashReports,
         .title = "Crash Reports",
         .description = "Reports created by crashed applications and services",
         .sizeBytes =
             directorySize(CRASH_REPORTS_PATH, [](const std::filesystem::path&) { return true; })},
        {.category = CleanerCategory::HistoricalLogs,
         .title = "Historical Logs",
         .description = "Rotated system, package manager, and application logs",
         .sizeBytes = directorySize(SYSTEM_LOGS_PATH, isHistoricalLog)},
        {.category = CleanerCategory::ApplicationCache,
         .title = "Application Cache",
         .description = "Temporary data created by applications",
         .sizeBytes =
             directorySize(userCachePath(), [](const std::filesystem::path&) { return true; })},
        {.category = CleanerCategory::Trash,
         .title = "Trash",
         .description = "Items stored in the desktop environment trash",
         .sizeBytes =
             directorySize(userTrashPath(), [](const std::filesystem::path&) { return true; })},
    };
}

}  // namespace xenon
