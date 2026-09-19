#include "ui/resource-monitor-view.hpp"

#include <glib.h>

#include <algorithm>
#include <exception>

namespace {

constexpr char RESOURCE_PATH[] = "/io/github/netizen_software/Xenon/ui/resource-monitor.ui";
constexpr char VIEW_ID[] = "resource_monitor_view";
constexpr char CPU_VALUE_ID[] = "cpu_value";
constexpr char MEMORY_VALUE_ID[] = "memory_value";
constexpr char CPU_GRAPH_ID[] = "cpu_graph";
constexpr char MEMORY_GRAPH_ID[] = "memory_graph";
constexpr std::size_t MAX_HISTORY_SAMPLES = 60;
constexpr double CPU_GRAPH_RED = 0.14;
constexpr double CPU_GRAPH_GREEN = 0.55;
constexpr double CPU_GRAPH_BLUE = 0.42;
constexpr double MEMORY_GRAPH_RED = 0.93;
constexpr double MEMORY_GRAPH_GREEN = 0.45;
constexpr double MEMORY_GRAPH_BLUE = 0.16;

}  // namespace

namespace xenon {

ResourceMonitorView::ResourceMonitorView()
    : widget_(nullptr),
      cpuValue_(nullptr),
      memoryValue_(nullptr),
      cpuGraph_(nullptr),
      memoryGraph_(nullptr),
      refreshSourceId_(0) {
    auto* builder = gtk_builder_new_from_resource(RESOURCE_PATH);
    widget_ = GTK_WIDGET(gtk_builder_get_object(builder, VIEW_ID));
    cpuValue_ = GTK_LABEL(gtk_builder_get_object(builder, CPU_VALUE_ID));
    memoryValue_ = GTK_LABEL(gtk_builder_get_object(builder, MEMORY_VALUE_ID));
    cpuGraph_ = GTK_DRAWING_AREA(gtk_builder_get_object(builder, CPU_GRAPH_ID));
    memoryGraph_ = GTK_DRAWING_AREA(gtk_builder_get_object(builder, MEMORY_GRAPH_ID));

    g_object_ref(widget_);
    g_object_unref(builder);

    gtk_drawing_area_set_draw_func(cpuGraph_, ResourceMonitorView::onCpuGraphDraw, this, nullptr);
    gtk_drawing_area_set_draw_func(memoryGraph_, ResourceMonitorView::onMemoryGraphDraw, this,
                                   nullptr);
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

void ResourceMonitorView::onCpuGraphDraw(GtkDrawingArea*, cairo_t* context, int width, int height,
                                         gpointer userData) {
    const auto* view = static_cast<const ResourceMonitorView*>(userData);
    view->drawGraph(context, width, height, view->cpuHistory_, CPU_GRAPH_RED, CPU_GRAPH_GREEN,
                    CPU_GRAPH_BLUE);
}

void ResourceMonitorView::onMemoryGraphDraw(GtkDrawingArea*, cairo_t* context, int width,
                                            int height, gpointer userData) {
    const auto* view = static_cast<const ResourceMonitorView*>(userData);
    view->drawGraph(context, width, height, view->memoryHistory_, MEMORY_GRAPH_RED,
                    MEMORY_GRAPH_GREEN, MEMORY_GRAPH_BLUE);
}

void ResourceMonitorView::addSample(std::deque<double>& history, double value) {
    if (history.size() == MAX_HISTORY_SAMPLES) {
        history.pop_front();
    }

    history.push_back(std::clamp(value, 0.0, 100.0));
}

void ResourceMonitorView::drawGraph(cairo_t* context, int width, int height,
                                    const std::deque<double>& history, double red, double green,
                                    double blue) const {
    constexpr double GRID_ALPHA = 0.12;
    constexpr double FILL_ALPHA = 0.18;
    constexpr double LINE_WIDTH = 2.0;
    constexpr int GRID_LINES = 4;

    cairo_set_source_rgba(context, 0.0, 0.0, 0.0, 0.06);
    cairo_rectangle(context, 0, 0, width, height);
    cairo_fill(context);

    cairo_set_source_rgba(context, 0.0, 0.0, 0.0, GRID_ALPHA);
    cairo_set_line_width(context, 1.0);
    for (int lineIndex = 1; lineIndex < GRID_LINES; ++lineIndex) {
        const double y = static_cast<double>(height) * lineIndex / GRID_LINES;
        cairo_move_to(context, 0, y);
        cairo_line_to(context, width, y);
    }
    cairo_stroke(context);

    if (history.empty()) {
        return;
    }

    const double sampleWidth = static_cast<double>(width) / (MAX_HISTORY_SAMPLES - 1);
    const double xOffset = width - sampleWidth * (history.size() - 1);
    const auto sampleY = [height](double value) {
        return static_cast<double>(height) * (1.0 - value / 100.0);
    };

    cairo_move_to(context, xOffset, height);
    for (std::size_t index = 0; index < history.size(); ++index) {
        const double x = xOffset + sampleWidth * index;
        cairo_line_to(context, x, sampleY(history[index]));
    }
    cairo_line_to(context, width, height);
    cairo_close_path(context);
    cairo_set_source_rgba(context, red, green, blue, FILL_ALPHA);
    cairo_fill_preserve(context);
    cairo_set_source_rgba(context, red, green, blue, 1.0);
    cairo_set_line_width(context, LINE_WIDTH);
    cairo_stroke(context);
}

void ResourceMonitorView::refresh() {
    try {
        const ResourceUsage usage = resourceMonitor_.sample();
        const std::uint64_t usedBytes = usage.memory.totalBytes - usage.memory.availableBytes;
        const double memoryPercent =
            usage.memory.totalBytes == 0
                ? 0.0
                : 100.0 * static_cast<double>(usedBytes) / usage.memory.totalBytes;
        auto* cpuText = g_strdup_printf("%.1f%%", usage.cpuPercent);
        auto* usedText = g_format_size(usedBytes);
        auto* totalText = g_format_size(usage.memory.totalBytes);
        auto* memoryText = g_strdup_printf("%s of %s", usedText, totalText);

        gtk_label_set_text(cpuValue_, cpuText);
        gtk_label_set_text(memoryValue_, memoryText);
        addSample(cpuHistory_, usage.cpuPercent);
        addSample(memoryHistory_, memoryPercent);
        gtk_widget_queue_draw(GTK_WIDGET(cpuGraph_));
        gtk_widget_queue_draw(GTK_WIDGET(memoryGraph_));

        g_free(cpuText);
        g_free(usedText);
        g_free(totalText);
        g_free(memoryText);
    } catch (const std::exception&) {
        gtk_label_set_text(cpuValue_, "Unavailable");
        gtk_label_set_text(memoryValue_, "Unavailable");
    }
}

}  // namespace xenon
