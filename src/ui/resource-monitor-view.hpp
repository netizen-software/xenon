#pragma once

#include <gtk/gtk.h>

#include <deque>

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
    static void onCpuGraphDraw(GtkDrawingArea* area, cairo_t* context, int width, int height,
                               gpointer userData);
    static void onMemoryGraphDraw(GtkDrawingArea* area, cairo_t* context, int width, int height,
                                  gpointer userData);
    static gboolean onRefresh(gpointer userData);
    void addSample(std::deque<double>& history, double value);
    void drawGraph(cairo_t* context, int width, int height, const std::deque<double>& history,
                   double red, double green, double blue) const;
    void refresh();

    GtkWidget* widget_;
    GtkLabel* cpuValue_;
    GtkLabel* memoryValue_;
    GtkDrawingArea* cpuGraph_;
    GtkDrawingArea* memoryGraph_;
    guint refreshSourceId_;
    std::deque<double> cpuHistory_;
    std::deque<double> memoryHistory_;
    ResourceMonitor resourceMonitor_;
};

}  // namespace xenon
