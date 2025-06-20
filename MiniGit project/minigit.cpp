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

