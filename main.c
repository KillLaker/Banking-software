#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define SALT_LENGTH 17
#define ID_LENGTH 7
#define MAX_PASSWORD_LENGTH 30
#define MAX_NAME_LENGTH 50
#define MAX_USERNAME_LENGTH 30
#define MAX_MAIL_LENGTH 100
#define MAX_LINE_LENGTH 30
#define KEY 1

struct passwordData {
    char *hash;
    char *salt;
};

struct account {
    char *ID;
    char* name;
    char* username;
    struct passwordData *password;
    double money;
};

struct linkedlist_node {
    struct account *acc;
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

char* hash_password(char *str) {
    char* hash = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(hash, str);
    hash[strlen(str)] = '\0';
    for (int i = 0; i < strlen(str); i++) {
        hash[i] += KEY;
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
        if (equals(username, curr->acc->username)) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

struct linkedlist_node* make_new_node(char* id, char* name, char* username, char* hash_password, char* salt, double money) {
    struct linkedlist_node* new_node = malloc(sizeof(struct linkedlist_node));
    new_node->next = NULL;
    new_node->acc = malloc(sizeof(struct account));
    new_node->acc->ID = strdup(id);
    new_node->acc->name = strdup(name);
    new_node->acc->username = strdup(username);
    new_node->acc->password = malloc(sizeof(struct passwordData));
    new_node->acc->password->hash = strdup(hash_password);
    new_node->acc->password->salt = strdup(salt);
    new_node->acc->money = money;

    return new_node;
}

bool isEmptyLL(struct linkedlist *ll) {
    return ll->head == NULL;
}

void linkedlist_add(struct linkedlist *ll, char* id, char* name, char* username, char* hash_password, char* salt, double money) {
    struct linkedlist_node *new_node = make_new_node(id, name, username, hash_password, salt, money);
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

void hashtable_add(struct hashtable* table, char* id, char* name, char* username, char* hash_password, char *salt, double money);

void transfer(struct hashtable *source, struct hashtable *destination) {
    for(int i = 0; i < source->num_of_buckets; i++) {
         struct linkedlist_node *tmp = source->buckets[i]->head;
         while(tmp != NULL) {
            hashtable_add(destination, tmp->acc->ID, tmp->acc->name, tmp->acc->username, tmp->acc->password->hash, tmp->acc->password->salt, tmp->acc->money);
            tmp = tmp->next;
         }
    }
}

struct hashtable* hashtable_resize(struct hashtable *table) {
    struct hashtable *new_table = hashtable_init(table->num_of_buckets * 2, table->max_elements_per_bucket);
    transfer(table, new_table);

    return new_table;
}

void hashtable_add(struct hashtable* table, char* id, char* name, char* username, char* hash_password, char *salt, double money) {
    int bucket_index = hash(username) % table->num_of_buckets;
    struct linkedlist* ll = table->buckets[bucket_index];

    if (!linkedlist_contains(ll, username)) {
        linkedlist_add(ll, id, name, username, hash_password, salt, money);

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
                printf("%s, %s, %s, %s, %s, %lf\n", tmp->acc->ID, tmp->acc->name, tmp->acc->username, tmp->acc->password->hash, tmp->acc->password->salt, tmp->acc->money);
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

char* generate_ID() {
    char* id = malloc(sizeof(char) * (SALT_LENGTH + 1));
    char charBuff[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    srand(time(NULL));
    for (int i = 0; i < ID_LENGTH; i++) {
        int index = rand() % (sizeof(charBuff) - 1);
        id[i] = charBuff[index];
    }
    id[SALT_LENGTH] = '\0';
    return id;
}

struct passwordData* hashNewPassword(char *password) {
    struct passwordData* data = malloc(sizeof(struct passwordData));
    char* salt = generate_salt();
    data->salt = strdup(salt);

    char *hash_str = malloc(strlen(salt) + strlen(password) + 1);
    strcpy(hash_str, salt);
    strcat(hash_str, password);

    char* hash = hash_password(hash_str);
    data->hash = strdup(hash);

    free(hash_str);
    free(hash);
    free(salt);

    return data;
}

void getLineCurr(char *str){
    fgets(str, MAX_MAIL_LENGTH, stdin);
    str[strcspn(str, "\n")] = 0;
}

void putInFile(char* str, char *filename) {
    FILE *f;
    f = fopen(filename, "a");
    fprintf(f, "%s\n", str);
    fclose(f);
}

void putInFileLastParameter(double money, char *filename) {
    FILE *f;
    f = fopen(filename, "a");
    fprintf(f, "%.2f\n\n", money); // Format money with 2 decimal places
    fclose(f);
}

void getName(char *username, char *filename) {
    char name[MAX_MAIL_LENGTH];
    printf("Enter name: ");
    getLineCurr(name);

    putInFile(name, filename);
}

void getUsername(char *username, char *filename) {
    char mail[MAX_MAIL_LENGTH];
    printf("Enter email: ");
    getLineCurr(mail);
    for(int i = 0; i < strlen(mail); i++) {
        if(mail[i] == '@')
            break;
        username[i] = mail[i];
    }
    username[strlen(username)] = '\0';

    putInFile(username, filename);

}

void getPassword(char *password, char *filename, bool isItForLogin) {
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

    if(isItForLogin == false) { //if it is for register we should put it in a file
        struct passwordData* data = hashNewPassword(password);

        putInFile(data->hash, filename);
        putInFile(data->salt, filename);
    }
}

void load_data_from_file(struct hashtable* table, const char* filename) {
    FILE *f;
    f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "Error opening password file.\n");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    char id[ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char salt[SALT_LENGTH];
    char hash[MAX_PASSWORD_LENGTH + SALT_LENGTH];
    double money;

    while (true) {
    if (fgets(line, MAX_LINE_LENGTH, f) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';
        strcpy(id, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading username file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(name, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading username file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(username, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading hash file.\n");
            exit(1);
        }

        line[strcspn(line, "\n")] = '\0';
        strcpy(hash, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading asd file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(salt, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading cvko file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        money = atof(line);

        hashtable_add(table, id, name, username, hash, salt, money);
        fgets(line, MAX_LINE_LENGTH, f);
    }

    fclose(f);
}

void registerUser(struct hashtable* table, char *filename) {
    char *id = generate_ID();
    putInFile(id, filename);
    char name[MAX_NAME_LENGTH] = "";
    getName(name, filename);
    char username[MAX_USERNAME_LENGTH] = "";
    getUsername(username, filename);
    char password[MAX_PASSWORD_LENGTH] = "";
    getPassword(password, filename, false);
    struct passwordData* data = hashNewPassword(password);
    putInFileLastParameter(0, filename);
    hashtable_add(table, id, name, username, data->hash, data->salt, 0);
    free(data);
}

bool isThereSuchAUser(struct hashtable* table, char* user) {
    int bucket_index = hash(user) % table->num_of_buckets;
    struct linkedlist* ll = table->buckets[bucket_index];

    struct linkedlist_node* curr = ll->head;
    while(curr != NULL) {
        if (equals(user, curr->acc->username)) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

struct linkedlist_node* findUser(struct hashtable* table, char* user) {
    int bucket_index = hash(user) % table->num_of_buckets;
    struct linkedlist* ll = table->buckets[bucket_index];

    struct linkedlist_node* curr = ll->head;
    while(curr != NULL) {
        if (equals(user, curr->acc->username)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

struct linkedlist_node* loginUser(struct hashtable* table, char *filename) {
    char user[MAX_USERNAME_LENGTH];
    printf("Enter username: ");
    scanf("%s", user);
    char password[MAX_PASSWORD_LENGTH];
    getPassword(password, filename, true);

    if(isThereSuchAUser(table, user)) {
        struct linkedlist_node *userNode = findUser(table, user);

        char *hash_str = malloc(strlen(userNode->acc->password->salt) + strlen(password) + 1);
        strcpy(hash_str, userNode->acc->password->salt);
        strcat(hash_str, password);

        char* hash = hash_password(hash_str);
        if(strcmp(hash, userNode->acc->password->hash) == 0) {
            printf("success!");
            return userNode;
        }
        else {
            printf("NOT SUCCESS!");
            struct  linkedlist_node *noLoggedUser = NULL;
            return noLoggedUser;
    }
    }

    else
        printf("No such an user!");

}

void putHashtableInFile(struct hashtable* table, char* filename) {
   FILE *f;
   f = fopen(filename, "w");
   fclose(f);
   for(int i = 0; i < table->num_of_buckets; i++) {
        struct linkedlist_node *tmp = table->buckets[i]->head;
        if(tmp != NULL) {
             while(tmp != NULL) {
                putInFile(tmp->acc->ID, filename);
                putInFile(tmp->acc->name, filename);
                putInFile(tmp->acc->username, filename);
                putInFile(tmp->acc->password->hash, filename);
                putInFile(tmp->acc->password->salt, filename);
                putInFileLastParameter(tmp->acc->money, filename);
                tmp = tmp->next;
             }
        }
    }
}

int main()
{
    struct hashtable *table = hashtable_init(10, 5);
    char* filenameForTempFile = "dataTmp.txt";
    char* filenameForResultFile = "data.txt";

    load_data_from_file(table, filenameForResultFile);

    //registerUser(table, filenameForTempFile);
    //registerUser(table, filenameForTempFile);
    //registerUser(table);
    struct linkedlist_node *loggedUser = loginUser(table, filenameForTempFile);
    if(loggedUser != NULL)
        printf("Logged user: %s\n", loggedUser->acc->password->hash);

    loggedUser->acc->money = 156;


    //print_hashtable(table);
    //printf("\n%d\n", table->num_of_buckets);
    putHashtableInFile(table, filenameForResultFile);

    return 0;
}
