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

//void specified_printout(char *response, char *action, char campusID, int building_id){
void specified_printout(char *response, char *action, int building_id){
	if(strstr(response, "Their Building IDs are")){
		printf("%s", response);
	}
	else if(strstr(response, "is not available")){
		if(strcmp(action, "reserve") == 0 && strstr(response, "room type") != 0){printf("Reservation failed: %s", response);}
		else{printf("The requested room is not available.\n");}
	}
	else if(strstr(response, "Room type")){
		if(strcmp(action, "reserve") == 0){printf("Reservation failed: Not able to find the room type.\n");}
		else{printf("Not able to find the room type.\n");}
	}
	else if(strstr(response, "Price $")){
		printf("%s", response);
	}
	else if(strstr(response, "Building ID") && strstr(response, "does not show up in")){
		printf("Reservation failed: %s", response);
	}
	else if(strstr(response, "successful")){
//		printf("Reservation is successful for Campus %c Building ID %d!\n", campusID, building_id);
		printf("%s", response);
	}
	else if(strstr(response, "does not exist")){
		printf("Reservation failed: %s", response);
	}
}

// Function to handle client-server communication
void communicate_with_server(int sockfd) {
    char buffer[MAXBUFLEN], username[50], password[50], department[50], room_type[5], action[20];
    int building_id = -11;
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

        if (strstr(buffer, "Failed")) {
            printf("Failed login. Please try again.\n");
            continue;
        } 
	else if(strstr(buffer, "Invalid department")){
	    printf("Invalid department. Please try again.\n");
	    continue;
	}
	else {
            printf("Welcome %s %s from %s!\n", (password[0] == '\0') ? "guest" : "member", username, department);
            authenticated = 1;
        }
    }

    // Phase 2: Query
    while (1) {
        printf("Please enter the room type S/D/T: ");
        scanf("%s", room_type);
        getchar(); // Clear buffer

        // Clear 'action' for each iteration
        memset(action, 0, sizeof(action)); 

        if (password[0] == '\0') {
            strcpy(action, "availability");
        } else {
            /*printf("Please enter request action (availability, price, reserve): ");
            scanf("%s", action);
            getchar(); */// Clear buffer
	    while(strcmp(action, "availability") != 0 && strcmp(action, "price") != 0 && strcmp(action, "reserve") != 0){
		printf("Please enter request action (availability, price, reserve): ");
                scanf("%s", action);
		getchar(); // Clear buffer
	    }
        }

	if(strcmp(action, "reserve") == 0){
	    printf("Please enter buildingID: ");
	    scanf("%d", &building_id);
	    getchar();
	}

        snprintf(buffer, sizeof(buffer), "%s %s %s %s %d", username, department, room_type, action, building_id);
        send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive response from server");
            continue;
        }
        buffer[bytes_received] = '\0';
//        printf("Response from server:\n%s\n", buffer);


        if(strcmp(action, "availability") == 0){
            printf("%s sent a request of Availability for type %s to the main server.\nThe client received the response from the main server using TCP over port 35575.\n", username, room_type);
        }
	else if(strcmp(action, "price") == 0){
	    printf("%s sent a request of Availability for type %s to the main server.\nThe client received the response from the main server using TCP over port 35575.\n", username, room_type);
	}
	else if(strcmp(action, "reserve") == 0){
	    printf("%s sent a request of Availability for type %s and Building ID %d to the main server.\nThe client received the response from the main server using TCP over port 35575.\n", username, room_type, building_id);
	}

	//specified_printout(response, action, campusID, building_id);
	specified_printout(buffer, action, building_id);

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

