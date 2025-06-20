#ifndef MINIGIT_HPP_INCLUDED
#define MINIGIT_HPP_INCLUDED

#include <unordered_map>
#include <string>
#include "utils.hpp"

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

    void save();
    void load();

public:
    MiniGit();

    void mergeBranch(const string& branchName);
    void diffCommits(int commit1, int commit2);
    void init();

    ~MiniGit();

    void addFile(const string& filename);
    void removeFile(const string& filename);
    void commit(const string& message);
    void checkout(const string& branchName);
    
    void printHistory();
    void printBranches();
    
    void createBranch(const string& name);
    void checkoutBranch(const string& name);
};


#endif // MINIGIT_HPP_INCLUDED
