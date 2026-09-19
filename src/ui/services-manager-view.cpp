#include "ui/services-manager-view.hpp"

#include <exception>

namespace {

constexpr char RESOURCE_PATH[] = "/io/github/netizen_software/Xenon/ui/services-manager.ui";
constexpr char VIEW_ID[] = "services_manager_view";
constexpr char SERVICE_LIST_ID[] = "service_list";
constexpr char STATUS_LABEL_ID[] = "status_label";
constexpr char REFRESH_BUTTON_ID[] = "refresh_button";
constexpr int ENABLEMENT_COLUMN_WIDTH = 140;
constexpr int RUNTIME_COLUMN_WIDTH = 120;

}  // namespace

namespace xenon {

ServicesManagerView::ServicesManagerView()
    : widget_(nullptr), serviceList_(nullptr), statusLabel_(nullptr) {
    auto* builder = gtk_builder_new_from_resource(RESOURCE_PATH);
    widget_ = GTK_WIDGET(gtk_builder_get_object(builder, VIEW_ID));
    serviceList_ = GTK_LIST_BOX(gtk_builder_get_object(builder, SERVICE_LIST_ID));
    statusLabel_ = GTK_LABEL(gtk_builder_get_object(builder, STATUS_LABEL_ID));
    auto* refreshButton = GTK_BUTTON(gtk_builder_get_object(builder, REFRESH_BUTTON_ID));

    g_object_ref(widget_);
    g_object_unref(builder);

    g_signal_connect(refreshButton, "clicked", G_CALLBACK(ServicesManagerView::onRefresh), this);
    refresh();
}

ServicesManagerView::~ServicesManagerView() { g_object_unref(widget_); }

GtkWidget* ServicesManagerView::getWidget() const { return widget_; }

void ServicesManagerView::onRefresh(GtkButton*, gpointer userData) {
    static_cast<ServicesManagerView*>(userData)->refresh();
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
            gtk_list_box_append(serviceList_, row);
        }

        auto* statusText = g_strdup_printf("%zu service unit files", services.size());
        gtk_label_set_text(statusLabel_, statusText);
        g_free(statusText);
    } catch (const std::exception&) {
        gtk_label_set_text(statusLabel_, "Unable to read system services");
    }
}

}  // namespace xenon
