#include "minigit.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <unordered_set>
using namespace std;

MiniGit::MiniGit() {
    createMinigitDirectory();
    load();
}

MiniGit::~MiniGit() {
    unordered_map<CommitNode*, bool> freed;
    for (auto& [b, head] : branches) {
        CommitNode* c = head;
        while (c && !freed[c]) {
            freed[c] = true;
            FileNode* f = c->fileHead;
            while (f) {
                FileNode* tmpF = f;
                f = f->next;
                delete tmpF;
            }
            CommitNode* tmpC = c;
            c = c->next;
            delete tmpC;
        }
    }
}

void MiniGit::addFile(const string& filename) {
    if (!fileExists(filename)) {
        cout << "File does not exist.";
        return;
    }
    for (FileNode* t = commitHead->fileHead; t; t = t->next) {
        if (t->fileName == filename) {
            cout << "File already added.";
            return;
        }
    }
    string hashs = computeFileHash(filename);
    FileNode* newNode = new FileNode{filename, "", hashs, commitHead->fileHead};
    commitHead->fileHead = newNode;
    cout << "File added and hashed (" << hashs << ").";
    save();
}

void MiniGit::removeFile(const string& filename) {
    FileNode *prev = nullptr, *cur = commitHead->fileHead;
    while (cur) {
        if (cur->fileName == filename) {
            if (prev) prev->next = cur->next;
            else commitHead->fileHead = cur->next;
            delete cur;
            cout << "File removed.";
            save();
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    cout << "File not tracked.";
}

void MiniGit::commit(const string& message) {
    FileNode *oldFiles = commitHead->fileHead;
    FileNode *newHead = nullptr, *newTail = nullptr;
    bool anyChange = false;

    // Build a set of previous commit's file names and hashes
    unordered_map<string, string> prevFiles;
    if (commitHead && commitHead->next) {
        for (FileNode* f = commitHead->next->fileHead; f; f = f->next) {
            prevFiles[f->fileName] = f->contentHash;
        }
    }

    while (oldFiles) {
        string newHash = computeFileHash(oldFiles->fileName);
        string objectPath = ".minigit/objects/" + newHash;

        // Always copy the file to the object path, overwriting if necessary
        copyFile(oldFiles->fileName, objectPath);

        // If file is new or hash changed, mark as change
        if (!prevFiles.count(oldFiles->fileName) || prevFiles[oldFiles->fileName] != newHash) {
            anyChange = true;
        }

        FileNode* copied = new FileNode{
            oldFiles->fileName,
            objectPath, // Always set versionedFileName to objectPath
            newHash,
            nullptr
        };

        if (!newHead) newHead = copied;
        else newTail->next = copied;
        newTail = copied;

        oldFiles = oldFiles->next;
    }

    if (!anyChange) {
        cout << "No changes to commit." << endl;
        // Clean up allocated newHead list
        while (newHead) {
            FileNode* tmp = newHead;
            newHead = newHead->next;
            delete tmp;
        }
        return;
    }

    CommitNode* newCommit = new CommitNode{message, ++currentCommitNumber, newHead, commitHead};
    commitHead = newCommit;
    branches[currentBranch] = commitHead;

    cout << "[" << currentBranch << "] Commit #" << currentCommitNumber << ": " << message << "";
    save();
}


void MiniGit::checkout(const string& branchName) {
    if (!branches.count(branchName)) {
        cout << "Branch not found." << endl;
        return;
    }
    
    if (commitHead) {
        for (FileNode* f = commitHead->fileHead; f; f = f->next) {
            if (fileExists(f->fileName)) {
                std::filesystem::remove(f->fileName);
            }
        }
    }
    currentBranch = branchName;
    commitHead = branches[branchName];
    // Restore files from the HEAD commit of the branch
    for (FileNode* f = commitHead->fileHead; f; f = f->next) {
        copyFile(f->versionedFileName, f->fileName);
    }
    cout << "Checked out branch '" << branchName << "' (HEAD -> #" << commitHead->commitNumber << ")." << endl;
    save();
}

void MiniGit::printHistory() {
    cout << "--- History for branch '" << currentBranch << "' ---";
    for (CommitNode* c = commitHead; c; c = c->next) {
        cout << "Commit #" << c->commitNumber << ": " << c->message << "";
        for (FileNode* f = c->fileHead; f; f = f->next)
            cout << "  " << f->fileName << " [hash: " << f->contentHash << "]";
    }
}

void MiniGit::printBranches() {
    cout << "Branches:";
    for (auto& [name, head] : branches) {
        cout << (name == currentBranch ? "* " : "  ") << name << " (HEAD -> #" << head->commitNumber << ")";
    }
}

void MiniGit::createBranch(const string& name) {
    if (branches.count(name)) {
        cout << "Branch already exists.";
        return;
    }
    branches[name] = commitHead;
    cout << "Created branch '" << name << "' at commit #" << commitHead->commitNumber << "";
    save();
}

void MiniGit::checkoutBranch(const string& name) {
    if (!branches.count(name)) {
        cout << "Branch not found.";
        return;
    }
    currentBranch = name;
    commitHead = branches[name];
    cout << "Switched to branch '" << name << "' (HEAD -> #" << commitHead->commitNumber << ")";
    save();
}


void MiniGit::diffCommits(int c1, int c2) {
    CommitNode *first = nullptr, *second = nullptr;
    for (CommitNode* c = commitHead; c; c = c->next) {
        if (c->commitNumber == c1) first = c;
        if (c->commitNumber == c2) second = c;
    }
    if (!first || !second) {
        cout << "Invalid commit numbers.";
        return;
    }

    unordered_map<string, string> h1, h2;
    for (FileNode* f = first->fileHead; f; f = f->next) h1[f->fileName] = f->contentHash;
    for (FileNode* f = second->fileHead; f; f = f->next) h2[f->fileName] = f->contentHash;

    unordered_set<string> all;
    for (auto& [k, _] : h1) all.insert(k);
    for (auto& [k, _] : h2) all.insert(k);

    cout << "Changed files between commits " << c1 << " and " << c2 << ":";
    for (const auto& f : all) {
        if (h1[f] != h2[f]) cout << "- " << f << "";
    }
}

void MiniGit::mergeBranch(const string& branchName) {
    if (!branches.count(branchName)) {
        cout << "Branch does not exist.";
        return;
    }
    CommitNode* other = branches[branchName];
    unordered_map<string, string> currentFiles, otherFiles;

    for (FileNode* f = commitHead->fileHead; f; f = f->next)
        currentFiles[f->fileName] = f->contentHash;

    for (FileNode* f = other->fileHead; f; f = f->next) {
        string fn = f->fileName;
        string otherHash = f->contentHash;

        if (!currentFiles.count(fn)) {
            cout << "Merged new file from " << branchName << ": " << fn << "";
            addFile(fn);
        } else if (currentFiles[fn] != otherHash) {
            cout << "CONFLICT: " << fn << " has changed in both branches.";
        }
    }
}

void MiniGit::init() {
    createMinigitDirectory();
    std::cout << "Initialized empty MiniGit repository in .minigit/\n";
}
