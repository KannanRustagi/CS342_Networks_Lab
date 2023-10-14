#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8080
#define n 4

//For demonstration of loss of frames, server sends negative ack for every nth frame

void stop_and_wait(int client_socket){
	char frame[100];       //Initializations
	int seqNo = 0;
	char ack[6];
	int i=0;
	int count = 1;
	char exit[4] = {'e','x','i','t'};
	while(1){
		int noExit = 0; 
		bzero(frame,100);     //Empty frame 
		
		recv(client_socket,frame,sizeof(frame),0); //Receive frame from client
		bzero(ack,6);  //Empty ack


		if(frame[0] == seqNo && count%n!=0){

			//If expected sequence number received, send ack and change seqNo
			ack[0] = 'A';
			ack[1] = 'C';
			ack[2] = 'K';
			ack[3] = seqNo;
			ack[4] = '\0';

			printf("Frame received from client : ");
			for(int j=1;frame[j]!='\0';j++) printf("%c",frame[j]);
			printf("\n");

			seqNo = seqNo==0?1:0;

		}
		else{

			//If expected sequence number not received, send negative ack
			ack[0] = 'N';
			ack[1] = 'A';
			ack[2] = 'C';
			ack[3] = 'K';
			ack[4] = seqNo;
			ack[5] = '\0';

		}
		sleep(1);
		send(client_socket,ack,sizeof(ack),0);
		
		//If frame contains the message "exit", exit the network
		for(int j=1;j<5;j++){
			if(frame[j]!=exit[j-1]) {
				noExit = 1;
				break;
			}
		}
		if(noExit==0){
			printf("Server has successfully exited the network ... \n");
			return;
		}

		count++;


	}

}

int main(){

	//Initialize socket descriptor
	int server_socket;
	
	//create a socket
	server_socket = socket(PF_INET,SOCK_STREAM,0);
	
	if(server_socket<0){
		printf("Error creating socket ...\n");
		exit(1);
	}
	else{
		printf("Socket created successfully ...\n");
	}
	
	//Binding socket to port
	//Initializign sockddr_in structure before binding
	
	struct sockaddr_in sa;
	bzero(&sa,sizeof(sa));
	sa.sin_family = PF_INET;
	sa.sin_port = htons(PORT);    //Port number 8080
	sa.sin_addr.s_addr = inet_addr("127.0.0.7");
	
	if( bind(server_socket,(struct sockaddr*)&sa, sizeof(sa))== 0){
		printf("Socket binded successfully ...\n");

	} 
	else{
		printf("Unable to bind server... An error occured\n");
		exit(1);		
	}
	
	//Listen
	
	if (listen(server_socket,10)==0){
		printf("Server listening...\n");
	}
	else{
		printf("Server listen failed\n");
	}
	
	//Accept connection
	struct sockaddr_in cli;
	int len = sizeof(cli);
	int client_socket = accept(server_socket,(struct sockaddr*)&cli,&len);

	if(client_socket<0){
		printf("Falied to accept client\n");
		exit(1);
	}
	else{
		printf("Server accepted client\n");
	}
	stop_and_wait(client_socket);
	close(server_socket);
	
}