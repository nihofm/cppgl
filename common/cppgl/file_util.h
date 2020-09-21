#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

inline std::string get_basepath(const std::string& filepath) {
    if (filepath.find_last_of("/\\") != std::string::npos)
        return filepath.substr(0, filepath.find_last_of("/\\")) + "/";
    return "./";
}

inline std::string get_filename(const std::string& filepath) {
    const size_t index = filepath.find_last_of("/\\");
    if (index != std::string::npos && index != filepath.size() - 1)
        return filepath.substr(filepath.find_last_of("/\\") + 1);
    return filepath;
}

inline std::string get_extension(const std::string& filename) {
    const size_t index = filename.find_last_of(".");
    return index != std::string::npos ? filename.substr(index + 1) : std::string();
}

inline std::string remove_extension(const std::string& filename) {
    const size_t index = filename.find_last_of(".");
    if (index != std::string::npos)
        return filename.substr(0, index);
    return filename;
}

inline std::string change_extension(const std::string& filename, const std::string& extension) {
    const size_t index = filename.find_last_of(".");
    if (index != std::string::npos)
        return filename.substr(0, index + 1) + extension;
    else
        return filename + extension;
}

inline std::string concat(const std::string& dir, const std::string& file) {
    if (dir.empty()) return file;
    if (file.empty()) return dir;
    const std::string a = dir.back() == '/' || dir.back() == '\\' ?  dir.substr(0, dir.size() - 1) : dir;
    const std::string b = file.front() == '/' || file.front() == '\\' ?  file.substr(1) : file;
    return a + '/' + b;
}

inline void list_dir(const std::string& base_dir, const std::string& rel_dir, std::vector<std::string>& entries) {
    DIR* dir;
    if ((dir = opendir(concat(base_dir, rel_dir).c_str())) != 0) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != 0) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
            if (ent->d_type == DT_DIR)
                list_dir(base_dir, concat(rel_dir, ent->d_name), entries);
            else
                entries.emplace_back(concat(rel_dir, ent->d_name));
        }
        closedir(dir);
    }
}

inline std::vector<std::string> list_dir(const std::string& dirname) {
    std::vector<std::string> entries;
    list_dir(dirname, "", entries);
    return entries;
}

inline std::string read_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
    return std::string();
}

inline size_t get_timestamp(const std::string& filepath) {
    std::ifstream in_file(filepath, std::ios::in);
    if (in_file.is_open()) {
        struct stat sb;
        if (stat(filepath.c_str(), &sb) == 0)
            return sb.st_mtime;
    }
    return 0;
}
