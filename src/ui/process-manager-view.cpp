#include "ui/process-manager-view.hpp"

#include <exception>

namespace {

constexpr char RESOURCE_PATH[] = "/io/github/netizen_software/Xenon/ui/process-manager.ui";
constexpr char VIEW_ID[] = "process_manager_view";
constexpr char PROCESS_LIST_ID[] = "process_list";
constexpr char TERMINATE_BUTTON_ID[] = "terminate_button";
constexpr char STATUS_LABEL_ID[] = "status_label";
constexpr char PROCESS_ID_KEY[] = "xenon-process-id";

}  // namespace

namespace xenon {

ProcessManagerView::ProcessManagerView()
    : widget_(nullptr),
      processList_(nullptr),
      terminateButton_(nullptr),
      statusLabel_(nullptr),
      refreshSourceId_(0) {
    auto* builder = gtk_builder_new_from_resource(RESOURCE_PATH);
    widget_ = GTK_WIDGET(gtk_builder_get_object(builder, VIEW_ID));
    processList_ = GTK_LIST_BOX(gtk_builder_get_object(builder, PROCESS_LIST_ID));
    terminateButton_ = GTK_BUTTON(gtk_builder_get_object(builder, TERMINATE_BUTTON_ID));
    statusLabel_ = GTK_LABEL(gtk_builder_get_object(builder, STATUS_LABEL_ID));

    g_object_ref(widget_);
    g_object_unref(builder);

    g_signal_connect(processList_, "row-selected",
                     G_CALLBACK(ProcessManagerView::onSelectionChanged), this);
    g_signal_connect(terminateButton_, "clicked", G_CALLBACK(ProcessManagerView::onTerminate),
                     this);
    refresh();
    refreshSourceId_ = g_timeout_add_seconds(3, ProcessManagerView::onRefresh, this);
}

ProcessManagerView::~ProcessManagerView() {
    if (refreshSourceId_ != 0) {
        g_source_remove(refreshSourceId_);
    }

    g_object_unref(widget_);
}

GtkWidget* ProcessManagerView::getWidget() const { return widget_; }

void ProcessManagerView::onSelectionChanged(GtkListBox*, GtkListBoxRow* row, gpointer userData) {
    auto* view = static_cast<ProcessManagerView*>(userData);
    gtk_widget_set_sensitive(GTK_WIDGET(view->terminateButton_), row != nullptr);
}

void ProcessManagerView::onTerminate(GtkButton*, gpointer userData) {
    auto* view = static_cast<ProcessManagerView*>(userData);
    auto* selectedRow = gtk_list_box_get_selected_row(view->processList_);
    if (selectedRow == nullptr) {
        return;
    }

    const auto pid = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(selectedRow), PROCESS_ID_KEY));
    if (view->processManager_.terminateProcess(pid)) {
        gtk_label_set_text(view->statusLabel_, "Termination request sent");
        view->refresh();
    } else {
        gtk_label_set_text(view->statusLabel_, "Unable to terminate process");
    }
}

gboolean ProcessManagerView::onRefresh(gpointer userData) {
    static_cast<ProcessManagerView*>(userData)->refresh();
    return G_SOURCE_CONTINUE;
}

void ProcessManagerView::refresh() {
    while (auto* row = gtk_widget_get_first_child(GTK_WIDGET(processList_))) {
        gtk_list_box_remove(processList_, row);
    }

    try {
        for (const ProcessInfo& process : processManager_.listProcesses()) {
            auto* row = gtk_list_box_row_new();
            auto* content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
            auto* name = gtk_label_new(process.name.c_str());
            auto* memoryText = g_format_size(process.memoryBytes);
            auto* details =
                g_strdup_printf("PID %u  %s  %s", process.pid, process.state.c_str(), memoryText);
            auto* metadata = gtk_label_new(details);

            gtk_widget_set_hexpand(name, TRUE);
            gtk_label_set_xalign(GTK_LABEL(name), 0);
            gtk_label_set_xalign(GTK_LABEL(metadata), 1);
            gtk_widget_add_css_class(metadata, "dim-label");
            gtk_box_append(GTK_BOX(content), name);
            gtk_box_append(GTK_BOX(content), metadata);
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), content);
            g_object_set_data(G_OBJECT(row), PROCESS_ID_KEY, GUINT_TO_POINTER(process.pid));
            gtk_list_box_append(processList_, row);
            g_free(details);
            g_free(memoryText);
        }
    } catch (const std::exception&) {
        gtk_label_set_text(statusLabel_, "Unable to read running processes");
    }
}

}  // namespace xenon
