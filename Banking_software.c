#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LINE_LENGTH 100
#define ID_LENGTH 10
#define MAX_NAME_LENGTH 50
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define SALT_LENGTH 20
#define MAX_MAIL_LENGTH 50
#define KEY 3

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_BLUE "\x1b[44m"
#define ANSI_COLOR_PINK "\x1b[45m"
#define ANSI_COLOR_BLACK "\x1b[30m"

struct PasswordData {
    char* hash;
    char* salt;
};

struct Account {
    char* ID;
    char* name;
    char* username;
    struct PasswordData* password;
    double money;
};

struct LinkedListNode {
    struct Account* acc;
    struct LinkedListNode* next;
};

struct LinkedList {
    struct LinkedListNode* head;
    struct LinkedListNode* tail;
    int count;
};

struct Hashtable {
    int num_of_buckets;
    struct LinkedList** buckets;
    int max_elements_per_bucket;
};

struct Transaction{
    char* transactionCode;
    struct Account* fromAccount;
    struct Account* toAccount;
    double amountToTransfer;
    struct Transaction* next;
};
struct TransactionQueue{
    struct Transaction* head;
    struct Transaction* tail;
};


int hash_password_for_hashtable(const char* str) {
    int hash = 0;
    for(int i = 0; i < strlen(str); i++) {
        hash += str[i];
    }

    return hash;
}

char* hash_password_for_file(const char *str) {
    char* hash = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(hash, str);
    hash[strlen(str)] = 0;
    for (int i = 0; i < strlen(str); i++) {
        hash[i] += KEY;
    }
    return hash;
}

char* mix_salt_password(const char* salt, const char* password) {
    int salt_length = strlen(salt);
    int password_length = strlen(password);
    int mixed_length = salt_length + password_length;

    char* mixed = malloc((mixed_length + 1) * sizeof(char));

    int i, j, k;
    i = j = k = 0;

    while (i < salt_length && j < password_length) {
        mixed[k++] = salt[i++];
        mixed[k++] = password[j++];
    }

    while (i < salt_length) {
        mixed[k++] = salt[i++];
    }

    while (j < password_length) {
        mixed[k++] = password[j++];
    }

    mixed[k] = '\0';

    return mixed;
}

struct Hashtable* hashtable_init(const int num_of_buckets, const int max_elements_per_bucket) {
    struct Hashtable* table = malloc(sizeof(struct Hashtable));
    table->num_of_buckets = num_of_buckets;
    table->max_elements_per_bucket = max_elements_per_bucket;
    table->buckets = malloc(sizeof(struct LinkedList*) * num_of_buckets);
    for(int i = 0; i < num_of_buckets; i++) {
        table->buckets[i] = malloc(sizeof(struct LinkedList));
        table->buckets[i]->head = NULL;
        table->buckets[i]->tail = NULL;
        table->buckets[i]->count = 0;
    }

    return table;
}

bool equals(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

bool linkedlist_contains(struct LinkedList* ll, const char* username) {
    struct LinkedListNode* curr = ll->head;
    while(curr != NULL) {
        if (equals(username, curr->acc->username)) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

struct LinkedListNode* make_new_node(const char* id, const char* name, const char* username, const char* hash_password, const char* salt, const double money) {
    struct LinkedListNode* new_node = malloc(sizeof(struct LinkedListNode));
    new_node->next = NULL;
    new_node->acc = malloc(sizeof(struct Account));
    new_node->acc->ID = strdup(id);
    new_node->acc->name = strdup(name);
    new_node->acc->username = strdup(username);
    new_node->acc->password = malloc(sizeof(struct PasswordData));
    new_node->acc->password->hash = strdup(hash_password);
    new_node->acc->password->salt = strdup(salt);
    new_node->acc->money = money;

    return new_node;
}

bool isEmptyLL(struct LinkedList* ll) {
    return ll->head == NULL;
}

void linkedlist_add(struct LinkedList* ll, const char* id, const char* name, const char* username, const char* hash_password, const char* salt, const double money) {
    struct LinkedListNode* new_node = make_new_node(id, name, username, hash_password, salt, money);
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

void hashtable_add(struct Hashtable* table, const char* id, const char* name, const char* username, const char* hash_password, const char *salt, const double money);

void transfer(struct Hashtable* source, struct Hashtable* destination) {
    for(int i = 0; i < source->num_of_buckets; i++) {
         struct LinkedListNode* tmp = source->buckets[i]->head;
         while(tmp != NULL) {
            hashtable_add(destination, tmp->acc->ID, tmp->acc->name, tmp->acc->username, tmp->acc->password->hash, tmp->acc->password->salt, tmp->acc->money);
            tmp = tmp->next;
         }
    }
}

struct Hashtable* hashtable_resize(struct Hashtable *table) {
    struct Hashtable* new_table = hashtable_init(table->num_of_buckets * 2, table->max_elements_per_bucket);
    transfer(table, new_table);

    return new_table;
}

void hashtable_add(struct Hashtable* table, const char* id, const char* name, const char* username, const char* hash_password, const char* salt, const double money) {
    int bucket_index = hash_password_for_hashtable(username) % table->num_of_buckets;
    struct LinkedList* ll = table->buckets[bucket_index];

    if (!linkedlist_contains(ll, username)) {
        linkedlist_add(ll, id, name, username, hash_password, salt, money);

        if (ll->count > table->max_elements_per_bucket) {
           printf("\ntime for resize!\n");
           struct Hashtable* new_table = hashtable_resize(table);
           *table = *new_table;
           for(int i = 0; i < new_table->num_of_buckets; i++) {
            struct LinkedListNode* tmp = new_table->buckets[i]->head;
                while(tmp != NULL) {
                    free(tmp->acc->password);
                    free(tmp->acc);
                    tmp = tmp->next;
                }
            }
            free(new_table);
        }
    }
}

char* generate(const int len) {
    char* str = malloc(sizeof(char) * (len + 1));
    char charBuff[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    srand(time(NULL));
    for (int i = 0; i < len; i++) {
        int index = rand() % (sizeof(charBuff) - 1);
        str[i] = charBuff[index];
    }
    str[len] = '\0';
    return str;
}

struct PasswordData* hash_new_password(const char* password) {
    struct PasswordData* data = malloc(sizeof(struct PasswordData));
    char* salt = generate(SALT_LENGTH);
    data->salt = strdup(salt);

    char* mix = mix_salt_password(salt, password);

    char* hash = hash_password_for_file(mix);
    data->hash = strdup(hash);

    free(mix);
    free(hash);
    free(salt);

    return data;
}

void get_line_curr(char* str, const int maxLength) {
    fgets(str, maxLength, stdin);
    str[strcspn(str, "\n")] = '\0';
}

void put_in_file(struct Account* acc, const char *filename) {
    FILE *f;
    f = fopen(filename, "a");
    fprintf(f, "%s\n", acc->ID);
    fprintf(f, "%s\n", acc->name);
    fprintf(f, "%s\n", acc->username);
    fprintf(f, "%s\n", acc->password->hash);
    fprintf(f, "%s\n", acc->password->salt);
    fprintf(f, "%.2f\n\n", acc->money);
    fclose(f);
}

char* get_name() {
    char* name = malloc(MAX_NAME_LENGTH * sizeof(char));
    printf(ANSI_COLOR_PINK);
    printf(ANSI_COLOR_BLACK "\nEnter name: ");
    fgets(name, MAX_NAME_LENGTH, stdin);

    name[strcspn(name, "\n")] = '\0';

    return name;
}

char* get_username() {
    char mail[MAX_MAIL_LENGTH];
    char* username = malloc(MAX_USERNAME_LENGTH * sizeof(char));
    printf(ANSI_COLOR_PINK);
    printf(ANSI_COLOR_BLACK "\nEnter email: ");
    fgets(mail, MAX_MAIL_LENGTH, stdin);
    for (int i = 0; i < strlen(mail); i++) {
        if (mail[i] == '@')
            break;
        username[i] = mail[i];
    }
    username[strlen(username)] = '\0';

    return username;
}

char* get_password() {
    char* password = malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    int index = 0;
    char ch;
    printf(ANSI_COLOR_PINK);
    printf("\nEnter password: ");
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

    return password;
}

bool is_there_such_a_user(struct Hashtable* table, const char* user) {
    int bucket_index = hash_password_for_hashtable(user) % table->num_of_buckets;
    struct LinkedList* ll = table->buckets[bucket_index];

    struct LinkedListNode* curr = ll->head;
    while(curr != NULL) {
        if (equals(user, curr->acc->username)) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

void register_user(struct Hashtable* table, const char* filename) {
    struct Account* account = malloc(sizeof(struct Account));
    account->ID = generate(ID_LENGTH);
    account->name = get_name();
    account->username = get_username();
    char* password = get_password();
    account->password = hash_new_password(password);
    account->money = 0;

    if(is_there_such_a_user(table, account->username))
    {
        printf("\nThere is already a user with this mail and username!\n");
        printf("Log in:\n");
    }
    else {
        put_in_file(account, filename);
        hashtable_add(table, account->ID, account->name, account->username, account->password->hash, account->password->salt, account->money);
        printf("\nYou registered succesfully! Your username is: %s\n", account->username);
        printf("Log in:\n");
    }
    free(account->password->hash);
    free(account->password->salt);
    free(account->password);
    free(account);
}

struct LinkedListNode* find_user(struct Hashtable* table, const char* user) {
    int bucket_index = hash_password_for_hashtable(user) % table->num_of_buckets;
    struct LinkedList* ll = table->buckets[bucket_index];

    struct LinkedListNode* curr = ll->head;
    while(curr != NULL) {
        if (equals(user, curr->acc->username)) {
            return curr;
        }
        curr = curr->next;
    }

    struct LinkedListNode *noUser = NULL;
    return noUser;
}

struct Account* login_user(struct Hashtable* table, const char *filename) {
    char user[MAX_USERNAME_LENGTH];
    printf(ANSI_COLOR_PINK);
    printf("Enter username:");
    scanf("%s", user);
    char *password = get_password(password);

    if(is_there_such_a_user(table, user)) {
        struct LinkedListNode *userNode = find_user(table, user);

        char *mix = mix_salt_password(userNode->acc->password->salt, password);

        char* hash = hash_password_for_file(mix);
        if(strcmp(hash, userNode->acc->password->hash) == 0) {
            return userNode->acc;
        }
        else {
            struct Account *noLoggedUser = NULL;
            return noLoggedUser;
    }
    }

    else{
        struct Account* noLoggedUser = NULL;
        return noLoggedUser;
    }
}

void put_hashtable_in_file(struct Hashtable* table, const char* filename) {
   FILE *f;
   f = fopen(filename, "w");
   fclose(f);
   for(int i = 0; i < table->num_of_buckets; i++) {
        struct LinkedListNode* tmp = table->buckets[i]->head;
        if(tmp != NULL) {
             while(tmp != NULL) {
                put_in_file(tmp->acc, filename);
                tmp = tmp->next;
             }
        }
    }
}

void deposit(struct Account* account){
    double depositMoney;
    do{
    printf(ANSI_COLOR_PINK);
    printf("Enter the amount of money you want to deposit (must be a positive number): ");
    scanf("%lf", &depositMoney);
    }while (depositMoney < 0);

    account->money += depositMoney;
    printf("The deposit was successful\n");

}

void withdraw(struct Account* account){
    double withdrawMoney;
    do{
        printf(ANSI_COLOR_PINK);
        printf("Enter the amount of money you want to withdraw (must be a positive number): ");
        scanf("%lf", &withdrawMoney);

        if(withdrawMoney > account->money){
            printf("The withdraw was not successful! You don't have this amount of money in your account!\n");
        }

    }while (withdrawMoney < 0 || withdrawMoney > account->money);

    account->money -= withdrawMoney;
    printf("The withdraw was successful!\n");
}


bool is_empty_transaction_list(struct TransactionQueue* q){
    return q->head == NULL;
}

void transaction(struct TransactionQueue* q, struct Account* account1, struct Account* account2){
    struct Transaction* newTransaction = calloc(1, sizeof(struct Transaction));
    newTransaction->fromAccount = account1;
    newTransaction->toAccount = account2;
    newTransaction->transactionCode = strdup("Transfer");
    newTransaction->next = NULL;

    do{
        printf("Enter the amount of money you want to transfer: ");
        scanf("%lf", &newTransaction->amountToTransfer);
    }while(newTransaction->amountToTransfer < 0);

    if(is_empty_transaction_list(q)){
        q->head = newTransaction;
        q->tail = newTransaction;
    }else{
        q->tail->next = newTransaction;
        q->tail = newTransaction;
    }
    FILE *f;
    f = fopen("transactions.txt", "a");
    fprintf(f, "%s\n", newTransaction->fromAccount->username);
    fprintf(f, "%s\n", newTransaction->toAccount->username);
    fprintf(f, "%.2f\n\n", newTransaction->amountToTransfer);
    fclose(f);
}

void popQueue(struct TransactionQueue* q){
    q->head->fromAccount->money -= q->head->amountToTransfer;
    q->head->toAccount->money += q->head->amountToTransfer;
    q->head = q->head->next;
}


void main_menu(struct Account* account, struct Hashtable* table, struct TransactionQueue* q) {
    int option;
    if (account == NULL) {
        return;
    }
    while (1) {
        system("cls");
        printf("\n\n\n\n\n\n\n\n\n\n");
        printf(ANSI_COLOR_BLUE);
        printf(ANSI_COLOR_RESET "\t\t                          BANKING SOFTWARE                    \n");
        printf(ANSI_COLOR_PINK);
        printf(ANSI_COLOR_BLACK "\t\t     1. Deposit money in your account                         \n");
        printf(ANSI_COLOR_BLACK "\t\t     2. Withdraw money from your account                      \n");
        printf(ANSI_COLOR_BLACK "\t\t     3. Transfer money from your account to another account   \n");
        printf(ANSI_COLOR_BLACK "\t\t     4. Log out from your account                             \n");
        printf(ANSI_COLOR_BLACK "\t\t     Enter operation:                                         ");
        printf(ANSI_COLOR_BLUE);
        scanf("%d", &option);
        getchar();

        switch (option) {
            case 1:
                system("cls");
                deposit(account);
                printf(ANSI_COLOR_BLUE);
                put_hashtable_in_file(table, "data.txt");
                break;
            case 2:
                system("cls");
                withdraw(account);
                printf(ANSI_COLOR_BLUE);
                put_hashtable_in_file(table, "data.txt");
                break;
            case 3: system("cls");
                    struct Account* account2 = calloc(1, sizeof(struct Account));
                    char* user;
                    printf("\n\t\t\tEnter the username of the person you want to transfer money to: ");
                    scanf("%s", user);
                    account2 = find_user(table, user);
                    if(account2 == NULL){
                        printf("There is no such user and the transaction is not successful!\n");
                        break;
                    }
                    transaction(q, account, account2);
                    put_hashtable_in_file(table, "data.txt");
                    break;
            case 4:
                cottonCollector(q);
                put_hashtable_in_file(table, "data.txt");
            case 5:
                return;
            default:
                break;
        }
    }
}

void register_login_menu(struct Hashtable* table, const char* filename, const char* filenameRes, struct TransactionQueue* q) {
    int option;

    while (1) {
        struct Account* loggedUser = calloc(1, sizeof(struct Account));
        system("cls");

        do {
            system("cls");
            printf(ANSI_COLOR_BLUE);
            printf("\n\n\n\n\n\n\n\n\n");
            printf(ANSI_COLOR_RESET "\t\t\t          BANKING SOFTWARE          \n");
            printf(ANSI_COLOR_PINK);
            printf(ANSI_COLOR_BLACK "\t\t\t          1. Register               \n");
            printf("\t\t\t          2. Log in                 \n");
            printf("\t\t\t          3. Close the software     \n");
            printf(ANSI_COLOR_BLACK);
            printf("\t\t\t          Enter option:             ");
            scanf("%d", &option);
            printf(ANSI_COLOR_RESET);
            getchar();
        } while (option <= 0 || option > 3);

        switch (option) {
            case 1:
                system("cls");
                printf(ANSI_COLOR_BLUE);
                register_user(table, filename);
                printf("\n");
                printf(ANSI_COLOR_BLUE);
                loggedUser = login_user(table, filename);
                break;

            case 2:
                system("cls");
                printf(ANSI_COLOR_BLUE);
                loggedUser = login_user(table, filename);
                break;

            case 3:
                put_hashtable_in_file(table, filenameRes);
                put_hashtable_in_file(table, filename);
                exit(0);

            default:
                break;
        }
        printf(ANSI_COLOR_BLUE);
        main_menu(loggedUser, table, q);
        loggedUser = NULL;
    }
}

void load_data_from_file(struct hashtable* table, const char* filename) {
    FILE *f;
    f = fopen(filename, "r");
    if (f == NULL) {
        f = fopen(filename, "a");
        fclose(f);
        f = fopen(filename, "r");
    }

    char line[MAX_LINE_LENGTH];
    char id[ID_LENGTH + 1];
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
            fprintf(stderr, "Error reading username from file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(name, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading username from file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(username, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading hash from file.\n");
            exit(1);
        }

        line[strcspn(line, "\n")] = '\0';
        strcpy(hash, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading salt from file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(salt, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading money from file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        money = atof(line);

        hashtable_add(table, id, name, username, hash, salt, money);
        fgets(line, MAX_LINE_LENGTH, f);
    }

    fclose(f);
}



struct TransactionQueue* init_transaction_queue(){
    struct TransactionQueue* q = calloc(1, sizeof(struct TransactionQueue));
    q->head = NULL;
    q->tail = NULL;
    return q;
}

void load_transactions_from_file(struct TransactionQueue* q, char* path, struct Hashtable* table){
    FILE *f;
    f = fopen(path, "r");

    if (f == NULL) {
        fprintf(stderr, "Error opening transaction file.\n");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    char username1[MAX_USERNAME_LENGTH];
    char username2[MAX_USERNAME_LENGTH];

    double money;

    while (true) {
    if (fgets(line, MAX_LINE_LENGTH, f) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';
        strcpy(username1, line);

        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading username for toAccount from file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(username2, line);


        if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
            fprintf(stderr, "Error reading money from file.\n");
            exit(1);
        }
        line[strcspn(line, "\n")] = '\0';
        money = atof(line);

       
        fgets(line, MAX_LINE_LENGTH, f);
        struct LinkedListNode* node1 = find_user(table, username1);
        struct LinkedListNode* node2 = find_user(table, username2);

        struct account* account1 = node1->acc;
        struct account* account2 = node2->acc;

        transaction(q, account1, account2);
        free(node1);
        free(node2);
    }

    fclose(f);
    
}

int hash_file(char* filePath){
    int h = 0;
    char line[MAX_LINE_LENGTH];
    
    FILE *f;
    f = fopen(filePath, "r");
    while(!feof(f)){
        if(fgets(line, MAX_LINE_LENGTH, f) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';
        
        for(size_t i = 0; i < strlen(line); ++i){
            h += i * line[i];
        }
    }

    return h;
}


bool compareFiles(char* filenameForBackupFile, char* filenameForMainFile){
    int tempFileHash = hash_file(filenameForBackupFile);
    int mainFileHash = hash_file(filenameForMainFile);

    return tempFileHash == mainFileHash;
}


int main()
{
    struct Hashtable *table = hashtable_init(32, 10);
    char* filenameForTempFile = "dataTmp.txt";
    char* filenameForResultFile = "data.txt";
    struct TransactionQueue* q = init_transaction_queue();
    load_data_from_file(table, filenameForResultFile);

    printf(ANSI_COLOR_BLUE);

    register_login_menu(table, filenameForTempFile, filenameForResultFile, q);

    return 0;
}
