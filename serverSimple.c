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
	bzero(&serverInfo,sizeof(serverInfo));	
	hints.ai_family = AF_INET ;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// getaddrinfo
	int errocode = getaddrinfo(
				NULL, 
				"80",
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


/* -------------------    FONCTIONS     -------------------- */

/* AUTEUR DE CETTE PARTIE : MR. VIDAL VINCENT*/

/* Lit la requete du client. Met le nom du fichier demande dans string.
* Si la syntaxe est incorrecte ou si il manque des retour charriots
* on renvoi -1. Autrement la fonction renvoie 0.
* requestFromClient est la chaine de 1000 octets censee contenir la requete provenant du client.
* requestSize doit etre egale a 1000 (et pas a la taille de la chaine de caractere). 
*/


int parseRequest(char* requestFromClient, int requestSize, char* string, int stringSize)
{
	/* charPtr[4] est un tableau de 4 pointeurs pointant sur le debut de la chaine, les 2 espaces 	*/
	/* de la requete (celui	apres le GET et celui apres le nom de fichier) et sur le premier '\r'.	*/
	/* Le pointeur end sera utilise pour mettre un '\0' a la fin du doubl retour charriot.		*/

	char *charPtr[4], *end;

	/* On cherche le double retour charriot	dans requestFromClient
	* suivant les systemes, on utilise \r ou \n (new line, new feed)
	* par convention en http on utilise les deux \r\n mais cela represente en pratique un seul retour charriot.
	* Pour simplifier ici, on ne recherche que les '\n'.
	* On placera un '\0' juste apres le double retour charriot permettant de traiter la requete 
	* comme une chaine de caractere et d'utiliser les fcts de la bibliotheque string.h. 
	*/

	/* Lecture jusqu'au double retour charriot	*/
	requestFromClient[requestSize-1]='\0';//Permet d'utiliser strchr() - attention ne marche pas si requestSize indique la taille 		de la chaine de caractere
	
	if( (end=strstr(requestFromClient,"\r\n\r\n"))==NULL) return(-1);
	*(end+4)='\0';

	// Verification de la syntaxe (GET fichier HTTP/1.1) 		
	charPtr[0]=requestFromClient;	//Debut de la requete (GET en principe)
	//On cherche le premier espace, code ascii en 0x20 (en hexa), c'est le debut du nom du fichier
	charPtr[1]=strchr(requestFromClient,' ');	
	if(charPtr[1]==NULL) return(-1);
	charPtr[2]=strchr(charPtr[1]+1,' ');	
	if(charPtr[2]==NULL) return(-1);
	charPtr[3]=strchr(charPtr[2]+1,'\r');	
	if(charPtr[3]==NULL) return(-1);

	//On separe les chaines
	*charPtr[1]='\0';
	*charPtr[2]='\0';
	*charPtr[3]='\0';

	if(strcmp(charPtr[0],"GET")!=0) return(-1);
	if(strcmp(charPtr[2]+1,"HTTP/1.1")!=0) return(-1);
	strncpy(string,charPtr[1]+2,stringSize);//On decale la chaine de 2 octets: le premier octet est le '\0', le deuxieme decalage permet de retirer le "/" 
	
	//Si stringSize n'est pas suffisement grand, la chaine ne contient pas de '\0'. Pour verifier il suffit de tester string[stringSize-1] qui
	// doit etre = '\0' car strncpy remplit la chaine avec des '\0' quand il y a de la place.
	if(string[stringSize-1]!='\0'){
		fprintf(stderr,"Erreur parseRequest(): la taille de la chaine string n'est pas suffisante (stringSize=%d)\n",stringSize);
		exit(3);
	}
	
	//DEBUG - Vous pouvez le supprimer si vous le souhaitez.
	if( *(charPtr[1]+2) == '\0') fprintf(stderr,"DEBUG-SERVEUR: le nom de fichier demande est vide -\nDEBUG-SERVEUR: - on associe donc le fichier par defaut index.html\n");
	else fprintf(stderr,"DEBUG-SERVEUR: le nom de fichier demande est %s\n",string);

	if( *(charPtr[1]+2) == '\0') strcpy(string,"index.html");
	
	return(0);
}


/* Affiche au bon format l'adresse IP passer en argument.
 * Il s'agit d'une structure sockaddr_in6 qui peut heberger en pratique une adresse IPv6 
 * ou bien une adresse IPv4 mappee. 
 * Cette fonction est particulierement utile pour afficher les adresses IPs des clients qui
 * se connecte sur un serveur utilisant une socket AF_INET6 (double stack).
*/

void printClientInfo(struct sockaddr_in6 clientAddr)
{
	/* La macro ci-dessous permet de verifier la version du protocole utilise par le client
	*  si c'est de l'ipv4, l'adresse IPv4 est mappee (mapped IPv4 address) dans une adresse ipv6:
	*  sous la forme ::ffff:<IPv4 address>
	*/

	char buffer[40];

	if(IN6_IS_ADDR_V4MAPPED(&clientAddr.sin6_addr))
	{
		fprintf(stderr,"--DEBUG-- IPv4 mapped address\n");
		//Declaration d'une sockaddr IPv4 dans lequel on va faire la copie
		struct sockaddr_in addr4;
        	memset(&addr4,0,sizeof(addr4));
        	addr4.sin_family=AF_INET;
        	addr4.sin_port=clientAddr.sin6_port;
		//On decale de 12 octets - pour arriver aux 4 derniers octets de l'adresse
		//copie-colle des 32 derniers bits de l'adresse IPv6 dans l'adresse IPv4
		//Attention au cast (char *) ci-dessous, necessaire pour decaler de 12 octets et pas 12 fois la taille de s6_addr
        	memcpy(&(addr4.sin_addr.s_addr), (char *) &(clientAddr.sin6_addr.s6_addr)+12,sizeof(addr4.sin_addr.s_addr));

		//Affichage:
		if((char *) inet_ntop(AF_INET,&(addr4.sin_addr),buffer,sizeof(buffer))==NULL)
			perror("Error on function inet_ntop() in the printClientInfo() function");

		fprintf(stderr,"-- Serveur logs -- connection from %s\n",buffer);

	} else { //IPv6
		//Affichage 
		fprintf(stderr,"--DEBUG-- IPv6 address\n");
		if(inet_ntop(AF_INET6,&(clientAddr.sin6_addr),buffer,sizeof(buffer))<0)
			perror("Error on function inet_ntop() in the printClientInfo() function");

		fprintf(stderr,"-- Serveur logs -- connection from %s\n",buffer);
	}


}//fin de printClientInfo()


