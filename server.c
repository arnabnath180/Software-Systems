/*

Author : Arnab Nath
Roll : MT2022020

*/

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
#include<stdlib.h>
extern int errno;

struct user{
	int type;
	char name[50],ph_no[11],pswd[20],name2[50];
	long amount;
	long account_no;
}; 
int administrator_login(int nsd){
	struct administrator{
		char ph_no[11];
		char pswd[20];
	}administrator_db;
	char ph_no[11],pswd[20];
	int fd;
	int nbytes;
	int ret;
	read(nsd,ph_no,sizeof(ph_no));
	read(nsd,pswd,sizeof(pswd));
	fd=open("administrator_db",O_RDWR);
	while(nbytes=read(fd,&administrator_db,sizeof(administrator_db))){
		if(!strcmp(administrator_db.ph_no,ph_no) && !strcmp(administrator_db.pswd,pswd)){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			break;
		}
	}
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
	}
	return ret;
}

int Add(int nsd){
	struct user u;
	int ret;
	read(nsd,&u.type,sizeof(u.type));
	if(u.type!=1 && u.type!=2){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	read(nsd,u.name,sizeof(u.name));
	if(u.type==2){
		read(nsd,u.name2,sizeof(u.name2));
	}
	read(nsd,&u.amount,sizeof(u.amount));
	read(nsd,u.ph_no,sizeof(u.ph_no));
	read(nsd,u.pswd,sizeof(u.pswd));
	if(u.type==2){
		int fd=open("joint_account_user_db",O_RDWR);
		lseek(fd,(-1)*sizeof(u),SEEK_END);
		struct user tmp;
		if(read(fd,&tmp,sizeof(tmp))){
			u.account_no=tmp.account_no+1;
		}
		else{
			u.account_no=100;
		}
		lseek(fd,0,SEEK_END);
		write(fd,&u,sizeof(u));
		close(fd);
	}
	else{
		int fd=open("normal_user_db",O_RDWR);
		lseek(fd,(-1)*sizeof(u),SEEK_END);
		struct user tmp;
		if(read(fd,&tmp,sizeof(tmp))){
			u.account_no=tmp.account_no+1;
		}
		else{
			u.account_no=1000;
		}
		lseek(fd,0,SEEK_END);
		write(fd,&u,sizeof(u));
		close(fd);
	}
	ret=u.account_no;
	write(nsd,&ret,sizeof(ret));
	return ret;
}
int main(){
	int fd=open("normal_user_db",O_RDWR|O_CREAT|O_EXCL,0764);
	close(fd);
	fd=open("joint_account_user_db",O_RDWR|O_CREAT|O_EXCL,0764);
	close(fd);
	fd=open("administrator_db",O_RDWR|O_CREAT|O_EXCL,0764);
	if(fd!=-1){
		struct administrator{
			char ph_no[11];
			char pswd[20];
		}administrator_db;
		printf("Enter administrator phone number:\n");
		scanf("%s",administrator_db.ph_no);
		printf("Enter password:\n");
		scanf("%s",administrator_db.pswd);
		write(fd,&administrator_db,sizeof(administrator_db));
	}
	close(fd);
	struct sockaddr_in serv,cli;
	int sd=socket(AF_INET,SOCK_STREAM,0);
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=INADDR_ANY;
	int portno;
	printf("Enter port number:\n");
	scanf("%d",&portno);
	serv.sin_port=htons(portno);
	bind(sd,(struct sockaddr*)&serv,(socklen_t)sizeof(serv));
	listen(sd,5);
	int size=sizeof(cli);
	while(1){
		int nsd=accept(sd,(struct sockaddr *)&cli,(socklen_t*)&size);
		if(!fork()){
			close(sd);
			int i,ret;
			read(nsd,&i,sizeof(i));
			if(i==1){
				
			}
			else if(i==2){
				
			}
			else if(i==3){
				ret=administrator_login(nsd);
				if(ret==1){
					read(nsd,&i,sizeof(i));
					if(i==1){
						ret=Add(nsd);
						if(ret==1){
							
						}
					}
					else if(i==2){
					
					}
					else if(i==3){
					
					}
					else if(i==4){
					
					}
				}
				
			}
			exit(0);
		}
		else{
			close(nsd);
		}
	}
}
