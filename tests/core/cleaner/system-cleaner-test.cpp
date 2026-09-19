#include "core/cleaner/system-cleaner.hpp"

#include <cassert>
#include <filesystem>

int main() {
    assert(xenon::SystemCleaner::isPackageArchive("package.deb"));
    assert(xenon::SystemCleaner::isPackageArchive("debug.ddeb"));
    assert(!xenon::SystemCleaner::isPackageArchive("lock"));

    assert(xenon::SystemCleaner::isHistoricalLog("syslog.1"));
    assert(xenon::SystemCleaner::isHistoricalLog("apt.log.gz"));
    assert(xenon::SystemCleaner::isHistoricalLog("application.log.old"));
    assert(!xenon::SystemCleaner::isHistoricalLog("syslog"));
    assert(!xenon::SystemCleaner::isHistoricalLog("syslog.1a"));

    return 0;
}