#include <fstream>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "minigit.hpp"

bool fileExists(const string& filename) {
return filesystem::exists(filename);
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






