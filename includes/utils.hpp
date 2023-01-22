#pragma once
#include "includes.hpp"

void join_executables(const char* output_file, const char** input_files, int file_count) {
    std::ofstream out(output_file, std::ios::out | std::ios::binary);
    if (!out) {
        printf("Failed to open output file %s\n", output_file);
        return;
    }

    for (int i = 0; i < file_count; i++) {
        std::ifstream in(input_files[i], std::ios::in | std::ios::binary);
        if (!in) {
            printf("Failed to open input file %s\n", input_files[i]);
            return;
        }

        //Check if the file is a valid executable file
        if(in.get() != 'M' || in.get() != 'Z')
        {
            printf("Input file %s is not a valid executable file\n", input_files[i]);
            in.close();
            continue;
        }
        in.seekg(0, std::ios::end);
        int file_size = in.tellg();
        in.seekg(0, std::ios::beg);

        char* buffer = new char[file_size];
        in.read(buffer, file_size);
        out.write(buffer, file_size);
        printf("Successfully joined %s\n", input_files[i]);
        delete[] buffer;
        in.close();
    }
    out.close();
    printf("Output file %s created successfully\n", output_file);
}
