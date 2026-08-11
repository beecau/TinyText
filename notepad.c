#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *text_view;
    GtkTextBuffer *text_buffer;
    char *current_file;
    gboolean modified;
} AppState;

AppState app_state = {
    .current_file = NULL,
    .modified = FALSE
};

// Update window title with filename and modification indicator
void update_title() {
    char title[512];
    if (app_state.current_file) {
        snprintf(title, sizeof(title), "%s%s - Notepad",
                 app_state.modified ? "*" : "",
                 g_path_get_basename(app_state.current_file));
    } else {
        snprintf(title, sizeof(title), "%sUntitled - Notepad",
                 app_state.modified ? "*" : "");
    }
    gtk_window_set_title(GTK_WINDOW(app_state.window), title);
}

// Mark document as modified
gboolean on_text_changed(GtkTextBuffer *buffer, gpointer data) {
    if (!app_state.modified) {
        app_state.modified = TRUE;
        update_title();
    }
    return FALSE;
}

// New file
void on_new(GtkWidget *widget, gpointer data) {
    if (app_state.modified) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(app_state.window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Document modified. Discard changes?"
        );
        
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response != GTK_RESPONSE_YES) return;
    }
    
    gtk_text_buffer_set_text(app_state.text_buffer, "", -1);
    
    if (app_state.current_file) {
        g_free(app_state.current_file);
        app_state.current_file = NULL;
    }
    
    app_state.modified = FALSE;
    update_title();
}

// Open file
void on_open(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Open File",
        GTK_WINDOW(app_state.window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        FILE *file = fopen(filename, "r");
        if (file) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char *content = malloc(size + 1);
            size_t read_size = fread(content, 1, size, file);
            content[read_size] = '\0';
            fclose(file);
            
            gtk_text_buffer_set_text(app_state.text_buffer, content, -1);
            free(content);
            
            if (app_state.current_file) {
                g_free(app_state.current_file);
            }
            app_state.current_file = g_strdup(filename);
            app_state.modified = FALSE;
            update_title();
        } else {
            GtkWidget *error = gtk_message_dialog_new(
                GTK_WINDOW(app_state.window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Could not open file"
            );
            gtk_dialog_run(GTK_DIALOG(error));
            gtk_widget_destroy(error);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Save file
void on_save(GtkWidget *widget, gpointer data) {
    if (!app_state.current_file) {
        on_save_as(widget, data);
        return;
    }
    
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(app_state.text_buffer, &start);
    gtk_text_buffer_get_end_iter(app_state.text_buffer, &end);
    char *content = gtk_text_buffer_get_text(app_state.text_buffer, &start, &end, FALSE);
    
    FILE *file = fopen(app_state.current_file, "w");
    if (file) {
        fputs(content, file);
        fclose(file);
        app_state.modified = FALSE;
        update_title();
    } else {
        GtkWidget *error = gtk_message_dialog_new(
            GTK_WINDOW(app_state.window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Could not save file"
        );
        gtk_dialog_run(GTK_DIALOG(error));
        gtk_widget_destroy(error);
    }
    
    g_free(content);
}

// Save as
void on_save_as(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Save File",
        GTK_WINDOW(app_state.window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        NULL
    );
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        if (app_state.current_file) {
            g_free(app_state.current_file);
        }
        app_state.current_file = g_strdup(filename);
        
        on_save(widget, data);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Exit
gboolean on_window_close(GtkWidget *widget, GdkEvent *event, gpointer data) {
    if (app_state.modified) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(app_state.window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Document modified. Save before closing?"
        );
        
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_YES) {
            on_save(widget, data);
        } else if (response != GTK_RESPONSE_NO) {
            return TRUE;
        }
    }
    
    if (app_state.current_file) {
        g_free(app_state.current_file);
    }
    
    gtk_main_quit();
    return FALSE;
}

// Edit callbacks
void on_cut(GtkWidget *widget, gpointer data) {
    gtk_text_buffer_cut_clipboard(app_state.text_buffer,
                                  gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
                                  TRUE);
}

void on_copy(GtkWidget *widget, gpointer data) {
    gtk_text_buffer_copy_clipboard(app_state.text_buffer,
                                   gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}

void on_paste(GtkWidget *widget, gpointer data) {
    gtk_text_buffer_paste_clipboard(app_state.text_buffer,
                                    gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
                                    NULL,
                                    TRUE);
}

void on_select_all(GtkWidget *widget, gpointer data) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(app_state.text_buffer, &start);
    gtk_text_buffer_get_end_iter(app_state.text_buffer, &end);
    gtk_text_buffer_select_range(app_state.text_buffer, &start, &end);
}

void on_find(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Find",
        GTK_WINDOW(app_state.window),
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Find", GTK_RESPONSE_ACCEPT,
        NULL
    );
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 5);
    gtk_widget_show(entry);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *search_text = gtk_entry_get_text(GTK_ENTRY(entry));
        
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(app_state.text_buffer, &start);
        
        if (gtk_text_iter_forward_search(&start, search_text, GTK_TEXT_SEARCH_TEXT_ONLY, &start, &end, NULL)) {
            gtk_text_buffer_select_range(app_state.text_buffer, &start, &end);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(app_state.text_view), &start, 0.0, FALSE, 0.0, 0.0);
        }
    }
    
    gtk_widget_destroy(dialog);
}

void on_about(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(app_state.window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Simple Notepad\nVersion 1.0\n\nA cross-platform text editor"
    );
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    // Main window
    app_state.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(app_state.window), 800, 600);
    gtk_window_set_type_hint(GTK_WINDOW(app_state.window), GDK_WINDOW_TYPE_HINT_NORMAL);
    
    g_signal_connect(app_state.window, "delete-event", G_CALLBACK(on_window_close), NULL);
    
    // CSS styling for black and white
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "* { background-color: white; color: black; font-family: monospace; font-size: 11pt; }\n"
        "textview { background-color: white; color: black; }\n",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // Main container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app_state.window), vbox);
    
    // Menu bar
    GtkWidget *menu_bar = gtk_menu_bar_new();
    
    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    
    GtkWidget *new_item = gtk_menu_item_new_with_label("New");
    GtkWidget *open_item = gtk_menu_item_new_with_label("Open");
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save");
    GtkWidget *save_as_item = gtk_menu_item_new_with_label("Save As");
    GtkWidget *sep1 = gtk_separator_menu_item_new();
    GtkWidget *exit_item = gtk_menu_item_new_with_label("Exit");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_as_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), sep1);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), exit_item);
    
    g_signal_connect(new_item, "activate", G_CALLBACK(on_new), NULL);
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open), NULL);
    g_signal_connect(save_item, "activate", G_CALLBACK(on_save), NULL);
    g_signal_connect(save_as_item, "activate", G_CALLBACK(on_save_as), NULL);
    g_signal_connect(exit_item, "activate", G_CALLBACK(on_window_close), NULL);
    
    // Edit menu
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *edit_item = gtk_menu_item_new_with_label("Edit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
    
    GtkWidget *cut_item = gtk_menu_item_new_with_label("Cut");
    GtkWidget *copy_item = gtk_menu_item_new_with_label("Copy");
    GtkWidget *paste_item = gtk_menu_item_new_with_label("Paste");
    GtkWidget *sep2 = gtk_separator_menu_item_new();
    GtkWidget *select_all_item = gtk_menu_item_new_with_label("Select All");
    GtkWidget *sep3 = gtk_separator_menu_item_new();
    GtkWidget *find_item = gtk_menu_item_new_with_label("Find");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), cut_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), copy_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), paste_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), sep2);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), select_all_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), sep3);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), find_item);
    
    g_signal_connect(cut_item, "activate", G_CALLBACK(on_cut), NULL);
    g_signal_connect(copy_item, "activate", G_CALLBACK(on_copy), NULL);
    g_signal_connect(paste_item, "activate", G_CALLBACK(on_paste), NULL);
    g_signal_connect(select_all_item, "activate", G_CALLBACK(on_select_all), NULL);
    g_signal_connect(find_item, "activate", G_CALLBACK(on_find), NULL);
    
    // Help menu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_about), NULL);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);
    
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    
    // Text view
    app_state.text_buffer = gtk_text_buffer_new(NULL);
    app_state.text_view = gtk_text_view_new_with_buffer(app_state.text_buffer);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state.text_view), GTK_WRAP_WORD);
    
    g_signal_connect(app_state.text_buffer, "changed", G_CALLBACK(on_text_changed), NULL);
    
    // Scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), app_state.text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
    
    update_title();
    
    gtk_widget_show_all(app_state.window);
    gtk_main();
    
    return 0;
}
