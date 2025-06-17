#include <fstream>
#include <filesystem>
#include "minigit.hpp"

bool fileExists(const string& filename) {
ifstream file(filename);
return file.good();
} //function that takes filename as input and returns true if it can be opened, false otherwise.
bool filesAreEqual(const string& file1, const string& file2) {
ifstream f1(file1), f2(file2);
if (!f1 || !f2) return false;

string line1, line2;  
while (getline(f1, line1) && getline(f2, line2)) {  
    if (line1 != line2) return false;  
}  
return f1.eof() && f2.eof();

}
//compares contents of two files line by line, and returns true if they exist and are the same and false otherwise.





