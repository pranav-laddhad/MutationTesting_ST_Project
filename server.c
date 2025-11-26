//*******SERVER CODE*******
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int id;
    char title[50];
    char author[50];
    int is_rented;
} Book;

typedef struct
{
    int id;
    int rented_book_id;
} Member;

typedef struct
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} user_credentials;

typedef struct
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} admin_credentials;

void *handle_client(void *client_socket);
void register_member(int client_socket, int id, int rent_id);
void rent_book(int client_socket);
void return_book(int client_socket);
void add_book(int client_socket);
void delete_book(int client_socket);
void modify_book(int client_socket);
void search_book(int client_socket);
void number_of_rented_books(/*int client_socket,*/ int ptr, int member_id);

// Function to authenticate
int authenticate(int client_socket)
{
    char buffer[1024];
    int role;

    recv(client_socket, &role, sizeof(int), 0);

    if (role == 1)
    {
        // user_credentials user;
        user_credentials valid_user = {"user", "user"};

        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0)
        {
            perror("Error receiving user credentials from client");
            return 0;
        }

        char username[50];
        char password[50];
        int rent_id;
        int member_id;
        sscanf(buffer, "%s %s %d", username, password, &member_id);

        Member new_entry;
        new_entry.id = member_id;

        if (strcmp(username, valid_user.username) == 0 && strcmp(password, valid_user.password) == 0)
        {
            printf("Logged in Succesfully!\n");
            send(client_socket, "Logged in Succesfully", strlen("Logged in Succesfully"), 0); // send-1

            recv(client_socket, &rent_id, sizeof(rent_id), 0);

            register_member(client_socket, member_id, rent_id);

            return 1;
        }
        else
        {
            printf("Authentication failed for user: %s\n", username);
            send(client_socket, "Authentication failed!", strlen("Authentication failed!"), 0);
            return 0;
        }
    }
    else if (role == 2)
    {
        // admin_credentials valid_user = {"admin", "admin"};

        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0)
        {
            perror("Error receiving user credentials from client");
            return 0;
        }
        char username[50];
        char password[50];
        sscanf(buffer, "%s %s", username, password);

        if (strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0)
        {
            printf("Logged in Succesfully!\n");
            send(client_socket, "Logged in Succesfully", strlen("Logged in Succesfully"), 0); // send-1
            return 1;
        }
        else
        {
            printf("Authentication failed for user: %s\n", username);
            send(client_socket, "Authentication failed!", strlen("Authentication failed!"), 0);
            return 0;
        }
    }
    return 0;
}

void *handle_client(void *client_socket)
{
    int sock = *(int *)client_socket;
    // char buffer[BUFFER_SIZE];
    int role;
    int choice;
    while (1)
    {
        read(sock, &role, sizeof(role));

        if (role == 1)
        {
            // User menu
            read(sock, &choice, sizeof(choice));

            switch (choice)
            {
            case 1:
                rent_book(sock);
                break;
            case 2:
                return_book(sock);
                break;
            case 3:
                search_book(sock);
                break;

            case 4:
                write(sock, "Exiting", strlen("Exiting"));
                close(sock);
                free(client_socket);
            default:
                write(sock, "Invalid Choice", strlen("Invalid Choice"));
                break;
            }
        }
        else if (role == 2)
        {
            // Admin menu
            read(sock, &choice, sizeof(choice));

            switch (choice)
            {
            case 1:
                add_book(sock);
                break;
            case 2:
                delete_book(sock);
                break;
            case 3:
                modify_book(sock);
                break;
            case 4:
                search_book(sock);
                break;
            case 5:
                write(sock, "Exiting", strlen("Exiting"));
                close(sock);
                free(client_socket);
                return NULL;
            default:
                write(sock, "Invalid Choice", strlen("Invalid Choice"));
                break;
            }
        }
        else
        {
            write(sock, "Invalid login option", strlen("Invalid login option"));
        }
    }
}

int get_next_id(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return 1;

    int id = 0;
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, file))
    {
        int current_id;
        sscanf(buffer, "%d", &current_id);
        if (current_id > id)
        {
            id = current_id;
        }
    }

    fclose(file);

    // Original Code (must be changed to create the mutant)
    return id + 1;
    
    // MUTANT CODE: Change '+' to '-'
    // return id - 1;
}

void register_member(int client_socket, int id, int rent_id)
{
    pthread_mutex_lock(&file_mutex);
    int fd = open("members.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    Member member;
    member.id = id;
    member.rented_book_id = rent_id;

    char buffer[BUFFER_SIZE];

    dprintf(fd, "%d  %d\n", member.id, member.rented_book_id);

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);

    sprintf(buffer, "Member with registered ID '%d' logged in succesfully", member.id);
    write(client_socket, buffer, strlen(buffer));
}

//ADD BOOK 
void add_book(int client_socket)
{
    pthread_mutex_lock(&file_mutex); // Lock the mutex before file operations) (ORIGINAL CODE)

    // Lock Deletion Mutant: Commenting out the mutex lock (MUTANT CODE))

    int fd = open("books.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    Book book;
    book.id = get_next_id("books.txt");

    char buffer[BUFFER_SIZE];
    read(client_socket, book.title, sizeof(book.title));
    read(client_socket, book.author, sizeof(book.author));

    book.is_rented = 0;

    dprintf(fd, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);

    sprintf(buffer, "Book added with ID: %d", book.id);
    write(client_socket, buffer, strlen(buffer));
}

//DELETE BOOK
void delete_book(int client_socket)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int book_id;
    char buffer[BUFFER_SIZE];
    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%d", &book_id);

    FILE *file = fdopen(fd, "r+");
    char temp_filename[] = "books_temp.txt";
    FILE *temp_file = fopen(temp_filename, "w");


    int found = 0;
    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id)
        {
            found = 1;
            continue;
        }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    fclose(file);
    fclose(temp_file);

    if (found)
    {
        rename(temp_filename, "books.txt");
        sprintf(buffer, "Book with ID %d has been deleted", book_id);
    }
    else
    {
        remove(temp_filename);
        sprintf(buffer, "Book with ID %d not found", book_id);
    }

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);

    write(client_socket, buffer, strlen(buffer));
}


//MODIFY BOOK
void modify_book(int client_socket)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    // char title[50];
    // char author[50];
    int book_id;
    char buffer[BUFFER_SIZE];

    FILE *file = fdopen(fd, "r+");
    char temp_filename[] = "books_temp.txt";
    FILE *temp_file = fopen(temp_filename, "w");

    int found = 0;
    Book new_book;
    read(client_socket, &book_id, sizeof(book_id));
    new_book.id = book_id;

    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%s %s", new_book.title, new_book.author);

    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id)
        {
            new_book.is_rented = book.is_rented;
            book = new_book;
            book.id = book_id;
            found = 1;
        }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    fclose(file);
    fclose(temp_file);

    if (found)
    {
        rename(temp_filename, "books.txt");
        sprintf(buffer, "Book with ID %d has been modified", book_id);
    }
    else
    {
        remove(temp_filename);
        sprintf(buffer, "Book with ID %d not found", book_id);
    }

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);

    write(client_socket, buffer, strlen(buffer));
}


//SEARCH BOOK
void search_book(int client_socket)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_SH) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int book_id;
    char buffer[BUFFER_SIZE];
    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%d", &book_id);

    FILE *file = fdopen(fd, "r");

    if (!file)
    {
        perror("Error opening file stream");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int found = 0;
    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id)
        {
            sprintf(buffer, "ID: %d, Title: %s, Author: %s, Rented: %d", book.id, book.title, book.author, book.is_rented);
            found = 1;
            break;
        }
    }

    if (!found)
    {
        sprintf(buffer, "Book with ID %d not found", book_id);
    }

    fclose(file);
    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);

    write(client_socket, buffer, strlen(buffer));
}

//Rented Books
void number_of_rented_books(/*int client_socket,*/ int ptr, int member_id)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("members.txt", O_RDWR);
    flock(fd, LOCK_EX);
    FILE *filee = fdopen(fd, "r+");

    char buffer[BUFFER_SIZE];
    char temp2_filename[] = "members_temp2.txt";
    FILE *temp2_file = fopen(temp2_filename, "w");

    int found = 0;
    while (fgets(buffer, BUFFER_SIZE, filee))
    {
        Member member;
        sscanf(buffer, "%d %d", &member.id, &member.rented_book_id);
        if (member.id == member_id)
        {
            if (ptr == 1)
            {
                found = 1;
                member.rented_book_id++;
            }
            else if (ptr == 0)
            {
                found = 1;
                member.rented_book_id--;
            }
        }

        fprintf(temp2_file, "%d %d\n", member.id, member.rented_book_id);
    }

    fclose(filee);
    fclose(temp2_file);

    if (found)
    {
        rename(temp2_filename, "members.txt");
    }
    else
    {
        remove(temp2_filename);
    }

    flock(fd, LOCK_UN);
    pthread_mutex_unlock(&file_mutex);
}


//RENT A BOOK
void rent_book(int client_socket)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int book_id;
    char buffer[BUFFER_SIZE];
    read(client_socket, &book_id, sizeof(book_id));
  

    FILE *file = fdopen(fd, "r+");
    char temp_filename[] = "books_temp.txt";
    FILE *temp_file = fopen(temp_filename, "w");

    int found = 0;

    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id && book.is_rented == 0)
        {
            book.is_rented = 1;
            found = 1;
        }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    fclose(file);
    fclose(temp_file);

    if (found)
    {
        rename(temp_filename, "books.txt");
        sprintf(buffer, "Book with ID %d has been rented", book_id);
    }
    else
    {
        remove(temp_filename);
        sprintf(buffer, "Book with ID %d not found or already rented", book_id);
    }

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);
    write(client_socket, buffer, strlen(buffer));

    // if (found==1)
    //  number_of_rented_books(client_socket,1,member_id);
}

//RETURN BOOK
void return_book(int client_socket)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int book_id;
    char buffer[BUFFER_SIZE];
    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%d", &book_id);

    FILE *file = fdopen(fd, "r+");
    char temp_filename[] = "books_temp.txt";
    FILE *temp_file = fopen(temp_filename, "w");

    int found = 0;

    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id && book.is_rented == 1)
        {
            book.is_rented = 0;
            found = 1;
        }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    fclose(file);
    fclose(temp_file);

    if (found)
    {
        rename(temp_filename, "books.txt");
        sprintf(buffer, "Book with ID %d has been returned", book_id);
    }
    else
    {
        remove(temp_filename);
        sprintf(buffer, "Book with ID %d not found or not rented", book_id);
    }

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);
    write(client_socket, buffer, strlen(buffer));

    //  if (found == 1)
    // number_of_rented_books(client_socket,0,member_id);
}







// Add this wrapper function to server.c (e.g., just before main)
// Purpose: Enables unit testing of file deletion logic by bypassing socket reads.
void delete_book_wrapper(int book_id)
{
    // Start critical section - Acquire pthread mutex lock (Lock 1)
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening file");
        // Must unlock mutex before exiting on failure
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    // Acquire advisory file lock (Lock 2) - Guarantees exclusive access to the file resource
    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd); // Close descriptor
        pthread_mutex_unlock(&file_mutex); // Unlock mutex
        return;
    }

    // Associate file descriptor with a C stream for fgets/fprintf
    FILE *file = fdopen(fd, "r+");
    // Ensure file stream was created successfully before proceeding
    if (file == NULL) {
        perror("Error creating file stream");
        flock(fd, LOCK_UN); // Release file lock if fdopen fails
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    char temp_filename[] = "books_temp.txt";
    FILE *temp_file = fopen(temp_filename, "w");
    // Handle error if temp file cannot be created
    if (temp_file == NULL) {
        perror("Error creating temporary file");
        fclose(file);
        flock(fd, LOCK_UN);
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int found = 0;
    char buffer[BUFFER_SIZE]; // Buffer for reading file lines

    // Read every line from books.txt and write non-matching lines to temp file
    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        // NOTE: Use %49s to prevent buffer overflow in sscanf for title/author
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id)
        {
            found = 1; // Mark as found, but skip writing this line
            continue;
        }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    // Close both file streams before attempting rename/remove
    fclose(file);
    fclose(temp_file);

    if (found)
    {
        // Successful deletion: Replace original file with temporary file
        rename(temp_filename, "books.txt");
    }
    else
    {
        // Book not found: Delete the unnecessary temporary file
        remove(temp_filename);
        
        // This is the VRR MUTATION TARGET: If activated, remove("books.txt") would be executed here.
        // remove("books.txt"); 
    }

    // Final cleanup: Release locks and resources in reverse order of acquisition
    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);
}



// TEST WRAPPER: Modified to take ID as argument instead of reading from socket.
// Logic copied from the original rent_book function.
void rent_book_wrapper(int book_id)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    FILE *file = fdopen(fd, "r+");
    char temp_filename[] = "books_temp_rent.txt"; // Use a unique temp file name
    FILE *temp_file = fopen(temp_filename, "w");

    int found = 0;
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id && book.is_rented == 0) // <--- THIS IS THE COR TARGET LINE (ORGINAL CODE)

        // MUTANT CODE: Change '&&' to '||'
        // if (book.id == book_id || book.is_rented == 0)
        {
            book.is_rented = 1;
            found = 1;
        }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    fclose(file);
    fclose(temp_file);

    if (found)
    {
        rename(temp_filename, "books.txt");
    }
    else
    {
        remove(temp_filename);
    }

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);

}


// server.c: Add this wrapper function

void return_book_wrapper(int book_id)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    // No socket read needed for book_id

    FILE *file = fdopen(fd, "r+");
    char temp_filename[] = "books_temp_return.txt"; // Unique temp file
    FILE *temp_file = fopen(temp_filename, "w");

    int found = 0;
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id && book.is_rented == 1) // <--- ROR TARGET LINE (ORIGINAL CODE)

        // MUTANT CODE: Change '== 1' to '!= 1'
        // if (book.id == book_id && book.is_rented != 1)
        {
            book.is_rented = 0; // Set to unrented
            found = 1;
        }
        // {
        //     book.is_rented = 0; // Set to unrented
        //     found = 1;
        // }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    fclose(file);
    fclose(temp_file);

    if (found)
    {
        rename(temp_filename, "books.txt");
    }
    else
    {
        remove(temp_filename);
    }

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);
}


// server.c: Helper function for CUnit testing ROR mutant
int check_admin_credentials_test(const char *username, const char *password)
{
    // ROR TARGET LINE  (ORIGINAL CODE)
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0)

    // MUTANT CODE: Change '== 0' to '!= 0'
    // if (strcmp(username, "admin") != 0 && strcmp(password, "admin")==0)
    {
        return 1; // Success
    }
    return 0; // Failure
}


// server.c: Add this wrapper function
void modify_book_wrapper(int book_id, const char *new_title, const char *new_author)
{
    pthread_mutex_lock(&file_mutex);

    int fd = open("books.txt", O_RDWR);
    // ... (Error checks for fd and flock(LOCK_EX) are the same as delete/rent wrappers)
    // [Skipping boilerplate for brevity]

    FILE *file = fdopen(fd, "r+");
    char temp_filename[] = "books_temp_modify.txt";
    FILE *temp_file = fopen(temp_filename, "w");

    int found = 0;
    Book new_book;
    new_book.id = book_id;
    // Copy new title/author into new_book structure
    strncpy(new_book.title, new_title, 49);
    strncpy(new_book.author, new_author, 49);

    char buffer[BUFFER_SIZE];
    
    while (fgets(buffer, BUFFER_SIZE, file))
    {
        Book book;
        sscanf(buffer, "%d %49s %49s %d", &book.id, book.title, book.author, &book.is_rented);

        if (book.id == book_id)
        {
            // SDL TARGET LINE: This line preserves the status. (ORIGINAL CODE)

            new_book.is_rented = book.is_rented; 
            
            // MUTANT CODE: SDL MUTANT APPLIED: DELETED (just comment the above line)

            book = new_book;
            book.id = book_id;
            found = 1;
        }

        fprintf(temp_file, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);
    }

    // ... (File close, rename/remove logic, and unlock/close fd are the same)
    // [Skipping boilerplate for brevity]
    flock(fd, LOCK_UN); 
    close(fd);
    pthread_mutex_unlock(&file_mutex);
}


// Logic copied from the original add_book function.
void add_book_wrapper(const char *title, const char *author)
{
    pthread_mutex_lock(&file_mutex);

    // INTEGRATION TARGET: O_WRONLY | O_APPEND | O_CREAT, 0644
    int fd = open("books.txt", O_WRONLY | O_APPEND | O_CREAT, 0644); 

    // int fd = open("books.txt", O_WRONLY | O_APPEND | O_CREAT, 0000); // MUTANT CODE: SCPR Applied

    // MUTANT CODE: System Call Replacement (SCR) - Remove O_CREAT flag.
    // int fd = open("books.txt", O_WRONLY | O_APPEND, 0644);


    if (fd < 0)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    if (flock(fd, LOCK_EX) < 0)
    {
        perror("Error locking file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    Book book;
    book.id = get_next_id("books.txt");

    // Copy data into book structure
    strncpy(book.title, title, 49);
    strncpy(book.author, author, 49);

    book.is_rented = 0;

    dprintf(fd, "%d %s %s %d\n", book.id, book.title, book.author, book.is_rented);

    flock(fd, LOCK_UN);
    close(fd);
    pthread_mutex_unlock(&file_mutex);
}





// int main()

// Modified line for testing:
int server_main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    pthread_t tid[MAX_CLIENTS];
    int thread_count = 0;

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind 
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen 
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Listening... \n" );

    while (1)
    {
        addr_len = sizeof(client_addr);
        //Accept connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            continue; 
        }

       printf("Connection Accepted\n");

        // Authenticate the client
        authenticate(client_socket);

        int *client_sock = malloc(sizeof(int));
        if (client_sock == NULL)
        {
            perror("Malloc failed");
            close(client_socket);
            continue;
        }
        *client_sock = client_socket;

        if (pthread_create(&tid[thread_count], NULL, handle_client, (void *)client_sock) != 0)
        {
            perror("Thread creation failed");
            close(client_socket);
            free(client_sock);
            continue;
        }
        pthread_detach(tid[thread_count]);
        thread_count = (thread_count + 1) % MAX_CLIENTS;
    }

    close(server_socket);
    return 0;
}
