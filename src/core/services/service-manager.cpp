#include "core/services/service-manager.hpp"

#include <gio/gio.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace {

constexpr char SYSTEMD_BUS_NAME[] = "org.freedesktop.systemd1";
constexpr char SYSTEMD_OBJECT_PATH[] = "/org/freedesktop/systemd1";
constexpr char SYSTEMD_MANAGER_INTERFACE[] = "org.freedesktop.systemd1.Manager";
constexpr char LIST_UNIT_FILES_METHOD[] = "ListUnitFiles";
constexpr char LIST_UNITS_METHOD[] = "ListUnits";
constexpr char START_UNIT_METHOD[] = "StartUnit";
constexpr char STOP_UNIT_METHOD[] = "StopUnit";
constexpr char ENABLE_UNIT_FILES_METHOD[] = "EnableUnitFiles";
constexpr char DISABLE_UNIT_FILES_METHOD[] = "DisableUnitFiles";

void throwGlibError(const char* context, GError* error) {
    const std::string message = error == nullptr ? "unknown error" : error->message;
    if (error != nullptr) {
        g_error_free(error);
    }

    throw std::runtime_error(std::string(context) + ": " + message);
}

GDBusProxy* createSystemdProxy() {
    GError* error = nullptr;
    auto* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                SYSTEMD_BUS_NAME, SYSTEMD_OBJECT_PATH,
                                                SYSTEMD_MANAGER_INTERFACE, nullptr, &error);
    if (proxy == nullptr) {
        throwGlibError("Unable to connect to systemd", error);
    }

    return proxy;
}

void callSystemdMethod(const char* methodName, GVariant* parameters) {
    auto* proxy = createSystemdProxy();
    auto proxyCleanup =
        std::unique_ptr<GDBusProxy, decltype(&g_object_unref)>(proxy, &g_object_unref);
    GError* error = nullptr;
    auto* result = g_dbus_proxy_call_sync(proxy, methodName, parameters,
                                          G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION, -1,
                                          nullptr, &error);
    if (result == nullptr) {
        throwGlibError("Unable to update system service", error);
    }

    g_variant_unref(result);
}

}  // namespace

namespace xenon {

std::vector<ServiceInfo> ServiceManager::filterServiceUnits(
    std::span<const std::pair<std::string, std::string>> unitFiles) {
    std::vector<ServiceInfo> services;

    for (const auto& [unitFile, enablement] : unitFiles) {
        const std::string unitName = std::filesystem::path(unitFile).filename().string();
        if (std::string_view(unitName).ends_with(".service")) {
            services.push_back(
                {.unitName = unitName, .enablement = enablement, .runtimeState = "inactive"});
        }
    }

    std::ranges::sort(services, {}, &ServiceInfo::unitName);
    return services;
}

void ServiceManager::addRuntimeStates(
    std::span<ServiceInfo> services,
    std::span<const std::pair<std::string, std::string>> unitStates) {
    const std::unordered_map<std::string, std::string> statesByUnit(unitStates.begin(),
                                                                    unitStates.end());

    for (ServiceInfo& service : services) {
        const auto iterator = statesByUnit.find(service.unitName);
        if (iterator != statesByUnit.end()) {
            service.runtimeState = iterator->second;
        }
    }
}

void ServiceManager::disableService(std::string_view unitName) const {
    const std::string unitNameCopy(unitName);
    const char* unitFiles[] = {unitNameCopy.c_str(), nullptr};
    callSystemdMethod(DISABLE_UNIT_FILES_METHOD, g_variant_new("(^asb)", unitFiles, FALSE));
}

void ServiceManager::enableService(std::string_view unitName) const {
    const std::string unitNameCopy(unitName);
    const char* unitFiles[] = {unitNameCopy.c_str(), nullptr};
    callSystemdMethod(ENABLE_UNIT_FILES_METHOD, g_variant_new("(^asbb)", unitFiles, FALSE, FALSE));
}

std::vector<ServiceInfo> ServiceManager::listServices() const {
    auto* proxy = createSystemdProxy();
    auto proxyCleanup =
        std::unique_ptr<GDBusProxy, decltype(&g_object_unref)>(proxy, &g_object_unref);
    GError* error = nullptr;
    auto* result = g_dbus_proxy_call_sync(proxy, LIST_UNIT_FILES_METHOD, nullptr,
                                          G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (result == nullptr) {
        throwGlibError("Unable to list systemd unit files", error);
    }

    auto resultCleanup =
        std::unique_ptr<GVariant, decltype(&g_variant_unref)>(result, &g_variant_unref);
    auto* unitFilesVariant = g_variant_get_child_value(result, 0);
    auto unitFilesCleanup =
        std::unique_ptr<GVariant, decltype(&g_variant_unref)>(unitFilesVariant, &g_variant_unref);
    GVariantIter iterator;
    const char* unitName = nullptr;
    const char* enablement = nullptr;
    std::vector<std::pair<std::string, std::string>> unitFiles;

    g_variant_iter_init(&iterator, unitFilesVariant);
    while (g_variant_iter_next(&iterator, "(&s&s)", &unitName, &enablement)) {
        unitFiles.emplace_back(unitName, enablement);
    }

    auto* unitsResult = g_dbus_proxy_call_sync(proxy, LIST_UNITS_METHOD, nullptr,
                                               G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (unitsResult == nullptr) {
        throwGlibError("Unable to list active systemd units", error);
    }

    auto unitsResultCleanup =
        std::unique_ptr<GVariant, decltype(&g_variant_unref)>(unitsResult, &g_variant_unref);
    auto* unitsVariant = g_variant_get_child_value(unitsResult, 0);
    auto unitsCleanup =
        std::unique_ptr<GVariant, decltype(&g_variant_unref)>(unitsVariant, &g_variant_unref);
    std::vector<std::pair<std::string, std::string>> unitStates;

    g_variant_iter_init(&iterator, unitsVariant);
    while (auto* unitVariant = g_variant_iter_next_value(&iterator)) {
        auto unitCleanup =
            std::unique_ptr<GVariant, decltype(&g_variant_unref)>(unitVariant, &g_variant_unref);
        auto* unitNameVariant = g_variant_get_child_value(unitVariant, 0);
        auto unitNameCleanup = std::unique_ptr<GVariant, decltype(&g_variant_unref)>(
            unitNameVariant, &g_variant_unref);
        auto* activeStateVariant = g_variant_get_child_value(unitVariant, 4);
        auto activeStateCleanup = std::unique_ptr<GVariant, decltype(&g_variant_unref)>(
            activeStateVariant, &g_variant_unref);

        unitStates.emplace_back(g_variant_get_string(unitNameVariant, nullptr),
                                g_variant_get_string(activeStateVariant, nullptr));
    }

    std::vector<ServiceInfo> services = filterServiceUnits(unitFiles);
    addRuntimeStates(services, unitStates);
    return services;
}

void ServiceManager::startService(std::string_view unitName) const {
    const std::string unitNameCopy(unitName);
    callSystemdMethod(START_UNIT_METHOD, g_variant_new("(ss)", unitNameCopy.c_str(), "replace"));
}

void ServiceManager::stopService(std::string_view unitName) const {
    const std::string unitNameCopy(unitName);
    callSystemdMethod(STOP_UNIT_METHOD, g_variant_new("(ss)", unitNameCopy.c_str(), "replace"));
}

}  // namespace xenon
