#include "application.hpp"

#include "main-window.hpp"

namespace {

constexpr char APPLICATION_ID[] = "io.github.netizen_software.Xenon";

}

namespace xenon {

Application::Application()
    : application_(adw_application_new(APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS)),
      styleLoaded_(false) {
    g_signal_connect(application_, "activate", G_CALLBACK(Application::onActivate), this);
}

Application::~Application() { g_object_unref(application_); }

int Application::run(int argc, char* argv[]) {
    return g_application_run(G_APPLICATION(application_), argc, argv);
}

void Application::onActivate(GApplication*, gpointer userData) {
    static_cast<Application*>(userData)->activate();
}

void Application::activate() {
    loadStyle();

    if (!mainWindow_) {
        mainWindow_ = std::make_unique<MainWindow>(application_);
    }

    mainWindow_->present();
}

void Application::loadStyle() {
    if (styleLoaded_) {
        return;
    }

    auto* display = gdk_display_get_default();
    if (display == nullptr) {
        return;
    }

    auto* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/io/github/netizen_software/Xenon/style.css");
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    styleLoaded_ = true;
}

}  // namespace xenon
