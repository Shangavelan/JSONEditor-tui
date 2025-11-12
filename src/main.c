#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "cJSON.h"
#include "tui.h"

char* read_file_to_string(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = malloc(length + 1);
    if (!data) {
        perror("Memory allocation failed");
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(data, 1, length, f);
    fclose(f);

    data[bytes_read] = '\0'; // Null-terminate
    return data;
}

int write_json_to_file(const char *filename, cJSON *root) {
    if (!filename || !root) {
        return -1; // invalid parameters
    }

    // Convert cJSON object to string
    char *json_str = cJSON_Print(root);
    if (!json_str) {
        return -2; // failed to print JSON
    }

    // Open file for writing
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        free(json_str);
        return -3; // failed to open file
    }

    // Write JSON string to file
    fprintf(fp, "%s\n", json_str);

    // Clean up
    fclose(fp);
    free(json_str);

    return 0; // success
}


int main(int argc, char *argv[]) {
//    printf("JSON Editor TUI - argc=%d\n", argc);

    if (argc > 3 || (argc == 3 && strcmp(argv[1], "-j") != 0)) {
        printf("Usage: jsontui -j [json filename] (or) jsontui\n");
        return 1;
    }

    if (argc == 1) {
        // From scratch
        printf("Starting from scratch (empty JSON)...\n");
        cJSON *root = cJSON_CreateObject(); // or CreateArray() based on user choice later
        run_ui(root);
        if (write_json_to_file("output.json", root) != 0) {
                printf("Failed to write JSON to file\n");
            } 
            else {
                printf("JSON written successfully\n");
            }

        cJSON_Delete(root);
    }
    else if (argc == 3) {
        if (strcmp(argv[1], "-j") == 0) {
            const char* filename = argv[2];
            printf("JSON file to load: %s\n", filename);

            // Read file contents
            char* json_data = read_file_to_string(filename);
            if (!json_data) {
                fprintf(stderr, "Failed to read JSON file.\n");
                return 1;
            }

            // Parse JSON
            cJSON* root = cJSON_Parse(json_data);
            if (!root) {
                const char* error_ptr = cJSON_GetErrorPtr();
                if (error_ptr)
                    fprintf(stderr, "Error before: %s\n", error_ptr);
                free(json_data);
                return 1;
            }

            printf("Loaded JSON successfully!\n");

            // Run TUI
            run_ui(root);

             // Write it to file
            if (write_json_to_file(filename, root) != 0) {
                printf("Failed to write JSON to file\n");
            } 
            else {
                printf("JSON written successfully\n");
            }


            // Cleanup
            cJSON_Delete(root);
            free(json_data);
        }
        else {
            printf("Invalid option %s\n", argv[1]);
            printf("Usage: jsontui -j [json filename] (or) jsontui\n");
            return 1;
        }
    }

    return 0;
}
