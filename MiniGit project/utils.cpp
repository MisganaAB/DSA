#include <fstream>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <sstream>
#include "utils.hpp"
#include "sha1.h"
using namespace std;


bool fileExists(const string& filename) {
ifstream file(filename);
    return file.good();
} 
bool filesAreEqual(const string& file1, const string& file2) {
    ifstream f1(file1, ios::binary), f2(file2, ios::binary);
    if (!f1.is_open() || !f2.is_open()) {
        cerr << "Error: Could not open files for comparison." << endl;
        return false;
    }
    return equal(
        istreambuf_iterator<char>(f1),
        istreambuf_iterator<char>(),
        istreambuf_iterator<char>(f2)
    );
}

void createMinigitDirectory() {
    if (!filesystem::exists(".minigit")) {
        filesystem::create_directory(".minigit");
    }
    if (!filesystem::exists(".minigit/objects")) {
        filesystem::create_directory(".minigit/objects");
    }
}

string generateVersionedFilename(string filename, int version) {
    int dot = filename.find('.');
    string base = filename.substr(0, dot);
    string ext = filename.substr(dot);
    return base + "_" + to_string(version) + ext;
}
string computeFileHash(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) return "";
    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();
    return SHA1::from_string(content);
}

void copyFile(const std::string& src, const std::string& dest) {
    try {
        std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error copying file from '" << src << "' to '" << dest << "': " << e.what() << std::endl;
    }
}










