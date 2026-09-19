#include "core/services/service-manager.hpp"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

int main() {
    const std::vector<std::pair<std::string, std::string>> unitFiles{
        {"beta.service", "disabled"},
        {"alpha.service", "enabled"},
        {"systemd.socket", "static"},
    };
    const std::vector<xenon::ServiceInfo> services =
        xenon::ServiceManager::filterServiceUnits(unitFiles);

    assert(services.size() == 2);
    assert(services[0].unitName == "alpha.service");
    assert(services[0].enablement == "enabled");
    assert(services[1].unitName == "beta.service");
    assert(services[1].enablement == "disabled");

    return 0;
}