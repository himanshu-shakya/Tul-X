#include "environment.h"

#define INITIAL_CAPACITY 16
#define MAX_LOAD_FACTOR  0.75

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

static char* copyStringSlice(const char* start, int length) {
    char* copy = (char*)malloc(length + 1);
    if (!copy) {
        fprintf(stderr, "Out of memory in copyStringSlice.\n");
        exit(EX_SOFTWARE);
    }
    memcpy(copy, start, length);
    copy[length] = '\0';
    return copy;
}

static void resizeTable(Environment* env) {
    int oldCapacity = env->capacity;
    Entry** oldBuckets = env->buckets;

    int newCapacity = (oldCapacity == 0) ? INITIAL_CAPACITY : oldCapacity * 2;
    Entry** newBuckets = (Entry**)calloc(newCapacity, sizeof(Entry*));
    if (!newBuckets) {
        fprintf(stderr, "Out of memory in resizeTable.\n");
        exit(EX_SOFTWARE);
    }

    /* Re-insert existing entries into new bucket array */
    for (int i = 0; i < oldCapacity; i++) {
        Entry* entry = oldBuckets[i];
        while (entry != NULL) {
            Entry* next = entry->next;
            uint32_t hash = hashString(entry->key, (int)strlen(entry->key));
            int index = (int)(hash % (uint32_t)newCapacity);

            entry->next = newBuckets[index];
            newBuckets[index] = entry;

            entry = next;
        }
    }

    if (oldBuckets != NULL) {
        free(oldBuckets);
    }

    env->buckets = newBuckets;
    env->capacity = newCapacity;
}

Environment* newEnvironment(Environment* enclosing) {
    Environment* env = (Environment*)malloc(sizeof(Environment));
    if (!env) {
        fprintf(stderr, "Out of memory in newEnvironment.\n");
        exit(EX_SOFTWARE);
    }

    env->enclosing = enclosing;
    env->capacity = INITIAL_CAPACITY;
    env->count = 0;
    env->buckets = (Entry**)calloc(INITIAL_CAPACITY, sizeof(Entry*));
    if (!env->buckets) {
        fprintf(stderr, "Out of memory allocating environment buckets.\n");
        exit(EX_SOFTWARE);
    }

    return env;
}

void freeEnvironment(Environment* env) {
    if (env == NULL) return;

    for (int i = 0; i < env->capacity; i++) {
        Entry* entry = env->buckets[i];
        while (entry != NULL) {
            Entry* next = entry->next;
            free(entry->key);
            freeValue(entry->value);
            free(entry);
            entry = next;
        }
    }

    free(env->buckets);
    free(env);
}

void envDefine(Environment* env, const char* name, int length, Value value) {
    if (env == NULL) return;

    if (env->capacity == 0 || ((double)(env->count + 1) / env->capacity) > MAX_LOAD_FACTOR) {
        resizeTable(env);
    }

    uint32_t hash = hashString(name, length);
    int index = (int)(hash % (uint32_t)env->capacity);

    /* Check if already declared in this immediate scope */
    Entry* entry = env->buckets[index];
    while (entry != NULL) {
        if ((int)strlen(entry->key) == length && memcmp(entry->key, name, length) == 0) {
            freeValue(entry->value);
            entry->value = copyValue(value);
            return;
        }
        entry = entry->next;
    }

    /* Insert new entry */
    Entry* newEntry = (Entry*)malloc(sizeof(Entry));
    if (!newEntry) {
        fprintf(stderr, "Out of memory in envDefine.\n");
        exit(EX_SOFTWARE);
    }

    newEntry->key = copyStringSlice(name, length);
    newEntry->value = copyValue(value);
    newEntry->next = env->buckets[index];
    env->buckets[index] = newEntry;
    env->count++;
}

bool envGet(Environment* env, Token name, Value* outValue) {
    if (env == NULL) return false;

    if (env->capacity > 0) {
        uint32_t hash = hashString(name.start, name.length);
        int index = (int)(hash % (uint32_t)env->capacity);

        Entry* entry = env->buckets[index];
        while (entry != NULL) {
            if ((int)strlen(entry->key) == name.length && memcmp(entry->key, name.start, name.length) == 0) {
                if (outValue != NULL) {
                    *outValue = copyValue(entry->value);
                }
                return true;
            }
            entry = entry->next;
        }
    }

    /* Look in enclosing scope */
    if (env->enclosing != NULL) {
        return envGet(env->enclosing, name, outValue);
    }

    return false;
}

bool envAssign(Environment* env, Token name, Value value) {
    if (env == NULL) return false;

    if (env->capacity > 0) {
        uint32_t hash = hashString(name.start, name.length);
        int index = (int)(hash % (uint32_t)env->capacity);

        Entry* entry = env->buckets[index];
        while (entry != NULL) {
            if ((int)strlen(entry->key) == name.length && memcmp(entry->key, name.start, name.length) == 0) {
                freeValue(entry->value);
                entry->value = copyValue(value);
                return true;
            }
            entry = entry->next;
        }
    }

    /* Assign to enclosing scope */
    if (env->enclosing != NULL) {
        return envAssign(env->enclosing, name, value);
    }

    return false;
}
