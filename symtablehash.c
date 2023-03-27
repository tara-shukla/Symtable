/*--------------------------------------------------------------------*/
/* symtablehash.c                                             */
/* Author: Tara Shukla                                              */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>


/*stores list of bucket counts*/
static const size_t auBucketCounts[] = {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521}; 
/*stores number of bucket counts*/
static const size_t numBucketCounts = sizeof(auBucketCounts)/sizeof(auBucketCounts[0]);

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
    /*pointer to array of node pointer linked lists (storing bindings)*/
    struct Node** hashVals;
    /*len = number of bindings in symboltable*/
    size_t len;
    /*number of buckets that bindings can hash to*/
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

/*helper function to rehash all values in oSymTableand expand the hash function*/
static void SymTable_expandHash(SymTable_T oSymTable){
    size_t count = 0;
    size_t bucketIndex = 0;
    struct Node **oldTable;
    struct Node*current;

    assert(oSymTable!=NULL);
    assert(oSymTable->hashVals!=NULL);

    /*find the bucketIndex of the current bucketCount*/
    while(auBucketCounts[bucketIndex]!=oSymTable->bucketCount){
        bucketIndex++;
    }

    /*check if bucketcount index is last index; if not, increase bucketcount*/
    if (bucketIndex==numBucketCounts){
        return;
    }

    /*update oSymTable's bucketCount*/
    oSymTable->bucketCount = auBucketCounts[++bucketIndex];

    oldTable = oSymTable->hashVals;

    oSymTable->hashVals = (struct Node**)malloc(sizeof(struct Node));
    while (count != oSymTable->bucketCount){
        oSymTable->hashVals[count] = (struct Node*)malloc(sizeof(struct Node));
        if (oSymTable->hashVals[count]==NULL) return;
        count++;
    }    

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
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable==NULL){
        return NULL;
    }
    oSymTable->bucketCount = auBucketCounts[0]; /*start at 509 buckets*/

    /*allocate space for all the nodes representing hash values in the hash table*/
    oSymTable->hashVals = (struct Node**)calloc(oSymTable->bucketCount,sizeof(struct Node*));
    if (oSymTable->hashVals==NULL) return NULL;

    oSymTable->len = 0;
    
    return oSymTable;
}

/*helper func: given key, return pointer to node if it exists in oSymTable, NULL otherwise*/
static struct Node * SymTable_exists(SymTable_T oSymTable,const char *pcKey, size_t hashVal){
    struct Node *current;
    
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
    struct Node*next;
    size_t i = 0;
    
    assert(oSymTable != NULL);

    while(i< oSymTable->bucketCount){
        
        for (current = oSymTable->hashVals[i];
        current != NULL;
        current = next)
    {
            next = current->next;
            free(current->pcKey);
            free(current);
    }
        i++;
    }
    free(oSymTable->hashVals);
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable){
    assert(oSymTable!=NULL);
    return oSymTable->len;
}

int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue)
{

    struct Node *newNode;
    struct Node* present;
    char *pcKeyCopy;
    size_t hashVal; 

    assert(oSymTable != NULL);
    assert(pcKey!=NULL);

    hashVal = SymTable_hash(pcKey,oSymTable->bucketCount);

    present = SymTable_exists(oSymTable, pcKey,hashVal);
    /*if the node is present, can't put: return 0*/
    if (present!=NULL) return 0;
    /*else put*/
    else {
        newNode = (struct Node*)malloc(sizeof(struct Node));
        if (newNode == NULL) return 0;

   
        /*check if binding count exceeds bucket count, and if so adjust bucket count*/
        if (oSymTable->len == (oSymTable->bucketCount)-1){
            SymTable_expandHash(oSymTable);
            /*rehash this new node*/
            hashVal = SymTable_hash(pcKey,oSymTable->bucketCount);
        }

        pcKeyCopy = (char*)malloc(sizeof(char)* (strlen(pcKey)+1));
        if (pcKeyCopy==NULL) {
            free(newNode); 
            return 0;
        }
        strcpy(pcKeyCopy,pcKey);
        newNode->pcKey = pcKeyCopy;
        newNode->pvValue = pvValue;

        /*set newnode-> next to current first node*/
        newNode->next = oSymTable->hashVals[hashVal];
    
        /*set newnode as first val in the list of the hashval*/
        oSymTable->hashVals[hashVal] = newNode;

         oSymTable->len ++;
        return 1;
    }
}

void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue){

    const void * oldVal;
    struct Node *present; 
    size_t hashVal = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert (oSymTable!=NULL);
     assert(pcKey!=NULL);


    present = SymTable_exists(oSymTable, pcKey, hashVal);
    if (present == NULL) return NULL;

    oldVal = present->pvValue;
    present->pvValue = pvValue;
    return (void*)oldVal;
    
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey){
    struct Node *present; 
    size_t hashVal = SymTable_hash(pcKey, oSymTable->bucketCount);

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    
    present = SymTable_exists(oSymTable, pcKey, hashVal);
    if (present==NULL) return 0;
    return 1;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey){
    struct Node *present; 
    size_t hashVal = SymTable_hash(pcKey, oSymTable->bucketCount);

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    present = SymTable_exists(oSymTable, pcKey,hashVal);
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
    assert(pcKey!=NULL);

    target = SymTable_exists(oSymTable,pcKey, hashVal);
    if (target==NULL){
        return NULL;
    }


    current = oSymTable->hashVals[hashVal];
    prev = NULL;
    while(current!=target&&current!=NULL){
        prev = current;
        current = current->next;
    }

    if (current == NULL) return NULL; 
    oSymTable->len--;
    val = current->pvValue;

    /*check if target was first*/
    if (current==oSymTable->hashVals[hashVal]){
        oSymTable->hashVals[hashVal] = current->next;
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
    size_t i = 0;
    
    assert(oSymTable != NULL);
    assert(pfApply!=NULL);

    while(i<oSymTable->bucketCount){
        current = oSymTable->hashVals[i];
        while(current!=NULL){
            (*pfApply)((char*)current->pcKey, (void*)current->pvValue, (void*)pvExtra);
            current = current->next;
        }
        i++;
    }
    
}

/*--------------------------------------------------------------------*/