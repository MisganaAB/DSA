#include "minigit.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    if (args.empty()) {
        cout << "Usage:\n"
             << "  add <filename>\n"
             << "  remove <filename>\n"
             << "  commit\n"
             << "  checkout <commit#>\n"
             << "  history\n"
             << "  branch <branchname>\n"
             << "  switch <branchname>\n"
             << "  branches\n"
             << "  diff <commit#1> <commit#2>\n";
        return 1;
    }
    MiniGit git;
    std::string cmd = args[0];
    if (cmd == "init") {
        git.init();
    } else if (cmd == "add" && args.size() >= 2) {
        git.addFile(args[1]);
    } else if (cmd == "remove" && args.size() >= 2) {
        git.removeFile(args[1]);
    } else if (cmd == "commit") {
        if (args.size() >= 3 && args[1] == "-m") {
            git.commit(args[2]);
        } else {
            cout << "Usage: commit -m <message>\n";
            return 1;
        }
    } else if (cmd == "checkout" && args.size() >= 2) {
        git.checkout(stoi(args[1]));
    } else if (cmd == "history") {
        git.printHistory();
    } else if (cmd == "branch" && args.size() >= 2) {
        git.createBranch(args[1]);
    } else if (cmd == "switch" && args.size() >= 2) {
        git.checkoutBranch(args[1]);
    } else if (cmd == "branches") {
        git.printBranches();
    } else if (cmd == "diff" && args.size() >= 3) {
        git.diffCommits(stoi(args[1]), stoi(args[2]));
    } else {
        cout << "Invalid command or missing argument.\n";
        return 1;
    }
    return 0;
}
