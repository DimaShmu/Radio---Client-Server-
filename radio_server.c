#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "radio_server.h"
#include <sys/time.h>



	
//globals	
uint32_t multicast_addr;
tcp_socket welcome_socket;
struct sockaddr_in newTCPsocket;
socklen_t addr_size;
char** songs;
int num_of_clients=0;
int num_of_stations;
int udp_port;
//client* head = NULL; 
//client* tail = NULL; 
client clients[100];
UDP_station *stations_fd;
char** argv_to_use;
int downloading_flag = 0;
int port;
int running_welcome_flag=1;
int client_thread_flag=1;
int user_input_flag=1;
int send_song_flag=1;
char *song_name;
char client_song_name[200] = { 0 };
int uploadingClient;
struct timeval time_val;






void free_all(){
	int i;
	for(i = 0; i < num_of_clients; i++) {
		close(clients[i].fd_id);
	}
	running_welcome_flag=0;
	client_thread_flag=0;
	user_input_flag=0;
	send_song_flag=0;
	free(stations_fd);
	printf("all data free!!!!!!\n");
}


void close_client(struct client client1){
	int i;
	i=client1.num;
	close(client1.fd_id);
	num_of_clients-=1;	
	while(i<num_of_clients){
		clients[i]=clients[i+1];
		clients[i].num-=1;
	}
	
}

//threads;

void* client_thread(int i){
	//select and cases (only for one station)
	
	uint8_t command_get_from_client,client_song_name_size;
	char get_msg_buffer[MSG_BUFFER_SIZE];
	char* welcome_msg_buffer;
	uint16_t station_asked_num;
	uint32_t client_song_size,byte_recived=0;
	int byte_get,struct_size;
	pthread_mutex_t lock;
	int j;
	struct timeval time_val;//.tv_usec = 3000000; 
	
	FILE *new_song;
	
	
	while(client_thread_flag){
	//i=clients[i].num;
	FD_ZERO(&clients[i].read_fd);
	FD_SET(clients[i].fd_id, &clients[i].read_fd);//set file discriptor for select
		int j=0;
		if (select(clients[i].fd_id + 1, &clients[i].read_fd, NULL, NULL, NULL) < 0){
			perror("select() failed\n");
			free_all();
			exit(1);
		}
		if (FD_ISSET(clients[i].fd_id,&clients[i].read_fd)) {//event occur
			bzero(get_msg_buffer, MSG_BUFFER_SIZE);
			byte_get=0;
		}
		
		byte_get = recv(clients[i].fd_id, get_msg_buffer,MSG_BUFFER_SIZE, 0);
		if(byte_get<0 || byte_get==0){
			printf("connection lost client will close\n");
			close_client(clients[i]);			
			pthread_exit(NULL);
		}
		//add defensive code here case byte_get==0
		
		command_get_from_client = get_msg_buffer[0];
		switch (command_get_from_client) {
			case 0: //hello received, send welcome

					if (get_msg_buffer[1] != 0 || get_msg_buffer[2] != 0) {
						perror("invalid hello message\n");
						send_invalid_msg(clients[i],command_get_from_client);
					} 
					else if (clients[i].state == OFF) {
						perror("invalid client\n");
						send_invalid_msg(clients[i],command_get_from_client);
					}
					else{
						clients[i].state = ESTABLISHED;
						//send welcome msg
						struct_size =  9;//size of(struct welcome_msg);
						struct welcome_msg msg = {0};
						msg.replyType			= 0;
						msg.numStations			= htons(num_of_stations);
						msg.multicast_group_ip	= htonl(multicast_addr);
						msg.portNumber			= htons(udp_port);
							
							
						welcome_msg_buffer = (char*)malloc(struct_size);
						memcpy(welcome_msg_buffer, &(msg.replyType), 1);
						memcpy((uint16_t*)(welcome_msg_buffer + 1), &(msg.numStations), 2);
						memcpy((uint32_t*)(welcome_msg_buffer + 3), &(msg.multicast_group_ip), 4);
						memcpy((uint16_t*)(welcome_msg_buffer + 7), &(msg.portNumber), 2);
						//memcpy(welcome_msg_buffer, &msg, struct_size);
							
						send(clients[i].fd_id, welcome_msg_buffer, struct_size, 0);
						free(welcome_msg_buffer);
						
						
						
						
						
					}
			
				break;//if (i>=0 && (size_t)i == s) if (num_of_stations-1>=0 && (size_t)i == s)
					
				
				
			case 1: // ask song received, send announce
				
				
				if (byte_get < 3) {
					send_invalid_msg(clients[i],command_get_from_client);
					perror("got invalid command askSong\n");
					
				} 
				else if (clients[i].state != ESTABLISHED) {
					send_invalid_msg(clients[i],command_get_from_client);
					perror("Invalid command askSong before hello\n");
					
				} 
				else {
					station_asked_num = (get_msg_buffer[1] << 8) + get_msg_buffer[2];
					int a = station_asked_num;
					
					if (a > num_of_stations) {
					
						send_invalid_msg(clients[i],command_get_from_client);
						perror("Station doesn't exist\n");
					} 
					else {//send announce msg
						
	
						struct announce_msg msg = {0};
						
						strcpy(msg.text, stations_fd[station_asked_num].song_a.songNameStation);
					
						msg.songNameSize = stations_fd[station_asked_num].song_a.song_name_size;

						
						struct_size =  2 + strlen(msg.text);//size of thus annaunce msg;
					
						msg.replyType = 1;
						welcome_msg_buffer = (char*)malloc(struct_size);
						memset(welcome_msg_buffer, 0, struct_size);
						memcpy(welcome_msg_buffer, &(msg.replyType), 1);
						memcpy(welcome_msg_buffer + 1, &(msg.songNameSize), 1);
						memcpy(welcome_msg_buffer + 2, msg.text,strlen(msg.text));
						send(clients[i].fd_id, welcome_msg_buffer, struct_size, 0);
						free(welcome_msg_buffer);

					

					}
				}
			case 2://up song
				printf("case2!!!!!!!!!!!!!!!!!!");
				struct permit_msg msg = {0};
				msg.replyType=2;
				if (clients[i].state != ESTABLISHED) {
					send_invalid_msg(clients[i],command_get_from_client);
					perror("Invalid command askSong before hello\n");	
				} 
				pthread_mutex_lock(&lock);//lock
				if(downloading_flag){ //download file can't download another one
					msg.permit_value=0;
					pthread_mutex_unlock(&lock);//unlock
				}
				if(downloading_flag==0){
					downloading_flag=1;
					pthread_mutex_unlock(&lock);//unlock
					client_song_size = ntohl(*((uint32_t*) (&get_msg_buffer[1])));
					client_song_name_size = (*(uint8_t*) (&get_msg_buffer[5]));
					memcpy(client_song_name, get_msg_buffer + 6, client_song_size);
					msg.permit_value=1;
					/*uploadingClient = i;
					clients[uploadingClient].state = UPSONG;*/
				}
				if(client_song_name_size > 10485760 || client_song_name_size < 2048){
					msg.permit_value=1;
				}
				for(j=0;j<num_of_stations;j++){
					if(strcmp(client_song_name, stations_fd[j].song_a.songNameStation) == 0 )
					msg.permit_value=1;
					break;
				}

				welcome_msg_buffer = (char*)malloc(2);
				memcpy(welcome_msg_buffer, &msg, 2);
				send(clients[i].fd_id, welcome_msg_buffer, 2, 0);
				free(welcome_msg_buffer);
				time_val.tv_usec = 300; 
			
				if(msg.permit_value){//wait for song to upload
					if(!(new_song = fopen("songName", "w"))) {	
						perror("cant open a file\n");
						send_invalid_msg(clients[i],command_get_from_client);
					}
					while(byte_recived<client_song_size){
						FD_ZERO(&clients[i].read_fd);
						FD_SET(clients[i].fd_id, &clients[i].read_fd);//set file discriptor for select
						if (select(clients[i].fd_id + 1, &clients[i].read_fd, NULL, NULL, &time_val)<= 0){
							send_invalid_msg(clients[i],command_get_from_client);
							perror("song upload error\n");
						}
						if (FD_ISSET(clients[i].fd_id,&clients[i].read_fd)) {//data arrived
							bzero(get_msg_buffer, MSG_BUFFER_SIZE);
							byte_get=0;
							byte_get = recv(clients[i].fd_id, get_msg_buffer,MSG_BUFFER_SIZE, 0);
							if(byte_get<0 || byte_get==0){
								printf("connection lost client will close\n");
								close_client(clients[i]);			
								pthread_exit(NULL);
							}
							byte_recived+=byte_get;
							fwrite(get_msg_buffer, sizeof(char), byte_get,new_song);
						}
					}
				}
			
			
			
	break;
		}
	}
}

void* user_input_thread(){
	//fd_set user_read_fd;
	char in;
	char p= 'p';
	char q= 'q';
	char station_ip[16] = { 0 };
	int i,scaned,c,counter;
	while(user_input_flag){
		fflush(stdin);
		scaned=0;
		printf("Please type q to quit or p to print the stations that we are advertising and the tcp clients\n");
		//FD_ZERO(&user_read_fd);
		//FD_SET(fileno(stdin), &user_read_fd);//set stdin in file discriptor for select
		/*while(scaned==0)*/{
			//scaned=scanf("%c",&in);
			in = getchar();
		}
		counter = 0;
		while ((c = getchar()) != '\n' && c != EOF) { counter++; }
		if(counter > 0) {
			printf("enter only 1 char\n");
		}
		else{
			if(in==p){
				printf("There are currently %d stations in our radio:\n",num_of_stations);
				for (i = 0; i < num_of_stations; i++) {
					//stations_fd[i].station.sin_addr.s_addr
					int_to_str(station_ip, stations_fd[i].station.sin_addr.s_addr);
					printf("station %d\t %s\t %s \n", i, station_ip, argv_to_use[i + 4]);
				}
				printf("There are currently %d clients:\n",num_of_clients);
				for (i = 0; i < num_of_clients; i++) {
					printf("Client %d ip is %s\n", i, inet_ntoa(clients[i].socket.sin_addr));
				}			
			}
			if(in==q){
				free_all();
				exit(1);
			}
		}
		
	}
}

void* welcome_socket_listening(){
	int new_socket=0 , struct_size;
	char* welcome_msg_buffer;
	pthread_t station_thread;
	int size_a= sizeof(struct sockaddr_in);
	if (listen(welcome_socket.fd_id, 100) < 0) {
		perror("Listening Error");
		free_all();
		exit(1);
	}
	
	while(running_welcome_flag){
		/*clients[num_of_clients].socket.sin_family = AF_INET; 
		clients[num_of_clients].socket.sin_port = htons(port);
		clients[num_of_clients].socket.sin_addr.s_addr = htonl(INADDR_ANY);*/

		clients[num_of_clients].fd_id = accept(welcome_socket.fd_id, (struct sockaddr *) &clients[num_of_clients].socket, &size_a);
		if(clients[num_of_clients].fd_id < 0){
			perror("Accepting Error");
			free_all();
			exit(1);
		}
		if(clients[num_of_clients].fd_id != 0){
	
			clients[num_of_clients].current_station = 0;
			clients[num_of_clients].num = num_of_clients;
			FD_ZERO(&clients[num_of_clients].read_fd);
			FD_SET(clients[num_of_clients].fd_id, &clients[num_of_clients].read_fd);//set file discriptor for select
			num_of_clients+=1;
			//add timeout 
		}
		if((pthread_create(&clients[num_of_clients-1].thread,NULL,client_thread,num_of_clients-1)!=0)){
			perror("thread create Error");
		}	
		/*if(pthread_join(station_thread,NULL)!=0){
			perror("pthread_join Error\n");	
		}*/
	}
	
	
	
}







int main(int argc, char* argv[]) {
	uint32_t tcp_welcome_addr;
	int  tcp_port;
	int fd = 0, new_socket = 0, receivedByte = 0, flag = 0, send_counter = 0,i=0;
	int ttl = 15, sendCounter = 0, bytesend = 0, socket_in = 0,addrlen = 0, PackSent = 0;
	uint32_t temp_multicast_addr;
	pthread_t listen_to_welcome_socket,user_input;
	char* buffer[BUFFER_SIZE]={0};
	addrlen = sizeof(struct sockaddr_in);
	num_of_stations=argc-4,i=0;
	argv_to_use=&(argv[0]);
	port = atoi(argv[2]);


	if (argc < 5) {

			perror("didn't get all the values\n");

			exit(1);

		}	

	// main Variables	
	tcp_port = atoi(argv[1]);
	multicast_addr = inet_addr(argv[2]);
	udp_port = atoi(argv[3]);
	songs = &(argv[4]);
	
	
	
	
	welcome_socket.fd_id = socket(AF_INET,SOCK_STREAM,0);
	if(welcome_socket.fd_id == -1) {
		perror("Open welcome socket Error");
		return EXIT_FAILURE;
	}
	if (setsockopt(welcome_socket.fd_id, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
		perror("setsockopt failed");
		return EXIT_FAILURE;
	}
	
	welcome_socket.socket.sin_family = AF_INET;
	welcome_socket.socket.sin_port = htons(tcp_port);
	welcome_socket.socket.sin_addr.s_addr = htonl(INADDR_ANY);//anycast
	memset(welcome_socket.socket.sin_zero, '\0',sizeof welcome_socket.socket.sin_zero);
	
	if (bind(welcome_socket.fd_id, (struct sockaddr *) &welcome_socket.socket,sizeof(welcome_socket.socket))) {
		perror("Binding Error");
		return EXIT_FAILURE;
	}
	if((pthread_create(&listen_to_welcome_socket,NULL,welcome_socket_listening,NULL)!=0)){
		perror("thread create Error");
	}	
	
			
	
	
	
	if (!(stations_fd = (UDP_station*) malloc(sizeof(UDP_station) * num_of_stations))){
		perror("malloc failed\n");
	}

	for(i=0;i<num_of_stations;i++){//create udp sockets
		
		if ((stations_fd[i].fd_id=socket(AF_INET,SOCK_DGRAM,0))== -1) {

			perror("Socket create Error");

			return EXIT_FAILURE;

		}
	

		memset(&stations_fd[i].station,0,sizeof(stations_fd[i].station));
		stations_fd[i].station.sin_family = AF_INET;
		stations_fd[i].station.sin_port = htons(udp_port);
		temp_multicast_addr= inet_addr(argv[2]) + htonl(i);
		stations_fd[i].station.sin_addr.s_addr = temp_multicast_addr;
		
		
	

	

		if (setsockopt(stations_fd[i].fd_id,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl)) < 0 ) {

			perror("Setsockopt Error");

			return EXIT_FAILURE;

		}
		stations_fd[i].song_a.songNameStation = argv[i + 4];
		stations_fd[i].num_of_station = i;
		if (!((stations_fd[i].song_a.song = fopen(songs[i], "rb")))) { //open file for every station
			perror("Error: failed open file\n");
		}
		
		
		
		
		fseek(stations_fd[i].song_a.song,0,SEEK_END);
		stations_fd[i].song_a.song_name_size=strlen(stations_fd[i].song_a.songNameStation);		
		fseek(stations_fd[i].song_a.song,0,SEEK_SET);
		


	}
	
	//start sending the songs
	printf("Welcome to the Radio Server!!!\n\n");
	if((pthread_create(&user_input,NULL,user_input_thread,NULL)!=0)){
		perror("thread create Error");
	}	
	
	
	
	while(send_song_flag){
		i=0;
		for(i;i<num_of_stations;i++){
			bytesend = fread(buffer,1,BUFFER_SIZE,stations_fd[i].song_a.song);
			if(bytesend<BUFFER_SIZE){
				fseek(stations_fd[i].song_a.song,0,SEEK_SET);//return to the start of the file
			}
			
			
			if((sendCounter = sendto(stations_fd[i].fd_id,buffer,sizeof(buffer),0,(struct sockaddr*)&stations_fd[i].station,sizeof(stations_fd[i].station))) == -1){
				perror("Sending Error");
				free_all();
				exit(0);
			}
	
		}
		usleep(62500);
	}
	
	

	if(pthread_join(listen_to_welcome_socket,NULL)!=0){
		perror("pthread_join Error\n");		
	}
	/*if(pthread_join(user_input,NULL)!=0){
		perror("pthread_join Error\n");	
		}*/
}


