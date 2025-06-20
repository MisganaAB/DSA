#include "minigit.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

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

    unordered_map<int, CommitNode*> allCommits;
    for (auto const& [branchName, branchHead] : branches) {
        for (CommitNode* c = branchHead; c; c = c->next) {
            if (allCommits.find(c->commitNumber) == allCommits.end()) {
                allCommits[c->commitNumber] = c;
            }
        }
    }

    if (allCommits.count(c1)) first = allCommits[c1];
    if (allCommits.count(c2)) second = allCommits[c2];
    
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


void MiniGit::save() {
    std::filesystem::create_directories(".minigit/meta");
    // Save HEAD (current branch)
    std::ofstream headFile(".minigit/meta/HEAD.txt");
    headFile << currentBranch << std::endl;
    headFile.close();
    // Save branches (branch name -> commit number)
    std::ofstream branchesFile(".minigit/meta/branches.txt");
    for (const auto& [name, head] : branches) {
        branchesFile << name << " " << (head ? head->commitNumber : -1) << std::endl;
    }
    branchesFile.close();
    // Save commits
    std::ofstream commitsFile(".minigit/meta/commits.txt");
    std::unordered_set<int> seen;
    for (const auto& [name, head] : branches) {
        for (CommitNode* c = head; c; c = c->next) {
            if (seen.count(c->commitNumber)) continue;
            seen.insert(c->commitNumber);
            commitsFile << c->commitNumber << "|" << c->message << std::endl;
            for (FileNode* f = c->fileHead; f; f = f->next) {
                commitsFile << "F|" << f->fileName << "|" << f->versionedFileName << "|" << f->contentHash << std::endl;
            }
            commitsFile << "ENDC" << std::endl;
        }
    }
    commitsFile.close();
}

void MiniGit::load() {
    // Clear current state
    for (auto& [b, head] : branches) {
        CommitNode* c = head;
        std::unordered_map<CommitNode*, bool> freed;
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
    branches.clear();
    commitHead = nullptr;
    currentCommitNumber = 0;
    currentBranch = "main";
    // Load HEAD
    std::ifstream headFile(".minigit/meta/HEAD.txt");
    if (headFile) {
        std::getline(headFile, currentBranch);
        headFile.close();
    }
    // Load branches
    std::unordered_map<int, CommitNode*> commitMap;
    std::ifstream branchesFile(".minigit/meta/branches.txt");
    std::vector<std::pair<std::string, int>> branchPairs;
    if (branchesFile) {
        std::string line;
        while (std::getline(branchesFile, line)) {
            std::istringstream iss(line);
            std::string name;
            int num;
            if (iss >> name >> num) {
                branchPairs.push_back({name, num});
            }
        }
        branchesFile.close();
    }
    // Load commits
    std::ifstream commitsFile(".minigit/meta/commits.txt");
    std::unordered_map<int, CommitNode*> prevMap;
    if (commitsFile) {
        std::string line;
        CommitNode* last = nullptr;
        while (std::getline(commitsFile, line)) {
            if (line.empty()) continue;
            if (line.find('|') != std::string::npos && line.substr(0, 1) != "F") {
                // Commit line
                size_t bar = line.find('|');
                int num = std::stoi(line.substr(0, bar));
                std::string msg = line.substr(bar + 1);
                CommitNode* c = new CommitNode{msg, num, nullptr, nullptr};
                commitMap[num] = c;
                if (last) last->next = c;
                last = c;
                prevMap[num] = c;
            } else if (line.substr(0, 2) == "F|") {
                // File line
                std::istringstream iss(line.substr(2));
                std::string fname, vfname, hash;
                std::getline(iss, fname, '|');
                std::getline(iss, vfname, '|');
                std::getline(iss, hash, '|');
                if (vfname.empty()) vfname = ".minigit/objects/" + hash;
                FileNode* f = new FileNode{fname, vfname, hash, nullptr};
                if (last) {
                    f->next = last->fileHead;
                    last->fileHead = f;
                }
            } else if (line == "ENDC") {
                last = nullptr;
            }
        }
        commitsFile.close();
    }
    // Rebuild branches
    for (auto& [name, num] : branchPairs) {
        if (commitMap.count(num)) {
            branches[name] = commitMap[num];
        }
    }
    // Set commitHead
    if (branches.count(currentBranch)) {
        commitHead = branches[currentBranch];
        if (commitHead) currentCommitNumber = commitHead->commitNumber;
    } else {
        // If nothing loaded, create initial commit
        commitHead = new CommitNode{"Initial commit", 0, nullptr, nullptr};
        currentCommitNumber = 0;
        branches[currentBranch] = commitHead;
    }
}

void MiniGit::init() {
    createMinigitDirectory();
    std::cout << "Initialized empty MiniGit repository in .minigit/\n";
}
