#ifndef MINIGIT_HPP_INCLUDED
#define MINIGIT_HPP_INCLUDED

#include <unordered_map>
#include <string>
using namespace std;

struct FileNode {
    string fileName;
    string versionedFileName;
    string contentHash;
    FileNode* next;
};

struct CommitNode {
    string message;
    int commitNumber;
    FileNode* fileHead;
    CommitNode* next;
};

class MiniGit {
private:
    CommitNode* commitHead;
    int currentCommitNumber;
    unordered_map<string, CommitNode*> branches;
    
    string currentBranch;

public:
    MiniGit();

    void mergeBranch(const string& branchName);
    void diffCommits(int commit1, int commit2);

    ~MiniGit();

    void addFile(const string& filename);
    void removeFile(const string& filename);
    void commit();
    void checkout(int commitNumber);
    
    void printHistory();
    void printBranches();
    
    void createBranch(const string& name);
    void checkoutBranch(const string& name);
};


#endif // MINIGIT_HPP_INCLUDED
