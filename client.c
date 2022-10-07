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

int login(int sd){
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
	return ret;
}



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
	printf("Enter login option:\n1:Normal User\n2:Joint Account User\n3:Administrator\n");
	int i,ret;
	scanf("%d",&i);
	write(sd,&i,sizeof(i));
	if(i==1){
			
			
	}
	else if(i==2){
			
		
	}
	else if(i==3){
		ret=login(sd);
		if(ret==1){
			printf("Enter operation you want to perform:\n1:Add\n2:Delete\n3:Modify\n4:Search\n");
			scanf("%d",&i);
			write(sd,&i,sizeof(i));
			if(i==1){
					
			}
			else if(i==2){
					
			}
			else if(i==3){
					
			}
			else if(i==4){
					
			}
		}
	}

	close(nsd);
}
