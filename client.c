#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <openssl/aes.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SERVEURNAME "127.0.0.1" // adresse IP de mon serveur
#define PORTNUMBER 6563
int to_server_socket = -1;
int errflg = 0;
int fileflag=0;
int nameflag=0;
int passflag=0;
int createflag=0;
char *file_name;

void hexdump(FILE *f, const char *title, const unsigned char *s, int length)
{
    for(int n = 0; n < length ; ++n) {
        if((n%16) == 0)
            fprintf(f, "\n%s  %04x", title, n);
        fprintf(f, " %02x", s[n]);
    }
    fprintf(f, "\n");
}    

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}


void main (int argc, char *argv[]){

	char *server_name = SERVEURNAME;
	struct sockaddr_in serverSockAddr;
	struct hostent *serverHostEnt;
	long hostAddr;
	long status;
	char buffer[512];
	int c,sz;
	char size_array[12]; 
	char createflag_array[12];
	char fileflag_array[12];
	extern char * optarg; 
	extern int optind, opterr; 
	char *progname = argv[0];
	FILE *fp;
	FILE *fkey;
	char chaine[1024];
	char *user_name;
	char *tot_chaine;
	char **tokens;
	int size_file_name=0;
	char *final_file_name = (char *) malloc(500);
	char *file_name_tmp = (char *) malloc(500);
	char *key;
	char *password;
	char newlogin[50];
	char newpass[50];

	  while ((c = getopt(argc , argv, "f:n:p:c")) != -1)
	    switch (c) {
	    case 'f':
	      file_name = optarg;
	      strcpy(file_name_tmp,file_name);
	      tokens = str_split(file_name,'/');
	      printf("%s\n",file_name_tmp);
	      while(*(tokens+size_file_name)){
		size_file_name++;
	      }
	      strcpy(final_file_name, *(tokens+size_file_name-1));
	      fp = fopen(file_name_tmp,"r");
	      if(fp){
	      	fseek(fp, 0L, SEEK_END);
              	sz = ftell(fp);
              	fseek(fp, 0L, SEEK_SET);
	      }else{
		printf("Impossible d'ouvrir le fichier\n");
		exit(1);
	      }
	      fileflag++;
	      break;
	    case 'n':
	      user_name=optarg;
	      nameflag++;
	      break;
	    case 'p':
              password=optarg;
              passflag++;
              break;
	    case 'c':
              createflag++;
	      fileflag=0;
	      passflag=0;
	      nameflag=0;
	      printf("Entrez le login de votre compte:\n");
	      scanf("%s",newlogin);
	      printf("Entrez le mot de passe de votre compte:\n");
    	      scanf("%s",newpass);
              break;	    
	    case '?':
	      errflg++;
	      break;
	    }
	if (errflg){
	    fprintf(stderr, "usage: %s [-n arg] [-f arg]\n", progname);
	    exit(1);
        }
	if(createflag ==0){
		if(nameflag && passflag){
            		printf("Hello\t%s\n", user_name);
		}else{
	    		fprintf(stderr,"Login ou mot de passe non renseignés !!");
	    		exit(1);
        	}
	}
	unsigned char enc_out[sz];
	if(fileflag){  
	    if(!fp){
               fprintf(stderr, "Le fichier n'existe pas\n");
	       exit(1);
	    }
	    else{
 		printf("Fichier OK !!\n");
		tot_chaine = (char *) malloc(sz*sizeof(char));
		while(fgets(chaine, 1024, fp)!=NULL){ 
			strcat(tot_chaine,chaine);
		}
    		fclose(fp);
		system("openssl enc -aes-128-cbc -k secret -P -md sha1 >> key.dat");
		fkey = fopen("key.dat","r");
		char *iv;
		if(fkey){
			printf("Traitement de la clé\n");
			while(fgets(chaine, 1024, fkey)!=NULL){
			
                        	if(chaine[0]=='k'){
					strtok_r(chaine,"=",&key);
				}else if(chaine[0]=='i'){
					strtok_r(chaine,"=",&iv);
				}
                	}
			fclose(fkey);
			system("rm key.dat");
			unsigned char dec_out[sz];
		        unsigned char iv2[AES_BLOCK_SIZE];
			memset(iv2,0x00,AES_BLOCK_SIZE);
			AES_KEY enc_key,dec_key;
        		AES_set_encrypt_key((unsigned char*) key, 128, &enc_key);
			AES_cbc_encrypt((unsigned char*) tot_chaine, enc_out, sz,&enc_key, iv2, AES_ENCRYPT);
			
		        	
		}
	    } 	
	}
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
	serverSockAddr.sin_port = htons(PORTNUMBER);
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
	
	int n;	
		if((n = recv(to_server_socket,buffer,sizeof(buffer),0))==-1){
                	fprintf(stderr, "Failure Receving Message\n");
            		close(to_server_socket);
            		exit(1);
		}
                printf("%s\n",buffer);
		sprintf(fileflag_array,"%d",fileflag);
		if((n=send(to_server_socket,fileflag_array,sizeof(fileflag_array),0))==-1){
			fprintf(stderr, "Failure Sending create flag\n");
                        close(to_server_socket);
                        exit(1);
		}
	
		if((n=recv(to_server_socket,buffer,sizeof(buffer),0))==-1){
                        fprintf(stderr, "Failure Receving message conf create flag\n");
                        close(to_server_socket);
                        exit(1);
                }	
                printf("%s\n",buffer);
		sprintf(createflag_array,"%d",createflag);
		if((n=send(to_server_socket,createflag_array,sizeof(createflag_array),0))==-1){
                        fprintf(stderr, "Failure Sending create flag\n");
                        close(to_server_socket);
                        exit(1);
                }

                if((n=recv(to_server_socket,buffer,sizeof(buffer),0))==-1){
                        fprintf(stderr, "Failure Receving message conf create flag\n");
                        close(to_server_socket);
                        exit(1);
                }
                printf("%s\n",buffer);
	if(createflag>0){	
		printf("Création de votre compte en cours %s\n",newlogin);
		if((n=send(to_server_socket,newlogin, strlen(newlogin),0))==-1){
                        fprintf(stderr, "Failure Sending Login\n");
                        close(to_server_socket);
                        exit(1);
                }
                if((n = recv(to_server_socket,buffer,512,0))==-1){
                        fprintf(stderr, "Failure Receving Message\n");
                        close(to_server_socket);
                        exit(1);
                }
                printf("%s\n",buffer);
		if((n=send(to_server_socket,newpass, strlen(newpass),0))==-1){
                        fprintf(stderr, "Failure Sending password\n");
                        close(to_server_socket);
                        exit(1);
                }
                if((n = recv(to_server_socket,buffer,512,0))==-1){
                        fprintf(stderr, "Failure Receving Message\n");
                        close(to_server_socket);
                        exit(1);
                }
                printf("%s\n",buffer);

		if((n = recv(to_server_socket,buffer,512,0))==-1){
                        fprintf(stderr, "Failure Receving Message\n");
                        close(to_server_socket);
                        exit(1);
                }
                printf("%s\n",buffer);
	}

	
	if(fileflag>0){

	        if((n=send(to_server_socket,user_name, strlen(user_name),0))==-1){
                        fprintf(stderr, "Failure Sending usernamee\n");
                        close(to_server_socket);
                        exit(1);
                }
                if((n = recv(to_server_socket,buffer,512,0))==-1){
                        fprintf(stderr, "Failure Receving Message\n");
                        close(to_server_socket);
                        exit(1);
                }
                printf("%s\n",buffer);

		if((n=send(to_server_socket,password, strlen(password),0))==-1){
                        fprintf(stderr, "Failure Sending password\n");
                        close(to_server_socket);
                        exit(1);
                }
                if((n = recv(to_server_socket,buffer,512,0))==-1){
                        fprintf(stderr, "Failure Receving Message\n");
                        close(to_server_socket);
                        exit(1);
                }
                printf("%s\n",buffer);	
		
		printf("Enregistrement du fichier %s\n",final_file_name);
		if((n=send(to_server_socket,key, strlen(key),0))==-1){
			fprintf(stderr, "Failure Sending Key\n");
            		close(to_server_socket);
            		exit(1);
		}
		//printf("%s\n",strlen(key));
		if((n = recv(to_server_socket,buffer,512,0))==-1){
			fprintf(stderr, "Failure Receving Message\n");
            		close(to_server_socket);
            		exit(1);
		}
		printf("%s\n",buffer);
		if((n=send(to_server_socket,final_file_name,strlen(final_file_name),0))==-1){
			fprintf(stderr, "Failure Sending File Name\n");
                        close(to_server_socket);
                        exit(1);
		}
		
		if((n=recv(to_server_socket,buffer,512,0))==-1){
			fprintf(stderr, "Failure Receving Confirmation\n");
                        close(to_server_socket);
                        exit(1);
		}
		printf("%s\n",buffer);

		sprintf(size_array,"%d",sz);
		if((n=send(to_server_socket,size_array,sizeof(size_array),0))==-1){
		 	fprintf(stderr, "Failure Sending size file\n");
                        close(to_server_socket);
                        exit(1);	
		}
		if((n=recv(to_server_socket,buffer,512,0))==-1){
			fprintf(stderr, "Failure Receving Confirmation size\n");
                        close(to_server_socket);
                        exit(1);
		}
		printf("%s\n",buffer);
		if((n=send(to_server_socket,enc_out,sz,0))==-1){
			fprintf(stderr, "Failure Sending Cipher text\n");
                        close(to_server_socket);
                        exit(1);
		}
		if((n=recv(to_server_socket,buffer,512,0))==-1){
			fprintf(stderr, "Failure Receving Confirmation cipher text\n");
                        close(to_server_socket);
                        exit(1);
		}
		printf("%s\n",buffer);	
		if((n=recv(to_server_socket,buffer,512,0))==-1){
                        fprintf(stderr, "Failure Receving Confirmation Sauvegarde\n");
                        close(to_server_socket);
                        exit(1);
                }
                printf("%s\n",buffer);
        }
	/* fermeture de la connection */
	shutdown(to_server_socket,2);
	if(close(to_server_socket)){
		printf("Socket fermée");
	}
}

