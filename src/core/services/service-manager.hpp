#pragma once

#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xenon {

struct ServiceInfo {
    std::string unitName;
    std::string enablement;
    std::string runtimeState;
};

class ServiceManager {
   public:
    void disableService(std::string_view unitName) const;
    void enableService(std::string_view unitName) const;
    std::vector<ServiceInfo> listServices() const;
    void startService(std::string_view unitName) const;
    void stopService(std::string_view unitName) const;

    static std::vector<ServiceInfo> filterServiceUnits(
        std::span<const std::pair<std::string, std::string>> unitFiles);
    static void addRuntimeStates(std::span<ServiceInfo> services,
                                 std::span<const std::pair<std::string, std::string>> unitStates);
};

}  // namespace xenon
