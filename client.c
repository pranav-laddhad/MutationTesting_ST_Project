//*******CLIENT CODE*******
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void user_menu(int sock);
void admin_menu(int sock);


int authenticate(int sock, int role) {
    char buffer[BUFFER_SIZE];

    if (role != 1 && role != 2) {
        printf("Invalid role. Authentication failed.\n");
        return 0;
    }

    if (role == 1) {
        char username[50];
        char password[50];
        int rented_book_id=0;
        int member_id;
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter Member ID: ");
        scanf("%d", &member_id);
        printf("Enter password: ");
        scanf("%s", password);

        sprintf(buffer, "%s %s %d", username, password,member_id);
        send(sock, buffer, sizeof(buffer), 0);
        send(sock,&rented_book_id,sizeof(rented_book_id),0);
        recv(sock,buffer,sizeof(buffer),0);
        printf("%s\n", buffer);


    } else if (role == 2) {
        char username[50];
        char password[50];

        printf("Enter admin username: ");
        scanf("%s", username);
        printf("Enter admin password: ");
        scanf("%s", password);

        sprintf(buffer, "%s %s", username, password);
        send(sock, buffer, sizeof(buffer), 0);
    }

    recv(sock, buffer, sizeof(buffer), 0);

    if (strcmp(buffer, "Authentication failed!") == 0) {
        printf("Authentication failed. Exiting...\n");
        return 0;
    }

    return 1;
}


//USER MENU
void user_menu(int sock)
{
    int role = 1;
    int choice;
    char buffer[BUFFER_SIZE] = {0};

    while (1)
    {
        printf("\nUSER MENU\n");
        printf("1. Rent Book\n");
        printf("2. Return Book\n");
        printf("3.search a book\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        send(sock, &role, sizeof(role), 0);
        send(sock, &choice, sizeof(choice), 0);

        switch (choice)
        {
        case 1:
        {
            int id;
            printf("Enter book ID to rent: ");
            scanf("%d", &id);
            send(sock, &id , sizeof(id), 0);

            break;
        }
        case 2:
        {
            int id;
            printf("Enter book ID to return: ");
            scanf("%d", &id);
            send(sock, &id, sizeof(id), 0);

            break;
        }
        case 3:
        {
            int id;
            printf("Enter book ID to search: ");
            scanf("%d", &id);
            sprintf(buffer, "%d", id);
            send(sock, buffer, strlen(buffer), 0);
            break;
        }
        
        case 4:
            printf("Exiting...\n");
            return;
        default:
            printf("Invalid Choice\n");
            break;
        }

        read(sock, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);
    }
}

//ADMIN MENU
void admin_menu(int sock)
{
    int role = 2;
    int choice;
    char buffer[BUFFER_SIZE] = {0};

    while (1)
    {
        printf("\nADMIN MENU\n");
        printf("1. Add Book\n");
        printf("2. Delete Book\n");
        printf("3. Modify Book\n");
        printf("4. Search Book by ID\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        send(sock, &role, sizeof(role), 0);
        send(sock, &choice, sizeof(choice), 0);

        switch (choice)
        {
        case 1:
        {
            char title[50];
            char author[50];
            printf("Enter title of the book: ");
            scanf("%s", title);
            printf("Enter author of the book: ");
            scanf("%s", author);
            send(sock, title, sizeof(title), 0);
            send(sock, author, sizeof(author), 0);
            break;
        }
        case 2:
        {
            int id;
            printf("Enter book ID to delete: ");
            scanf("%d", &id);
            sprintf(buffer, "%d", id);
            send(sock, buffer, strlen(buffer), 0);
            break;
        }
        case 3:
        {
            int id;
            char title[50];
            char author[50];
            printf("Enter book ID to modify: ");
            scanf("%d", &id);
            send(sock,&id, sizeof(id), 0);
            printf("Enter new title ");
            scanf("%s",title);
            printf("Enter new author: ");
            scanf("%s",author);
            sprintf(buffer, "%s %s", title,author);
            send(sock,buffer,sizeof(buffer),0);
         
            break;
        }
        case 4:
        {
            int id;
            printf("Enter book ID to search: ");
            scanf("%d", &id);
            sprintf(buffer, "%d", id);
            send(sock, buffer, strlen(buffer), 0);
            break;
        }
        case 5:
            printf("Exiting...\n");
            return;
        default:
            printf("Invalid Choice\n");
            break;
        }

        read(sock, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);
    }
}

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char username[50];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("Connection Accepted\n\n");

    int role;

    //LOGIN MENU
    printf("WELCOME TO LIBRARY MANAGEMENT SYSTEM\n\n");
    printf("Choose option:\n");
    printf("1. Login as User\n");
    printf("2. Login as Admin\n");
    scanf("%d", &role);

    send(sock, &role, sizeof(role), 0);


    if (authenticate(sock, role))
    {
        printf("Authentication successful!\n");

        if (role == 1)
        {
            user_menu(sock);
        }
        else if (role == 2)
        {
            admin_menu(sock);
        }
        else
        {
            printf("Invalid option\n");
        }
    }

    close(sock);
    return 0;
}
