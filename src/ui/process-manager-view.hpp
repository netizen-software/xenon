#pragma once

#include <gtk/gtk.h>

#include "core/processes/process-manager.hpp"

namespace xenon {

class ProcessManagerView {
   public:
    ProcessManagerView();
    ~ProcessManagerView();

    ProcessManagerView(const ProcessManagerView&) = delete;
    ProcessManagerView& operator=(const ProcessManagerView&) = delete;

    GtkWidget* getWidget() const;

   private:
    static void onSelectionChanged(GtkListBox* listBox, GtkListBoxRow* row, gpointer userData);
    static void onTerminate(GtkButton* button, gpointer userData);
    static gboolean onRefresh(gpointer userData);
    void refresh();

    GtkWidget* widget_;
    GtkListBox* processList_;
    GtkButton* terminateButton_;
    GtkLabel* statusLabel_;
    guint refreshSourceId_;
    ProcessManager processManager_;
};

}  // namespace xenon
