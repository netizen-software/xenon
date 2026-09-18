#include "main-window.hpp"

#include "ui/process-manager-view.hpp"
#include "ui/resource-monitor-view.hpp"

namespace xenon {

MainWindow::MainWindow(AdwApplication* application)
    : window_(ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(application)))),
      processManagerView_(std::make_unique<ProcessManagerView>()),
      resourceMonitorView_(std::make_unique<ResourceMonitorView>()) {
    gtk_window_set_title(GTK_WINDOW(window_), "Xenon");
    gtk_window_set_default_size(GTK_WINDOW(window_), 960, 640);

    auto* toolbarView = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    auto* headerBar = ADW_HEADER_BAR(adw_header_bar_new());
    auto* viewStack = ADW_VIEW_STACK(adw_view_stack_new());
    auto* viewSwitcher = adw_view_switcher_new();

    adw_view_stack_add_titled(viewStack, resourceMonitorView_->getWidget(), "resources",
                              "Resources");
    adw_view_stack_add_titled(viewStack, processManagerView_->getWidget(), "processes",
                              "Processes");
    adw_view_switcher_set_stack(ADW_VIEW_SWITCHER(viewSwitcher), viewStack);
    adw_header_bar_set_title_widget(headerBar, viewSwitcher);
    adw_toolbar_view_add_top_bar(toolbarView, GTK_WIDGET(headerBar));
    adw_toolbar_view_set_content(toolbarView, GTK_WIDGET(viewStack));
    adw_application_window_set_content(window_, GTK_WIDGET(toolbarView));
}

MainWindow::~MainWindow() { g_object_unref(window_); }

void MainWindow::present() const { gtk_window_present(GTK_WINDOW(window_)); }

}  // namespace xenon
