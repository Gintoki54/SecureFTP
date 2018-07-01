#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#define PORTNUMBER 6554
void fin (int i);
char buffer[512];
int ma_socket;
FILE *accounts;
void hexdump(FILE *f, const char *title, const unsigned char *s, int length)
{
    for(int n = 0; n < length ; ++n) {
        if((n%16) == 0)
            fprintf(f, "\n%s  %04x", title, n);
        fprintf(f, " %02x", s[n]);
    }
    fprintf(f, "\n");
}


void main (int argc, char *argv[]){
	int client_socket;
	struct sockaddr_in mon_address, client_address;
	int mon_address_longueur, lg,size_file;
	char file_name[512];
	//char content_file[5000];
	bzero(&mon_address,sizeof(mon_address));
	mon_address.sin_port = htons(PORTNUMBER);
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
	char *key = (char *) malloc(32);
	int createflag=0;
	int fileflag=0;
	char newlogin[50];
	char newpass[50];
	memset(newlogin,0,50);
	memset(newpass,0,50);
	while(1){
		client_socket = accept(ma_socket,
		(struct sockaddr *)&client_address,
		&mon_address_longueur);
		if (fork() == 0){
			close(ma_socket);
			if((lg=send(client_socket,"Connecté au serveur",111,0))==-1){
				fprintf(stderr, "Failure Sending Message\n");
                        	close(client_socket);
                        	exit(1);
			}
			if((lg=recv(client_socket,buffer,512,0))==-1){
                                fprintf(stderr, "Failure Receving File flag\n");
                                close(client_socket);
                                exit(1);
                        }
			fileflag = atoi(buffer);
			printf("File Flag=%d\n",fileflag);
			if((lg=send(client_socket,"File flag reçu",111,0))==-1){
                                fprintf(stderr, "Failure Sending Message\n");
                                close(client_socket);
                                exit(1);
                        }
			if((lg=recv(client_socket,buffer,512,0))==-1){
                                fprintf(stderr, "Failure Receving Create flag\n");
                                close(client_socket);
                                exit(1);
                        }
                        createflag = atoi(buffer);
                        printf("Create Flag=%d\n",createflag);
                        if((lg=send(client_socket,"Create flag reçu",111,0))==-1){
                                fprintf(stderr, "Failure Sending Message\n");
                                close(client_socket);
                                exit(1);
                        }
			if(createflag>0){
				fileflag=0;
				if((lg=recv(client_socket,newlogin,512,0))==-1){
                                	fprintf(stderr, "Failure Receving Login\n");
                                	close(client_socket);
                                	exit(1);
                        	}
				printf("%s\n",newlogin);
				int is_present=0;
				char cmd_dir[50] = "mkdir ./Server/";
                                strcat(cmd_dir,newlogin);
                                is_present = system(cmd_dir);
				printf("%d\n",is_present);
				if(is_present == 256){
                                	if((lg=send(client_socket,"Login déjà pris",51,0))==-1){
                                        	fprintf(stderr, "Failure Sending Confirmation\n");
                                        	close(client_socket);
                                        	exit(1);
                                	}
				}
				else{
					if((lg=send(client_socket,"new login reçu ",111,0))==-1){
						fprintf(stderr, "Failure Sending Message\n");
						close(client_socket);
						exit(1);
					}

					
					if((lg=recv(client_socket,newpass,512,0))==-1){
						fprintf(stderr, "Failure Receving Password\n");
						close(client_socket);
						exit(1);
					}
					printf("%s\n",newpass);

					if((lg=send(client_socket,"new password reçu ",111,0))==-1){
						fprintf(stderr, "Failure Sending Message\n");
						close(client_socket);
						exit(1);
					}
					 EVP_MD_CTX *mdctx;
                                	mdctx = EVP_MD_CTX_create();
                                	unsigned char digest[EVP_MAX_MD_SIZE];
                                	unsigned int digest_len;
                                	int i;
                                	EVP_DigestInit(mdctx, EVP_sha256());
                                	EVP_DigestUpdate(mdctx, newpass, strlen(newpass));
                                	EVP_DigestFinal(mdctx, digest, &digest_len);
                                	EVP_MD_CTX_destroy(mdctx);
                                	char hash[digest_len];
                                	memset(hash,0,digest_len);
                                	char hash_tmp[2];
                                	for(i=0;i<digest_len;i++){
                                        	sprintf(hash_tmp,"%02x",digest[i]);
                                        	strcat(hash,hash_tmp);
                                	}

                                	printf("%s\n",hash);

					accounts = fopen("accounts.txt","a");
					char account[512];
					strcpy(account,newlogin);
					strcat(account,":");
					strcat(account,hash);
					fputs(account,accounts);
					fclose(accounts);
					if((lg=send(client_socket,"Compte crée",51,0))==-1){
						fprintf(stderr, "Failure Sending Confirmation\n");
						close(client_socket);
						exit(1);
					}
				}
			}
			if(fileflag>0){
				char current_username[50];
				char current_password[50];
				memset(current_username,0,50);
				memset(current_password,0,50);
				if((lg=recv(client_socket,current_username,512,0))==-1){
                                        fprintf(stderr, "Failure Receving user name\n");
                                        close(client_socket);
                                        exit(1);
                                }
				printf("%s\n",current_username);
				if((lg=send(client_socket,"Username reçu",51,0))==-1){
                                        fprintf(stderr, "Failure Sending Confirmation\n");
                                        close(client_socket);
                                        exit(1);
                                }

				if((lg=recv(client_socket,current_password,512,0))==-1){
                                        fprintf(stderr, "Failure Receving password\n");
                                        close(client_socket);
                                        exit(1);
                                }
                                //printf("%s\n",current_password);
                                if((lg=send(client_socket,"Password reçu",51,0))==-1){
                                        fprintf(stderr, "Failure Sending Confirmation\n");
                                        close(client_socket);
                                        exit(1);
                                }
				EVP_MD_CTX *mdctx;
				mdctx = EVP_MD_CTX_create();
				unsigned char digest[EVP_MAX_MD_SIZE];
				unsigned int digest_len;
				int i;
				EVP_DigestInit(mdctx, EVP_sha256());
				EVP_DigestUpdate(mdctx, current_password, strlen(current_password));
				EVP_DigestFinal(mdctx, digest, &digest_len);
				EVP_MD_CTX_destroy(mdctx);
				char hash[digest_len];
				memset(hash,0,digest_len);
				char hash_tmp[2];
				for(i=0;i<digest_len;i++){
					sprintf(hash_tmp,"%02x",digest[i]);
					strcat(hash,hash_tmp);
				}
				hash[strlen(hash)]='\0';
				
				//printf("%s\n",hash);
				FILE *accounts;
				accounts = fopen("accounts.txt","r");
				char account[512];
				char *account_username;
				char *account_hash;
				int auth = 0;
				int user_cmp;
				int pass_cmp;
				while(fgets(account, 512, accounts)!=NULL){
					account_username = strtok_r(account,":",&account_hash);
					account_hash[strlen(account_hash)-1] = '\0';
					if(strcmp(account_username,current_username)==0){
						if(strcmp(account_hash,hash)==0){
							auth=1;
							break;
						}
					}	
                                	
                        	}
				fclose(accounts);
				printf("Auth = %d\n",auth);
				if(auth==0){
					 if((lg=send(client_socket,"Mot de passe ou login incorrects",512,0))==-1){
                                                fprintf(stderr, "Failure Sending Confirmation\n");
                                                close(client_socket);
                                                exit(1);
                                        }
					

				}else{
					
					if((lg=recv(client_socket,buffer,512,0))==-1){
						fprintf(stderr, "Failure Receving Key\n");
						close(client_socket);
						exit(1);
					}
					
					strcpy(key,buffer);
					//printf("%s\n",key);
					memset(buffer,0,512);
					if((lg=send(client_socket,"Key reçue",51,0))==-1){
						fprintf(stderr, "Failure Sending Confirmation\n");
						close(client_socket);
						exit(1);
					}
					if((lg=recv(client_socket,file_name,512,0))==-1){
						fprintf(stderr, "Failure Receving File name\n");
						close(client_socket);
						exit(1);
					}
					//strcpy(file_name,buffer);
					//printf("%s\n",file_name);
					memset(buffer,0,512);
					if((lg=send(client_socket,"Nom du fichier reçu",100,0))==-1){
						fprintf(stderr, "Failure Sending Confirmation file\n");
						close(client_socket);
						exit(1);
					}
					if((lg=recv(client_socket,buffer,512,0))==-1){
						fprintf(stderr, "Failure Receving Size file\n");
						close(client_socket);
						exit(1);	
					}
					size_file = atoi(buffer);
					printf("%d\n",size_file);
					if((lg=send(client_socket,"Taille du fichier reçu !!!",512,0))==-1){
						fprintf(stderr, "Failure Sending Confirmation size file\n");
						close(client_socket);
						exit(1);
					}
					unsigned char cipher_content[size_file];
					
					if((lg=recv(client_socket,cipher_content,size_file,0))==-1){
						fprintf(stderr, "Failure Receving Cipher text\n");
						close(client_socket);
						exit(1);
					}
					//strcpy(content_file,buffer);
					//printf("%s\n",content_file);
					//hexdump(stdout, "AES_ENCRYPT", (unsigned char*) &cipher_content, sizeof(cipher_content));
					if((lg=send(client_socket,"Contenu du fichier reçu !!!",512,0))==-1){
						fprintf(stderr, "Failure Sending Confirmation cipher text\n");
						close(client_socket);
						exit(1);
					}
					unsigned char dec_out[size_file];
					unsigned char iv[AES_BLOCK_SIZE];
					memset(iv,0,AES_BLOCK_SIZE);
					AES_KEY dec_key;
					
					AES_set_decrypt_key((unsigned char*) key,128, &dec_key);
					AES_cbc_encrypt(cipher_content, dec_out, size_file, &dec_key, iv, AES_DECRYPT);

					char *chaine_dechiffree = (char *) dec_out;
					//printf("%s\n",chaine_dechiffree);
				}
			}
			shutdown(client_socket,2);
			close(client_socket);
			exit(0);
		}
                /*memset(file_name,0,500);
		memset(content_file,0,size_file);
		size_file=0;*/
	}
	shutdown(ma_socket,2);
	close(ma_socket);

}
void fin(int i)
{
shutdown(ma_socket,2);
close(ma_socket);
}
