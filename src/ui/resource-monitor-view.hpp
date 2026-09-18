#pragma once

#include <gtk/gtk.h>

#include "core/resources/resource-monitor.hpp"

namespace xenon {

class ResourceMonitorView {
  public:
    ResourceMonitorView();
    ~ResourceMonitorView();

    ResourceMonitorView(const ResourceMonitorView&) = delete;
    ResourceMonitorView& operator=(const ResourceMonitorView&) = delete;

    GtkWidget* getWidget() const;

  private:
    static gboolean onRefresh(gpointer userData);
    void refresh();

    GtkWidget* widget_;
    GtkLabel* cpuValue_;
    GtkLabel* memoryValue_;
    guint refreshSourceId_;
    ResourceMonitor resourceMonitor_;
};

}  // namespace xenon
