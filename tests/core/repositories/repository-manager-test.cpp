#include "core/repositories/repository-manager.hpp"

#include <cassert>
#include <filesystem>

int main() {
    const auto listRepository = xenon::RepositoryManager::parseSourceFile(
        "/etc/apt/sources.list.d/example.list",
        "# deb https://disabled.example/ubuntu noble main\n"
        "deb [arch=amd64] https://enabled.example/ubuntu noble main\n");
    assert(listRepository.has_value());
    assert(listRepository->enabled);
    assert(listRepository->removable);
    assert(listRepository->summary == "deb https://disabled.example/ubuntu noble main");

    const auto deb822Repository =
        xenon::RepositoryManager::parseSourceFile("/etc/apt/sources.list.d/example.sources",
                                                  "Types: deb\n"
                                                  "URIs: https://deb822.example/ubuntu\n"
                                                  "Suites: noble\n"
                                                  "Enabled: no\n");
    assert(deb822Repository.has_value());
    assert(!deb822Repository->enabled);
    assert(deb822Repository->summary == "https://deb822.example/ubuntu noble");

    assert(xenon::RepositoryManager::isValidRepositoryLine(
        "deb https://example.org/ubuntu noble main"));
    assert(xenon::RepositoryManager::isValidRepositoryLine(
        "deb-src [signed-by=/key] https://example.org source main"));
    assert(!xenon::RepositoryManager::isValidRepositoryLine("https://example.org/ubuntu"));
    assert(!xenon::RepositoryManager::isValidRepositoryLine(
        "deb https://example.org/ubuntu\nmalicious"));

    return 0;
}