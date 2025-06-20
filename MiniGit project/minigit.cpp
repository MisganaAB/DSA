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
