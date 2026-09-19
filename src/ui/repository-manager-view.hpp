#pragma once

#include <gtk/gtk.h>

#include "core/repositories/repository-manager.hpp"

namespace xenon {

class RepositoryManagerView {
   public:
    RepositoryManagerView();
    ~RepositoryManagerView();

    RepositoryManagerView(const RepositoryManagerView&) = delete;
    RepositoryManagerView& operator=(const RepositoryManagerView&) = delete;

    GtkWidget* getWidget() const;

   private:
    static void onAdd(GtkButton* button, gpointer userData);
    static void onAddCancel(GtkButton* button, gpointer userData);
    static void onAddConfirm(GtkButton* button, gpointer userData);
    static void onDelete(GtkButton* button, gpointer userData);
    static void onDisable(GtkButton* button, gpointer userData);
    static void onEnable(GtkButton* button, gpointer userData);
    static void onRefresh(GtkButton* button, gpointer userData);
    static void onSelectionChanged(GtkListBox* listBox, GtkListBoxRow* row, gpointer userData);
    void addRepository(const char* repositoryLine);
    void deleteSelected();
    void refresh();
    void setActionButtonsSensitive(GtkListBoxRow* row);
    void setSelectedEnabled(bool enabled);

    GtkWidget* widget_;
    GtkButton* deleteButton_;
    GtkButton* disableButton_;
    GtkButton* enableButton_;
    GtkListBox* repositoryList_;
    GtkLabel* statusLabel_;
    RepositoryManager repositoryManager_;
};

}  // namespace xenon
