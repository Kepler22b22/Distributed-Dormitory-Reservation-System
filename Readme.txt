# Student Dormitory Reservation System - EE450 Socket Programming Project
# Name: Muqi Zhang
# Date: Dec 11 2024

* Overview

This repository contains the implementation of a Student Dormitory Reservation System using UNIX socket programming. The project is part of the EE450 Computer Networks course (Fall 2024, Part 3) and demonstrates inter-process communication (IPC) using TCP and UDP sockets.

The system is designed to efficiently manage student housing reservations across multiple campuses using a client-server architecture. It enables students to search for room availability, check prices, and make reservations through a structured network of servers.

* Architecture

The project consists of six main components:

1. Client: Handles user interaction, allowing students to log in as a Member or Guest to search for dormitory availability or make reservations.
2. Main Server: Authenticates users and acts as a central hub, forwarding client requests to the correct Campus Server.
3. Campus Servers (A, B, C): Store dormitory data for different campuses and process queries related to room availability, pricing, and reservations.
4. Data Files: Store encrypted user login credentials (login.txt) and dormitory details (dataA.txt, dataB.txt, dataC.txt).

* Features

✅ TCP and UDP Socket Communication

✅ User Authentication with Encrypted Credentials

✅ Multi-threaded or Multi-process Architecture

✅ Search for Room Availability by Type (Single, Double, Triple)

✅ Sort Rooms by Price (Members Only)

✅ Reserve a Room (Members Only)

✅ Dynamic Handling of Requests and Responses

* Communication Flow

(1) Clients communicate with the Main Server via TCP.
(2) The Main Server interacts with Campus Servers using UDP.
(3) Campus Servers respond with dormitory availability and reservation updates.

* How to Run

1. Compile the project using the provided Makefile:

   make all

2. Start the servers in this order (each in a separate terminal):

   ./serverA

   ./serverB

   ./serverC

   ./servermain

3. Run the clients:

   ./client

4. Follow the on-screen instructions to log in and interact with the system.

* File Features: 

📂 EE450_Project

 ├── client.cpp            # Client program


 ├── servermain.cpp        # Main Server handling authentication and coordination

 ├── serverA.cpp           # Campus Server A

 ├── serverB.cpp           # Campus Server B

 ├── serverC.cpp           # Campus Server C

 ├── login.txt             # Encrypted login credentials

 ├── dataA.txt             # Dormitory data for Campus A

 ├── dataB.txt             # Dormitory data for Campus B

 ├── dataC.txt             # Dormitory data for Campus C

 ├── Makefile              # Compilation instructions

 ├── README.md             # Project documentation

* Technologies Used

(1) C/C++ (for socket programming)

(2) UNIX Sockets (TCP, UDP)

(3) Multi-threading / Process Forking (for handling concurrent requests)

(4) Encryption & Decryption (for user authentication)

(5) Data Structures (Maps, Lists for managing dormitory records)

* Acknowledgment

This project is based on the EE450 Computer Networks course assignment at USC. Some concepts were inspired by Beej's Guide to Network Programming.