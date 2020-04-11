#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/time.h> 

/*ALL THE DEFINES VALUES*/
#define BUFFER_SIZE 1024
#define UPLOAD_TIMER 0.008
#define TRUE   1  
#define FALSE  0 

/*FUNCTIONS*/
int check_valid_input(char *);
void convert_to_str(char *, uint32_t );


/*ALL THE GLOBAL VALUES*/
struct timeval time_val;
struct timeval *p_time;
struct sockaddr_in address, udp_addr;
struct ip_mreq mreq;


int fd = 0, udp_fd = 0; //discriptors of tcp and udp
int song_size = 0;
int input;
char g_rx_buffer[BUFFER_SIZE];
char g_tx_buffer[BUFFER_SIZE];
char g_user_buffer[100] = { 0 };
char multicastGroup_str[20] = { 0 };
char askSong[3] = { 0 };
char upSong[106] = { 0 };
char replyString[1024] = { 0 };
uint8_t replyStringSize;
uint16_t numStations = 0;
uint32_t current_station = 0;
uint32_t station_to_change = 0;
uint32_t multicastGroup = 0;





FILE *player;
FILE *song_info;
pthread_t udp_thread;
fd_set readfds;




////////////////////////////dont forget to make a function that close everything and exit the socket//////////////////////////////////////




/*user input*/

/*
	ask for input 
	
	1->asksong
	2->upsong
	3->invalid
	4->'q' for quit and close all

*/

int user_input()
{
	int i;
	int byte_counter;
	
	
	if((byte_counter = read(0, g_user_buffer, sizeof(g_user_buffer))) < 0 )  // read user input
	{
		printf("Problem on reading from user\n");
		
		//closeeeeeeeeeeeeeeeeee
	}
	
	input = check_valid_input(g_user_buffer); //check which input we got
	printf("Input from user is :%d\n",input);			
	//HERE WE WILL SEND THE CORRECT MESSAGE TO THE SERVER
	switch(input)
	{
		case 1:// askSong
			//printf("asKsong\n");
			askSong[0] = (uint8_t) 1;
			*((uint16_t*) &askSong + 1) = atoi(g_user_buffer);
			
			printf("asksong with station number :%s\n",g_user_buffer);			

			byte_counter = write(fd, askSong, sizeof(askSong));
			if(byte_counter < 0)
			{
				perror("read from server");
				close(fd);
				close(udp_fd);
				exit(1);
			}
			
			station_to_change = atoi(g_user_buffer);
			/*afer we get answer from the server about the  askSong,
			 *we should check if we have to change the station
			 *or we are alredt in the correct group*/
			
			
			return 1;
				
		
		case 2://upSong
			
			upSong[0] = (uint8_t) 2;
			printf("Please insert the song name\n");
			if(read(0, g_user_buffer, sizeof(g_user_buffer)) < 0)
			{
					perror("Got an Error to read from the user_buffer\n");
					close(fd);
					close(udp_fd);
					exit(1);
					
					//closeeeeeeeeeeeeeeeeee
			}
			
			g_user_buffer[strlen(g_user_buffer) - 1] = '\0';
			printf("%s\n",g_user_buffer);

			song_info = fopen(g_user_buffer, "r"); 
			if(song_info == NULL)
			{
				printf("song doesnt exist ...you are going back to the menu\n");
				return 0;
			}
			
			//CHECK THE SIZE OF THE FILE
			
			if (song_info > 0)
			{
				while ((getc(song_info)) != EOF ) song_size++;
				song_size --;
				fclose(song_info);
			}
			if((song_size < 0) || (song_size > 10000000))				// check the song size
			{
				printf("bad song size\n");
				return 0;
			}else {
			
				*((uint32_t*) &upSong[1]) = htonl(song_size);
				upSong[5] = (uint8_t) (strlen(g_user_buffer));
				for (i = 0; i < strlen(g_user_buffer); i++) upSong[i + 6] = g_user_buffer[i];
				if (write(fd, upSong, (strlen(g_user_buffer)) + 6) < 0) 
				{
					perror("Failed in sending askSong");
					close(fd);
					close(udp_fd);
					exit(1);
						
				}
			printf("UpSong was sent and the song size is:%d\n",song_size);
			}
			return 2;
			break;

		case 9://quit
			perror("Thank you for using Our Radio...Bye Bye\n");
			close(udp_fd);
			close(fd);
			return 3;
			//closeeeeeeeeeeeeeeeeee
		
		
		
		
		case 4://bad input,try again
			perror("Sorry Invalid Option\n");
			return 0;
			
			
	}
	
}


void server_to_client()
{
	int i, f = 0;
	int byte_counter = 0;
	int byte_song = 0;
	int song_fd = 0;
	uint8_t messageType = 0;
	uint8_t songNameSize,permit_status = 0;
	char songName[200] = {0};
	
	if(f = read(fd, g_rx_buffer, 1024) < 0) //receive input from buffer
	{
		perror("read from server");
		close(fd);
		close(udp_fd);
		exit(1);
	}
	messageType = g_rx_buffer[0];
	

		switch (messageType)
		{
			/*wait for annouce
			  didnt check for errors....
			Announce:
				uint8_t replyType = 1;
				uint8_t songNameSize;
				char songName[songNameSize];*/
			case 1:
				printf("we are in server client and option 1\n");
				songNameSize = *(uint8_t*) (&g_rx_buffer[1]);
				for(i = 0; i<songNameSize; i++)
				{
					songName[i] = g_rx_buffer[2 + i];
				}
				songName[i+1] = '\0';
				printf("The song in this station is: %s", songName);
				//printf("Please choose one of our stations\n"); 
				//printf("To switch a station press the wanted station between 0 -> %d\n",numStations -1); 
				//printf("or press 's' to upSong, or enter 'q' to Exit\n");
				//printf("---------------------------------------------------\n");
				return;


			/*wait for permit
			 *means that if we got 1, we can upload a song,
			 *and do lock our client to exit or do something stuped....
			 
			 PermitSong:
				uint8_t replyType = 2;
				uint8_t permit;
			 */
			case 2:	
			
				permit_status = g_rx_buffer[1];/////////*(uint8_t*) (&g_rx_buffer[1]);
				
				if(permit_status == 0)
				{
					printf("Somebody probably upload sone right now,any way ,you cant... !\n");
					return;
				} else if(permit_status == 1)
				{
					printf("got permit! we can start sending the Data!\n");
					printf("The song size is:%d\n",song_size);
					song_fd =  open(g_user_buffer, O_RDONLY);
					// WE SHOULD USE MUTEX FOR NOT GETTING ANY DATA FROM THE SERVER WHILE UPLOADING UNLESS GETTING INVALIDCOMMAND FROM THE SERVER
					while (song_size > 0) 
					{
						if (song_size < BUFFER_SIZE) //if the song size is less then the buffer size;
						{
							byte_song = read(song_fd, g_tx_buffer, song_size);
							byte_counter = send(fd, g_tx_buffer, byte_song, 0);
							song_size = 0;
							if(byte_counter < 0)
							{
								perror("Song sending Failed!!!!");
								close(fd);
								close(udp_fd);
								exit(1);
								return;
							}
							printf("The song was sent successfully\n");
							return;
				
						} else {
						
							byte_song = read(song_fd, g_tx_buffer, BUFFER_SIZE);
							byte_counter = send(fd, g_tx_buffer, byte_song, 0);
							sleep(UPLOAD_TIMER);
							bzero(g_tx_buffer, BUFFER_SIZE);
							if(byte_counter < 0)
							{

								perror("Song sending Failed!!!!");
								close(fd);
								close(udp_fd);
								exit(1);
								return;

							}
							else if (byte_counter > 0) song_size -= byte_counter;
								
						}
					}//while
					close(song_fd);	
				}
				else {
					printf("Unknown permit command\n");
					close(fd);
					close(udp_fd);
					exit(1);
					return;

				}
				printf("Uploading is done!\n");
				return;
					
			
			
			/*
			 *InvalidCommand:
			 *uint8_t replyType = 3;
			 *uint8_t replyStringSize;
			 *char replyString[replyStringSize];
			 */	

			case 3:	
				replyStringSize = *(uint8_t*) (&g_rx_buffer[1]);
				for (i = 0; i < replyStringSize - 1; i++) 
				{
					replyString[i] = g_rx_buffer[i + 2];
				}
				replyString[i+1] = '\n';
				printf("InvalidCommand is: %s", replyString);
				close(fd);
				close(udp_fd);
				exit(1);
				return;
			
			/*
			 *NewStations:
			 *uint8_t replyType = 4;
			 *uint16_t newStationNumber;
			 */	
			case 4:
				numStations = htons(*(uint16_t*) (&g_rx_buffer[1]));
				printf("Dear client, we have new station!!! This is station number %d \n",numStations);
				return;
				
		}
	
}
















/*udp_play*/


void *udp_play(void *args)
{
	int byte_counter = 0;
	char songBuffer[BUFFER_SIZE] = { 0 };
	uint32_t multicastGroupChange = 0;
	int change = 0;
	socklen_t length = sizeof(udp_addr);

	player = popen("play -t mp3 -> /dev/null 2>&1", "w");//open a pipe. output is sent to dev/null (hell).
	
		/*check if we have to change station
		 *if yes , change the corrent station a
		 *drop old multicastGroup
		 *join new multicastGroup
		 *keep plating
		 */
	while(TRUE)
	{
		/*ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);*/
		byte_counter = recvfrom(udp_fd, songBuffer, BUFFER_SIZE, 0,(struct sockaddr *)&udp_addr,&length);
		if (byte_counter <= 0)
		{
			perror("recieved Error");
			//close(fd);
			close(udp_fd);
			exit(1);
		}
		
		fwrite (songBuffer , sizeof(char), byte_counter, player);//write a buffer of size numbyts into player
		
		if(current_station != station_to_change) 
		{
			current_station = station_to_change;
			multicastGroupChange = multicastGroup + station_to_change;
			convert_to_str(multicastGroup_str, multicastGroupChange );
			if (setsockopt(udp_fd, IPPROTO_IP,IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
			{
				perror("drop multicast group  failed\n");
				close(udp_fd);
				exit(1);
			}
			 
				
			//JOIN THE MULTICAST GROUP!
			mreq.imr_multiaddr.s_addr = inet_addr(multicastGroup_str);
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
				
			if (setsockopt(udp_fd, IPPROTO_IP,IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
			{
				perror("join multicast group failed\n");
				close(udp_fd);
				exit(1);
			}
			multicastGroupChange = 0;
		}
	}
	
}






/*convert int ip address to string*/
void convert_to_str(char *string_ip, uint32_t int_ip) 
{
	int i;
	uint8_t octet[4] = { 0 };
	for (i = 0; i < 4; ++i)
		octet[i] = (int_ip >> (i * 8)) & 0xFF;
	sprintf(string_ip, "%d.%d.%d.%d", octet[3], octet[2], octet[1], octet[0]);
}



int check_valid_input(char *input)
{
	int i;
	int intNum = atoi(input);
	
	if(input[0] == 's'){return 2;} //for Upsong
	else if(input[0] == 'q'){return 9;} //quit
	else if((intNum > numStations) || (intNum < 0) || !isdigit(input[0])) {return 4;} //bad options	
	return 1;	 //askSong
}


/*main gets ip address and port*/ 
/* 
	-send hello and wait for welcome
	-after getting the welcome message back from the server, start playing the first station
	-open a new thread for udp 
	-udp_client --> start playing 
	-user_input()
	-go to tcp func and do switch case 
	*/
	
int main(int argc,char *argv[])
{
	
	int t, byte_counter = 0, nready, receivedByte = 0;
	int counter = 0;
	uint16_t portNumber = 0;

	//SAVE TAKE FROM INPUT THE ADDRESS 
	char* ip_addr = argv[1];
	int port = atoi(argv[2]);
	
	if(argc < 3)
	{
		printf("input Fail!!");
		return EXIT_FAILURE;
	}
	//Open a socket 
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1) 
	{
		perror("Open Error");
		return EXIT_FAILURE;
	}
	// Filling server information
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET; 
	address.sin_port = htons(port); 
	address.sin_addr.s_addr = inet_addr(ip_addr); 
	
	
	//Connect to server;
	if(connect(fd, (struct sockaddr *) &address, sizeof(address)) == -1)
	{
		perror("Connect Error");
		return EXIT_FAILURE;
	}
	
	
	/* Send hello to the server*/
	bzero(g_tx_buffer, BUFFER_SIZE);
	if (write(fd, g_tx_buffer, 3) < 3)
	{
		perror("Error: hello failed\n");
		//closeeeeeeeeeeeeeeeeee
	}
	printf("Hello sent\n");
	
	//here we will check the welcome message with select
	time_val.tv_sec = 0;  // TIMEOUT FOR GETTING THE WELCOME 
	time_val.tv_usec = 300; 
	fflush(stdout);
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds); //set 
	
	nready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL); //WAIT FOR INCOMING WELCOME SOCKET &time_val
	printf("select the fd tcp\n");
	if(nready < 0)
	{
		printf("Select error");
		//closeeeeeeeeeeeeeeeeeee
	}	
	
	if(FD_ISSET(fd, &readfds))
	{
		printf("is this the welcome from the server?\n");
		
    		memset(g_rx_buffer, 0, BUFFER_SIZE);
		//bzero(g_rx_buffer, BUFFER_SIZE);
		if((byte_counter = read(fd, g_rx_buffer, BUFFER_SIZE) == -1))
		{
			printf("Something Went wrong in receving data from server\n");
			close(fd);
			exit(1);

		}
		//printf("byte counter line 503 is :::: %d\n",byte_counter);
		if(g_rx_buffer[0] == 0)
		{
			printf("We got the Welcome socket!!!\n");
			//GET THE DATA FROM THE SOCKET
			numStations = htons(*(uint16_t*) (&g_rx_buffer[1]));
			multicastGroup = (*(uint32_t*) (&g_rx_buffer[3]));
			portNumber = htons(*(uint16_t*) (&g_rx_buffer[7]));
			
			//convert the multicastGroup to string
			convert_to_str(multicastGroup_str, multicastGroup);
			
			
			printf("We have %d stations\n",numStations);
			printf("the Multicast group you are going to join is : %s \n",multicastGroup_str);
			printf("and the connection will be to port : %d\n",portNumber);
			printf("---------------------------------------------------\n");

			
			//OPEN UDP SOCKET
		
			udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (udp_fd < 0)
			{
				printf("error in openning the socket\n");
				close(fd);
				exit(1);
			}
		
			//SET THE SERVER ADDRESS
			memset(&udp_addr, 0, sizeof(udp_addr));
			udp_addr.sin_family = AF_INET;
			udp_addr.sin_port = htons(portNumber);
			udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			
			if (setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
			        perror("setsockopt(SO_REUSEADDR) failed");
			//BIND
			if(bind(udp_fd,(struct sockaddr *) &udp_addr, sizeof(udp_addr)))
			{	
				perror("bind");
				close(udp_fd);
				exit(1);
				//closeeeeeeeeeeeeeeeeee
			}
			
			//JOIN THE MULTICAST GROUP!
			mreq.imr_multiaddr.s_addr = inet_addr(multicastGroup_str);
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			
			if (setsockopt(udp_fd, IPPROTO_IP,IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
			{
				perror("Error: join multicast group failed");
				close(udp_fd);
			}
			if (pthread_create(&udp_thread, NULL, &udp_play, &udp_fd))
			{
				perror("THREAD CREATE");
				printf("Error: crate udp_thread failed\n");
				close(fd);
				close(udp_fd);
				exit(1);
			}
			//pthread_join(udp_thread,NULL);
		}	
	}
	
	printf("Please choose one of our stations\n"); 
	printf("To switch a station press the wanted station between 0 -> %d\n",numStations -1); 
	printf("or press 's' to upSong, or enter 'q' to Exit\n");
	printf("---------------------------------------------------\n");
	
	/* here we finished the first part of the program.
	   now we wiil have 2 options :
	   1-go to go to user_intrface() and check if the user pressed something
	   2-if we want to upload 
	*/


	while(TRUE)
	{
		/*initial file descriptors for the select
		  do select to check from where we got the interrupt (user / server)
		  **
		*/
		t = 0;
		
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(fd, &readfds);
		
		nready = select(FD_SETSIZE, &readfds, NULL, NULL, p_time); //WAIT FOR INCOMING WELCOME SOCKET
		if(nready < 0)
		{
			perror("Select error");
			close(fd);
			close(udp_fd);
			exit(1);
			
		}	
		
		/*check from where we got the interrupt*/
		
		if(FD_ISSET(STDIN_FILENO, &readfds))
		{
			//printf("interupt from user\n");
			t = user_input(); //check what input the user gave to know if we need to change the timeout
			if(t == 1 || t == 2)
			{
				time_val.tv_sec = 0;  // TIMEOUT FOR RESPONSE FROM THE SERVER 
				time_val.tv_usec = 300;
				p_time = &time_val;
			}
			
			if(t == 0)
			{
				printf("Please choose one of our stations\n"); 
				printf("To switch a station press the wanted station between 0 -> %d\n",numStations -1); 
				printf("or press 's' to upSong, or enter 'q' to Exit\n");
			}
			if(t == 3) break;
			p_time = NULL;
			 /////////////////////////////////////not sure about this !!!!///////////////////////////
		}
		
		else if(FD_ISSET(fd, &readfds))
		{
			//printf("interupt from server\n");
			server_to_client();

		}
		else
		{ 
		    printf("We got timeout...close all!\n");
		    close(fd);
	     	    close(udp_fd);
	 	    break;
		}
	}
	close(fd);
	close(udp_fd);
	//pthread_exit(udp_thread,NULL);

}

