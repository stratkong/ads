#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

// Structure to represent Huffman tree nodes
typedef struct Node {
    unsigned char data;
    unsigned freq;
    struct Node *left, *right;
} Node;

// Function to create a new Huffman tree node
Node* newNode(unsigned char data, unsigned freq) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->left = node->right = NULL;
    node->data = data;
    node->freq = freq;
    return node;
}

// Priority queue node structure
typedef struct pqNode {
    Node* data;
    struct pqNode* next;
} pqNode;

// Function to create a new priority queue node
pqNode* newPqNode(Node* data) {
    pqNode* node = (pqNode*)malloc(sizeof(pqNode));
    node->data = data;
    node->next = NULL;
    return node;
}

// Function to create a min heap priority queue
typedef struct PriorityQueue {
    pqNode *front;
} PriorityQueue;

// Function to create an empty priority queue
PriorityQueue* createPriorityQueue() {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    pq->front = NULL;
    return pq;
}

// Function to insert a node into the priority queue
void enqueue(PriorityQueue* pq, Node* node) {
    pqNode* newNode = newPqNode(node);
    if (pq->front == NULL || pq->front->data->freq >= node->freq) {
        newNode->next = pq->front;
        pq->front = newNode;
    } else {
        pqNode* current = pq->front;
        while (current->next != NULL && current->next->data->freq < node->freq) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

// Function to remove the node with the lowest frequency from the priority queue
Node* dequeue(PriorityQueue* pq) {
    if (pq->front == NULL) return NULL;
    pqNode* temp = pq->front;
    Node* data = temp->data;
    pq->front = pq->front->next;
    free(temp);
    return data;
}

// Function to build Huffman tree from character frequencies
Node* buildHuffmanTree(unsigned freq[]) {
    PriorityQueue* pq = createPriorityQueue();
    for (int i = 0; i < 256; ++i) {
        if (freq[i] != 0) {
            enqueue(pq, newNode((unsigned char)i, freq[i]));
        }
    }
    while (pq->front->next != NULL) {
        Node* left = dequeue(pq);
        Node* right = dequeue(pq);
        Node* top = newNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        enqueue(pq, top);
    }
    return dequeue(pq);
}

// Function to decode the encoded file
void decode_file(const char* file_name) {
    FILE* fp_in, * fp_out;
    unsigned int freq[256] = { 0 };
    int input_data = 0, output_data = 0;
    char buf[1024], * code[256] = { 0 };

    // Read the frequency table from the encoded file
    if ((fp_in = fopen(file_name, "r")) == NULL) {
        printf("\nERROR: No such file\n");
        return;
    }

    int num_chars;
    fscanf(fp_in, "%d\n", &num_chars);
    unsigned char c;
    int f;
    for (int i = 0; i < num_chars; i++) {
        fscanf(fp_in, "%c %d\n", &c, &f);
        freq[c] = f;
    }

    // Rebuild Huffman tree from frequency table
    Node* root = buildHuffmanTree(freq);

    // Decode and save the decoded file
    char out_filename[strlen(file_name) - strlen(".huffman") + 1];
    strncpy(out_filename, file_name, strlen(file_name) - strlen(".huffman"));
    out_filename[strlen(file_name) - strlen(".huffman")] = '\0';

    if ((fp_out = fopen(out_filename, "w")) == NULL) {
        printf("\nERROR: Unable to create output file\n");
        fclose(fp_in);
        return;
    }

    Node* current = root;
    int bit_count = 0;
    unsigned char byte;
    while ((byte = fgetc(fp_in)) != EOF) {
        for (int i = 0; i < 8; i++) {
            if (byte & (1 << (7 - i))) {
                current = current->right;
            } else {
                current = current->left;
            }
            if (current->left == NULL && current->right == NULL) {
                fputc(current->data, fp_out);
                current = root;
            }
            bit_count++;
            if (bit_count == num_chars) {
                break;
            }
        }
        if (bit_count == num_chars) {
            break;
        }
    }

    fclose(fp_in);
    fclose(fp_out);
    printf("\nDecoding completed successfully.\n");
}

// Callback function for file chooser dialog response
void file_chosen(GtkFileChooserButton* chooser_button, GtkLabel* label) {
    const gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser_button));
    g_return_if_fail(filename != NULL);
    decode_file(filename);
    g_free((gpointer)filename);
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget* window;
    GtkWidget* chooser_button;
    GtkWidget* label;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Huffman Decoder");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    chooser_button = gtk_file_chooser_button_new("Choose a file", GTK_FILE_CHOOSER_ACTION_OPEN);
    label = gtk_label_new("Select a file to decode.");

    g_signal_connect(chooser_button, "file-set", G_CALLBACK(file_chosen), label);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), chooser_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
