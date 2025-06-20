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
