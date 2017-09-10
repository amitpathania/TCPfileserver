/*
FILE NAME: server1.cpp
CREATED BY:163054001 AMIT PATHANIA
			15305r007 Nivia Jatain
This files creates a central server. Server takes one commandline input arguments ie own listning port.
Clients will use Server IP and port to connect this server for fetching files.

*/
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <thread>
#include <cstdlib>
#include <string>
#include <fcntl.h> 
#include <unistd.h> 
#include <future>
using namespace std;

#define ERROR     -1  // defines error message
#define MAX_CLIENTS    10//defines maximum number of clients that can connect simultaneously
#define MAX_BUFFER   512 //used to set max size of buffer for send recieve data 

// Defining Constant Variables for database connections
//#define AUTH_SERVER_IP "127.0.0.1"
//#define AUTH_SERVER_PORT 20000

char AUTH_SERVER_PORT[MAX_BUFFER]; // authentication server port
char AUTH_SERVER_IP[MAX_BUFFER]; // auth server IP address


time_t current_time; // variable to store current time

void *client_handler(void *);


int connect_to_auth_server(char*,char*,char*); // function to connect to authentication server
//string get_event(string,string);
int upload_file(int , char*);




int main(int argc, char **argv)  
{
	int sock; // sock is socket desriptor for server 
	int new1; // socket descriptor for new client
	struct sockaddr_in server; //server structure 
	struct sockaddr_in client; //structure for server to bind to particular machine
	socklen_t sockaddr_len=sizeof (struct sockaddr_in);	//stores length of socket address
	int *new_sock;

	int pid;// to manage child process

	if (argc < 2)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, no listening port provided\n");
		exit(1);
	}


	printf("Enter Authentication server IP   ");
	scanf(" %[^\t\n]s",AUTH_SERVER_IP);

	printf("Enter Authentication server listening port  ");
	scanf(" %[^\t\n]s",AUTH_SERVER_PORT);


	/*get socket descriptor */
	if ((sock= socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{ 
		perror("server socket error: ");  // error checking the socket
		exit(-1);  
	} 
	 
	/*server structure */ 
	server.sin_family = AF_INET; // protocol family
	server.sin_port =htons(atoi(argv[1])); // Port No and htons to convert from host to network byte order. atoi to convert asci to integer
	server.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY means server will bind to all netwrok interfaces on machine for given port no
	bzero(&server.sin_zero, 8); //padding zeros


	// set SO_REUSEADDR on a socket to true (1):
	// kill "Address already in use" error message
	int opt;
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
	}
	
	
	/*binding the socket */
	if((bind(sock, (struct sockaddr *)&server, sockaddr_len)) == ERROR) //pointer casted to sockaddr*
	{
		perror("bind");
		exit(-1);
	}

	printf("SERVER STARTED...... \n ");
	/*listen the incoming connections */
	if((listen(sock, MAX_CLIENTS)) == ERROR) // listen for max connections
	{
		perror("listen");
		exit(-1);
	}

	pthread_t thread_id;

	while(new1 = accept(sock, (struct sockaddr *)&client, &sockaddr_len))
	{
		printf("New client connection accepted \n");
		

		pthread_t sniffer_thread;
        new_sock = new int(1);
        *new_sock = new1;
         
        if( pthread_create( &thread_id , NULL ,  client_handler , (void*) &new1) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        printf("New client connected from port no %d and IP %s\n", ntohs(client.sin_port), 
inet_ntoa(client.sin_addr));
	 	
        printf("Handler assigned to client");
    }

    if ((new1 < 0)) // accept takes pointer to variable containing len of struct
		{
			perror("ACCEPT.Error accepting new connection");
			exit(-1);
		}

	

     return 0;
   }


/*
 * This will handle connection for each client
 * 
 */
void *client_handler(void *socket_cl)
{
    //Get the socket descriptor
    int new1 = *(int*)socket_cl;
    char *temp;

    //variables for add new user operation
	char buffer[MAX_BUFFER]; // Receiver buffer; 
	//char file_name[MAX_BUFFER];//Buffer to store filename,path and port recieved from client
	//char *client_ip;//variable to store IP address of client
	
	int success=0;

	//variable for authenticating existinguser operation
	char user_name[MAX_BUFFER];
	char user_key[MAX_BUFFER];
	char file_name[MAX_BUFFER];//file keyword for search by user
	int len;// variable to measure MAX_BUFFER of incoming stream from user
	
	pid_t child_pid;// to manage child process

     
    struct sockaddr_in client;
    socklen_t sin_size = sizeof(client);
    int res = getpeername(new1, (struct sockaddr *)&client, &sin_size);
    char *client_ip ;
    client_ip=inet_ntoa(client.sin_addr);
    //printf(client_ip);
    //strcpy(client_ip, inet_ntoa(client.sin_addr));
		
	
	 	
		while(1)
		{
		len=recv(new1, buffer , MAX_BUFFER, 0);
		buffer[len] = '\0';
		printf("%s\n",buffer);

		//conenctionerror checking
			if(len<=0)//connection closed by client or error
				{
				if(len==0)//connection closed
				{
					printf("client %s hung up\n",inet_ntoa(client.sin_addr));
					
					return 0;
					
				}
				else //error
				{
					perror("ERROR IN RECIEVE");
					return 0;
				}
			
			}//clos if loop

			//ADD NEW USER OPERATION
		if(buffer[0]=='n' && buffer[1]=='e' && buffer[2]=='w') // check if user wants to publish a file
		{

			char ack[]="Request_received";
			send(new1, ack, sizeof(ack), 0); // recieve confirmation message from server

	
		//Recieve username and password
		char login_name[MAX_BUFFER];
		char login_password[MAX_BUFFER];
		//struct user new_user = {0}; //Initializing to null
		
		bzero(buffer,MAX_BUFFER);	
		len=recv(new1, login_name, MAX_BUFFER, 0); //recv user name from client, data is pointer where data recd will be stored
		login_name[len] = '\0';
		char Report1[] = "Username recieved"; 
		send(new1,Report1,sizeof(Report1),0);

		bzero(buffer,MAX_BUFFER);
		len=recv(new1, login_password, MAX_BUFFER, 0); //recv user password from client, data is pointer where data recd will be st
		login_password[len] = '\0';
		//char Report2[] = "Password recieved"; 
		//send(new1,Report2,sizeof(Report2),0);	

		//bzero(buffer,MAX_BUFFER);

		//strcpy(new_user.uname, login_name);
        //strcpy(new_user.upassword, login_password);
        
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//INSERT CODE TO ADD USERS TO DATABASE
        temp ="new";
        success = connect_to_auth_server(login_name,login_password,temp);



        cout<<success<<endl;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		if (success)
		{
		char Report3[] = "USER ADDED   "; 
		send(new1,Report3,sizeof(Report3),0);	
		printf("%s\n",Report3);
		
		}
		else
		{
		char Report3[] = "ADDING USER FAILED.TRY ANOTHER USERNAME   "; 
		send(new1,Report3,sizeof(Report3),0);
		printf("%s\n",Report3);	
		
		}
		printf("Closing connection\n");
	    printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	inet_ntoa(client.sin_addr));
			client_ip = inet_ntoa(client.sin_addr); // return the IP
			
		close(new1); /* close connected socket*/
	    return 0;

		}


		

		//AUTHENTICATE USER AND GET EVENTS
		else if(buffer[0]=='g' && buffer[1]=='e' && buffer[2]=='t') //check keyword for search sent by client
		{

			char ack[]="Request_received";
			send(new1, ack, sizeof(ack), 0); // recieve confirmation message from server
			success=0;
			bzero(buffer,MAX_BUFFER); // clearing the buffer by padding


			len=recv(new1, buffer, MAX_BUFFER, 0); //recv user_name from client to search
			char Report1[] = "username recieved"; 
			send(new1,Report1,sizeof(Report1),0);
			sscanf (buffer, "%s\n", user_name); /* discard CR/LF */
			bzero(buffer,MAX_BUFFER); // clearing the buffer by padding
			//user_name[len] = '\0';
			printf("Username recieved %s\n",user_name);

			len=recv(new1, buffer, MAX_BUFFER, 0); //recv user_name from client to search
			sscanf (buffer, "%s\n", user_key); /* discard CR/LF */
			bzero(buffer,MAX_BUFFER); // clearing the buffer by padding
			//user_name[len] = '\0';
			//char Report2[] = "password recieved"; 
			//send(new1,Report2,sizeof(Report2),0);
			//user_key[len] = '\0';
			printf("Password recieved %s\n \n",user_key);

		
			//printf("%s\n",file_name);

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			temp="auth"; // keyword to be send to server so that server knows it is a publish operation

			//	thread t1(connect_to_auth_server,user_name,user_key,temp );
			// auto future = std::async(connect_to_auth_server,user_name,user_key,temp );
			//success = future.get(); 
			success = connect_to_auth_server(user_name,user_key,temp);
			//	t1.join();
			

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		if (success)
			{
				char Report3[] = "Successfull login  "; 
				send(new1,Report3,sizeof(Report3),0);
				strcat(Report3,user_name);
				
			
				char receive_buff[MAX_BUFFER]; /* to store received string */
				ssize_t len; /* bytes received from socket */
				/* read name of requested file from socket */
				if ( (len = recv(new1, receive_buff, MAX_BUFFER, 0)) < 0) {
				perror("Error in recieve");
					}
				sscanf (receive_buff, "%s\n", file_name); /* discard CR/LF */


			    upload_file(new1, file_name);

			 

			}
		else
		{
			char Report3[] = "Unsuccessfull login.Try again   ";
			send(new1,Report3,sizeof(Report3),0);
			

		}
		
		
		
	    printf("Closing connection\n");
	    printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	inet_ntoa(client.sin_addr));
			client_ip = inet_ntoa(client.sin_addr); // return the IP
			
		close(new1); /* close connected socket*/
	    return 0;
		}// close get condition


		
		
			//TERMINATE OPERATION:when user want to disconnect from server
		else if(buffer[0]=='t' && buffer[1]=='e' && buffer[2]=='r')
		{

			char ack[]="Request_received";
			send(new1, ack, sizeof(ack), 0); // recieve confirmation message from server
			printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	inet_ntoa(client.sin_addr));
			client_ip = inet_ntoa(client.sin_addr); // return the IP
			
			printf("Closing connection\n");
			close(new1); /* close connected socket*/
        
	    
			return 0;
			
		} //close terminate loop
	
	}// close while loop inside fork.server will keep listening client till disconnected
	
	
         
    //Free the socket pointer
    free(socket_cl);
     
    return 0;
}





int upload_file(int sock, char *file_name)
{

	ssize_t read_bytes, sent_bytes, sent_file_size; // to keep count of total data being sent
	int sent_counter; // to count number of packets transfered

	char send_buf[MAX_BUFFER]; //buffer to store the data read from file
	//char * errmsg_notfound = "File not found\n";
	int fp; //file descriptor

	sent_counter = 0;
	sent_file_size = 0;


	char file_name_full[MAX_BUFFER];
	strcpy(file_name_full,"uploads/");
	strcat(file_name_full,file_name);


	if( (fp = open(file_name_full, O_RDONLY)) < 0) 
	{
		
		perror("File not found " );

	}
	else 
	{
		printf("Sending file: %s\n", file_name_full);
		while( (read_bytes = read(fp, send_buf, MAX_BUFFER)) > 0 )
		{
			if( (sent_bytes = send(sock, send_buf, read_bytes, 0))< read_bytes )
			{
				perror("Error in send");
				return -1;
			}
		sent_counter++;
		sent_file_size += sent_bytes;
		}
	close(fp);
	printf("File sent to client: %s\n", file_name_full);
	} //end loop
	
	printf("Sent %zu (bytes) in %d send(s)\n\n",sent_file_size, sent_counter);
	return sent_counter;
}





int connect_to_auth_server(char* user_name, char* user_key, char* keyword)
{

	int auth_sock; // auth_sock is socket desriptor for connecting to remote server 
	struct sockaddr_in auth_server; // contains IP and port no of remote server
	char send_buff[MAX_BUFFER];  //user input stored 
	char receive_buff[MAX_BUFFER]; //recd from remote server
	int len;//to measure length of recieved input stram on TCP
	char *temp = keyword; // variable to store temporary values
	//char const *username1 =username.c_str();
	//char const *password1 = password.c_str();
	char* username1=user_name;
	char *password1=user_key;
	
	//printf("%s","Inside authenticating funtion");
	//for connecting with server for publishing and search files
	if ((auth_sock= socket(AF_INET, SOCK_STREAM, 0)) <0)
	{ 
		perror("Problem in creating socket");  // error checking the socket
		exit(-1);  
	} 
	  
	memset(&auth_server,0,sizeof(auth_server));
	auth_server.sin_family = AF_INET; // family
	auth_server.sin_port =htons(atoi(AUTH_SERVER_PORT)); // Port No and htons to convert from host to network byte order. 
	auth_server.sin_addr.s_addr = inet_addr((AUTH_SERVER_IP));//IP addr in ACSI form to network byte order converted using inet
	bzero(&auth_server.sin_zero, 8); //padding zeros
	

	if((connect(auth_sock, (struct sockaddr *)&auth_server,sizeof(auth_server)))  == ERROR) //pointer casted to sockaddr*
	{
		perror("Problem in connect");
		exit(-1);
	}
	printf("%s","\n Connected to Authentication server\t \n");

	
	send(auth_sock, temp, sizeof(temp) ,0); // send input to server
	len = recv(auth_sock, receive_buff, MAX_BUFFER, 0); // recieve confirmation message from server
	receive_buff[len] = '\0';
	printf("%s\n" , receive_buff); // display confirmation message
	bzero(receive_buff,MAX_BUFFER); // pad MAX_BUFFER with zeros


	printf("Sending username and password...");
	
	send(auth_sock, username1, sizeof(username1) ,0); // send input to server
	len = recv(auth_sock, receive_buff, MAX_BUFFER, 0); // recieve confirmation message from server
	receive_buff[len] = '\0';
	printf("%s\n" , receive_buff); // display confirmation message
	bzero(receive_buff,MAX_BUFFER); // pad MAX_BUFFER with zeros

	send(auth_sock, password1, sizeof(password1) ,0); // send input to server
	len = recv(auth_sock, receive_buff, MAX_BUFFER, 0); // recieve confirmation message from server
	receive_buff[len] = '\0';
	printf("%s\n" , receive_buff); // display confirmation message
	
	printf("Connection terminated with AUTHENTICATION server.\n");

	//cout<<numrows<<endl;
    if(receive_buff[0]=='S' && receive_buff[1]=='u' && receive_buff[2]=='c') 

       {
       	      	return 1;
       }
    else
      {
      	      	return 0;
      }

     // bzero(receive_buff,MAX_BUFFER); // pad MAX_BUFFER with zeros




}
