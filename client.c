/*
  Author: Muqi Zhang
  Date: Dec 8 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXBUFLEN 1024
#define SERVER_TCP_PORT 35575 // Replace with the correct port from your setup
#define LOCALHOST "127.0.0.1"

// Function to handle client-server communication
/*void communicate_with_server(int sockfd) {
    char buffer[MAXBUFLEN], username[50], password[50], department[50], room_type[5], action[20];
    int authenticated = 0;

    // Phase 1: Login
    while (!authenticated) {
        while (1) {
            printf("Please enter username: ");
            scanf("%s", username);
            getchar(); // Clear buffer

            printf("Please enter password (Enter to skip for guests): ");
            fgets(password, sizeof(password), stdin);
            if (password[0] == '\n') {
                password[0] = '\0'; // Guest user
            } else {
                password[strcspn(password, "\n")] = '\0';
            }

            printf("Please enter department name: ");
            scanf("%s", department);
            getchar();

            snprintf(buffer, sizeof(buffer), "%s%s%s %s", username, (password[0] != '\0') ? "," : "", password, department);
            send(sockfd, buffer, strlen(buffer), 0);
            printf("%s sent an authentication request to the main server.\n", username);

            // Receive authentication result
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                perror("Failed to receive authentication response");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            buffer[bytes_received] = '\0';

            if (strstr(buffer, "Invalid department")) {
                printf("Invalid department. Please try again.\n");
                continue;
            } else if (strstr(buffer, "Failed")) {
                printf("Failed login. Invalid username or password. Please try again.\n");
                continue;
            } else {
                printf("Welcome %s %s from %s!\n", (password[0] == '\0') ? "guest" : "member", username, department);
                authenticated = 1;
                break;
            }
        }
    }

    // Phase 2: Query
    while (1) {
        printf("Please enter the room type S/D/T: ");
        scanf("%s", room_type);
        getchar(); // Clear buffer

        // Enforce guest action restriction
        if (authenticated == 1 && password[0] == '\0') { // Guest check
            strcpy(action, "availability");
        } else {
            printf("Please enter request action (availability, price, reserve): ");
            scanf("%s", action);
            getchar(); // Clear buffer
        }

        // Send query to the server
        snprintf(buffer, sizeof(buffer), "%s %s %s", room_type, action, department);
        send(sockfd, buffer, strlen(buffer), 0);
        printf("%s sent a request of %s for type %s to the main server.\n", username, action, room_type);

        // Handle reservation action (only applicable for members)
        if (strcmp(action, "reserve") == 0) {
            char building_id[20];
            printf("Please enter Building ID for reservation: ");
            scanf("%s", building_id);
            getchar(); // Clear buffer
            snprintf(buffer, sizeof(buffer), "%s %s %s %s", department, room_type, action, building_id);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        // Receive and handle the server's response
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive response from server");
            continue; // Retry on failure
        }
        buffer[bytes_received] = '\0';

        printf("HERE!!! %s\n", buffer); // Debug print to confirm response

        // Parse and display multiple responses
        char *token = strtok(buffer, "\n");
        while (token != NULL) {
            printf("The client received the response from the main server:\n%s\n", token);
            token = strtok(NULL, "\n");
        }

        printf("-----Start a new request-----\n");
    }
}*/
// Function to handle client-server communication
void communicate_with_server(int sockfd) {
    char buffer[MAXBUFLEN], username[50], password[50], department[50], room_type[5], action[20];
    int authenticated = 0;

    // Phase 1: Login
    while (!authenticated) {
        printf("Please enter username: ");
        scanf("%s", username);
        getchar(); // Clear buffer

        printf("Please enter password (Enter to skip for guests): ");
        fgets(password, sizeof(password), stdin);
        if (password[0] == '\n') {
            password[0] = '\0'; // Guest user
        } else {
            password[strcspn(password, "\n")] = '\0';
        }

        printf("Please enter department name: ");
        scanf("%s", department);
        getchar();

        snprintf(buffer, sizeof(buffer), "%s%s%s %s", username, (password[0] != '\0') ? "," : "", password, department);
        send(sockfd, buffer, strlen(buffer), 0);

        // Receive authentication result
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive authentication response");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        buffer[bytes_received] = '\0';

        if (strstr(buffer, "Failed")) {
            printf("Failed login. Please try again.\n");
            continue;
        } else {
            printf("Welcome %s %s from %s!\n", (password[0] == '\0') ? "guest" : "member", username, department);
            authenticated = 1;
        }
    }

    // Phase 2: Query
    while (1) {
        printf("Please enter the room type S/D/T: ");
        scanf("%s", room_type);
        getchar(); // Clear buffer

        if (password[0] == '\0') {
            strcpy(action, "availability");
        } else {
            printf("Please enter request action (availability, price, reserve): ");
            scanf("%s", action);
            getchar(); // Clear buffer
        }

        snprintf(buffer, sizeof(buffer), "%s %s %s", room_type, action, department);
        send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive response from server");
            continue;
        }
        buffer[bytes_received] = '\0';
        printf("Response from server:\n%s\n", buffer);

        printf("-----Start a new request-----\n");
    }
}


int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_TCP_PORT);
    inet_pton(AF_INET, LOCALHOST, &server_addr.sin_addr);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection to server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client is up and running.\n");

    // Communicate with server
    communicate_with_server(sockfd);

    // Close socket
    close(sockfd);

    return 0;
}

