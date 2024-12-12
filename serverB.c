/*
  Author: Muqi Zhang
  Date: Dec 8 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <netdb.h> // Required for addrinfo, getaddrinfo, freeaddrinfo
#include <errno.h> // Required for errno handling and error codes

#define MAX_LINE 1024
#define MAX_DEPARTMENTS 10
#define CAMPUS_SERVER_PORT "32575"
#define MAIN_SERVER_PORT "34575"
#define MAXBUFLEN 1024


// Structure to store room details
typedef struct Room {
    char type[2];          // Room type: S, D, or T
    int building_id;       // Building ID
    int availability;      // Available slots
    int price;             // Room price
} Room;

// Structure to store department and room information
typedef struct Department {
    char name[20];         // Department name
    Room rooms[MAX_LINE];  // Array of rooms
    int room_count;        // Number of rooms
} Department;

Department departments[MAX_DEPARTMENTS];
int department_count = 0;

// Function to parse the data file
void parseData(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];
    fgets(line, sizeof(line), file); // Read the first line (department names)
    char* token = strtok(line, ",");
    while (token) {
        strcpy(departments[department_count].name, token);
        departments[department_count].room_count = 0;
        department_count++;
        token = strtok(NULL, ",");
    }

    while (fgets(line, sizeof(line), file)) {
        Room room;
        sscanf(line, "%1s,%d,%d,%d", room.type, &room.building_id, &room.availability, &room.price);
        for (int i = 0; i < department_count; i++) {
            departments[i].rooms[departments[i].room_count++] = room;
        }
    }

    fclose(file);
}

// Function to send department list to the Main Server
void sendDepartmentList(const char *main_server_ip) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM;  // UDP socket

    if ((rv = getaddrinfo(main_server_ip, MAIN_SERVER_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Socket creation failed");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to create socket\n");
        freeaddrinfo(servinfo);
        exit(1);
    }

    // Create the department list message
    char message[MAX_LINE] = "B"; // Start with server identifier 'B'
    for (int i = 0; i < department_count; i++) {
        strcat(message, ",");
        strcat(message, departments[i].name);
    }

    // Send the message
    if (sendto(sockfd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen) == -1) {
        perror("sendto failed");
        close(sockfd);
        freeaddrinfo(servinfo);
        exit(1);
    }

    printf("Server B has sent the department list to the Main Server\n");

    close(sockfd);
    freeaddrinfo(servinfo);
}


// Helper function to handle queries
void handleAvailability(const char *room_type, char* response) {
    int total_available = 0, found = 0;
    char available_buildings[100] = {0};

    for (int j = 0; j < departments[0].room_count; j++) {
        Room room = departments[0].rooms[j];
	if(!found && strcmp(room.type, room_type) == 0){found = 1;}
        if (strcmp(room.type, room_type) == 0 && room.availability > 0) {
            total_available += room.availability;
            char temp[50];
            sprintf(temp, "%d,", room.building_id);
            strcat(available_buildings, temp);
        }
    }

    if(!found){
	printf("Room type %s does not available in Server B.\n", room_type);
        printf("Server B has sent the results to Main Server\n");
        snprintf(response, MAX_LINE, "The client received the response from the main server using TCP over port 35575.\nNot able to find Room type.\n");
	return;
    }

    // Remove the trailing comma if there are any buildings added
    if (strlen(available_buildings) > 0) {
        available_buildings[strlen(available_buildings) - 1] = '\0';
    }

    if (total_available > 0) {
	printf("Server B found totally %d available rooms for %s type dormitory in Building: %s\n", total_available, room_type, available_buildings);
	printf("Server B has sent the results to Main Server\n");
        snprintf(response, MAX_LINE, "Campus B found %d available rooms in %s type dormitory. Their Building IDs are: %s\n",
                 total_available, room_type, available_buildings);
    } else {
	printf("Room %s is not available in Server B.\n", room_type);
        printf("Server B has sent the results to Main Server\n");
        snprintf(response, MAX_LINE, "The client received the response from the main server using TCP over port 35575.\nThe requested room is not available.\n");
    }
}

// Helper function to sort rooms by price
void sortRoomsByPrice(Room* rooms, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (rooms[j].price > rooms[j + 1].price) {
                Room temp = rooms[j];
                rooms[j] = rooms[j + 1];
                rooms[j + 1] = temp;
            }
        }
    }
}

// Function to handle price queries
void handlePrice(const char *room_type, char* response) {
//    char room_type_str[2];
//    snprintf(room_type_str, sizeof(room_type_str), "%c", room_type);
    Room available_rooms[MAX_LINE];
    int room_count = 0, found = 0;

    for (int i = 0; i < departments[0].room_count; i++) {
        Room room = departments[0].rooms[i];
        if(!found && strcmp(room.type, room_type) == 0){found = 1;}
        if (strcmp(room.type, room_type) == 0 && room.availability > 0) {
            available_rooms[room_count++] = room;
        }
    }

    if(!found){
        printf("Room type %s does not available in Server B.\n", room_type);
        printf("Server B has sent the results to Main Server\n");
        snprintf(response, MAX_LINE, "The client received the response from the main server using TCP over port 35575.\nNot able to find Room type.\n");
        return;
    }

    if (room_count > 0) {
        sortRoomsByPrice(available_rooms, room_count);
        snprintf(response, MAX_LINE, "Server B found room type %s with prices:\n", room_type);
        printf("Server B found room type %s with prices:\n", room_type);
        for (int i = 0; i < room_count; i++) {
            char temp[50];
            sprintf(temp, "Building ID %d, Price $%d\n", available_rooms[i].building_id, available_rooms[i].price);
	    printf("Building ID %d, Price $%d\n", available_rooms[i].building_id, available_rooms[i].price);
            strcat(response, temp);
        }
	printf("Server B has sent the results to Main Server\n");
    } else {
	printf("Room type %s is not available in Server B.\n", room_type);
        printf("Server B has sent the results to Main Server\n");
        snprintf(response, MAX_LINE, "Room type %s is not available in Server B.\n", room_type);
    }
}

// Function to handle reservation queries
void handleReservation(const char *room_type, int building_id, char* response) {
    bool room_found = false;
    bool building_found = false;

    for (int i = 0; i < department_count; i++) {
        for (int j = 0; j < departments[i].room_count; j++) {
            Room* room = &departments[i].rooms[j];
            if (strcmp(room->type, room_type) == 0) {
                room_found = true;
                if (room->building_id == building_id) {
                    building_found = true;
                    if (room->availability > 0) {
                        room->availability--;
                        snprintf(response, MAX_LINE,
                                 "Reservation is successful for Campus B Building ID %d!\n", building_id);
			printf("Server B found room type %s in Building ID %d.\nThis room is reserved, and availability is updated to %d.\n",
                                 room_type, building_id, room->availability);
		        printf("Server B has sent the results to Main Server\n");
                    } else {
                        snprintf(response, MAX_LINE,
                                 "Building ID %d room type %s is not available.\n",
                                 building_id, room_type);
			printf("Server B found room type %s in Building ID %d.\nThis room is not available.",
                                 room_type, building_id);
                        printf("Server B has sent the results to Main Server\n");
                    }
                    return; // Exit once the reservation is processed
                }
            }
        }
    }

    if (!room_found) {
	printf("Room type %s does not show up in Server B.\n", room_type);
        printf("Server B has sent the results to Main Server\n");
        snprintf(response, MAX_LINE, "Not able to find Room type.\n");
    } else if (!building_found) {
	printf("Building ID %d does not show up in Server B.\n", building_id);
        printf("Server B has sent the results to Main Server\n");
        snprintf(response, MAX_LINE, "Building ID %d does not exist.\n", building_id);
    }
}

// Function to handle queries from Main Server
void handleQueries(const char* server_ip) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXBUFLEN];
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM;  // UDP socket
    hints.ai_flags = AI_PASSIVE;     // Use my IP

    if (getaddrinfo(NULL, CAMPUS_SERVER_PORT, &hints, &servinfo) != 0) {
        perror("getaddrinfo failed");
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Socket creation failed");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Bind failed");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to bind socket\n");
        freeaddrinfo(servinfo);
        exit(1);
    }

    freeaddrinfo(servinfo);
 
    printf("Server B is up and running using UDP on port %s.\n", CAMPUS_SERVER_PORT);

    // Wait for wake-up message from the main server
    memset(buf, 0, sizeof(buf));
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom failed");
        close(sockfd);
        exit(1);
    }

    buf[numbytes] = '\0';
    if (strcmp(buf, "wake-up") == 0) {
        sendDepartmentList(server_ip);
    }

    while (1) {
        memset(buf, 0, sizeof(buf));
        if ((numbytes = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom failed");
            continue; // Do not exit; continue receiving queries
        }
	char buffer[19];
	memcpy(buffer, buf, 18);
	buffer[18] = '\0';

/*        char query_type[100];
	char room_type;
        int building_id = -11;*/
char room_type[100];    // For the first string (e.g., "Deluxe")
char query_type[100];       // For the second string (e.g., "Add")
int building_id = -11;  // For the integer (e.g., 123)

//printf("HERE!!!%c\n", buffer[strlen(buffer)-2]);
/*	room_type = buffer[0];
	if (strstr(buffer, "avail") != NULL) {
		strcpy(query_type, "availability");
	} else if (strstr(buffer, "pri") != NULL) {
		strcpy(query_type, "price");
	} else if (strstr(buffer, "res") != NULL) {
		strcpy(query_type, "reserve");
	}
	building_id = atoi(&buffer[strlen(buffer)-1]);
	if (buffer[strlen(buffer)-3] == '-') {
		building_id *= -1;
	}*/
	
//        sscanf(buffer, "%c %s %d", &query_type, room_type, &building_id);
sscanf(buffer, "%99s %99s %d", room_type, query_type, &building_id);

        char response[MAX_LINE] = {0};
        if (strcmp(query_type, "availability") == 0) {
            printf("Server B has received a query of Availability for room type %s.\n", room_type);
            handleAvailability(room_type, response);
        } else if (strcmp(query_type, "price") == 0) {
            printf("Server B has received a query of Price for room type %s.\n", room_type);
            handlePrice(room_type, response); 
        } else if (strcmp(query_type, "reserve") == 0) {
            printf("Server B has received a query of Reserve for room type %s at Building ID %d.\n", room_type, building_id);
	    handleReservation(room_type, building_id, response);
        }

        if (sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&their_addr, addr_len) == -1) {
            perror("sendto failed");
        }
    }

    close(sockfd);
}

// Main function
int main() {
    const char* filename = "dataB.txt";
    const char* server_ip = "127.0.0.1";

    parseData(filename);
    handleQueries(server_ip);

    return 0;
}
