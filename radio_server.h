#define BUFFER_SIZE 1024
#define OFF -1
#define ESTABLISHED 0
#define MSG_BUFFER_SIZE 1024






typedef struct SONG_DATA {
	char *songNameStation;
	uint8_t song_name_size;
	FILE *song;
}SONG_DATA;


typedef struct UDP_station {
	int fd_id;
	int num_of_station;//num of station
	struct sockaddr_in station;
	struct SONG_DATA song_a;
}UDP_station;



typedef struct tcp_socket {
	struct sockaddr_in socket;
	int fd_id;
	int fd_a;//delete if not used
}tcp_socket;



typedef struct client {
	int fd_id;
	struct sockaddr_in socket;
	int num;
	int current_station;
	char* ip_addr;
	uint16_t tcp_port;
	int state;
	fd_set read_fd;
	pthread_t thread;
}client;



struct welcome_msg{
	uint8_t replyType;
	uint16_t numStations;
	uint32_t multicast_group_ip;
	uint16_t portNumber;
}welcome_msg;

typedef struct announce_msg{
	uint8_t replyType;
	uint8_t songNameSize;
	char	text[100];
}announce_msg;


typedef struct invalid_msg{
	uint8_t replyType;
	uint8_t replySize;
	char	text[100];
}invalid_msg;

typedef struct permit_msg{
	uint8_t replyType;
	uint8_t permit_value;
}permit_msg;





void send_invalid_msg(struct client client1 , uint8_t i) {
	int struct_size,msg_len;
	char* text;
	char* func_buffer;
	printf("sending invalid msg\n");
	struct invalid_msg msg = {0};
	if (i==0){

		text = "case0 problem, didn't get all data or client in ilegal state";
		strcpy(msg.text,text);
		msg_len=strlen(text);
		struct_size=msg_len+2;
	}
	if (i==1){

		text = "case1 problem, didn't get all data or client in ilegal state or station does't exist";
		strcpy(msg.text,text);
		msg_len=strlen(text);
		struct_size=msg_len+2;
	}
	if (i==2){
		text = "client in ilegal state or station";
		strcpy(msg.text,text);
		msg_len=strlen(text);
		struct_size=msg_len+2;
	}
	msg.replyType = 3;
	msg.replySize = msg_len;
	func_buffer = (char*)malloc(struct_size);		
	memset(func_buffer, 0, struct_size);
	memcpy(func_buffer, &(msg.replyType), 1);
	memcpy(func_buffer + 1, &(msg.replySize), 1);
	memcpy(func_buffer + 2, msg.text,msg_len);
	send(client1.fd_id, func_buffer, struct_size, 0);
	free(func_buffer);
	printf("Please type q to quit or p to print the stations that we are advertising and the tcp clients\n");
	
}


void int_to_str(char *s, uint32_t ip) {
	int i;
	uint8_t octet[4] = { 0 };
	for (i = 0; i < 4; ++i)
		octet[i] = (ip >> (i * 8)) & 0xFF;
	sprintf(s, "%d.%d.%d.%d", octet[3], octet[2], octet[1], octet[0]);
}


