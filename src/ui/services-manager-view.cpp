#include "ui/services-manager-view.hpp"

#include <exception>

namespace {

constexpr char RESOURCE_PATH[] = "/io/github/netizen_software/Xenon/ui/services-manager.ui";
constexpr char VIEW_ID[] = "services_manager_view";
constexpr char SERVICE_LIST_ID[] = "service_list";
constexpr char STATUS_LABEL_ID[] = "status_label";
constexpr char REFRESH_BUTTON_ID[] = "refresh_button";
constexpr char START_BUTTON_ID[] = "start_button";
constexpr char STOP_BUTTON_ID[] = "stop_button";
constexpr char ENABLE_BUTTON_ID[] = "enable_button";
constexpr char DISABLE_BUTTON_ID[] = "disable_button";
constexpr char SERVICE_NAME_KEY[] = "xenon-service-name";
constexpr int ENABLEMENT_COLUMN_WIDTH = 140;
constexpr int RUNTIME_COLUMN_WIDTH = 120;

}  // namespace

namespace xenon {

ServicesManagerView::ServicesManagerView()
    : widget_(nullptr),
      disableButton_(nullptr),
      enableButton_(nullptr),
      serviceList_(nullptr),
      startButton_(nullptr),
      statusLabel_(nullptr),
      stopButton_(nullptr) {
    auto* builder = gtk_builder_new_from_resource(RESOURCE_PATH);
    widget_ = GTK_WIDGET(gtk_builder_get_object(builder, VIEW_ID));
    serviceList_ = GTK_LIST_BOX(gtk_builder_get_object(builder, SERVICE_LIST_ID));
    statusLabel_ = GTK_LABEL(gtk_builder_get_object(builder, STATUS_LABEL_ID));
    auto* refreshButton = GTK_BUTTON(gtk_builder_get_object(builder, REFRESH_BUTTON_ID));
    startButton_ = GTK_BUTTON(gtk_builder_get_object(builder, START_BUTTON_ID));
    stopButton_ = GTK_BUTTON(gtk_builder_get_object(builder, STOP_BUTTON_ID));
    enableButton_ = GTK_BUTTON(gtk_builder_get_object(builder, ENABLE_BUTTON_ID));
    disableButton_ = GTK_BUTTON(gtk_builder_get_object(builder, DISABLE_BUTTON_ID));

    g_object_ref(widget_);
    g_object_unref(builder);

    g_signal_connect(refreshButton, "clicked", G_CALLBACK(ServicesManagerView::onRefresh), this);
    g_signal_connect(serviceList_, "row-selected",
                     G_CALLBACK(ServicesManagerView::onSelectionChanged), this);
    g_signal_connect(startButton_, "clicked", G_CALLBACK(ServicesManagerView::onStart), this);
    g_signal_connect(stopButton_, "clicked", G_CALLBACK(ServicesManagerView::onStop), this);
    g_signal_connect(enableButton_, "clicked", G_CALLBACK(ServicesManagerView::onEnable), this);
    g_signal_connect(disableButton_, "clicked", G_CALLBACK(ServicesManagerView::onDisable), this);
    refresh();
}

ServicesManagerView::~ServicesManagerView() { g_object_unref(widget_); }

GtkWidget* ServicesManagerView::getWidget() const { return widget_; }

void ServicesManagerView::onRefresh(GtkButton*, gpointer userData) {
    static_cast<ServicesManagerView*>(userData)->refresh();
}

void ServicesManagerView::onSelectionChanged(GtkListBox*, GtkListBoxRow* row, gpointer userData) {
    static_cast<ServicesManagerView*>(userData)->setActionButtonsSensitive(row != nullptr);
}

void ServicesManagerView::onStart(GtkButton*, gpointer userData) {
    static_cast<ServicesManagerView*>(userData)->performAction(ServiceAction::Start);
}

void ServicesManagerView::onStop(GtkButton*, gpointer userData) {
    static_cast<ServicesManagerView*>(userData)->performAction(ServiceAction::Stop);
}

void ServicesManagerView::onEnable(GtkButton*, gpointer userData) {
    static_cast<ServicesManagerView*>(userData)->performAction(ServiceAction::Enable);
}

void ServicesManagerView::onDisable(GtkButton*, gpointer userData) {
    static_cast<ServicesManagerView*>(userData)->performAction(ServiceAction::Disable);
}

void ServicesManagerView::performAction(ServiceAction action) {
    auto* selectedRow = gtk_list_box_get_selected_row(serviceList_);
    if (selectedRow == nullptr) {
        return;
    }

    const auto* unitName =
        static_cast<const char*>(g_object_get_data(G_OBJECT(selectedRow), SERVICE_NAME_KEY));
    if (unitName == nullptr) {
        return;
    }

    try {
        const char* actionName = nullptr;
        switch (action) {
            case ServiceAction::Start:
                serviceManager_.startService(unitName);
                actionName = "Start requested for";
                break;
            case ServiceAction::Stop:
                serviceManager_.stopService(unitName);
                actionName = "Stop requested for";
                break;
            case ServiceAction::Enable:
                serviceManager_.enableService(unitName);
                actionName = "Enabled";
                break;
            case ServiceAction::Disable:
                serviceManager_.disableService(unitName);
                actionName = "Disabled";
                break;
        }

        auto* statusText = g_strdup_printf("%s %s", actionName, unitName);
        refresh();
        gtk_label_set_text(statusLabel_, statusText);
        g_free(statusText);
    } catch (const std::exception& error) {
        gtk_label_set_text(statusLabel_, error.what());
    }
}

void ServicesManagerView::refresh() {
    while (auto* row = gtk_widget_get_first_child(GTK_WIDGET(serviceList_))) {
        gtk_list_box_remove(serviceList_, row);
    }

    try {
        const std::vector<ServiceInfo> services = serviceManager_.listServices();
        for (const ServiceInfo& service : services) {
            auto* row = gtk_list_box_row_new();
            auto* content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
            auto* unitName = gtk_label_new(service.unitName.c_str());
            auto* runtimeState = gtk_label_new(service.runtimeState.c_str());
            auto* enablement = gtk_label_new(service.enablement.c_str());

            gtk_widget_set_hexpand(unitName, TRUE);
            gtk_label_set_xalign(GTK_LABEL(unitName), 0);
            gtk_label_set_ellipsize(GTK_LABEL(unitName), PANGO_ELLIPSIZE_END);
            gtk_widget_set_size_request(runtimeState, RUNTIME_COLUMN_WIDTH, -1);
            gtk_widget_set_size_request(enablement, ENABLEMENT_COLUMN_WIDTH, -1);
            gtk_label_set_xalign(GTK_LABEL(runtimeState), 1);
            gtk_label_set_xalign(GTK_LABEL(enablement), 1);
            gtk_widget_add_css_class(runtimeState, "dim-label");
            gtk_widget_add_css_class(enablement, "dim-label");
            gtk_box_append(GTK_BOX(content), unitName);
            gtk_box_append(GTK_BOX(content), runtimeState);
            gtk_box_append(GTK_BOX(content), enablement);
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), content);
            g_object_set_data_full(G_OBJECT(row), SERVICE_NAME_KEY,
                                   g_strdup(service.unitName.c_str()), g_free);
            gtk_list_box_append(serviceList_, row);
        }

        auto* statusText = g_strdup_printf("%zu service unit files", services.size());
        gtk_label_set_text(statusLabel_, statusText);
        g_free(statusText);
    } catch (const std::exception&) {
        gtk_label_set_text(statusLabel_, "Unable to read system services");
    }
}

void ServicesManagerView::setActionButtonsSensitive(bool sensitive) {
    gtk_widget_set_sensitive(GTK_WIDGET(startButton_), sensitive);
    gtk_widget_set_sensitive(GTK_WIDGET(stopButton_), sensitive);
    gtk_widget_set_sensitive(GTK_WIDGET(enableButton_), sensitive);
    gtk_widget_set_sensitive(GTK_WIDGET(disableButton_), sensitive);
}

}  // namespace xenon
