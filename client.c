#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#define SERVEURNAME "127.0.0.1" // adresse IP de mon serveur

int to_server_socket = -1;
int errflg = 0;
int fileflag=0;
int nameflag=0;

void main (int argc, char *argv[]){

	char *server_name = SERVEURNAME;
	struct sockaddr_in serverSockAddr;
	struct hostent *serverHostEnt;
	long hostAddr;
	long status;
	char buffer[512];
	int c,sz;
	char size_array[12]; 
	extern char * optarg; 
	extern int optind, opterr; 
	char *progname = argv[0];
	FILE *fp;
	char *file_name;
	char *user_name;
	char *tot_chaine;
	  while ((c = getopt(argc , argv, "f:n:")) != -1)
	    switch (c) {
	    case 'f':
	      file_name = optarg;
	      fileflag++;
	      break;
	    case 'n':
	      user_name=optarg;
	      nameflag++;
	      break;
	    case '?':
	      errflg++;
	      break;
	    }
	if (errflg){
	    fprintf(stderr, "usage: %s [-n arg] [-f arg]\n", progname);
	    exit(1);
        }
	if(fileflag){    
	    printf("%s\n", file_name);
	    fp = fopen(file_name,"r");
	    if(!fp){
               fprintf(stderr, "Le fichier n'existe pas\n");
	       exit(1);
	    }
	    else{
 		printf("Fichier OK !!\n");
		fseek(fp, 0L, SEEK_END);
		sz = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		char chaine[1024];
		tot_chaine = (char *) malloc(sz*sizeof(char));
		while(fgets(chaine, 1024, fp)!=NULL){ 
			strcat(tot_chaine,chaine);
		}
    		fclose(fp);
	    } 	
	}
	if(nameflag)
            printf("Hello\t%s\n", user_name);

	bzero(&serverSockAddr,sizeof(serverSockAddr));
	hostAddr = inet_addr(SERVEURNAME);
	if ( (long)hostAddr != (long)-1)
		bcopy(&hostAddr,&serverSockAddr.sin_addr,sizeof(hostAddr));
	else
	{
		serverHostEnt = gethostbyname(SERVEURNAME);
		if (serverHostEnt == NULL)
		{
			printf("gethost rate\n");
			exit(0);
		}
		bcopy(serverHostEnt->h_addr,&serverSockAddr.sin_addr,serverHostEnt->h_length);
	}
	serverSockAddr.sin_port = htons(3672);
	serverSockAddr.sin_family = AF_INET;

	if ( (to_server_socket = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("creation socket client ratee\n");
		exit(0);
	}

	if(connect(to_server_socket,(struct sockaddr *)&serverSockAddr,sizeof(serverSockAddr)) < 0 )
	{
		printf("demande de connection ratee\n");
		exit(1);
	}
        if(fileflag>0){
		//printf("%s\n",tot_chaine);
		write(to_server_socket,file_name,strlen(file_name));
		read(to_server_socket,buffer,512);
		printf("%s\n",buffer);
		sprintf(size_array,"%d",sz);
		write(to_server_socket,size_array,strlen(size_array));
		read(to_server_socket,buffer,512);
		printf("%s\n",buffer);	
		write(to_server_socket,tot_chaine,sz);
		read(to_server_socket,buffer,512);
		printf("%s\n",buffer);	

        }
	/* fermeture de la connection */
	shutdown(to_server_socket,2);
	if(close(to_server_socket)){
		printf("Socket fermée");
	}
}

