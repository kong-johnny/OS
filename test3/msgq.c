#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/msg.h>
#include<sys/ipc.h>


#define KEY 75
#define MSIZE 1024
struct msgbuff {
	long msgtype;
	char msgtext[1024];
}msg;
int msgqid, times = 10;

void SERVER() {
	// lockf(1, 1, 0);
	while( (msgqid = msgget(KEY, IPC_CREAT | 0777)) == -1);
	// lockf(1, 0, 0);
	do {
		// 如果mtype的值是零的话，函数将不做类型检查而自动返回队列中的最旧的消息
		// 第五个参数依然是是控制函数行为的标志，取值可以是：0,表示忽略
		msgrcv(msgqid, &msg, MSIZE, 0, 0);
		printf("%s\n", msg.msgtext);
		sprintf(msg.msgtext, "(Client) received from SERVER %d, type %ld", getpid(), msg.msgtype);
		msgsnd(msgqid, &msg, MSIZE, 0);
		printf("(Server) sent\n");
		sleep(1);
	}while(msg.msgtype != 1);
	// 将队列从系统内核中删除。
	msgctl(msgqid, IPC_RMID, 0);
	exit(0);
}

void CLIENT() {
	while ( (msgqid = msgget(KEY, IPC_CREAT | 0777)) == -1);
	do {
		msg.msgtype = times;
		sprintf(msg.msgtext, "(Server) received from CLIENT %d, type %ld", getpid(), msg.msgtype);
		msgsnd(msgqid, &msg, MSIZE, 0);
		printf("(Client) sent %ld\n", msg.msgtype);
		
		sleep(1);
		msgrcv(msgqid, &msg, MSIZE, 0, 0);
		printf("%s\n", msg.msgtext);
		times --;
	}while(times >= 1);
	exit(0);
}

int main(){
	pid_t pid1, pid2;
	while((pid1 = fork()) == -1);
	if(pid1 == 0){
		SERVER();
	}
	else{
		while((pid2 = fork()) == -1);
		if(pid2 == 0){
			CLIENT();
		}
		else wait(0);
	}
	
	return 0;
}
