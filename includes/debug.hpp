#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <windows.h>

/* DEBUG FUNCTIONS */

void dump_strings(const char* output_file) {
    std::ifstream out(output_file, std::ios::in | std::ios::binary);
    if (!out) {
        printf("Failed to open output file %s\n", output_file);
        return;
    }
    out.seekg(0, std::ios::end);
    int file_size = out.tellg();
    out.seekg(0, std::ios::beg);

    char* buffer = new char[file_size];
    out.read(buffer, file_size);
    out.close();

    for (int i = 0; i < file_size; i++) {
        if (buffer[i] < ' ' || buffer[i] > '~') {
            buffer[i] = '\0';
        }
    }

    char* current_string = buffer;
    while (current_string < buffer + file_size) {
        if (*current_string != '\0') {
            printf("%s\n", current_string);
        }
        current_string += strlen(current_string) + 1;
    }
    delete[] buffer;
}
