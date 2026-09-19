#pragma once

#include <gtk/gtk.h>

#include "core/services/service-manager.hpp"

namespace xenon {

class ServicesManagerView {
   public:
    ServicesManagerView();
    ~ServicesManagerView();

    ServicesManagerView(const ServicesManagerView&) = delete;
    ServicesManagerView& operator=(const ServicesManagerView&) = delete;

    GtkWidget* getWidget() const;

   private:
    enum class ServiceAction {
        Start,
        Stop,
        Enable,
        Disable,
    };

    static void onDisable(GtkButton* button, gpointer userData);
    static void onEnable(GtkButton* button, gpointer userData);
    static void onRefresh(GtkButton* button, gpointer userData);
    static void onSelectionChanged(GtkListBox* listBox, GtkListBoxRow* row, gpointer userData);
    static void onStart(GtkButton* button, gpointer userData);
    static void onStop(GtkButton* button, gpointer userData);
    void performAction(ServiceAction action);
    void refresh();
    void setActionButtonsSensitive(bool sensitive);

    GtkWidget* widget_;
    GtkButton* disableButton_;
    GtkButton* enableButton_;
    GtkListBox* serviceList_;
    GtkButton* startButton_;
    GtkLabel* statusLabel_;
    GtkButton* stopButton_;
    ServiceManager serviceManager_;
};

}  // namespace xenon
