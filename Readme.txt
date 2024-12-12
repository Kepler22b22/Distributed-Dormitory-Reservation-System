Name: Muqi Zhang
Date: Dec 11 2024
Student ID: 6788977575

SUCCESSFULLY RUN IN Ubuntu 22.04!
Note:
Sometimes the UDP is not that reliable, maybe try not to boot-up main server too fast after booting up campus. Or maybe need to boot up more than once(shouldn't be more than 3 times). 
I guess the reason for the above two senario is UDP is not reliable, and I didn't write all the check conditions?

Reference: 
   Used Beej’s Guide as a starting point, and also modified and merged my previous code from term project 1 and 2.

Work that I have done: 
   Implement client.c, servermain.c, serverA/B/C.c file. 
   Write a Makefile to make the .c file run automatically by typing "make"/"make all" to the keyboard. 
   Typing "make clean" to clean all files other than *.c files.
   There are 7 files in the tar.gz: client.c, servermain.c, serverA.c, serverB.c, serverC.c, Readme.txt, Makefile.
  
   Bootup Sequence: serverA/B/B --> main server --> client

   (The following instruction's sequence is not the bootup sequence.)
   1) Within the client.c file:
      Can handle 2 users. 
      Use 35575 as the TCP port that communicates with main server. When it boots up successfully, 
      it ask the client to type username and password and department name. 
      Then it sends all these 3 messages to main server to verify. If one of the info is wrong, the user needs to retype all three again, with infinite chances to login. 
      There are two different types for user: member or guest. If you skip the password, then the program assumes you are a guest.

      When all the verification success, the screen asks the user to input the room type and action.
      (Note: if you login as a guest, the program will set the action as availability automatically!)
      (Note: if your action is "reserve", you will also see a line to ask you to input building ID.)
      (Note: if your action is invaild, the program infinitlly asks you to try again.)
      Then the client program sends the message to main server and wait for the response. 

      When the program receives the corresponding message, it will show corresponding printout on the scree. 
      And you can now start a new request. 

      The program can run infinite requests, you can kill it mannually by typing "ctrl+c".

   2) Within the servermain.c file: 
      After the main server is up and running, it should recieve the department names from 3 campus servers, store them and print them out. 
      Then it goes to an infinite loop to do the query search: 
      a.If the username/password/department are/is wrong, it sends the corresponding back to client, and wait for the next message from client. 
      b. When the client successfully login, it parses the room type and action and/or building id and send this to the corresponding campus server, 
         and wait for its response. When it receives the response, it prints the corresponding prompts on the screen, 
	 sends the message back to client, and wait for another request from client.

   3) Within the serverA/B/C.c file: 
      Each campus server reads the .txt file first and store the info. When the main server is up, it will send each one a wake-up
      message, so that each campus server can send the department info to it. Then during the query in main server, it will send 
      requests to a specific campus server and get the correct feedback.
      (Note: if the room type is invaild at all, then it shows "Room type <input room type> does not show up in Server <A/B/C>")
      (Note: if the room type is valid but the availability is 0, it shows "This room is not available.")

