#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "md5.h"

/**
* This class contains methods used to
* manipulate a Merkle Hash Tree for
* the CIS241 project
*
* The md5.h and md5.c files are open source
* files provided by Alexander Peslyak to
* the public domain
*
* @author Alec Allain
* @version 7/26/18
*/

/**
* Global variables
*/
char* exitResult;
//char* fileName;
int numOfFiles;
int result;
struct node* root;
int running;
struct node* temp;

/**
* Main method
*/
int main() {
    // instanciates the methods
    struct node* addToTree();
    char* changeToMd();
    struct node* createNode();
    void finalHash();
    char* readTheFile();
    void scanDirectory();

    // instanciates the global variables
    exitResult = "";
    numOfFiles = 0;
    result = "";
    root = NULL;
    running = 1;
    temp = NULL;


    while(running == 1) {
        printf("\nWhat would you like to do?\n1.Scan the file directory\n2. Compute the final hash (Warning: Won't work unless you've already chose #1)\n3. Exit the program\nPlease enter a corresponding number: ");
        scanf("%d", &result);
        switch (result) {
            case 1:
                printf("\nScanning...\n");
                scanDirectory();
                printf("\nThe directory has been successfully scanned and hashed\n");
                break;
            case 2:
                printf("\nCalculating the final parent hash...\n");
                finalHash(root, NULL);
                break;
            case 3:
                printf("\nAre you sure you want to exit?\nExiting will delete your tree\nType yes or no: ");
                scanf("%s", &exitResult);
                if (strcmp(&exitResult, "yes") == 0) {
                    printf("\nYou have chosen to exit. Goodbye\n");
                    running = 0;
                    break;
                } else if (strcmp(&exitResult, "no") == 0) {
                    printf("\nBringing you back to the main menu\n");
                    break;
                }

            default:
                printf("\nNot a valid selection. Try again.\n");
                break;
        }
    }
    return running;
}

/**
* This structure is the basis for
* a binary tree node
*
* Every other method wont work unless
* this method is before the others
*/
struct node {
    // value can be hash value or name of file
    char* value;
    struct node* left;
    struct node* right;
};

/**
* This method adds a node to a
* currently existing tree
*
* @return node that was added to tree
*/
struct node* addToTree (struct node* root, char* val) {
    if (root->right == NULL) {
        root->right = createNode(val);
        printf("\nNode has been added to the tree");
    } else if (root->left == NULL) {
        root->left = createNode(val);
        printf("\nNode has been added to the tree");
    }
    return root;
}

/**
* This method hashes the string in a node
* using MD5 protocol
*
* @return result of MD5 hashing
*
* This function was part of an answer provided
* by Todd on Stack Overflow and edited
* by Alec
*
* URL: https://stackoverflow.com/questions/7627723/how-to-create-a-md5-hash-of-a-string-in-c
*/
char* changeToMd (char* val, int wordLength) {
    int num;
    MD5_CTX c;
    char holder[16];
    char* result = (char*) malloc(32);

    MD5_Init(&c);

    while (wordLength > 0) {
        if (wordLength > 512) {
            MD5_Update(&c, val, 512);
        } else {
            MD5_Update(&c, val, wordLength);
        }
        wordLength -= 512;
        val += 512;
    }

    MD5_Final(holder, &c);

    for (num = 0; num < 16; num++) {
        snprintf(&(result[num*2]), 32, "%02x", (int) holder);
    }

    return result;
}

/**
* This method creates a new node
* in memory
*
* @return newly created node for tree
*/
struct node* createNode (char* value) {
    struct node* node = (struct node*)malloc(sizeof(node));

    // sets the main parts of the node with
    // the given value
    node->value = value;
    node->right = NULL;
    node->left = NULL;

    return node;
}

/**
* This method calculates the final hash
* of all the hashed files in the directory
*/
void finalHash (struct node* root, struct node* lastNode) {
    char* mixHash;
    char* newHash;
    struct node* nextNode;

    // sets up a temporary node for holding information
    // of the current node to use with the next
    temp = malloc(sizeof(struct node));

    if (root->value == NULL) {
        printf("\nFinal hash couldn't be generated\n");
    }  else if (root->right == NULL && root->left == NULL) {
        printf("\nThe final hash is: %s\n", root->value);
    } else if (root->right == NULL) {
        nextNode = root->left;
    } else if (root->left == NULL) {
        nextNode = root->right;
    } else {
        // the hashes are mixed together
        mixHash = strcat(root->value, nextNode->value);

        // new hash is created from previous mixed hash
        newHash = changeToMd(mixHash, strlen(mixHash));

        // Newly created hash gets placed in temporary node
        temp->value = newHash;

        // previous node get compared with temporary node
        // as long as lastNode has information embedded
        if (lastNode != NULL) {
            lastNode->right = temp;
            temp->left = lastNode;
        } else if (lastNode != NULL && lastNode->right != root) {
            lastNode->left = temp;
            temp->left = lastNode;
        } else if (lastNode != NULL && lastNode->left != root) {
            lastNode->right = temp;
            temp->right = lastNode;
        } else {
            temp->right = NULL;
            temp->left = NULL;
        }

        printf("\nThe final hash of the tree is: ");
        return finalHash(nextNode->left, temp);
        printf("\n");
    }
}

/**
* This method looks through each file and scans
* them for any possible changes
*/
char* readTheFile (char* name) {
    char* change = NULL;
    int length;
    FILE* file = fopen(name, "rt");

    // reads the fie from begining to end
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    change = (char*) malloc(sizeof(char) * (length + 1));

    change[length] = "\0";

    // checks for length change, adding of characters, etc.
    fread(change, sizeof(char), length, file);
    fclose(file);

    return change;
}

/**
* This method scans the entire directory and
* hashes each file
*/
void scanDirectory () {
    struct dirent *d;
    //struct node* root = NULL;
    //int numOfFiles;
    struct node* firstNode = NULL;
    struct node* lastNode = NULL;
    char* dirPath = "/Users/aleclightyear/Desktop/projectFiles/";
    DIR *direct = opendir(dirPath);
    char* fileName;
    char* material;
    char* hash;
    int sizeOfFile;

    // reads the directory end to end and scans all files
    // for their information
    while ((d = readdir(direct)) != NULL) {
        if (strncmp(fileName, ".", 1)) {
            fileName = d->d_name;
            material = readTheFile(fileName);
            sizeOfFile = strlen(material);
            hash = changeToMd(material, sizeOfFile);

            if (lastNode == NULL) {
                lastNode = createNode(hash);
                firstNode = lastNode;
                root = firstNode;
            } else {
                lastNode = addToTree(lastNode, hash);
            }

        }
        numOfFiles++;
    }

    // Scanning produces an addition of three "files". Number is adjusted
    numOfFiles -= 3;
    closedir(direct);
    printf("\nDirectory has been scanned. Number of files: %d\n", numOfFiles);

}

