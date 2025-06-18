#include "minigit.hpp"
#include <iostream>
using namespace std;

int main() {
    MiniGit git;
    int choice;
    string input;

    while (true) {
        cout << "\n===== MiniGit Menu =====\n";
        cout << "1. Add File\n2. Remove File\n3. Commit Changes\n4. Checkout Commit\n5. View History\n6. Create Branch\n7. Switch Branch\n8. List Branches\n9. Exit\nSelect: ";
        cin >> choice;
        switch (choice) {
            case 1:
                cout << "File to add: "; 
                cin >> input; 
                git.addFile(input); 
                break;
            case 2:
                cout << "File to remove: "; 
                cin >> input; 
                git.removeFile(input); 
                break;
            case 3:
                git.commit(); 
                break;
            case 4:
                cout << "Commit # to checkout: "; 
                cin >> choice; 
                git.checkout(choice); 
                break;
            case 5:
                git.printHistory(); 
                break;
            case 6:
                cout << "New branch name: "; 
                cin >> input; 
                git.createBranch(input); 
                break;
            case 7:
                cout << "Branch to switch: "; 
                cin >> input; 
                git.checkoutBranch(input); 
                break;
            case 8:
                git.printBranches(); 
                break;
            case 9:
                return 0;
            default:
                cout << "Invalid option.\n";
        }
    }
}
