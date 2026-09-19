#include "ui/repository-manager-view.hpp"

#include <adwaita.h>

#include <exception>

namespace {

constexpr char RESOURCE_PATH[] = "/io/github/netizen_software/Xenon/ui/repository-manager.ui";
constexpr char VIEW_ID[] = "repository_manager_view";
constexpr char REPOSITORY_LIST_ID[] = "repository_list";
constexpr char STATUS_LABEL_ID[] = "status_label";
constexpr char ADD_BUTTON_ID[] = "add_button";
constexpr char REFRESH_BUTTON_ID[] = "refresh_button";
constexpr char ENABLE_BUTTON_ID[] = "enable_button";
constexpr char DISABLE_BUTTON_ID[] = "disable_button";
constexpr char DELETE_BUTTON_ID[] = "delete_button";
constexpr char REPOSITORY_KEY[] = "xenon-repository";
constexpr char REPOSITORY_ENTRY_KEY[] = "xenon-repository-entry";
constexpr char REPOSITORY_VIEW_KEY[] = "xenon-repository-view";
constexpr int STATUS_COLUMN_WIDTH = 120;

}  // namespace

namespace xenon {

RepositoryManagerView::RepositoryManagerView()
    : widget_(nullptr),
      deleteButton_(nullptr),
      disableButton_(nullptr),
      enableButton_(nullptr),
      repositoryList_(nullptr),
      statusLabel_(nullptr) {
    auto* builder = gtk_builder_new_from_resource(RESOURCE_PATH);
    widget_ = GTK_WIDGET(gtk_builder_get_object(builder, VIEW_ID));
    repositoryList_ = GTK_LIST_BOX(gtk_builder_get_object(builder, REPOSITORY_LIST_ID));
    statusLabel_ = GTK_LABEL(gtk_builder_get_object(builder, STATUS_LABEL_ID));
    auto* addButton = GTK_BUTTON(gtk_builder_get_object(builder, ADD_BUTTON_ID));
    auto* refreshButton = GTK_BUTTON(gtk_builder_get_object(builder, REFRESH_BUTTON_ID));
    enableButton_ = GTK_BUTTON(gtk_builder_get_object(builder, ENABLE_BUTTON_ID));
    disableButton_ = GTK_BUTTON(gtk_builder_get_object(builder, DISABLE_BUTTON_ID));
    deleteButton_ = GTK_BUTTON(gtk_builder_get_object(builder, DELETE_BUTTON_ID));

    g_object_ref(widget_);
    g_object_unref(builder);

    g_signal_connect(addButton, "clicked", G_CALLBACK(RepositoryManagerView::onAdd), this);
    g_signal_connect(refreshButton, "clicked", G_CALLBACK(RepositoryManagerView::onRefresh), this);
    g_signal_connect(repositoryList_, "row-selected",
                     G_CALLBACK(RepositoryManagerView::onSelectionChanged), this);
    g_signal_connect(enableButton_, "clicked", G_CALLBACK(RepositoryManagerView::onEnable), this);
    g_signal_connect(disableButton_, "clicked", G_CALLBACK(RepositoryManagerView::onDisable), this);
    g_signal_connect(deleteButton_, "clicked", G_CALLBACK(RepositoryManagerView::onDelete), this);
    refresh();
}

RepositoryManagerView::~RepositoryManagerView() { g_object_unref(widget_); }

GtkWidget* RepositoryManagerView::getWidget() const { return widget_; }

void RepositoryManagerView::onAdd(GtkButton*, gpointer userData) {
    auto* view = static_cast<RepositoryManagerView*>(userData);
    auto* dialog = adw_dialog_new();
    auto* entry = GTK_ENTRY(gtk_entry_new());
    auto* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    auto* actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    auto* cancelButton = gtk_button_new_with_label("Cancel");
    auto* addButton = gtk_button_new_with_label("Add Repository");

    adw_dialog_set_title(dialog, "Add Repository");
    adw_dialog_set_content_width(dialog, 520);
    gtk_entry_set_placeholder_text(entry, "deb https://example.org/ubuntu noble main");
    gtk_widget_set_margin_top(content, 18);
    gtk_widget_set_margin_bottom(content, 18);
    gtk_widget_set_margin_start(content, 18);
    gtk_widget_set_margin_end(content, 18);
    gtk_widget_set_hexpand(GTK_WIDGET(entry), TRUE);
    gtk_widget_set_hexpand(actions, TRUE);
    gtk_widget_set_halign(actions, GTK_ALIGN_END);
    gtk_widget_add_css_class(addButton, "suggested-action");
    gtk_box_append(GTK_BOX(content), GTK_WIDGET(entry));
    gtk_box_append(GTK_BOX(actions), cancelButton);
    gtk_box_append(GTK_BOX(actions), addButton);
    gtk_box_append(GTK_BOX(content), actions);
    g_object_set_data(G_OBJECT(dialog), REPOSITORY_ENTRY_KEY, entry);
    g_object_set_data(G_OBJECT(dialog), REPOSITORY_VIEW_KEY, view);
    g_signal_connect(cancelButton, "clicked", G_CALLBACK(RepositoryManagerView::onAddCancel),
                     dialog);
    g_signal_connect(addButton, "clicked", G_CALLBACK(RepositoryManagerView::onAddConfirm), dialog);
    adw_dialog_set_default_widget(dialog, addButton);
    adw_dialog_set_child(dialog, content);
    adw_dialog_present(dialog, view->widget_);
}

void RepositoryManagerView::onAddCancel(GtkButton*, gpointer userData) {
    adw_dialog_close(ADW_DIALOG(userData));
}

void RepositoryManagerView::onAddConfirm(GtkButton*, gpointer userData) {
    auto* dialog = ADW_DIALOG(userData);
    auto* view = static_cast<RepositoryManagerView*>(
        g_object_get_data(G_OBJECT(dialog), REPOSITORY_VIEW_KEY));
    auto* entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), REPOSITORY_ENTRY_KEY));
    view->addRepository(gtk_editable_get_text(GTK_EDITABLE(entry)));
    adw_dialog_close(dialog);
}

void RepositoryManagerView::onDelete(GtkButton*, gpointer userData) {
    static_cast<RepositoryManagerView*>(userData)->deleteSelected();
}

void RepositoryManagerView::onDisable(GtkButton*, gpointer userData) {
    static_cast<RepositoryManagerView*>(userData)->setSelectedEnabled(false);
}

void RepositoryManagerView::onEnable(GtkButton*, gpointer userData) {
    static_cast<RepositoryManagerView*>(userData)->setSelectedEnabled(true);
}

void RepositoryManagerView::onRefresh(GtkButton*, gpointer userData) {
    static_cast<RepositoryManagerView*>(userData)->refresh();
}

void RepositoryManagerView::onSelectionChanged(GtkListBox*, GtkListBoxRow* row, gpointer userData) {
    static_cast<RepositoryManagerView*>(userData)->setActionButtonsSensitive(row);
}

void RepositoryManagerView::addRepository(const char* repositoryLine) {
    try {
        repositoryManager_.addRepository(repositoryLine);
        refresh();
        gtk_label_set_text(statusLabel_, "Repository added");
    } catch (const std::exception& error) {
        gtk_label_set_text(statusLabel_, error.what());
    }
}

void RepositoryManagerView::deleteSelected() {
    auto* selectedRow = gtk_list_box_get_selected_row(repositoryList_);
    if (selectedRow == nullptr) {
        return;
    }

    const auto* repository = static_cast<const RepositoryInfo*>(
        g_object_get_data(G_OBJECT(selectedRow), REPOSITORY_KEY));
    if (repository == nullptr) {
        return;
    }

    try {
        repositoryManager_.deleteRepository(*repository);
        refresh();
        gtk_label_set_text(statusLabel_, "Repository removed");
    } catch (const std::exception& error) {
        gtk_label_set_text(statusLabel_, error.what());
    }
}

void RepositoryManagerView::refresh() {
    while (auto* row = gtk_widget_get_first_child(GTK_WIDGET(repositoryList_))) {
        gtk_list_box_remove(repositoryList_, row);
    }

    try {
        const std::vector<RepositoryInfo> repositories = repositoryManager_.listRepositories();
        for (const RepositoryInfo& repository : repositories) {
            auto* row = gtk_list_box_row_new();
            auto* content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
            auto* labels = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
            auto* sourceFile = gtk_label_new(repository.sourceFile.filename().c_str());
            auto* summary = gtk_label_new(repository.summary.c_str());
            auto* status = gtk_label_new(repository.enabled ? "Enabled" : "Disabled");

            gtk_widget_set_hexpand(labels, TRUE);
            gtk_label_set_xalign(GTK_LABEL(sourceFile), 0);
            gtk_label_set_xalign(GTK_LABEL(summary), 0);
            gtk_label_set_ellipsize(GTK_LABEL(summary), PANGO_ELLIPSIZE_END);
            gtk_widget_set_size_request(status, STATUS_COLUMN_WIDTH, -1);
            gtk_label_set_xalign(GTK_LABEL(status), 1);
            gtk_widget_add_css_class(summary, "dim-label");
            gtk_widget_add_css_class(status, "dim-label");
            gtk_box_append(GTK_BOX(labels), sourceFile);
            gtk_box_append(GTK_BOX(labels), summary);
            gtk_box_append(GTK_BOX(content), labels);
            gtk_box_append(GTK_BOX(content), status);
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), content);
            g_object_set_data_full(
                G_OBJECT(row), REPOSITORY_KEY, new RepositoryInfo(repository),
                [](gpointer data) { delete static_cast<RepositoryInfo*>(data); });
            gtk_list_box_append(repositoryList_, row);
        }

        auto* statusText = g_strdup_printf("%zu APT repository sources", repositories.size());
        gtk_label_set_text(statusLabel_, statusText);
        g_free(statusText);
    } catch (const std::exception&) {
        gtk_label_set_text(statusLabel_, "Unable to read APT repositories");
    }
}

void RepositoryManagerView::setActionButtonsSensitive(GtkListBoxRow* row) {
    const auto* repository =
        row == nullptr
            ? nullptr
            : static_cast<const RepositoryInfo*>(g_object_get_data(G_OBJECT(row), REPOSITORY_KEY));
    gtk_widget_set_sensitive(GTK_WIDGET(enableButton_),
                             repository != nullptr && !repository->enabled);
    gtk_widget_set_sensitive(GTK_WIDGET(disableButton_),
                             repository != nullptr && repository->enabled);
    gtk_widget_set_sensitive(GTK_WIDGET(deleteButton_),
                             repository != nullptr && repository->removable);
}

void RepositoryManagerView::setSelectedEnabled(bool enabled) {
    auto* selectedRow = gtk_list_box_get_selected_row(repositoryList_);
    if (selectedRow == nullptr) {
        return;
    }

    const auto* repository = static_cast<const RepositoryInfo*>(
        g_object_get_data(G_OBJECT(selectedRow), REPOSITORY_KEY));
    if (repository == nullptr) {
        return;
    }

    try {
        repositoryManager_.setRepositoryEnabled(*repository, enabled);
        refresh();
        gtk_label_set_text(statusLabel_, enabled ? "Repository enabled" : "Repository disabled");
    } catch (const std::exception& error) {
        gtk_label_set_text(statusLabel_, error.what());
    }
}

}  // namespace xenon
