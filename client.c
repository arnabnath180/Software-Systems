/*

Author : Arnab Nath
Roll : MT2022020

*/

#include<stdbool.h>
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
#include<time.h>
extern int errno;

struct user{
	int type;
	char name[50],ph_no[11],pswd[20],name2[50];
	long amount;
	int account_no;
	bool status;
}; 

struct transaction{
	int account_no;
	int amount;
	struct tm time;
	bool debited;
	bool credited;
};

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

int Add(int sd){
	printf("Enter account type:\n1:Normal user\n2:Joint account\n");
	int type;
	int ret;
	scanf("%d",&type);
	getchar();
	write(sd,&type,sizeof(type));
	if(type!=1 && type!=2){
		read(sd,&ret,sizeof(ret));
		printf("Adding failed\n");
		return ret;
	}
	printf("Enter account holder name:\n");
	char name[50];
	fgets(name,50,stdin);
	name[strlen(name)-1]='\0';
	write(sd,name,sizeof(name));
	if(type==2){
		printf("Enter account holder 2 name:\n");
		char name2[50];
		fgets(name2,50,stdin);
		name2[strlen(name2)-1]='\0';
		write(sd,name2,sizeof(name2));
	}
	printf("Enter amount:\n");
	long amount;
	scanf("%ld",&amount);
	getchar();
	write(sd,&amount,sizeof(amount));
	printf("Enter phone number:\n");
	char ph_no[11];
	fgets(ph_no,11,stdin);
	getchar();
	write(sd,ph_no,sizeof(ph_no));
	printf("Enter password:\n");
	char pswd[20];
	fgets(pswd,20,stdin);
	pswd[strlen(pswd)-1]='\0';
	write(sd,pswd,sizeof(pswd));
	read(sd,&ret,sizeof(ret));
	printf("Account added successfully\n");
	printf("Account number:%d\n",ret);
	return ret;
} 

int Search(int sd){
	struct user u;
	struct transaction tr;
	printf("Enter account type:\n1:Normal user\n2:Joint account\n");
	int type;
	int ret;
	scanf("%d",&type);
	write(sd,&type,sizeof(type));
	if(type!=1 && type!=2){
		read(sd,&ret,sizeof(ret));
		printf("Search failed\n");
		return ret;
	}
	printf("Enter account number:\n");
	int account_no;
	scanf("%d",&account_no);
	write(sd,&account_no,sizeof(account_no));
	read(sd,&ret,sizeof(ret));
	if(ret==1){
		read(sd,&u,sizeof(u));
		printf("Name:%s\n",u.name);
		if(type==2){
			printf("Name2:%s\n",u.name2);
		}
		printf("Phone number:%s\n",u.ph_no);
		printf("Amount:%ld\n",u.amount);
		printf("Account number:%d\n",u.account_no);
		printf("Transactions Details:\n");
		while(1){
			read(sd,&tr,sizeof(tr));
			if(tr.account_no==0) 
				break;	
			if(tr.debited==true)
				printf("Debited\n");
			if(tr.credited==true)
				printf("Credited\n");
			printf("Amount:%d\n",tr.amount);
			printf("Time:%d:%d:%d\n",tr.time.tm_hour,tr.time.tm_min,tr.time.tm_sec);
			printf("Time:%d:%d:%d\n",tr.time.tm_year+1900,tr.time.tm_mon+1,tr.time.tm_mday);
		}
		return ret;
	}
	printf("Search failed\n");
	return ret;
}

int Modify(int sd){
	struct user u;
	printf("Enter account type:\n1:Normal user\n2:Joint account\n");
	int type;
	int ret,i;
	scanf("%d",&type);
	write(sd,&type,sizeof(type));
	if(type!=1 && type!=2){
		read(sd,&ret,sizeof(ret));
		printf("Search failed\n");
		return ret;
	}
	printf("Enter account number\n");
	int account_no;
	scanf("%d",&account_no);
	write(sd,&account_no,sizeof(account_no));
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Search failed\n");
		return ret;
	}
	read(sd,&u,sizeof(u));
	printf("Name:%s\n",u.name);
	if(type==2){
		printf("Name2:%s\n",u.name2);
	}
	printf("Phone number:%s\n",u.ph_no);
	printf("Amount:%ld\n",u.amount);
	printf("Account number:%d\n",u.account_no);
	printf("Change username\n1:Yes\n2:No\n");
	scanf("%d",&i);
	if(i==1){
		printf("Enter Name:\n");
		getchar();
		fgets(u.name,50,stdin);
		u.name[strlen(u.name)-1]='\0';
	}
	else{
		u.name[0]='\0';
	}
	if(type==2){
		printf("Change username2\n1:Yes\n2:No\n");
		scanf("%d",&i);
		if(i==1){
			printf("Enter Name:\n");
			getchar();
			fgets(u.name2,50,stdin);
			u.name2[strlen(u.name2)-1]='\0';
		}
		else{
			u.name2[0]='\0';
		}
	}
	printf("Change phone number\n1:Yes\n2:No\n");
	scanf("%d",&i);
	if(i==1){
		printf("Enter phone number:\n");
		getchar();
		fgets(u.ph_no,11,stdin);
	}
	else{
		u.ph_no[0]='\0';
	}
	write(sd,&u,sizeof(u));
	read(sd,&ret,sizeof(ret));
	return ret;
}

int Delete(int sd){
	struct user u;
	printf("Enter account type:\n1:Normal user\n2:Joint account\n");
	int type;
	int ret,i;
	scanf("%d",&type);
	write(sd,&type,sizeof(type));
	if(type!=1 && type!=2){
		read(sd,&ret,sizeof(ret));
		printf("Search failed\n");
		return ret;
	}
	printf("Enter account number\n");
	int account_no;
	scanf("%d",&account_no);
	write(sd,&account_no,sizeof(account_no));
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Search failed\n");
		return ret;
	}
	read(sd,&u,sizeof(u));
	printf("Name:%s\n",u.name);
	if(type==2){
		printf("Name2:%s\n",u.name2);
	}
	printf("Phone number:%s\n",u.ph_no);
	printf("Amount:%ld\n",u.amount);
	printf("Account number:%d\n",u.account_no);
	read(sd,&ret,sizeof(ret));
	if(ret==1){
		printf("Deleted Successfully\n");
	}
	return ret;
}

int login_user(int sd){
	printf("Enter account number:\n");
	int account_no;
	scanf("%d",&account_no);
	printf("Enter password:\n");
	char pswd[20];
	scanf("%s",pswd);
	write(sd,&account_no,sizeof(account_no));
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
		ret=login_user(sd);
		if(ret==1){
			while(1){
				printf("Enter operation you want to perform:\n1:Deposit\n2:Withdraw\n3:Balance Ecquiry\n4:Password Change\n5:View Details\n6:Exit\n");
				scanf("%d",&i);
				write(sd,&i,sizeof(i));
				if(i==1){
					//ret=Deposit(sd);					
				}
				else if(i==2){
					//ret=Withdraw(sd);
				}
				else if(i==3){
					//ret=BalanceEnquiry(sd);	
				}
				else if(i==4){
					//ret=PasswordChange(sd);	
				}
				else if(i==5){
					//ret=ViewDetails(sd);	
				}
				else{
					break;
				}
			}
		}			
	}
	else if(i==2){
			
		
	}
	else if(i==3){
		ret=login(sd);
		if(ret==1){
			while(1){
				printf("Enter operation you want to perform:\n1:Add\n2:Delete\n3:Modify\n4:Search\n5:Exit\n");
				scanf("%d",&i);
				write(sd,&i,sizeof(i));
				if(i==1){
					ret=Add(sd);					
				}
				else if(i==2){
					ret=Delete(sd);
				}
				else if(i==3){
					ret=Modify(sd);	
				}
				else if(i==4){
					ret=Search(sd);	
				}
				else{
					break;
				}
			}
		}
	}
	close(nsd);
}
