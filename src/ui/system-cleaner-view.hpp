#pragma once

#include <gtk/gtk.h>

#include "core/cleaner/system-cleaner.hpp"

namespace xenon {

class SystemCleanerView {
   public:
    SystemCleanerView();
    ~SystemCleanerView();

    SystemCleanerView(const SystemCleanerView&) = delete;
    SystemCleanerView& operator=(const SystemCleanerView&) = delete;

    GtkWidget* getWidget() const;

   private:
    static void onClean(GtkButton* button, gpointer userData);
    static void onRefresh(GtkButton* button, gpointer userData);
    void cleanSelected();
    void refresh();

    GtkWidget* widget_;
    GtkButton* cleanButton_;
    GtkListBox* cleanerList_;
    GtkLabel* statusLabel_;
    SystemCleaner systemCleaner_;
};

}  // namespace xenon
