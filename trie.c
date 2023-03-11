#include "trie.h"
#include "code.h"
#include <stdlib.h>

TrieNode *trie_node_create(uint16_t index) {
    //initialize the node and allocate memory for it
    //use calloc to set all children to NULL
    TrieNode *node = (TrieNode *) calloc(1, sizeof(TrieNode));
    node->code = index; //set node = index
    return node;
}

void trie_node_delete(TrieNode *n) {
    //free n if its not null
    if (n != NULL) {
        free(n);
    }
}

TrieNode *trie_create(void) {
    //create root node
    TrieNode *root = trie_node_create(EMPTY_CODE);
    if (root == NULL) {
        return NULL;
    }
    return root;
}

void trie_reset(TrieNode *root) {
    //loop through all children of the root
    for (int i = 0; i < ALPHABET; i++) {
        //if child isnt NULL then loop through its children and set them to null
        if (root->children[i] != NULL) {
            trie_reset(root->children[i]);
            trie_node_delete(root->children[i]);
            root->children[i] = NULL;
        }
    }
}

void trie_delete(TrieNode *n) {
    //set base case
    if (n == NULL) {
        return;
    }
    //loop through all children of the node
    for (int i = 0; i < ALPHABET; i++) {
        //recursively call function on each child
        trie_delete(n->children[i]);
        //set child to NULL
        n->children[i] = NULL;
    }
    //delete node after setting all children to NULL
    trie_node_delete(n);
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    //if n is empty return NULL
    if (n == NULL) {
        return NULL;
    }
    //return the sym index of children
    return n->children[sym];
}
