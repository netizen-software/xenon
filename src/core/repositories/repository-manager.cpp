#include "core/repositories/repository-manager.hpp"

#include <gio/gio.h>

#include <algorithm>
#include <fstream>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

constexpr char PRIMARY_SOURCES_PATH[] = "/etc/apt/sources.list";
constexpr char SOURCES_DIRECTORY[] = "/etc/apt/sources.list.d";
constexpr char LIST_EXTENSION[] = ".list";
constexpr char DEB822_EXTENSION[] = ".sources";
constexpr char LIST_SOURCE_PREFIX[] = "deb ";
constexpr char LIST_SOURCE_WITH_OPTIONS_PREFIX[] = "deb [";

std::string trim(std::string_view value) {
    const std::size_t start = value.find_first_not_of(" \t\r");
    if (start == std::string_view::npos) {
        return {};
    }

    const std::size_t end = value.find_last_not_of(" \t\r");
    return std::string(value.substr(start, end - start + 1));
}

bool isListSourceLine(std::string_view line) {
    const std::string source = trim(line);
    return source.starts_with(LIST_SOURCE_PREFIX) ||
           source.starts_with(LIST_SOURCE_WITH_OPTIONS_PREFIX) || source.starts_with("deb-src ") ||
           source.starts_with("deb-src [");
}

std::string uncomment(std::string_view line) {
    std::string source = trim(line);
    if (source.starts_with('#')) {
        source.erase(0, 1);
    }

    return trim(source);
}

bool hasSourceExtension(const std::filesystem::path& sourceFile) {
    const std::string extension = sourceFile.extension().string();
    return extension == LIST_EXTENSION || extension == DEB822_EXTENSION;
}

bool isManagedSourceFile(const std::filesystem::path& sourceFile) {
    const std::filesystem::path normalizedPath = sourceFile.lexically_normal();
    return (normalizedPath == PRIMARY_SOURCES_PATH ||
            normalizedPath.parent_path() == SOURCES_DIRECTORY) &&
           hasSourceExtension(normalizedPath);
}

bool isRemovableSourceFile(const std::filesystem::path& sourceFile) {
    return sourceFile.lexically_normal().parent_path() == SOURCES_DIRECTORY;
}

std::string readFile(const std::filesystem::path& sourceFile) {
    std::ifstream input(sourceFile);
    if (!input) {
        throw std::runtime_error("Unable to read " + sourceFile.string());
    }

    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void throwGlibError(const char* context, GError* error) {
    const std::string message = error == nullptr ? "unknown error" : error->message;
    if (error != nullptr) {
        g_error_free(error);
    }

    throw std::runtime_error(std::string(context) + ": " + message);
}

void runPrivilegedCommand(const std::vector<const char*>& command, const char* input = nullptr) {
    std::vector<const char*> arguments{"pkexec"};
    arguments.insert(arguments.end(), command.begin(), command.end());
    arguments.push_back(nullptr);
    GError* error = nullptr;
    const auto flags = static_cast<GSubprocessFlags>(G_SUBPROCESS_FLAGS_STDIN_PIPE |
                                                     G_SUBPROCESS_FLAGS_STDERR_PIPE);
    auto* process = g_subprocess_newv(arguments.data(), flags, &error);
    if (process == nullptr) {
        throwGlibError("Unable to authorize repository update", error);
    }

    auto processCleanup =
        std::unique_ptr<GSubprocess, decltype(&g_object_unref)>(process, &g_object_unref);
    gchar* errorText = nullptr;
    if (!g_subprocess_communicate_utf8(process, input, nullptr, nullptr, &errorText, &error)) {
        const std::string message = error == nullptr ? "unknown error" : error->message;
        if (error != nullptr) {
            g_error_free(error);
        }
        g_free(errorText);
        throw std::runtime_error("Repository update failed: " + message);
    }

    if (!g_subprocess_get_successful(process)) {
        const std::string message = errorText == nullptr ? "authorization denied" : errorText;
        g_free(errorText);
        throw std::runtime_error("Repository update failed: " + message);
    }

    g_free(errorText);
}

std::string listContentWithEnabledState(std::string_view content, bool enabled) {
    std::istringstream input{std::string(content)};
    std::ostringstream output;
    std::string line;

    while (std::getline(input, line)) {
        const std::string sourceLine = uncomment(line);
        if (isListSourceLine(sourceLine)) {
            output << (enabled ? sourceLine : "# " + sourceLine) << '\n';
        } else {
            output << line << '\n';
        }
    }

    return output.str();
}

std::string deb822ContentWithEnabledState(std::string_view content, bool enabled) {
    std::istringstream input{std::string(content)};
    std::ostringstream output;
    std::string line;
    bool enabledFieldFound = false;

    while (std::getline(input, line)) {
        if (trim(line).starts_with("Enabled:")) {
            output << "Enabled: " << (enabled ? "yes" : "no") << '\n';
            enabledFieldFound = true;
        } else {
            output << line << '\n';
        }
    }

    if (!enabledFieldFound) {
        output << "Enabled: " << (enabled ? "yes" : "no") << '\n';
    }

    return output.str();
}

}  // namespace

namespace xenon {

void RepositoryManager::addRepository(std::string_view repositoryLine) const {
    if (!isValidRepositoryLine(repositoryLine)) {
        throw std::runtime_error("Enter a valid deb or deb-src repository line");
    }

    auto* identifier = g_uuid_string_random();
    const std::filesystem::path sourceFile = std::filesystem::path(SOURCES_DIRECTORY) /
                                             ("xenon-" + std::string(identifier) + LIST_EXTENSION);
    const std::string sourceContent = trim(repositoryLine) + "\n";
    g_free(identifier);
    runPrivilegedCommand({"/usr/bin/tee", sourceFile.c_str()}, sourceContent.c_str());
}

void RepositoryManager::deleteRepository(const RepositoryInfo& repository) const {
    if (!repository.removable || !isRemovableSourceFile(repository.sourceFile)) {
        throw std::runtime_error("The primary APT source file cannot be deleted");
    }

    const std::string sourcePath = repository.sourceFile.string();
    runPrivilegedCommand({"/usr/bin/rm", "--", sourcePath.c_str()});
}

bool RepositoryManager::isValidRepositoryLine(std::string_view repositoryLine) {
    const std::string value = trim(repositoryLine);
    return value.find_first_of("\r\n") == std::string::npos && isListSourceLine(value);
}

std::vector<RepositoryInfo> RepositoryManager::listRepositories() const {
    std::vector<RepositoryInfo> repositories;
    const std::filesystem::path primarySourceFile(PRIMARY_SOURCES_PATH);
    std::error_code error;

    if (std::filesystem::is_regular_file(primarySourceFile, error)) {
        if (const auto repository =
                parseSourceFile(primarySourceFile, readFile(primarySourceFile))) {
            repositories.push_back(*repository);
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(
             SOURCES_DIRECTORY, std::filesystem::directory_options::skip_permission_denied,
             error)) {
        if (error) {
            error.clear();
            continue;
        }

        if (!entry.is_regular_file(error) || !hasSourceExtension(entry.path())) {
            error.clear();
            continue;
        }

        if (const auto repository = parseSourceFile(entry.path(), readFile(entry.path()))) {
            repositories.push_back(*repository);
        }
    }

    std::ranges::sort(repositories, {}, &RepositoryInfo::sourceFile);
    return repositories;
}

std::optional<RepositoryInfo> RepositoryManager::parseSourceFile(
    const std::filesystem::path& sourceFile, std::string_view content) {
    if (!hasSourceExtension(sourceFile)) {
        return std::nullopt;
    }

    if (sourceFile.extension() == LIST_EXTENSION) {
        std::istringstream input{std::string(content)};
        std::string line;
        bool foundSource = false;
        bool enabled = false;
        std::string summary;

        while (std::getline(input, line)) {
            const std::string sourceLine = uncomment(line);
            if (!isListSourceLine(sourceLine)) {
                continue;
            }

            foundSource = true;
            enabled = enabled || !trim(line).starts_with('#');
            if (summary.empty()) {
                summary = sourceLine;
            }
        }

        if (!foundSource) {
            return std::nullopt;
        }

        return RepositoryInfo{.sourceFile = sourceFile,
                              .summary = summary,
                              .enabled = enabled,
                              .removable = isRemovableSourceFile(sourceFile)};
    }

    bool hasDebType = false;
    bool enabled = true;
    std::string uri;
    std::string suite;
    std::istringstream input{std::string(content)};
    std::string line;
    while (std::getline(input, line)) {
        const std::string field = trim(line);
        if (field.starts_with("Types:")) {
            hasDebType = field.find("deb", 6) != std::string::npos;
        } else if (field.starts_with("URIs:")) {
            uri = trim(field.substr(5));
        } else if (field.starts_with("Suites:")) {
            suite = trim(field.substr(7));
        } else if (field.starts_with("Enabled:")) {
            enabled = trim(field.substr(8)) != "no";
        }
    }

    if (!hasDebType) {
        return std::nullopt;
    }

    return RepositoryInfo{.sourceFile = sourceFile,
                          .summary = uri.empty() ? "Deb822 APT source" : uri + " " + suite,
                          .enabled = enabled,
                          .removable = isRemovableSourceFile(sourceFile)};
}

void RepositoryManager::setRepositoryEnabled(const RepositoryInfo& repository, bool enabled) const {
    if (!isManagedSourceFile(repository.sourceFile)) {
        throw std::runtime_error("Unsupported APT source file");
    }

    const std::string content = readFile(repository.sourceFile);
    const std::string updatedContent = repository.sourceFile.extension() == LIST_EXTENSION
                                           ? listContentWithEnabledState(content, enabled)
                                           : deb822ContentWithEnabledState(content, enabled);
    const std::string sourcePath = repository.sourceFile.string();
    runPrivilegedCommand({"/usr/bin/tee", sourcePath.c_str()}, updatedContent.c_str());
}

}  // namespace xenon
