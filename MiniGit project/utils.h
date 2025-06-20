#ifndef UTILS_HPP_INCLUDED 
#define UTILS_HPP_INCLUDED 
#include <string> 

bool fileExists(const std::string& filename);
bool filesAreEqual(const std::string& file1, const std::string& file2);
void createMinigitDirectory(); 
std::string generateVersionedFilename(std::string filename, int version);
std::string computeFileHash(const std::string& filename);
void copyFile(const std::string& src, const std::string& dest);

#endif
