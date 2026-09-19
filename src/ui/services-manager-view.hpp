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
    static void onRefresh(GtkButton* button, gpointer userData);
    void refresh();

    GtkWidget* widget_;
    GtkListBox* serviceList_;
    GtkLabel* statusLabel_;
    ServiceManager serviceManager_;
};

}  // namespace xenon
