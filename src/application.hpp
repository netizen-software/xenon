#pragma once

#include <adwaita.h>

#include <memory>

namespace xenon {

class MainWindow;

class Application {
   public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run(int argc, char* argv[]);

   private:
    static void onActivate(GApplication* application, gpointer userData);
    void activate();
    void loadStyle();

    AdwApplication* application_;
    bool styleLoaded_;
    std::unique_ptr<MainWindow> mainWindow_;
};

}  // namespace xenon
