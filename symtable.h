/*--------------------------------------------------------------------*/
/* symtable.h                                       */
/* Author: Tara Shukla                                               */
/*--------------------------------------------------------------------*/
#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED
#include<stddef.h>
#include <stdlib.h>
#include <string.h>



struct SymTable;
typedef struct SymTable *SymTable_T;

/*return a new SymTable object that contains no bindings, 
or NULL if insufficient memory is available.*/
SymTable_T SymTable_new(void);

/*free all memory occupied by oSymTable*/
void SymTable_free(SymTable_T oSymTable);

/*return size_t the number of bindings in oSymTable*/
size_t SymTable_getLength(SymTable_T oSymTable);

/*add a new binding to oSymTable w key pcKey, value pvValue and return 1 (TRUE)*/
/*if already exists leave oSymTable unchanged and return 0 (FALSE)*/
/*if insufficient mem, return 0*/
int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

/*if binding w key pcKey exists in oSymTable, replace value w pvValue and return old val*/
/*else leave unchanged, return null*/
void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue);

/*return 1 (TRUE) if oSymTable contains a binding whose key is pcKey, else 0 (FALSE)*/
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/*return the value in oSymTable w key pcKey, or NULL if no such binding exists*/
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/*remove and return value with key pcKey; if nonexistent, return null*/
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/*apply pfApply to each binding, passing pvExtra as parameter*/
void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra);

/*--------------------------------------------------------------------*/
#endif