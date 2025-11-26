This project utilizes socket programming and system calls to implement a Library Management System. It employs a client-server model, where the server handles requests from multiple clients simultaneously. The system incorporates distinct user roles, enabling administrators to manage library resources efficiently while providing users with seamless access to book rental and return services.
Concurrency is achieved through pthreads, allowing efficient management of library resources.


Steps to Run:
1. Compile the server code (gcc server.c -o server) and client code (gcc client.c -o client).
2. Run the server executable (./server) on a terminal.
3. Run the client executable (./client) on another terminal.
4. Follow the prompts to log in as a user or admin and perform library management actions.


