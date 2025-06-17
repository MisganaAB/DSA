#ifndef MINIGIT_HPP_INCLUDED
#define MINIGIT_HPP_INCLUDED

#include <string>
using namespace std;

struct FileNode {
    string fileName;
    string versionedFileName;
    FileNode* next;
};

struct CommitNode {
    int commitNumber;
    FileNode* fileHead;
    CommitNode* next;
};

class MiniGit {
private:
    CommitNode* commitHead;
    int currentCommitNumber;
    unordered_map<string, CommitNode*> branches;
    
    string currentBranch
    int currentCommitNumber;

public:
    MiniGit();
    ~MiniGit();

    void addFile(const string& filename);
    void removeFile(const string& filename);
    void commit();
    void checkout(int commitNumber);
    
    void printHistory();
    void pringBranches();
    
    void createBranch(const string& name);
    void checkoutBranch(const string& name);
};


#endif // MINIGIT_HPP_INCLUDED
