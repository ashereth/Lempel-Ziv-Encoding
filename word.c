#include "word.h"
#include "code.h"
#include <stdlib.h>
#include <string.h>

Word *word_create(uint8_t *syms, uint32_t len) {
    //if syms ==null or len == 0  return NULL
    //make new word and return null if it fails
    Word *word = (Word *) calloc(1, sizeof(Word));
    if (word == NULL) {
        return NULL;
    }
    //allocate memory for the syms array
    word->syms = (uint8_t *) malloc(sizeof(uint8_t) * len);
    //copy syms array into word->syms
    memcpy(word->syms, syms, sizeof(uint8_t) * len);
    word->len = len;
    return word;
}

Word *word_append_sym(Word *w, uint8_t sym) {
    //create new word with space to append
    Word *word = (Word *) calloc(1, sizeof(Word));
    if (word == NULL) {
        return NULL;
    }
    //allocate memory for the syms array
    word->syms = (uint8_t *) malloc(sizeof(uint8_t) * (w->len + 1));
    //copy syms array into word->syms
    memcpy(word->syms, w->syms, sizeof(uint8_t) * w->len);
    word->syms[w->len] = sym;
    word->len = w->len + 1;
    return word;
}

void word_delete(Word *w) {
    //if w isnt empty free it
    if (w != NULL) {
        //free syms array
        free(w->syms);
        free(w);
        w = NULL;
    }
}

WordTable *wt_create(void) {
    //initialize a new wordtable of size MAX_CODE
    WordTable *wt = (WordTable *) calloc(MAX_CODE, sizeof(Word *));
    if (wt == NULL) {
        return NULL;
    }
    //create an empty word to go in wt
    Word *emptyword = word_create(NULL, 0);
    wt[EMPTY_CODE] = emptyword;
    return wt;
}

void wt_reset(WordTable *wt) {
    //iterate through words up to max_code
    for (int i = 0; i < MAX_CODE; i++) {
        //if word isnt empty and its not the EMPTY_CODE word then delete it
        if (wt[i] != NULL && i != EMPTY_CODE) {
            word_delete(wt[i]);
            wt[i] = NULL;
        }
    }
}

void wt_delete(WordTable *wt) {
    //delete all the words inside wordtable
    for (int i = 0; i < MAX_CODE; i++) {
        word_delete(wt[i]);
    } //free all allocated space for wordtable
    if (wt != NULL) {
        free(wt); //wt is just an array so free it once
    }
}
