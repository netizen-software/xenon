#include "application.hpp"

#include "main-window.hpp"

namespace {

constexpr char APPLICATION_ID[] = "io.github.netizen_software.Xenon";

}

namespace xenon {

Application::Application()
    : application_(adw_application_new(APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS)) {
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
    if (!mainWindow_) {
        mainWindow_ = std::make_unique<MainWindow>(application_);
    }

    mainWindow_->present();
}

}  // namespace xenon
