#include "main-window.hpp"

#include "ui/resource-monitor-view.hpp"

namespace xenon {

MainWindow::MainWindow(AdwApplication* application)
    : window_(ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(application)))),
      resourceMonitorView_(std::make_unique<ResourceMonitorView>()) {
    gtk_window_set_title(GTK_WINDOW(window_), "Xenon");
    gtk_window_set_default_size(GTK_WINDOW(window_), 960, 640);

    auto* toolbarView = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    auto* headerBar = ADW_HEADER_BAR(adw_header_bar_new());
    auto* title = adw_window_title_new("Xenon", "Resource Monitor");

    adw_header_bar_set_title_widget(headerBar, title);
    adw_toolbar_view_add_top_bar(toolbarView, GTK_WIDGET(headerBar));
    adw_toolbar_view_set_content(toolbarView, resourceMonitorView_->getWidget());
    gtk_window_set_child(GTK_WINDOW(window_), GTK_WIDGET(toolbarView));
}

MainWindow::~MainWindow() {
    g_object_unref(window_);
}

void MainWindow::present() const {
    gtk_window_present(GTK_WINDOW(window_));
}

}  // namespace xenon
