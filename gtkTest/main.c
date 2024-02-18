#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

gchar* open_file_dialog(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(window),
                                         action,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        gtk_widget_destroy(dialog);
        return filename;
    }

    gtk_widget_destroy(dialog);
    return NULL;
}

void encode_and_print_info(const char *file_name) {
    FILE *fp_in, *fp_out;
    unsigned int freq[128] = {0};
    int input_data = 0, output_data = 0;
    char buf[1024], *code[128] = {0};

    // Import the file into the program and update the Huffman tree
    if ((fp_in = fopen(file_name, "r")) == NULL) {
        printf("\nERROR: No such file\n");
        return;
    }
    char c;
    while ((c = fgetc(fp_in)) != EOF) {
        freq[(int) c]++;
        input_data++;
    }
    rewind(fp_in);

    // Print the code table
    printf("\n---------CODE TABLE---------\n----------------------------\nCHAR  FREQ  CODE\n----------------------------\n");
    for (int i = 0; i < 128; i++) {
        if (isprint((char) i) && freq[i] != 0 && i != ' ') {
            printf("%-4c  %-4d  %16s\n", i, freq[i], code[i]);
        }
    }
    printf("----------------------------\n");

    // Encode and save the encoded file
    strcat(file_name, ".huffman");
    if ((fp_out = fopen(file_name, "w")) == NULL) {
        printf("\nERROR: Unable to create output file\n");
        fclose(fp_in);
        return;
    }
    int lim = 0;
    for (int i = 0; i < 128; i++) {
        if (freq[i]) lim += (freq[i] * strlen(code[i]));
    }
    output_data = lim;
    fprintf(fp_out, "%04d\n", lim);
    printf("\nEncoded:\n");
    char temp[20] = {0}, in, output_byte = 0, bits_written = 0;
    for (int i = 0; i < lim; i++) {
        if (temp[0] == '\0') {
            in = fgetc(fp_in);
            strcpy(temp, code[in]);
        }
        char bit = temp[0];
        memmove(temp, temp + 1, strlen(temp)); // Shift the string left by one
        if (bit == '1') {
            output_byte |= (1 << (7 - bits_written));
        }
        bits_written++;
        if (bits_written == 8 || i == lim - 1) {
            bits_written = 0;
            fputc(output_byte, fp_out);
            output_byte = 0;
        }
    }
    putchar('\n');

    fclose(fp_in);
    fclose(fp_out);

    output_data = (output_data % 8) ? (output_data / 8) + 1 : (output_data / 8);
    printf("\nInput bytes:\t\t%d\n", input_data);
    printf("Output bytes:\t\t%d\n", output_data);
    printf("\nCompression ratio:\t%.2f%%\n\n\n", ((double) (input_data - output_data) / input_data) * 100);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window;
    GtkWidget *button;
    gchar *selectedFile;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "File Explorer");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    button = gtk_button_new_with_label("Open File Explorer");
    g_signal_connect(button, "clicked", G_CALLBACK(open_file_dialog), (gpointer) window);

    gtk_container_add(GTK_CONTAINER(window), button);
    gtk_widget_show_all(window);

    gtk_main();

    selectedFile = open_file_dialog(NULL, (gpointer) window);

    if (selectedFile != NULL) {
        g_print("Selected file from main: %s\n", selectedFile);
        encode_and_print_info(selectedFile);
        g_free(selectedFile);
    }

    return 0;
}
