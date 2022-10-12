/*

Author : Arnab Nath
Roll : MT2022020

*/

#include<stdbool.h>
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

void lock(int fd,short ltype){
	struct flock lock;
	lock.l_type=ltype;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	fcntl(fd,F_SETLKW,&lock);
}

void unlock(int fd){
	struct flock lock;
	lock.l_type=F_UNLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	fcntl(fd,F_SETLK,&lock);
}



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
	lock(fd,F_RDLCK);
	while(nbytes=read(fd,&administrator_db,sizeof(administrator_db))){
		if(!strcmp(administrator_db.ph_no,ph_no) && !strcmp(administrator_db.pswd,pswd)){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			break;
		}
	}
	unlock(fd);
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
		lock(fd,F_WRLCK);
		lseek(fd,(-1)*sizeof(u),SEEK_END);
		struct user tmp;
		if(read(fd,&tmp,sizeof(tmp))){
			u.account_no=tmp.account_no+2;
		}
		else{
			u.account_no=100;
		}
		lseek(fd,0,SEEK_END);
		u.status=true;
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("normal_user_db",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,(-1)*sizeof(u),SEEK_END);
		struct user tmp;
		if(read(fd,&tmp,sizeof(tmp))){
			u.account_no=tmp.account_no+2;
		}
		else{
			u.account_no=101;
		}
		lseek(fd,0,SEEK_END);
		u.status=true;
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	int fd=open("transactions_db",O_RDWR);
	lock(fd,F_WRLCK);
	lseek(fd,0,SEEK_END);
	struct transaction tr;
	tr.account_no=u.account_no;
	tr.amount=u.amount;
	tr.debited=false;
	tr.credited=true;
	time_t t=time(NULL);
	struct tm *tm=localtime(&t);
	tr.time.tm_sec=tm->tm_sec;
	tr.time.tm_min=tm->tm_min;
	tr.time.tm_hour=tm->tm_hour;
	tr.time.tm_year=tm->tm_year;
	tr.time.tm_mon=tm->tm_mon;
	tr.time.tm_mday=tm->tm_mday;
	write(fd,&tr,sizeof(tr));
	unlock(fd);
	close(fd);
	ret=u.account_no;
	write(nsd,&ret,sizeof(ret));
	return ret;
}

int Search(int nsd){
	struct user u;
	int ret;
	int type;
	read(nsd,&type,sizeof(type));
	if(type!=1 && type!=2){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	int account_no;
	read(nsd,&account_no,sizeof(account_no));
	if(type==1){
		int fd=open("normal_user_db",O_RDONLY);
		lock(fd,F_RDLCK);
		while(read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				unlock(fd);
				close(fd);
				fd=open("transactions_db",O_RDONLY);
				lock(fd,F_RDLCK);
				struct transaction tr;
				while(read(fd,&tr,sizeof(tr))){
					if(tr.account_no==account_no){
						write(nsd,&tr,sizeof(tr));	
					}
				}
				tr.account_no=0;
				write(nsd,&tr,sizeof(tr));	
				unlock(fd);
				close(fd);
				return 1;
			}
		}
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db",O_RDONLY);
		lock(fd,F_RDLCK);
		while(read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				unlock(fd);
				close(fd);
				fd=open("transactions_db",O_RDONLY);
				lock(fd,F_RDLCK);
				struct transaction tr;
				while(read(fd,&tr,sizeof(tr))){
					if(tr.account_no==account_no){
						write(nsd,&tr,sizeof(tr));	
					}
				}
				tr.account_no=0;
				write(nsd,&tr,sizeof(tr));	
				unlock(fd);
				close(fd);
				return 1;	
			}
		}
		unlock(fd);
		close(fd);
	}
	ret=0;
	write(nsd,&ret,sizeof(ret));
	return 0;
}

int Modify(int nsd){
	int type,ret,nbytes,nr=0;
	struct user u;
	read(nsd,&type,sizeof(type));
	if(type!=1 && type!=2){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	int account_no;
	read(nsd,&account_no,sizeof(account_no));
	if(type==1){
		int fd=open("normal_user_db",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;	
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	read(nsd,&u,sizeof(u));
	if(type==1){
		int fd=open("normal_user_db",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		struct user u1;
		read(fd,&u1,sizeof(u1));
		lseek(fd,(-1)*sizeof(u),SEEK_CUR);
		if(strlen(u.name)==0){
			strcpy(u.name,u1.name);
		}
		if(strlen(u.ph_no)==0){
			strcpy(u.ph_no,u1.ph_no);
		}
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		struct user u1;
		read(fd,&u1,sizeof(u1));
		lseek(fd,(-1)*sizeof(u),SEEK_CUR);
		if(strlen(u.name)==0){
			strcpy(u.name,u1.name);
		}
		if(strlen(u.name2)==0){
			strcpy(u.name2,u1.name2);
		}
		if(strlen(u.ph_no)==0){
			strcpy(u.ph_no,u1.ph_no);
		}
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return 1;
}

int Delete(int nsd){
	int type,ret,nbytes,nr=0;
	struct user u;
	read(nsd,&type,sizeof(type));
	if(type!=1 && type!=2){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	int account_no;
	read(nsd,&account_no,sizeof(account_no));
	if(type==1){
		int fd=open("normal_user_db",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;	
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	if(type==1){
		int fd=open("normal_user_db",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		read(fd,&u,sizeof(u));
		u.status=false;
		lseek(fd,nr*sizeof(u),SEEK_SET);
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		read(fd,&u,sizeof(u));
		u.status=false;
		lseek(fd,nr*sizeof(u),SEEK_SET);
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return 1;
}

int login_user(int nsd){
	struct user u;
	int account_no;
	int fd;
	int nbytes;
	int ret;
	char pswd[20];
	read(nsd,&account_no,sizeof(account_no));
	read(nsd,pswd,sizeof(pswd));
	if(account_no & 1)
		fd=open("normal_user_db",O_RDONLY);
	else
		fd=open("joint_account_user_db",O_RDONLY);
	lock(fd,F_RDLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && !strcmp(u.pswd,pswd) && u.status==true){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			break;
		}
	}
	unlock(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	return account_no;	
}

int Deposit(int nsd,int account_no){
	int amount,nbytes,ret;
	struct user u;
	struct transaction tr;
	read(nsd,&amount,sizeof(amount));
	int fd;
	if(account_no & 1)
		fd=open("normal_user_db",O_RDWR);
	else
		fd=open("joint_account_user_db",O_RDWR);
	lock(fd,F_WRLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true){
			int fd1=open("transactions_db",O_RDWR);
			lock(fd1,F_WRLCK);
			lseek(fd1,0,SEEK_END);
			tr.account_no=u.account_no;
			tr.amount=amount;
			tr.debited=false;
			tr.credited=true;
			time_t t=time(NULL);
			struct tm *tm=localtime(&t);
			tr.time.tm_sec=tm->tm_sec;
			tr.time.tm_min=tm->tm_min;
			tr.time.tm_hour=tm->tm_hour;
			tr.time.tm_year=tm->tm_year;
			tr.time.tm_mon=tm->tm_mon;
			tr.time.tm_mday=tm->tm_mday;
			write(fd1,&tr,sizeof(tr));
			unlock(fd1);
			close(fd1);
			u.amount=u.amount+amount;
			lseek(fd,(-1)*sizeof(u),SEEK_CUR);
			write(fd,&u,sizeof(u));
			break;	
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return ret;
}

int Withdraw(int nsd,int account_no){
	int amount,nbytes,ret;
	struct user u;
	struct transaction tr;
	read(nsd,&amount,sizeof(amount));
	int fd;
	if(account_no & 1)
		fd=open("normal_user_db",O_RDWR);
	else
		fd=open("joint_account_user_db",O_RDWR);
	lock(fd,F_WRLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true && amount<=u.amount){
			int fd1=open("transactions_db",O_RDWR);
			lock(fd1,F_WRLCK);
			lseek(fd1,0,SEEK_END);
			tr.account_no=u.account_no;
			tr.amount=amount;
			tr.debited=true;
			tr.credited=false;
			time_t t=time(NULL);
			struct tm *tm=localtime(&t);
			tr.time.tm_sec=tm->tm_sec;
			tr.time.tm_min=tm->tm_min;
			tr.time.tm_hour=tm->tm_hour;
			tr.time.tm_year=tm->tm_year;
			tr.time.tm_mon=tm->tm_mon;
			tr.time.tm_mday=tm->tm_mday;
			write(fd1,&tr,sizeof(tr));
			unlock(fd1);
			close(fd1);
			u.amount=u.amount-amount;
			lseek(fd,(-1)*sizeof(u),SEEK_CUR);
			write(fd,&u,sizeof(u));
			break;	
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return ret;
}

int BalanceEnquiry(int nsd,int account_no){
	int fd,nbytes,ret;
	struct user u;
	if(account_no & 1)
		fd=open("normal_user_db",O_RDONLY);
	else
		fd=open("joint_account_user_db",O_RDONLY);
	lock(fd,F_RDLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			write(nsd,&u.amount,sizeof(u.amount));
			break;		
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	return 1;
}

int main(){
	int fd=open("normal_user_db",O_RDWR|O_CREAT|O_EXCL,0764);
	close(fd);
	fd=open("joint_account_user_db",O_RDWR|O_CREAT|O_EXCL,0764);
	close(fd);
	fd=open("transactions_db",O_RDWR|O_CREAT|O_EXCL,0764);
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
				ret=login_user(nsd);
				if(ret!=0){
					int account_no=ret;
					while(1){
						read(nsd,&i,sizeof(i));
						if(i==1){
							ret=Deposit(nsd,account_no);
						}
						else if(i==2){
							ret=Withdraw(nsd,account_no);
						}
						else if(i==3){
							ret=BalanceEnquiry(nsd,account_no);
						}
						else if(i==4){
							//ret=PasswordChange(nsd);
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
				ret=administrator_login(nsd);
				if(ret==1){
					while(1){
						read(nsd,&i,sizeof(i));
						if(i==1){
							ret=Add(nsd);
						}
						else if(i==2){
							ret=Delete(nsd);
						}
						else if(i==3){
							ret=Modify(nsd);
						}
						else if(i==4){
							ret=Search(nsd);
						}
						else{
							break;
						}
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
