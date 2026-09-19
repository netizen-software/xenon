#include "main-window.hpp"

#include "ui/process-manager-view.hpp"
#include "ui/repository-manager-view.hpp"
#include "ui/resource-monitor-view.hpp"
#include "ui/services-manager-view.hpp"
#include "ui/system-cleaner-view.hpp"

namespace xenon {

MainWindow::MainWindow(AdwApplication* application)
    : window_(ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(application)))),
      processManagerView_(std::make_unique<ProcessManagerView>()),
      resourceMonitorView_(std::make_unique<ResourceMonitorView>()) {
    gtk_window_set_title(GTK_WINDOW(window_), "Xenon");
    gtk_window_set_default_size(GTK_WINDOW(window_), 1120, 720);
    gtk_window_set_resizable(GTK_WINDOW(window_), TRUE);

    auto* shell = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    auto* sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    auto* brand = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    auto* brandName = gtk_label_new("Xenon");
    auto* brandTagline = gtk_label_new("SYSTEM TOOLS");
    auto* viewStack = GTK_STACK(gtk_stack_new());
    auto* stackSidebar = gtk_stack_sidebar_new();
    auto* toolbarView = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());
    auto* headerBar = ADW_HEADER_BAR(adw_header_bar_new());
    auto* windowTitle = adw_window_title_new("Xenon", "System utility suite");

    servicesManagerView_ = std::make_unique<ServicesManagerView>();
    systemCleanerView_ = std::make_unique<SystemCleanerView>();
    repositoryManagerView_ = std::make_unique<RepositoryManagerView>();

    auto* resourcePage = gtk_stack_add_titled(viewStack, resourceMonitorView_->getWidget(),
                                              "resources", "Resources");
    auto* processPage =
        gtk_stack_add_titled(viewStack, processManagerView_->getWidget(), "processes", "Processes");
    auto* servicePage =
        gtk_stack_add_titled(viewStack, servicesManagerView_->getWidget(), "services", "Services");
    auto* cleanerPage =
        gtk_stack_add_titled(viewStack, systemCleanerView_->getWidget(), "cleaner", "Cleaner");
    auto* repositoryPage = gtk_stack_add_titled(viewStack, repositoryManagerView_->getWidget(),
                                                "repositories", "Repositories");
    gtk_stack_page_set_icon_name(resourcePage, "utilities-system-monitor-symbolic");
    gtk_stack_page_set_icon_name(processPage, "view-list-symbolic");
    gtk_stack_page_set_icon_name(servicePage, "system-run-symbolic");
    gtk_stack_page_set_icon_name(cleanerPage, "user-trash-symbolic");
    gtk_stack_page_set_icon_name(repositoryPage, "folder-remote-symbolic");
    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(stackSidebar), viewStack);
    gtk_widget_set_vexpand(stackSidebar, TRUE);
    gtk_label_set_xalign(GTK_LABEL(brandName), 0);
    gtk_label_set_xalign(GTK_LABEL(brandTagline), 0);
    gtk_widget_add_css_class(sidebar, "app-sidebar");
    gtk_widget_add_css_class(brand, "app-brand");
    gtk_widget_add_css_class(brandName, "app-brand-name");
    gtk_widget_add_css_class(brandTagline, "app-brand-tagline");
    gtk_box_append(GTK_BOX(brand), brandName);
    gtk_box_append(GTK_BOX(brand), brandTagline);
    gtk_box_append(GTK_BOX(sidebar), brand);
    gtk_box_append(GTK_BOX(sidebar), stackSidebar);
    gtk_widget_set_size_request(sidebar, 224, -1);
    adw_header_bar_set_title_widget(headerBar, windowTitle);
    adw_toolbar_view_add_top_bar(toolbarView, GTK_WIDGET(headerBar));
    adw_toolbar_view_set_content(toolbarView, GTK_WIDGET(viewStack));
    gtk_box_append(GTK_BOX(shell), sidebar);
    gtk_box_append(GTK_BOX(shell), GTK_WIDGET(toolbarView));
    gtk_widget_set_hexpand(GTK_WIDGET(toolbarView), TRUE);
    adw_application_window_set_content(window_, shell);
}

MainWindow::~MainWindow() { g_object_unref(window_); }

void MainWindow::present() const { gtk_window_present(GTK_WINDOW(window_)); }

}  // namespace xenon
