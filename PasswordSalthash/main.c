#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <stdbool.h>

#define SALT_LENGTH 17
#define MAX_PASSWORD_LENGTH 30
#define MAX_USERNAME_LENGTH 30
#define MAX_MAIL_LENGTH 100
#define MAX_LINE_LENGTH 30

struct passwordData {
    int hash;
    char *salt;
};

struct linkedlist_node {
    char* username;
    struct passwordData* password;
    struct linkedlist_node* next;
};

struct linkedlist {
    struct linkedlist_node* head;
    struct linkedlist_node* tail;
    int count;
};

struct hashtable {
    int num_of_buckets;
    struct linkedlist** buckets;
    int max_elements_per_bucket;
};

int hash(char *str) {
    int hash = 0;
    for(int i = 0; i < strlen(str); i++) {
        hash += str[i];
    }

    return hash;
}

struct hashtable* hashtable_init(int num_of_buckets, int max_elements_per_bucket) {
    struct hashtable *table = malloc(sizeof(struct hashtable));
    table->num_of_buckets = num_of_buckets;
    table->max_elements_per_bucket = max_elements_per_bucket;
    table->buckets = malloc(sizeof(struct linkedlist*) * num_of_buckets);
    for(int i = 0; i < num_of_buckets; i++) {
        table->buckets[i] = malloc(sizeof(struct linkedlist));
        table->buckets[i]->head = NULL;
        table->buckets[i]->tail = NULL;
        table->buckets[i]->count = 0;
    }

    return table;
}

int equals(char* a, char* b) {
    return strcmp(a, b) == 0;
}

int linkedlist_contains(struct linkedlist* ll, char* username) {
    struct linkedlist_node* curr = ll->head;
    while(curr != NULL) {
        if (equals(username, curr->username)) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

struct linkedlist_node* make_new_node(char* username, int hash_password, char* salt) {
    struct linkedlist_node* new_node = malloc(sizeof(struct linkedlist_node));
    new_node->next = NULL;
    new_node->username = username;
    new_node->password = malloc(sizeof(struct passwordData));
    new_node->password->hash = hash_password;
    new_node->password->salt = salt;

    return new_node;
}


bool isEmptyLL(struct linkedlist *ll) {
    return ll->head == NULL;
}

void linkedlist_add(struct linkedlist *ll, char *username, int hash_password, char *salt) {
    struct linkedlist_node *new_node = make_new_node(username, hash_password, salt);
    if(isEmptyLL(ll)) {
        ll->head = new_node;
        ll->tail = new_node;
    }
    else {
        ll->tail->next = new_node;
        ll->tail = new_node;
    }

    ll->count++;
}

void hashtable_add(struct hashtable* table, char* username, int hash_password, char *salt);

void transfer(struct hashtable *source, struct hashtable *destination) {
    for(int i = 0; i < source->num_of_buckets; i++) {
         struct linkedlist_node *tmp = source->buckets[i]->head;
         while(tmp != NULL) {
            hashtable_add(destination, tmp->username, tmp->password->hash, tmp->password->salt);
            tmp = tmp->next;
         }
    }
}

struct hashtable* hashtable_resize(struct hashtable *table) {
    struct hashtable *new_table = hashtable_init(table->num_of_buckets * 2, table->max_elements_per_bucket);
    transfer(table, new_table);

    return new_table;
}

void hashtable_add(struct hashtable* table, char* username, int hash_password, char *salt) {
    int bucket_index = hash(username) % table->num_of_buckets;
    struct linkedlist* ll = table->buckets[bucket_index];

    if (!linkedlist_contains(ll, username)) {
        linkedlist_add(ll, username, hash_password, salt);

        if (ll->count > table->max_elements_per_bucket) {
           struct hashtable* new_table = hashtable_resize(table);
           *table = *new_table;
           free(new_table);
        }
    }
}


void print_hashtable(struct hashtable *table) {
    for(int i = 0; i < table->num_of_buckets; i++) {
        struct linkedlist_node *tmp = table->buckets[i]->head;
        if(tmp != NULL) {
             while(tmp != NULL) {
                printf("%s, ", tmp->username);
                printf("%d, ", tmp->password->hash);
                printf("%s\n", tmp->password->salt);
                tmp = tmp->next;
             }
             printf("\n");
        }
    }
}

char* generate_salt() {
    char* salt = malloc(sizeof(char) * (SALT_LENGTH + 1));
    char charBuff[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    srand(time(NULL));
    for (int i = 0; i < SALT_LENGTH; i++) {
        int index = rand() % (sizeof(charBuff) - 1);
        salt[i] = charBuff[index];
    }
    salt[SALT_LENGTH] = '\0';
    return salt;
}

struct passwordData* hashNewPassword(char *password) {
    struct passwordData* data = malloc(sizeof(struct passwordData));
    char* salt = generate_salt();
    data->salt = strdup(salt);

    char *hash_str = malloc(strlen(salt) + strlen(password) + 1);
    strcpy(hash_str, salt);
    strcat(hash_str, password);

    data->hash = hash(hash_str);
    free(hash_str);

    return data;
}

void getLineCurr(char *str){
    fgets(str, MAX_MAIL_LENGTH, stdin);
    str[strcspn(str, "\n")] = 0;
}

void getUsername(char *username) {
    char mail[MAX_MAIL_LENGTH];
    printf("Enter email: ");
    getLineCurr(mail);
    for(int i = 0; i < strlen(mail); i++) {
        if(mail[i] == '@')
            break;
        username[i] = mail[i];
    }
    username[strlen(username)] = '\0';

    FILE *f;
    f = fopen("data.txt", "a");

    fprintf(f, "%s\n", username);

    fclose(f);

}

void getPassword(char *password) {
    int index = 0;
    char ch;
    printf("Enter password: ");
    while (1) {
        ch = getch();
        if (ch == '\r' || ch == '\n') {
            break;
        } else if (ch == '\b' && index > 0) {
            index--;
            printf("\b \b");
        } else if (index < MAX_PASSWORD_LENGTH - 1) {
            password[index] = ch;
            index++;
            printf("*");
        }
    }
    password[index] = '\0';

    struct passwordData* data = hashNewPassword(password);

    FILE *f;
    f = fopen("data.txt", "a");

    fprintf(f, "%d\n", data->hash);
    fprintf(f, "%s\n\n", data->salt);

    fclose(f);
    free(data->salt);
    free(data);
}

int main()
{
    /*
    char name[MAX_USERNAME_LENGTH] = "";
    getUsername(name);
    char password[MAX_PASSWORD_LENGTH] = "";
    getPassword(password);
    */

    struct hashtable *table = hashtable_init(10, 5);
    FILE *f;
    f = fopen("data.txt", "r");
    char line[MAX_LINE_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char salt[SALT_LENGTH];
    int hash;
    while (true) {
        if (fgets(line, MAX_LINE_LENGTH, f)==NULL) break;
        line[strcspn(line, "\n")] = '\0';
        strcpy(username, line);
        if (fgets(line, MAX_LINE_LENGTH, f)==NULL){
            fprintf(stderr, "Error reading password file.");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        hash = atoi(line);
        if (fgets(line, MAX_LINE_LENGTH, f)==NULL){
            fprintf(stderr, "Error reading password file.");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(salt, line);
        hashtable_add(table, username, hash, salt);
    }

    fclose(f);
    print_hashtable(table);

    return 0;
}
