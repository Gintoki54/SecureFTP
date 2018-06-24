#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>

void fin (int i);
char buffer[4096];
int ma_socket;

void main (int argc, char *argv[]){
	int client_socket;
	struct sockaddr_in mon_address, client_address;
	int mon_address_longueur, lg,size_file;
	char file_name[500];
	char content_file[5000];
	bzero(&mon_address,sizeof(mon_address));
	mon_address.sin_port = htons(3672);
	mon_address.sin_family = AF_INET;
	mon_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((ma_socket = socket(AF_INET,SOCK_STREAM,0))== -1)
	{
	printf("la creation rate\n");
	exit(0);
	}
	signal(SIGINT,fin);
	
	bind(ma_socket,(struct sockaddr *)&mon_address,sizeof(mon_address));

	/* ecoute sur la socket */
	listen(ma_socket,5);
	/* accept la connexion */
	mon_address_longueur = sizeof(client_address);
	while(1){
		client_socket = accept(ma_socket,
		(struct sockaddr *)&client_address,
		&mon_address_longueur);
		if (fork() == 0){
			close(ma_socket);

			lg = read(client_socket,buffer, 512);
			strcpy(file_name,buffer);
			//sprintf(buffer,"%s du serveur",buffer);
			write(client_socket,"Nom du fichier reçu",512);
			lg=read(client_socket,buffer,512);
				//strcat(content_file,buffer);
			size_file = atoi(buffer);
		        printf("%d\n",size_file);	
			write(client_socket,"Taille du fichier reçu !!!",512);	
			lg=read(client_socket,buffer,size_file);
		        strcpy(content_file,buffer);
			printf("%s\n",content_file);
			write(client_socket,"Contenu du fichier reçu !!!",512);	
			shutdown(client_socket,2);
			close(client_socket);
			exit(0);
		}
                memset(file_name,0,500);
		memset(content_file,0,size_file);
		size_file=0;
	}
	shutdown(ma_socket,2);
	close(ma_socket);

}
void fin(int i)
{
shutdown(ma_socket,2);
close(ma_socket);
}
