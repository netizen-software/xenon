#include "ui/system-cleaner-view.hpp"

#include <exception>

namespace {

constexpr char RESOURCE_PATH[] = "/io/github/netizen_software/Xenon/ui/system-cleaner.ui";
constexpr char VIEW_ID[] = "system_cleaner_view";
constexpr char CLEANER_LIST_ID[] = "cleaner_list";
constexpr char STATUS_LABEL_ID[] = "status_label";
constexpr char CLEAN_BUTTON_ID[] = "clean_button";
constexpr char REFRESH_BUTTON_ID[] = "refresh_button";
constexpr char CATEGORY_KEY[] = "xenon-cleaner-category";
constexpr char CHECK_BUTTON_KEY[] = "xenon-cleaner-check-button";

}  // namespace

namespace xenon {

SystemCleanerView::SystemCleanerView()
    : widget_(nullptr), cleanButton_(nullptr), cleanerList_(nullptr), statusLabel_(nullptr) {
    auto* builder = gtk_builder_new_from_resource(RESOURCE_PATH);
    widget_ = GTK_WIDGET(gtk_builder_get_object(builder, VIEW_ID));
    cleanButton_ = GTK_BUTTON(gtk_builder_get_object(builder, CLEAN_BUTTON_ID));
    cleanerList_ = GTK_LIST_BOX(gtk_builder_get_object(builder, CLEANER_LIST_ID));
    statusLabel_ = GTK_LABEL(gtk_builder_get_object(builder, STATUS_LABEL_ID));
    auto* refreshButton = GTK_BUTTON(gtk_builder_get_object(builder, REFRESH_BUTTON_ID));

    g_object_ref(widget_);
    g_object_unref(builder);

    g_signal_connect(cleanButton_, "clicked", G_CALLBACK(SystemCleanerView::onClean), this);
    g_signal_connect(refreshButton, "clicked", G_CALLBACK(SystemCleanerView::onRefresh), this);
    refresh();
}

SystemCleanerView::~SystemCleanerView() { g_object_unref(widget_); }

GtkWidget* SystemCleanerView::getWidget() const { return widget_; }

void SystemCleanerView::onClean(GtkButton*, gpointer userData) {
    static_cast<SystemCleanerView*>(userData)->cleanSelected();
}

void SystemCleanerView::onRefresh(GtkButton*, gpointer userData) {
    static_cast<SystemCleanerView*>(userData)->refresh();
}

void SystemCleanerView::cleanSelected() {
    std::size_t cleanedCategories = 0;

    try {
        for (auto* row = gtk_widget_get_first_child(GTK_WIDGET(cleanerList_)); row != nullptr;
             row = gtk_widget_get_next_sibling(row)) {
            auto* checkButton =
                GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(row), CHECK_BUTTON_KEY));
            if (!gtk_check_button_get_active(checkButton)) {
                continue;
            }

            const auto category = static_cast<CleanerCategory>(
                GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), CATEGORY_KEY)) - 1);
            systemCleaner_.clean(category);
            ++cleanedCategories;
        }

        if (cleanedCategories == 0) {
            gtk_label_set_text(statusLabel_, "Select one or more cleanup categories");
            return;
        }

        refresh();
        auto* statusText = g_strdup_printf("Cleaned %zu categories", cleanedCategories);
        gtk_label_set_text(statusLabel_, statusText);
        g_free(statusText);
    } catch (const std::exception& error) {
        gtk_label_set_text(statusLabel_, error.what());
    }
}

void SystemCleanerView::refresh() {
    while (auto* row = gtk_widget_get_first_child(GTK_WIDGET(cleanerList_))) {
        gtk_list_box_remove(cleanerList_, row);
    }

    try {
        std::uintmax_t totalSize = 0;
        for (const CleanerItem& item : systemCleaner_.listItems()) {
            auto* row = gtk_list_box_row_new();
            auto* content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
            auto* checkButton = gtk_check_button_new();
            auto* labels = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
            auto* title = gtk_label_new(item.title.c_str());
            auto* description = gtk_label_new(item.description.c_str());
            auto* sizeText = g_format_size(item.sizeBytes);
            auto* size = gtk_label_new(sizeText);

            gtk_widget_set_hexpand(labels, TRUE);
            gtk_label_set_xalign(GTK_LABEL(title), 0);
            gtk_label_set_xalign(GTK_LABEL(description), 0);
            gtk_label_set_xalign(GTK_LABEL(size), 1);
            gtk_widget_add_css_class(description, "dim-label");
            gtk_widget_add_css_class(size, "heading");
            gtk_box_append(GTK_BOX(labels), title);
            gtk_box_append(GTK_BOX(labels), description);
            gtk_box_append(GTK_BOX(content), checkButton);
            gtk_box_append(GTK_BOX(content), labels);
            gtk_box_append(GTK_BOX(content), size);
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), content);
            g_object_set_data(G_OBJECT(row), CHECK_BUTTON_KEY, checkButton);
            g_object_set_data(G_OBJECT(row), CATEGORY_KEY,
                              GINT_TO_POINTER(static_cast<int>(item.category) + 1));
            gtk_list_box_append(cleanerList_, row);
            totalSize += item.sizeBytes;
            g_free(sizeText);
        }

        auto* totalText = g_format_size(totalSize);
        auto* statusText = g_strdup_printf("%s reclaimable", totalText);
        gtk_label_set_text(statusLabel_, statusText);
        g_free(totalText);
        g_free(statusText);
    } catch (const std::exception&) {
        gtk_label_set_text(statusLabel_, "Unable to inspect cleanup data");
    }
}

}  // namespace xenon
