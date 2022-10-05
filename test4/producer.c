#include <stdio.h>  
#include <sys/types.h>  
#include <sys/ipc.h>  
#include <sys/sem.h> 
#include <sys/shm.h>
#include <errno.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N 10
#define SIZE 1024
// #define KEY 75
#define SEM_NUM  3

struct SharingMemory{
    char memory[N][SIZE];
    int in, out;
};

union semun {
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};

/***对信号量数组semnum编号的信号量做P操作***/

int P(int semid, int semnum)
{
    //struct sembuf sops={semnum,-1, SEM_UNDO};
	struct sembuf sops={semnum,-1, 0};
    return (semop(semid,&sops,1));
}

/***对信号量数组semnum编号的信号量做V操作***/

int V(int semid, int semnum)
{
    //struct sembuf sops={semnum,+1, SEM_UNDO};
	struct sembuf sops={semnum,+1, 0};
    return (semop(semid,&sops,1));
}
int KEY;
int main(){
    int semid,ret, shmid;
    union semun arg;
    struct sembuf semop;
    int flag;
    struct SharingMemory *share;
    KEY = ftok("./consumer.c", 1); // proj_id是可以根据自己的约定，随意设置。这个数字,有的称之为project ID; 在UNIX系统上,它的取值是1到255;
    semid = semget(KEY, SEM_NUM, 0660|IPC_CREAT|IPC_EXCL);

    if(semid == -1)   //semget 创建失败
	{
		if(errno == EEXIST) /*no need to init sem*/
		{
			printf("semget() warning: %s\n", strerror(errno));
			semid = semget(KEY, 0, 0660|IPC_CREAT); /*sems can be 0 when semid already exists*/
			if(semid == -1)
			{
				printf("semget() error: %s\n", strerror(errno));
				return -1;
			}
			printf("existe semget() success. semid=[%d]\n", semid);
		}
		else
		{
			printf("semget() error: %s\n", strerror(errno));
			return -1;
		}
	}
	else  /*need to init sem*/
	{	
        printf("creat semget() success. semid=[%d]\n", semid);				
		arg.val = 1; 	        
	    ret = semctl(semid, 0, SETVAL, arg);  /*对0号信号量设置初始值*/
        if (ret < 0 )
	    {
	        perror("ctl sem error");
	        semctl(semid,0,IPC_RMID,arg);
	        return -1 ;
	    }
        arg.val = 0;
        ret = semctl(semid, 1, SETVAL, arg);
        if (ret < 0 )
	    {
	        perror("ctl sem error");
	        semctl(semid,0,IPC_RMID,arg);
	        return -1 ;
	    }
        arg.val = N;
        ret = semctl(semid, 2, SETVAL, arg);
	    if (ret < 0 )
	    {
	        perror("ctl sem error");
	        semctl(semid,0,IPC_RMID,arg);
	        return -1 ;
	    }
    }
    shmid = shmget(KEY, sizeof(struct SharingMemory), 0660|IPC_CREAT|IPC_EXCL);
    void *SharePnt = NULL; // void 指针没有特定的类型，因此它可以指向任何类型的数据
    if(shmid == -1){
        shmid = shmget(KEY, sizeof(struct SharingMemory), 0660|IPC_CREAT);
        SharePnt = shmat(shmid, NULL, 0); //shmaddr 是NULL，系统将自动选择一个合适的地址
        share = (struct SharingMemory *) SharePnt;
    }
    else{
        SharePnt = shmat(shmid, NULL, 0); //shmaddr 是NULL，系统将自动选择一个合适的地址
        share = (struct SharingMemory *) SharePnt;
        for(int i = 0; i < N; i ++){
            strcpy(share->memory[i], "");
        }
        share->in = 0;
        share->out = 0;
    }
    
    int f, e, mtx;
    char msg[1024];
    while(1){
        mtx = semctl(semid, 0, GETVAL, 0);
        f = semctl(semid, 1, GETVAL, 0);
        e = semctl(semid, 2, GETVAL, 0);
        printf("Producer pid: %d, mutex: %d, full: %d, empty: %d\n", getpid(), mtx, f, e);
        printf("in: %d, out: %d\n", share->in, share->out);
        printf("Press your operation ID: 1. produce a product; 2. exit without delete; 3. delete Sharing Momery and exit\n");
        int op;
        scanf("%d", &op);
        mtx = semctl(semid, 0, GETVAL, 0);
        if(semid == -1 || mtx == -1){ // check sharing memory
            printf("Sharing memory ERROR.\n");
            exit(0);
        }
        switch (op)
        {
        case 1:
            printf("Message: ");
            scanf("%s", msg);
            P(semid, 2);
            P(semid, 0);
            strcpy(share->memory[share->in++], msg);
            share->in %= N;
            V(semid, 0);
            V(semid, 1);
            mtx = semctl(semid, 0, GETVAL, 0);
            f = semctl(semid, 1, GETVAL, 0);
            e = semctl(semid, 2, GETVAL, 0);

            // printf("Produced successfully, mutex: %d, full: %d, empty: %d\n", mtx, f, e);
            // printf("in: %d, out: %d\n", share->in, share->out);
            break;
        
        case 2:
            shmdt(share);
            printf("Process disconnected with Sharing Memory\n");
            exit(0);
            // break;

        case 3:
            shmdt(share);
            semctl(semid, IPC_RMID, 0);
            shmctl(shmid, IPC_RMID, 0);
            printf("Successfully delete sharing memory and exit.");
            exit(1);

        default:
            printf("Try again\n");
            break;
        }
    }
    return 0;
}