/*

Author : Arnab Nath
Roll : MT2022020

*/
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<string.h>
#include<errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <sys/socket.h>
extern int errno;

int main(){
	struct sockaddr_in serv;
	int sd=socket(AF_INET,SOCK_STREAM,0);
	serv.sin_family=AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
	int portno=8080;
	serv.sin_port=htons(portno);
	int nsd=connect(sd,(struct sockaddr *)&serv,sizeof(serv));
	if(nsd<0){
		perror("Error");
	}
	while(1){
		printf("Enter login option\n1:Normal User\n2:Joint Account User\n3:Administrator\n");
		int i;
		scanf("%d",&i);
		write(sd,&i,sizeof(i));
		if(i==1){
			printf("Enter phone number:\n");
			char ph_no[11];
			scanf("%s",ph_no);
			printf("Enter password:\n");
			char pswd[20];
			scanf("%s",pswd);
			write(sd,ph_no,sizeof(ph_no));
			write(sd,pswd,sizeof(pswd));
			
		}
		else if(i==2){
			printf("Enter phone number:\n");
			char ph_no[11];
			scanf("%s",ph_no);
			printf("Enter password:\n");
			char pswd[20];
			scanf("%s",pswd);
			write(sd,ph_no,sizeof(ph_no));
			write(sd,pswd,sizeof(pswd));
		
		}
		else if(i==3){
			printf("Enter phone number:\n");
			char ph_no[11];
			scanf("%s",ph_no);
			printf("Enter password:\n");
			char pswd[20];
			scanf("%s",pswd);
			write(sd,ph_no,sizeof(ph_no));
			write(sd,pswd,sizeof(pswd));
			int ret;
			read(sd,&ret,sizeof(ret));
			if(ret==0){
				printf("login failed\n");
			}
			else{
				printf("login success\n");
			}
		}
		else{
			printf("Wrong Input\n");
		}
	}
	
	
	
	close(nsd);
}
