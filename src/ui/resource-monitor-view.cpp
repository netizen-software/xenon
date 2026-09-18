#include "ui/resource-monitor-view.hpp"

#include <glib.h>

#include <exception>

namespace {

constexpr char RESOURCE_PATH[] = "/io/github/netizen_software/Xenon/ui/resource-monitor.ui";
constexpr char VIEW_ID[] = "resource_monitor_view";
constexpr char CPU_VALUE_ID[] = "cpu_value";
constexpr char MEMORY_VALUE_ID[] = "memory_value";

}  // namespace

namespace xenon {

ResourceMonitorView::ResourceMonitorView()
    : widget_(nullptr), cpuValue_(nullptr), memoryValue_(nullptr), refreshSourceId_(0) {
    auto* builder = gtk_builder_new_from_resource(RESOURCE_PATH);
    widget_ = GTK_WIDGET(gtk_builder_get_object(builder, VIEW_ID));
    cpuValue_ = GTK_LABEL(gtk_builder_get_object(builder, CPU_VALUE_ID));
    memoryValue_ = GTK_LABEL(gtk_builder_get_object(builder, MEMORY_VALUE_ID));

    g_object_ref(widget_);
    g_object_unref(builder);

    refresh();
    refreshSourceId_ = g_timeout_add_seconds(1, ResourceMonitorView::onRefresh, this);
}

ResourceMonitorView::~ResourceMonitorView() {
    if (refreshSourceId_ != 0) {
        g_source_remove(refreshSourceId_);
    }

    g_object_unref(widget_);
}

GtkWidget* ResourceMonitorView::getWidget() const { return widget_; }

gboolean ResourceMonitorView::onRefresh(gpointer userData) {
    static_cast<ResourceMonitorView*>(userData)->refresh();
    return G_SOURCE_CONTINUE;
}

void ResourceMonitorView::refresh() {
    try {
        const ResourceUsage usage = resourceMonitor_.sample();
        const std::uint64_t usedBytes = usage.memory.totalBytes - usage.memory.availableBytes;
        auto* cpuText = g_strdup_printf("%.1f%%", usage.cpuPercent);
        auto* memoryText = g_format_size(usedBytes);

        gtk_label_set_text(cpuValue_, cpuText);
        gtk_label_set_text(memoryValue_, memoryText);

        g_free(cpuText);
        g_free(memoryText);
    } catch (const std::exception&) {
        gtk_label_set_text(cpuValue_, "Unavailable");
        gtk_label_set_text(memoryValue_, "Unavailable");
    }
}

}  // namespace xenon
