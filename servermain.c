/*
  Author: Muqi Zhang
  Date: Dec 8 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>

// Define constants
#define MAIN_SERVER_UDP_PORT "34575"
#define MAIN_SERVER_TCP_PORT "35575"
#define MAXBUFLEN 1024
#define MAX_RETRIES 100
#define MAX_DEPARTMENTS 100
#define MAX_ROOMS 10
#define CAMPUS_SERVER 3

const char *CAMPUS_PORTS[] = {"31575", "32575", "33575"};
const char *CAMPUS_SERVERS[] = {"ServerA", "ServerB", "ServerC"};
char department[50];

void sigchld_handler(int s){
   int saved_errno = errno;
   while (waitpid(-1, NULL, WNOHANG) > 0);
   errno = saved_errno;
}

// Structure for room details
typedef struct {
    char roomType[2]; // Room type: S, D, T
    char buildingID[20]; // Building ID
    int availability; // Number of available slots
    int price; // Price of the room
} RoomInfo;

// Structure for department information
typedef struct {
    char department[50]; // Department name
    RoomInfo rooms[MAX_ROOMS]; // Array of rooms for this department
    int roomCount; // Number of rooms
    char server; // Server responsible: 'A', 'B', or 'C'
} DepartmentInfo;

DepartmentInfo departmentList[MAX_DEPARTMENTS];
int departmentCount = 0;

// Helper function to send a UDP message
void send_udp_message(const char *message, const char *port, const char *server) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    // Set up hints for address resolution
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    // Get address info for the server
    if ((rv = getaddrinfo("127.0.0.1", port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // Loop through all the results and connect to the first valid address
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("UDP socket creation failed");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to create socket\n");
        freeaddrinfo(servinfo);
        exit(1);
    }

    // Send the message to the server
    if (sendto(sockfd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen) == -1) {
        perror("sendto failed");
        close(sockfd);
        freeaddrinfo(servinfo);
        exit(1);
    }

    // Clean up
    freeaddrinfo(servinfo);
    close(sockfd);
}

// Function to parse and store the received department data
void parse_department_data(const char *data) {
    char *line = strdup(data); // Make a copy of the input data
    char *token = strtok(line, ","); // Tokenize by commas

    // The first token is the server identifier (e.g., "A", "B", "C")
    if (token == NULL) {
        fprintf(stderr, "Error: Invalid department data format.\n");
        free(line);
        return;
    }

    char server = token[0]; // Server identifier (e.g., 'A')

    // Parse the remaining department names
    while ((token = strtok(NULL, ",")) != NULL) {
        // Trim whitespace and newlines
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\n')) *end-- = '\0';

        // Add the department to the list
        strncpy(departmentList[departmentCount].department, token, sizeof(departmentList[departmentCount].department) - 1);
        departmentList[departmentCount].server = server;
        departmentList[departmentCount].roomCount = 0;
        departmentCount++;
    }

    free(line);
}

// Function to receive and process department data from campus servers
void receive_department_lists(const char *main_server_port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char buf[MAXBUFLEN];
    int numbytes;

    // Prepare hints for address resolution
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE;    // Bind to the local IP address

    if (getaddrinfo(NULL, main_server_port, &hints, &servinfo) != 0) {
        perror("getaddrinfo failed");
        exit(1);
    }

    // Create and bind a UDP socket
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

    printf("Main server is ready to receive department data on port %s\n", main_server_port);

    // Receive department data from campus servers
    for (int i = 0; i < CAMPUS_SERVER; ++i) {
        addr_len = sizeof their_addr;
        memset(buf, 0, MAXBUFLEN);

        // Receive message from a campus server
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom failed");
            close(sockfd);
            exit(1);
        }

        buf[numbytes] = '\0'; // Null-terminate the received string

        // Parse the data
        parse_department_data(buf);
    }

    printf("All department data has been received and stored.\n");

    // Close the socket
    close(sockfd);
}

// Encrypt a single character according to the rules
char encrypt_char(char c) {
    if (islower(c)) {
        return ((c - 'a' + 3) % 26) + 'a'; // Cyclically shift lowercase letters
    } else if (isupper(c)) {
        return ((c - 'A' + 3) % 26) + 'A'; // Cyclically shift uppercase letters
    } else if (isdigit(c)) {
        return ((c - '0' + 3) % 10) + '0'; // Cyclically shift digits
    } else {
        return c; // Leave special characters unchanged
    }
}

// Encrypt the entire string
void encrypt_string(const char *input, char *output) {
    int i = 0;
    while (input[i] != '\0') {
        output[i] = encrypt_char(input[i]);
        i++;
    }
    output[i] = '\0'; // Null-terminate the encrypted string
}

int is_valid_department(const char *department_name) {
    for (int i = 0; i < departmentCount; i++) {
        if (strcmp(departmentList[i].department, department_name) == 0) {
            return 1; // Department is valid
        }
    }
    return 0; // Department is invalid
}

// Function to authenticate client
int authenticate_client(int client_fd) {
    char buffer[MAXBUFLEN];
    char username[MAXBUFLEN];
    char password[MAXBUFLEN];
    FILE *file;
    char line[MAXBUFLEN];
    int authenticated = 0;
    int is_guest = 1; // Default to guest

    // Open the member.txt file that contains encrypted usernames and passwords
    file = fopen("member.txt", "r");
    if (file == NULL) {
        perror("Failed to open member.txt");
        return 0; // Fail if file cannot be opened
    }

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        // Clear the buffers
        memset(buffer, 0, MAXBUFLEN);
        memset(username, 0, MAXBUFLEN);
        memset(password, 0, MAXBUFLEN);
	memset(department, 0, sizeof(department));

        // Receive message from client
        if (recv(client_fd, buffer, MAXBUFLEN - 1, 0) <= 0) {
            perror("Failed to receive data");
            fclose(file);
            return 0;
        }
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

        // Parse the received message
        char *comma_pos = strchr(buffer, ','); // Find the comma separating username and password
        char *space_pos = strrchr(buffer, ' '); // Find the last space separating department

        if (space_pos == NULL) {
            // Invalid format, no department found
            send(client_fd, "Invalid message format.\n", 24, 0);
            printf("Invalid message format received: %s\n", buffer);
            continue;
        }

        // Extract department (always present)
        strcpy(department, space_pos + 1); // Everything after the last space is the department

        if (comma_pos != NULL && comma_pos < space_pos) {
            // Format: username,password department
            is_guest = 0; // It's a member login
            strncpy(username, buffer, comma_pos - buffer); // Extract username
            username[comma_pos - buffer] = '\0'; // Null-terminate username
            strncpy(password, comma_pos + 1, space_pos - comma_pos - 1); // Extract password
            password[space_pos - comma_pos - 1] = '\0'; // Null-terminate password
        } else {
            // Format: username department
            is_guest = 1; // It's a guest login
            strncpy(username, buffer, space_pos - buffer); // Extract username
            username[space_pos - buffer] = '\0'; // Null-terminate username
        }

        // Encrypt the username and password
        char encrypted_username[MAXBUFLEN];
        char encrypted_password[MAXBUFLEN];
        encrypt_string(username, encrypted_username);
        if (!is_guest) {
            encrypt_string(password, encrypted_password);
        }

        // Rewind the file for each attempt
        rewind(file);

        // Check credentials against the file
        while (fgets(line, sizeof(line), file) != NULL) {
            char file_username[MAXBUFLEN];
            char file_password[MAXBUFLEN];

            // Parse the line to extract the stored encrypted username and password
            sscanf(line, "%[^,],%s", file_username, file_password);

            // For guests, only match the username
            if (is_guest) {
                if (strcmp(encrypted_username, file_username) == 0) {
                    authenticated = 2;
                    break;
                }
            } else { // For members, match both username and password
                if (strcmp(encrypted_username, file_username) == 0 &&
                    strcmp(encrypted_password, file_password) == 0) {
                    authenticated = 1;
                    break;
                }
            }
        }


        // Check if the department is valid
        if (authenticated && !is_valid_department(department)) {
            send(client_fd, "Invalid department. Please try again.\n", 39, 0);
            printf("Invalid department: %s. Sent response to client.\n", department);
            authenticated = 0; // Reset authentication for invalid department
            continue;
        }

        if (authenticated) {
	    if(!is_guest){
                send(client_fd, "Member authentication passed\n", 22, 0);
                printf("The main server received the authentication for %s using TCP over port %s.\n", encrypted_username, MAIN_SERVER_TCP_PORT);
		printf("The authentication passed.\nThe main server sent the authentication result to the client.\n");
                fclose(file);
                return 1;
	    }
	    else{
		send(client_fd, "Guest authentication passed\n", 22, 0);
                printf("The main server received the authentication for %s using TCP over port %s.\n", encrypted_username, MAIN_SERVER_TCP_PORT);
		printf("The main server accepts %s as a guest.\nThe main server sent the guest response to the client.\n", encrypted_username);
		fclose(file);
		return 2;
	    }
//            fclose(file);
//            return 1;
        } else {
            send(client_fd, "Failed login. Invalid username or password.\n", 44, 0);
            if(!is_guest){
                printf("The main server received the authentication for %s using TCP over port %s.\n", encrypted_username, MAIN_SERVER_TCP_PORT);
		printf("The authentication failed.\nThe main server sent the authentication result to the client.\n");
	    }
        }
    }

    // If all attempts fail
    fclose(file);
    return 0;
}

// Helper function to find the campus server responsible for a department
char find_campus_server() {
    for (int i = 0; i < departmentCount; i++) {
        if (strcmp(departmentList[i].department, department) == 0) {
            return departmentList[i].server; // Return 'A', 'B', or 'C'
        }
    }
    return '\0'; // Return null character if the department doesn't exist
}

void send_responseToClient(int is_member, char *encrypted_username, char *depar, char *action, char campusID, char *response){
/*char temp[10];
if (is_member == 1) {
    strcpy(temp, "member");
} else {
    strcpy(temp, "guest");
}*/

//    if(is_member){
        printf("Main server has received the query from %s %s in department %s for the request of %s.\n",
               is_member? "member" : "guest", encrypted_username, depar, action);
        printf("The main server forwarded a request of %s to Server %c using UDP over port %s.\n", action, campusID, MAIN_SERVER_UDP_PORT);
        printf("The Main server has received result for the request of %s from Campus server %c using UDP over port %s.\n", action, campusID, MAIN_SERVER_UDP_PORT);
        printf("The Main server has sent back the result for the request of %s to the client %s %s using TCP over port %s.\n", action, is_member? "member" : "guest", encrypted_username, MAIN_SERVER_TCP_PORT);
/*    }
    else{
        printf("Main server has received the query from %s %s in department %s for the request of %s.\n",
               temp, encrypted_username, depar, action);
        printf("The main server forwarded a request of %s to Server %c using UDP over port %s.\n", action, campusID, MAIN_SERVER_UDP_PORT);
        printf("The Main server has received result for the request of %s from Campus server %c using UDP over port %s.\n", action, campusID, MAIN_SERVER_UDP_PORT);
        printf("The Main server has sent back the result for the request of %s to the client %s %s using TCP over port %s.\n", action, temp, encrypted_username, MAIN_SERVER_TCP_PORT);
    }*/
}

void handle_client_query(int client_fd, int is_member) {
    char buffer[MAXBUFLEN];
    char username[MAXBUFLEN];
    char encrypted_username[MAXBUFLEN];
    char depar[50];
    char room_type[5], action[20];
    char campusID;
    int building_id = -1;
    char response[MAXBUFLEN];
    int bytes_received;
    int sockfd;

    // Create a UDP socket for communication with campus servers
    /*int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        perror("UDP socket creation failed");
        close(client_fd);
        return;
    }*/

    while (1) {
        // Clear buffer and receive room type and action from client
        memset(buffer, 0, MAXBUFLEN);
        memset(username, 0, MAXBUFLEN);
        memset(encrypted_username, 0, MAXBUFLEN);
        memset(depar, 0, sizeof(depar));
        bytes_received = recv(client_fd, buffer, MAXBUFLEN - 1, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive room type and action");
            close(client_fd);
            //close(sockfd);
            return;
        }
        buffer[bytes_received] = '\0'; // Null-terminate the input
        sscanf(buffer, "%s %s %s %s %d", username, depar, room_type, action, &building_id); // Extract room type and action
	encrypt_string(username, encrypted_username);
	campusID = find_campus_server();

        // Prepare the query to send to the campus server
        snprintf(buffer, MAXBUFLEN, "%s %s %d%c", room_type, action, building_id, '\0');

	// Determine the port based on campusID
	int portIndex = campusID - 'A'; // Map 'A' to 0, 'B' to 1, 'C' to 2
	const char *campusPort = CAMPUS_PORTS[portIndex];

        // Resolve the address of the campus server
        struct addrinfo hints, *servinfo, *p;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;       // IPv4
        hints.ai_socktype = SOCK_DGRAM; // UDP socket
        //hints.ai_flags = AI_PASSIVE;    // Bind to the local IP address

        if (getaddrinfo("127.0.0.1", campusPort, &hints, &servinfo) != 0) {
            perror("getaddrinfo failed");
            close(client_fd);
            //close(udp_sockfd);
            return;
        }
	for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                perror("Socket creation failed");
                continue;
            }
            break;
        }

        // Send the query to the campus server
/*     for (p = servinfo; p != NULL; p = p->ai_next) {
            if (sendto(udp_sockfd, buffer, strlen(buffer), 0, p->ai_addr, p->ai_addrlen) < 0) {
                perror("sendto failed");
                continue;
            }
            break;
        }*/
        if (sendto(sockfd, buffer, strlen(buffer), 0, p->ai_addr, p->ai_addrlen) == -1) {
            perror("sendto failed from main to campus");
            close(sockfd);
            freeaddrinfo(servinfo);
            exit(1);
        }

        if (p == NULL) {
            fprintf(stderr, "Failed to send query to campus server\n");
            freeaddrinfo(servinfo);
            close(client_fd);
            close(sockfd);
            return;
        }

        freeaddrinfo(servinfo);

        // Wait for and receive the response from the campus server
        struct sockaddr_storage their_addr;
        socklen_t addr_len = sizeof their_addr;
        memset(response, 0, MAXBUFLEN);

        int numbytes = recvfrom(sockfd, response, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len);
        if (numbytes == -1) {
            perror("recvfrom failed");
            close(client_fd);
            close(sockfd);
            return;
        }
        response[numbytes] = '\0'; // Null-terminate the received string

//        printf("Received response from Campus Server: %s\n", response);

        // Send response to the client
	send_responseToClient(is_member, encrypted_username, depar, action, campusID, response);
        send(client_fd, response, strlen(response), 0);
    }

    // Close the UDP socket and client connection
    close(client_fd);
    close(sockfd);
}

// Main function
int main() {
    printf("Main server is up and running.\n");

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
       perror("sigaction");
       exit(1);
    }

    // Step 1: Send wake-up messages to campus servers
    for (int i = 0; i < CAMPUS_SERVER; i++) {
        send_udp_message("wake-up", CAMPUS_PORTS[i], CAMPUS_SERVERS[i]);
    }

    // Step 2: Receive department lists from campus servers
    receive_department_lists(MAIN_SERVER_UDP_PORT);
    printf("Server A\n");
    int is_first = 1; // Track the first item
    for (int i = 0; i < departmentCount; i++) {
        if (departmentList[i].server == 'A') {
            if (!is_first) printf(", ");
            printf("%s", departmentList[i].department);
            is_first = 0; // After the first department, don't prepend a comma
        }
    }
    printf("\n");

    printf("Server B\n");
    is_first = 1; // Reset the flag for server B
    for (int i = 0; i < departmentCount; i++) {
        if (departmentList[i].server == 'B') {
            if (!is_first) printf(", ");
            printf("%s", departmentList[i].department);
            is_first = 0;
        }
    }
    printf("\n");

    printf("Server C\n");
    is_first = 1; // Reset the flag for server C
    for (int i = 0; i < departmentCount; i++) {
        if (departmentList[i].server == 'C') {
            if (!is_first) printf(", ");
            printf("%s", departmentList[i].department);
            is_first = 0;
        }
    }
    printf("\n");

    // Step 3: Set up TCP socket to handle client connections
    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) {
        perror("TCP socket creation failed");
        exit(1);
    }

   int reuse = 1;
    if (setsockopt(tcp_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        exit(1);
    }

    // Bind and listen on the TCP socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(MAIN_SERVER_TCP_PORT));

    while (1) {
	int ret = 0;
    	ret = bind(tcp_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    	if (ret < 0) {
		if (errno == EADDRINUSE) {
			continue;
		}
        	perror("TCP bind failed");
        	exit(1);
    	}
	break;
    }

    if (listen(tcp_sockfd, 10) < 0) {
        perror("TCP listen failed");
        exit(1);
    }

    // Accept and handle client connections
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(tcp_sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("TCP accept failed");
            continue;
        }

        // Authenticate the client
/*        int is_member = authenticate_client(client_fd) == 1? 1 : 0;

        // Handle client queries
        handle_client_query(client_fd, is_member);*/
    if (!fork()) { // Child process
        close(tcp_sockfd); // Child does not need the listening socket
        int is_member = authenticate_client(client_fd) == 1? 1 : 0; // Authenticate client
        handle_client_query(client_fd, is_member); // Handle the client's queries
        close(client_fd); // Clean up
        exit(0); // Exit the child process
    }

    close(client_fd); // Parent closes client socket and continues
    }

    // Cleanup
    close(tcp_sockfd);
    return 0;
}
