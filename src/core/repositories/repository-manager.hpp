#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace xenon {

struct RepositoryInfo {
    std::filesystem::path sourceFile;
    std::string summary;
    bool enabled;
    bool removable;
};

class RepositoryManager {
   public:
    void addRepository(std::string_view repositoryLine) const;
    void deleteRepository(const RepositoryInfo& repository) const;
    std::vector<RepositoryInfo> listRepositories() const;
    void setRepositoryEnabled(const RepositoryInfo& repository, bool enabled) const;

    static bool isValidRepositoryLine(std::string_view repositoryLine);
    static std::optional<RepositoryInfo> parseSourceFile(const std::filesystem::path& sourceFile,
                                                         std::string_view content);
};

}  // namespace xenon
