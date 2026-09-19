#include "core/services/service-manager.hpp"

#include <gio/gio.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {

constexpr char SYSTEMD_BUS_NAME[] = "org.freedesktop.systemd1";
constexpr char SYSTEMD_OBJECT_PATH[] = "/org/freedesktop/systemd1";
constexpr char SYSTEMD_MANAGER_INTERFACE[] = "org.freedesktop.systemd1.Manager";
constexpr char LIST_UNIT_FILES_METHOD[] = "ListUnitFiles";

void throwGlibError(const char* context, GError* error) {
    const std::string message = error == nullptr ? "unknown error" : error->message;
    if (error != nullptr) {
        g_error_free(error);
    }

    throw std::runtime_error(std::string(context) + ": " + message);
}

}  // namespace

namespace xenon {

std::vector<ServiceInfo> ServiceManager::filterServiceUnits(
    std::span<const std::pair<std::string, std::string>> unitFiles) {
    std::vector<ServiceInfo> services;

    for (const auto& [unitName, enablement] : unitFiles) {
        if (std::string_view(unitName).ends_with(".service")) {
            services.push_back({.unitName = unitName, .enablement = enablement});
        }
    }

    std::ranges::sort(services, {}, &ServiceInfo::unitName);
    return services;
}

std::vector<ServiceInfo> ServiceManager::listServices() const {
    GError* error = nullptr;
    auto* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                SYSTEMD_BUS_NAME, SYSTEMD_OBJECT_PATH,
                                                SYSTEMD_MANAGER_INTERFACE, nullptr, &error);
    if (proxy == nullptr) {
        throwGlibError("Unable to connect to systemd", error);
    }

    auto proxyCleanup =
        std::unique_ptr<GDBusProxy, decltype(&g_object_unref)>(proxy, &g_object_unref);
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

    return filterServiceUnits(unitFiles);
}

}  // namespace xenon
