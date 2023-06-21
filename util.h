#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "typing.h"

#ifndef MY_UTIL_H
#define MY_UTIL_H

/* token vector */
typedef struct {
    size_t size, capacity;
    Token *arr;
} TokenVector;

TokenVector *init_token_vector(){ // TODO: move to util.c
    TokenVector *ret = (TokenVector *)malloc(sizeof(TokenVector));
    Token *arr = (Token *)malloc(2 * sizeof(Token));
    *ret = (TokenVector){
        .size = 0,
        .capacity = 2,
        .arr = arr,
    };
    return ret;
};

void push_back_token(TokenVector *vec, Token t){
    if(vec->size == vec->capacity){
        vec->capacity *= 2;
        Token *newarr = (Token *)realloc(vec->arr, vec->capacity * sizeof(Token));
        assert(newarr);
        vec->arr = newarr;
    }
    vec->arr[vec->size++] = t;
}

#endif