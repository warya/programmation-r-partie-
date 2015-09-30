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
	int sockfd, confd, recvd;
	char nomFichier[100];
 
	// Initialisation des champs de addrinfo
	bzero(&hints,sizeof(hints));	
	hints.ai_family = AF_INET ;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// getaddrinfo
	int errocode = getaddrinfo(
				NULL, // addresse ou hotname
				"80", // le port
				&hints,
				&serverInfo);
	if(errocode != 0)
		perror("Erreur getaddrinfo():");
		exit(EXIT_FAILURE);

	// socket
	if( (sockfd=socket(serverInfo->ai_family,serverInfo->ai_socktype,serverInfo->ai_protocol)) < 0){
		perror("Erreur socket():");
		exit(EXIT_FAILURE);
	}
	
	int on = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	
	// bind
	if(bind(sockfd,serverInfo->ai_addr,serverInfo->ai_addrlen)<0)
		perror("Erreur bind():");
		close(sockfd);
		exit(EXIT_FAILURE);
	
	// listen
	if(listen(sockfd,20)<0){
		perror("Erreur listen():");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	
	if( (confd=accept(sockfd,NULL,NULL))>0 ){
		// recv
		recvd=recv(confd,msg, sizeof(msg), 0);

		if(recvd < 0){
			perror("Erreur recv():");
			close(sockfd);
			exit(EXIT_FAILURE);
		}else if(recvd == 0){
			perror("Deconnexion du client");
			close(sockfd);
			exit(EXIT_FAILURE);
		}else{
			msg[recvd]='\0' ;
			
			// on recupere le nom du fichier demande par le client
			/**
			 * parseRequest permet de récupérer le nom du fichier demande
			 * return 0 si le fichier existe sinon -1 en cas d'erreur
			 * */
			int fd = parseRequest(msg,sizeof(msg),nomFichier,sizeof(nomFichier));
			if(fd < 0){
				perror("Erreur fonction parseRequest:");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
		}
		
		char codeSuccess[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";	
		
		// renvoie du code 200 et la page demandée
		int sendfd1 = send(confd,codeSuccess,sizeof(codeSuccess), 0);
		
		if(sendfd1 < 0){
			perror("Erreur send:");
			close(sockfd);
			exit(EXIT_FAILURE);
		}else{
			FILE* fichier = fopen(nomFichier,"r");
			char c[1000];
			int i =0;
			if(fichier == NULL){
				perror("Erreur fopen() :");
				close(sockfd);
				exit(EXIT_FAILURE);
			}else{
                char h;
				while((h=getc(fichier)) != EOF){
					c[i] = h;
					i++;
				}
				c[i] = '\0';
				fclose(fichier);
			}
			printf("%s\n",c);
			int sendfd2 = send(confd,c,strlen(c), 0);
			if(sendfd2<0)
				perror("Erreur send:");
				close(sockfd);
				exit(EXIT_FAILURE);
		}
		close(confd); 
	}else{
		close(sockfd);
		perror("Erreur accept:");
		exit(EXIT_FAILURE);
	}
	close(sockfd);
return EXIT_SUCCESS;
}
