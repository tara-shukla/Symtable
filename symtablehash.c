/*--------------------------------------------------------------------*/
/* symtablehash.c                                             */
/* Author: Tara Shukla                                              */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>


/*stores list of bucket counts*/
static const size_t auBucketCounts[] = {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521}; 
/*static const size_t numBucketCounts = sizeof(auBucketCounts)/sizeof(auBucketCounts[0]);*/


struct Node {
   /* The binding key */
    char *pcKey;
    /*the matching value*/
    const void *pvValue;

   /* The address of the next StackNode. */
   struct Node *next;
};

struct SymTable{
    struct Node** hashVals;
    size_t len;
    size_t bucketCount;
};

/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}

/*helper function to rehash all values and expand the hash function*/
void expandHash(SymTable_T oSymTable){
    size_t count = 0;
    int bucketIndex = 0;
    struct Node **oldTable;
    struct Node*current;

    while(auBucketCounts[bucketIndex]!=oSymTable->bucketCount){
        bucketIndex++;
    }

    oSymTable->bucketCount = auBucketCounts[bucketIndex++];
    oldTable = oSymTable->hashVals;

    oSymTable->hashVals = (struct Node**)malloc(sizeof(struct Node));
    while (count != oSymTable->bucketCount){
        oSymTable->hashVals[count] = (struct Node*)malloc(sizeof(struct Node));
        if (oSymTable->hashVals[count]==NULL) return;
        count++;
    }    

    /*this do not work bc its not accessing hash val old and new right*/
    count = 0;
    while(count<auBucketCounts[bucketIndex]){
        current = oSymTable->hashVals[count];
        while(current != NULL){
            SymTable_put(oSymTable,current->pcKey,current->pvValue);
            free(oldTable[count]);
            current = current->next;
        }
        count++;
    }
    free(oldTable);
}

SymTable_T SymTable_new(void){
    SymTable_T oSymTable;
    size_t count = 0;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable==NULL){
        return NULL;
    }
    oSymTable->bucketCount = auBucketCounts[0]; /*start at 509 buckets*/

    /*allocate space for all the nodes representing hash values in the hash table*/
    oSymTable->hashVals = (struct Node**)malloc(sizeof(struct Node));
    if (oSymTable->hashVals==NULL) return NULL;

    while (count != oSymTable->bucketCount){
        oSymTable->hashVals[count] = (struct Node*)malloc(sizeof(struct Node));
        if (oSymTable->hashVals[count]==NULL) return NULL;
        count++;
    }

    oSymTable->len = 0;
    
    return oSymTable;
}

/*helper func: given key, return pointer to node if it exists, NULL otherwise*/
struct Node * exists(SymTable_T oSymTable,const char *pcKey, size_t hashVal){
    struct Node *current;
    struct Node *next;
    
    assert(oSymTable != NULL);
    assert(pcKey!=NULL);

    current = oSymTable->hashVals[hashVal];
    while(current!=NULL){
        if (strcmp((current->pcKey), pcKey)==0){
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void SymTable_free(SymTable_T oSymTable){
    struct Node *current;
    size_t i = 0;
    
    assert(oSymTable != NULL);

    while(i<oSymTable->bucketCount){
        current = oSymTable->hashVals[i]->next;
        while(current!=NULL){
            free(current->pcKey);
            free(current);
            current = current->next;

        }
        i++;
    }

   free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable){
    
    return oSymTable->len;
}

int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue)
{

    struct Node *newNode;
    struct Node* present;
    char *pcKeyCopy;
    size_t hashVal = SymTable_hash(pcKey,oSymTable->bucketCount);

    assert(oSymTable != NULL);
    assert(pcKey!=NULL);

    present = exists(oSymTable, pcKey,hashVal);
    if (present!=NULL) return 0;
    else {
        newNode = (struct Node*)malloc(sizeof(struct Node));
        if (newNode == NULL) return 0;

        oSymTable->len ++;
        /*check if binding count exceeds bucket count, and if so adjust bucket count*/
        if (oSymTable->len > oSymTable->bucketCount){
            expandHash(oSymTable);

            /*rehash this new node*/
            hashVal = SymTable_hash(pcKey,oSymTable->bucketCount);
        }

        /*should this be size of char or size of char*  ? */
        pcKeyCopy = malloc(sizeof(char*)* (strlen(pcKey)+1));
        if (pcKeyCopy==NULL) return 0;
        strcpy(pcKeyCopy,pcKey);
        newNode->pcKey = pcKeyCopy;
        newNode->pvValue = pvValue;

        /*check this, possible bug*/
        newNode->next = oSymTable->hashVals[hashVal]->next->next;
        oSymTable->hashVals[hashVal]->next = newNode;
        return 1;
    }
}

void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue){

    const void * oldVal;
    struct Node *present; 
    size_t hashVal = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert (oSymTable!=NULL);
    

    present = exists(oSymTable, pcKey, hashVal);
    if (present == NULL) return NULL;

    oldVal = present->pvValue;
    present->pvValue = pvValue;
    return (void*)oldVal;
    
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey){
    struct Node *present; 
    size_t hashVal = SymTable_hash(pcKey, oSymTable->bucketCount);

    assert(oSymTable!=NULL);

    
    present = exists(oSymTable, pcKey, hashVal);
    if (present==NULL) return 0;
    return 1;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey){
    struct Node *present; 
    size_t hashVal = SymTable_hash(pcKey, oSymTable->bucketCount);

    assert(oSymTable!=NULL);

    present = exists(oSymTable, pcKey,hashVal);
    if (present==NULL) return NULL;
    return (void*)(present->pvValue);
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey){
   struct Node *current;
    struct Node *prev;
    struct Node *target;
    const void *val;
    size_t hashVal = SymTable_hash(pcKey, oSymTable->bucketCount);

    
    assert(oSymTable != NULL);


    target = exists(oSymTable,pcKey, hashVal);
    if (target==NULL){
        return NULL;
    }


    current = oSymTable->hashVals[hashVal]->next;
    while(current!=target&&current!=NULL){
        prev = current;
        current = current->next;
    }


    oSymTable->len--;
    val = current->pvValue;

    /*check if target was first*/
    if (current==oSymTable->hashVals[hashVal]->next){
        oSymTable->hashVals[hashVal]->next = current->next;
    }
    else {
        prev->next = current->next;

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
    size_t i = 0;
    
    assert(oSymTable != NULL);

    while(i<oSymTable->bucketCount){
        current = oSymTable->hashVals[i]->next;
        while(current!=NULL){
            (*pfApply)((char*)current->pcKey, (void*)current->pvValue, (void*)pvExtra);
            current = current->next;
        }
        i++;
    }
    
}

/*--------------------------------------------------------------------*/