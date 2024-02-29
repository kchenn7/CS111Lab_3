#include "hash-table-base.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <pthread.h>

struct list_entry {
    const char *key;
    uint32_t value;
    SLIST_ENTRY(list_entry) pointers;
};

SLIST_HEAD(list_head, list_entry);

struct hash_table_entry {
    struct list_head list_head;
};

struct hash_table_v1 {
    struct hash_table_entry entries[HASH_TABLE_CAPACITY];
    pthread_mutex_t mutex; // Add a mutex for synchronization
};

void hash_table_v1_destroy(struct hash_table_v1 *hash_table);

struct hash_table_v1 *hash_table_v1_create() {
    struct hash_table_v1 *hash_table = calloc(1, sizeof(struct hash_table_v1));
    assert(hash_table != NULL);
    int init_ret = pthread_mutex_init(&hash_table->mutex, NULL); // Initialize the mutex
    if(init_ret!=0)
    {
        hash_table_v1_destroy(hash_table);
        exit(init_ret);
    }

    for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
        struct hash_table_entry *entry = &hash_table->entries[i];
        SLIST_INIT(&entry->list_head);
    }
    return hash_table;
}

static struct hash_table_entry *get_hash_table_entry(struct hash_table_v1 *hash_table,
                                                     const char *key) {
    assert(key != NULL);
    uint32_t index = bernstein_hash(key) % HASH_TABLE_CAPACITY;
    struct hash_table_entry *entry = &hash_table->entries[index];
    return entry;
}

static struct list_entry *get_list_entry(struct hash_table_v1 *hash_table,
                                         const char *key,
                                         struct list_head *list_head) {
    assert(key != NULL);

    struct list_entry *entry = NULL;

    SLIST_FOREACH(entry, list_head, pointers) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
    }
    return NULL;
}

bool hash_table_v1_contains(struct hash_table_v1 *hash_table,
                            const char *key) {
    struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);
    struct list_head *list_head = &hash_table_entry->list_head;
    struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);
    return list_entry != NULL;
}

void hash_table_v1_add_entry(struct hash_table_v1 *hash_table, const char *key, uint32_t value) {
    int lock_ret = pthread_mutex_lock(&hash_table->mutex);
    if (lock_ret!= 0) // Lock the mutex for synchronization
    {
        hash_table_v1_destroy(hash_table);
        exit(lock_ret);
    }

    struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);
    struct list_head *list_head = &hash_table_entry->list_head;
    struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);

    /* Update the value if it already exists */
    if (list_entry != NULL) {
        list_entry->value = value;
        int unlock_ret = pthread_mutex_unlock(&hash_table->mutex); // unlock the mutex before returning
        if (unlock_ret!= 0)
        {
            hash_table_v1_destroy(hash_table);
            exit(unlock_ret);
        }
        return;
    }

    list_entry = calloc(1, sizeof(struct list_entry));
    list_entry->key = key;
    list_entry->value = value;
    SLIST_INSERT_HEAD(list_head, list_entry, pointers);

    int unlock_ret2 = pthread_mutex_unlock(&hash_table->mutex); // unlock the mutex before returning
    if (unlock_ret2!= 0)
    {
        hash_table_v1_destroy(hash_table);
        exit(unlock_ret2);
    } // Unlock the mutex after adding the entry

}

uint32_t hash_table_v1_get_value(struct hash_table_v1 *hash_table, const char *key) {
    struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);
    struct list_head *list_head = &hash_table_entry->list_head;
    struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);
    assert(list_entry != NULL);
    return list_entry->value;
}

void hash_table_v1_destroy(struct hash_table_v1 *hash_table) {

    for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
        struct hash_table_entry *entry = &hash_table->entries[i];
        struct list_head *list_head = &entry->list_head;
        struct list_entry *list_entry = NULL;
        while (!SLIST_EMPTY(list_head)) {
            list_entry = SLIST_FIRST(list_head);
            SLIST_REMOVE_HEAD(list_head, pointers);
            free(list_entry);
        }
    }
    int destroy_ret = pthread_mutex_destroy(&hash_table->mutex); // Destroy the mutex before freeing memory
    if (destroy_ret != 0)
    {
        free(hash_table);
        exit(destroy_ret);
    }
    free(hash_table);
}
