/*--------------------------------------------------------------------*/
/* symtablelist.c                                             */
/* Author: Tara Shukla                                              */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>

/*Nodes for linked list imp of symboltable*/
struct Node {
   /* The binding key */
    char *pcKey;
    /*the matching value*/
    const void *pvValue;

   /* The address of the next StackNode. */
   struct Node *next;
};

/*stores SymTable struct*/
struct SymTable{
    /*address of first node*/
    struct Node *first;
    /*len = number of bindings in symboltable*/
    size_t len;
};


SymTable_T SymTable_new(void){
    SymTable_T oSymTable;
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable==NULL){
        return NULL;
    }
    oSymTable->first = NULL;
    oSymTable->len = 0;
   return oSymTable;
}

/*helper func: given pcKey, return pointer to node if it exists in oSymTable, NULL otherwise*/
static struct Node * SymTable_exists(SymTable_T oSymTable,const char *pcKey){
    struct Node *current;
    struct Node *next;
    
    assert(oSymTable != NULL);
    assert(pcKey!=NULL);

    for (current = oSymTable->first;
        current != NULL;
        current = next)
    {
        if (strcmp((current->pcKey), pcKey)==0){
            return current;
        }
        next = current->next;
    }
    return NULL;
}

void SymTable_free(SymTable_T oSymTable){
    struct Node *current;
    struct Node *next;
    
    assert(oSymTable != NULL);

   for (current = oSymTable->first;
        current != NULL;
        current = next)
   {
      next = current->next;
      free(current->pcKey);
      free(current);
   }

   free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable){
    assert(oSymTable!=NULL);
    return oSymTable->len;
}


int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue){
    struct Node *newNode;
    struct Node* present;
    char *pcKeyCopy;

    assert(oSymTable != NULL);
    assert(pcKey!=NULL);

    present = SymTable_exists(oSymTable, pcKey);
    if (present!=NULL) return 0;
    else {
        newNode = (struct Node*)malloc(sizeof(struct Node));
        if (newNode == NULL)
            return 0;

        pcKeyCopy = malloc(sizeof(char)* (strlen(pcKey)+1));
        if (pcKeyCopy==NULL) {
            free(newNode);
            return 0;
        }
        strcpy(pcKeyCopy,pcKey);
        newNode->pcKey = pcKeyCopy;
        newNode->pvValue = pvValue;
        newNode->next = oSymTable->first;
        oSymTable->first = newNode;
        oSymTable->len ++;
        return 1;
    }
}


void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue){
    
    const void * oldVal;
    struct Node *present; 

    assert (oSymTable!=NULL);
    assert(pcKey!=NULL);

    present = SymTable_exists(oSymTable, pcKey);
    if (present == NULL) return NULL;

    oldVal = present->pvValue;
    present->pvValue = pvValue;
    return (void*)oldVal;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey){
    struct Node *present; 
    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    present = SymTable_exists(oSymTable, pcKey);
    if (present==NULL) return 0;
    return 1;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey){
    struct Node *present; 
    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    present = SymTable_exists(oSymTable, pcKey);
    if (present==NULL) return NULL;
    return (void*)(present->pvValue);
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey){
    struct Node *current;
    struct Node *prev;
    struct Node *target;
    const void *val;

    
    assert(oSymTable != NULL);
    assert(pcKey!=NULL);

    target = SymTable_exists(oSymTable,pcKey);
    if (target==NULL){
        return NULL;
    }


    current = oSymTable->first;
    prev = NULL;
    while(current!=target&&current!=NULL){
        prev = current;
        current = current->next;
    }

    if (current == NULL) return NULL; 
    oSymTable->len--;
    val = current->pvValue;

    /*check if target was first*/
    if (current==oSymTable->first){
        oSymTable->first = current->next;
    }
    else {
        if (prev!=NULL){
            prev->next = current->next;
        }
        
    }

    free(current->pcKey);
    free(current);
    return (void*)val;
}

void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra)
{

    struct Node *current;
    struct Node *next;
    
    assert(oSymTable != NULL);
    assert(pfApply!= NULL);

    for (current = oSymTable->first;
        current != NULL;
        current = next)
    {
        
        /*call (*pfApply)(pcKey, pvValue, pvExtra) for each pcKey/pvValue binding in oSymTable.*/
        (*pfApply)((char*)current->pcKey, (void*)current->pvValue, (void*)pvExtra);
        next = current->next;

    }
}

/*--------------------------------------------------------------------*/
