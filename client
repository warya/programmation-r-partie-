#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<fcntl.h>

int main(void){
	char msg[1000];
	struct addrinfo hints, *serverInfo, *p ;
	int sockfd, confd;
 
	// initialisation de la structure addrinfo
	bzero(&hints,sizeof(hints));	
	hints.ai_family = AF_INET ;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// getaddrinfo
	int errocode = getaddrinfo(
				NULL, 
				"2000",
				&hints,
				&serverInfo);
	if(errocode != 0){
		perror("Erreur getaddrinfo():");
		exit(EXIT_FAILURE);
	}

	// Création de le socket
	if( (sockfd=socket(serverInfo->ai_family,serverInfo->ai_socktype,serverInfo->ai_protocol)) < 0){
		perror("Erreur socket():");
		exit(EXIT_FAILURE);
	}
	
	// connect();
	int cnx = connect(sockfd,serverInfo->ai_addr,sizeof(serverInfo->ai_addr));
	if(cnx < 0){
		perror("Erreur connect() :");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	
	// message du client 
	msg[1000] = "Hello";
	
	// send();
	int sd = send(sockfd,msg,strlen(msg), 0);
	if(sd <0){
		perror("Erreur send():");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
		
	// recv();
	int rc = recv(sockfd,msg, sizeof(msg), 0);
	if(rc <0){
		close(sockfd);
		exit(EXIT_FAILURE);
		perror("Erreur recv() :");
	}	

	close(sockfd); // fermeture de la socket
return EXIT_SUCCESS;
}
