#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_PRODUCTS 100
#define MAX_USERS 100
#define MAX_LOGS 1000
#define NAME_LENGTH 50
#define PHONE_LENGTH 15
#define HASH_LENGTH 64
#define LOG_LENGTH 256

typedef struct {
    int id;
    char username[NAME_LENGTH];
    char phone[PHONE_LENGTH];
    char password_hash[HASH_LENGTH];
    char role[NAME_LENGTH];
} User;

typedef struct {
    int id;
    char name[NAME_LENGTH];
    int quantity;
    float price;
} Product;

typedef struct {
    int id;
    time_t timestamp;
    char username[NAME_LENGTH];
    char action[LOG_LENGTH];
} Log;

Product products[MAX_PRODUCTS];
User users[MAX_USERS];
Log logs[MAX_LOGS];
int product_count = 0;
int user_count = 0;
int log_count = 0;
User* current_user = NULL;

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    printf("Created by erisrtg\n\n");
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

bool validate_string(const char* str, size_t max_length) {
    if (str == NULL || strlen(str) == 0 || strlen(str) >= max_length) {
        return false;
    }
    return true;
}

bool validate_phone(const char* phone) {
    if (!validate_string(phone, PHONE_LENGTH)) {
        return false;
    }
    for (int i = 0; phone[i] != '\0'; i++) {
        if (!isdigit(phone[i]) && phone[i] != '+') {
            return false;
        }
    }
    return true;
}

void hash_password(const char* password, char* hash) {
    if (password == NULL || hash == NULL) {
        return;
    }

    unsigned long hash_value = 5381;
    int i;

    for (i = 0; password[i] != '\0'; i++) {
        hash_value = ((hash_value << 5) + hash_value) + password[i];
    }

    snprintf(hash, HASH_LENGTH, "%lx", hash_value);
}

bool save_to_file(const void* data, size_t size, size_t count, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        return false;
    }

    size_t written = fwrite(data, size, count, file);
    fclose(file);

    return written == count;
}

bool load_from_file(void* data, size_t size, size_t* count, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        *count = 0;
        return false;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    size_t max_items = file_size / size;
    size_t items_read = fread(data, size, max_items, file);
    *count = items_read;

    fclose(file);
    return true;
}

bool read_string(char* buffer, size_t size, const char* prompt) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        return false;
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }

    return true;
}

void add_log(const char* action) {
    if (current_user == NULL || log_count >= MAX_LOGS || action == NULL) {
        return;
    }

    Log new_log;
    new_log.id = log_count + 1;
    new_log.timestamp = time(NULL);
    strncpy(new_log.username, current_user->username, NAME_LENGTH - 1);
    new_log.username[NAME_LENGTH - 1] = '\0';
    strncpy(new_log.action, action, LOG_LENGTH - 1);
    new_log.action[LOG_LENGTH - 1] = '\0';

    logs[log_count++] = new_log;
    save_to_file(logs, sizeof(Log), log_count, "logs.dat");
}

bool register_user() {
    if (user_count >= MAX_USERS) {
        printf("Maximum user limit reached.\n");
        return false;
    }

    User new_user;
    char password[NAME_LENGTH];
    char role[NAME_LENGTH];

    clear_input_buffer();

    if (!read_string(new_user.username, NAME_LENGTH, "Enter username: ") ||
        !validate_string(new_user.username, NAME_LENGTH)) {
        printf("Invalid username.\n");
        return false;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, new_user.username) == 0) {
            printf("Username already exists.\n");
            return false;
        }
    }

    if (!read_string(new_user.phone, PHONE_LENGTH, "Enter phone number: ") ||
        !validate_phone(new_user.phone)) {
        printf("Invalid phone number.\n");
        return false;
    }

    if (!read_string(password, NAME_LENGTH, "Enter password: ") ||
        !validate_string(password, NAME_LENGTH)) {
        printf("Invalid password.\n");
        return false;
    }

    if (!read_string(role, NAME_LENGTH, "Enter role (admin/user): ") ||
        !validate_string(role, NAME_LENGTH)) {
        printf("Invalid role.\n");
        return false;
    }

    new_user.id = user_count + 1;
    hash_password(password, new_user.password_hash);
    strncpy(new_user.role, role, NAME_LENGTH - 1);
    new_user.role[NAME_LENGTH - 1] = '\0';

    users[user_count++] = new_user;
    save_to_file(users, sizeof(User), user_count, "users.dat");

    printf("Registration successful!\n");
    return true;
}

bool login() {
    char username[NAME_LENGTH];
    char password[NAME_LENGTH];
    char hash[HASH_LENGTH];

    clear_input_buffer();

    if (!read_string(username, NAME_LENGTH, "Enter username: ") ||
        !validate_string(username, NAME_LENGTH)) {
        printf("Invalid username.\n");
        return false;
    }

    if (!read_string(password, NAME_LENGTH, "Enter password: ") ||
        !validate_string(password, NAME_LENGTH)) {
        printf("Invalid password.\n");
        return false;
    }

    hash_password(password, hash);

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password_hash, hash) == 0) {
            current_user = &users[i];
            add_log("User logged in");
            printf("Login successful!\n");
            return true;
        }
    }

    printf("Invalid username or password.\n");
    return false;
}

void load_users() {
    size_t loaded_count;
    if (!load_from_file(users, sizeof(User), &loaded_count, "users.dat")) {
        user_count = 0;
    } else {
        user_count = loaded_count;
    }
}

void load_logs() {
    size_t loaded_count;
    if (!load_from_file(logs, sizeof(Log), &loaded_count, "logs.dat")) {
        log_count = 0;
    } else {
        log_count = loaded_count;
    }
}

void view_logs() {
    if (log_count == 0) {
        printf("No logs found.\n");
        return;
    }

    printf("\n--- Activity Logs ---\n");
    for (int i = 0; i < log_count; i++) {
        char* time_str = ctime(&logs[i].timestamp);
        if (time_str != NULL) {
            time_str[strlen(time_str) - 1] = '\0';
            printf("[%s] %s: %s\n", time_str, logs[i].username, logs[i].action);
        }
    }
}

void add_product() {
    if (product_count >= MAX_PRODUCTS) {
        printf("Inventory is full! Cannot add more products.\n");
        return;
    }

    Product new_product;

    clear_input_buffer();

    if (!read_string(new_product.name, NAME_LENGTH, "Enter product name: ") ||
        !validate_string(new_product.name, NAME_LENGTH)) {
        printf("Invalid product name.\n");
        return;
    }

    printf("Enter product quantity: ");
    if (scanf("%d", &new_product.quantity) != 1 || new_product.quantity < 0) {
        printf("Invalid quantity.\n");
        clear_input_buffer();
        return;
    }

    printf("Enter product price: ");
    if (scanf("%f", &new_product.price) != 1 || new_product.price < 0) {
        printf("Invalid price.\n");
        clear_input_buffer();
        return;
    }

    new_product.id = product_count + 1;
    products[product_count++] = new_product;

    save_to_file(products, sizeof(Product), product_count, "inventory.dat");
    printf("Product added successfully!\n");
}

void view_products() {
    if (product_count == 0) {
        printf("No products found in inventory.\n");
        return;
    }

    printf("\n--- Inventory ---\n");
    printf("ID\tName\t\tQuantity\tPrice\n");
    printf("-----------------------------------------\n");
    for (int i = 0; i < product_count; i++) {
        printf("%d\t%-16s%d\t\t$%.2f\n",
               products[i].id,
               products[i].name,
               products[i].quantity,
               products[i].price);
    }
    printf("-----------------------------------------\n");
}

void update_product_quantity() {
    int id, new_quantity;

    view_products();

    printf("Enter product ID to update: ");
    if (scanf("%d", &id) != 1 || id < 1 || id > product_count) {
        printf("Invalid product ID.\n");
        clear_input_buffer();
        return;
    }

    printf("Enter new quantity: ");
    if (scanf("%d", &new_quantity) != 1 || new_quantity < 0) {
        printf("Invalid quantity.\n");
        clear_input_buffer();
        return;
    }

    products[id - 1].quantity = new_quantity;
    save_to_file(products, sizeof(Product), product_count, "inventory.dat");
    printf("Product quantity updated successfully!\n");
}

void delete_product() {
    int id;

    view_products();

    printf("Enter product ID to delete: ");
    if (scanf("%d", &id) != 1 || id < 1 || id > product_count) {
        printf("Invalid product ID.\n");
        clear_input_buffer();
        return;
    }

    for (int i = id - 1; i < product_count - 1; i++) {
        products[i] = products[i + 1];
        products[i].id = i + 1;
    }

    product_count--;
    save_to_file(products, sizeof(Product), product_count, "inventory.dat");
    printf("Product deleted successfully!\n");
}

void view_users() {
    if (current_user == NULL || strcmp(current_user->role, "admin") != 0) {
        printf("Access denied. You must be an admin to view users.\n");
        return;
    }

    if (user_count == 0) {
        printf("No users found.\n");
        return;
    }

    printf("\n--- User List ---\n");
    printf("ID\tUsername\tPhone\t\tRole\n");
    printf("-----------------------------------------\n");
    for (int i = 0; i < user_count; i++) {
        printf("%d\t%-16s%s\t%s\n",
               users[i].id,
               users[i].username,
               users[i].phone,
               users[i].role);
    }
    printf("-----------------------------------------\n");
}

int main() {
    size_t loaded_count;

    load_users();
    load_logs();

    if (!load_from_file(products, sizeof(Product), &loaded_count, "inventory.dat")) {
        product_count = 0;
    } else {
        product_count = loaded_count;
    }

    int choice;
    bool logged_in = false;

    do {
        clear_screen();
        if (!logged_in) {
            printf("\n=== Inventory Management System ===\n");
            printf("1. Login\n");
            printf("2. Register\n");
            printf("3. Exit\n");
            printf("Choose an option: ");

            if (scanf("%d", &choice) != 1) {
                clear_input_buffer();
                continue;
            }

            switch (choice) {
                case 1:
                    logged_in = login();
                    break;
                case 2:
                    register_user();
                    break;
                case 3:
                    printf("Exiting...\n");
                    return 0;
                default:
                    printf("Invalid option.\n");
            }
        } else {
            printf("\n=== Inventory Management System ===\n");
            printf("Logged in as: %s (Role: %s)\n", current_user->username, current_user->role);
            printf("1. Add Product\n");
            printf("2. View Products\n");
            printf("3. Update Product Quantity\n");
            printf("4. Delete Product\n");
            printf("5. View Logs\n");
            if (strcmp(current_user->role, "admin") == 0) {
                printf("6. View Users\n");
            }
            printf("7. Logout\n");
            printf("8. Exit\n");
            printf("Choose an option: ");

            if (scanf("%d", &choice) != 1) {
                clear_input_buffer();
                continue;
            }

            switch (choice) {
                case 1:
                    add_product();
                    add_log("Added a new product");
                    break;
                case 2:
                    view_products();
                    add_log("Viewed products");
                    break;
                case 3:
                    update_product_quantity();
                    add_log("Updated product quantity");
                    break;
                case 4:
                    delete_product();
                    add_log("Deleted a product");
                    break;
                case 5:
                    view_logs();
                    break;
                case 6:
                    if (strcmp(current_user->role, "admin") == 0) {
                        view_users();
                    } else {
                        printf("Invalid option.\n");
                    }
                    break;
                case 7:
                    add_log("User logged out");
                    current_user = NULL;
                    logged_in = false;
                    printf("Logged out successfully!\n");
                    break;
                case 8:
                    save_to_file(products, sizeof(Product), product_count, "inventory.dat");
                    printf("Exiting...\n");
                    return 0;
                default:
                    printf("Invalid option.\n");
            }
        }

        printf("\nPress Enter to continue...");
        clear_input_buffer();
        getchar();

    } while (true);

    return 0;
}
