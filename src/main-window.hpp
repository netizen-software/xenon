#pragma once

#include <adwaita.h>

#include <memory>

namespace xenon {

class ProcessManagerView;
class ResourceMonitorView;

class MainWindow {
   public:
    explicit MainWindow(AdwApplication* application);
    ~MainWindow();

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    void present() const;

   private:
    AdwApplicationWindow* window_;
    std::unique_ptr<ProcessManagerView> processManagerView_;
    std::unique_ptr<ResourceMonitorView> resourceMonitorView_;
};

}  // namespace xenon
