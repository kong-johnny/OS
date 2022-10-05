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
        printf("Run producer to create and init sem first.\n");
        exit(0);
    }
    shmid = shmget(KEY, sizeof(struct SharingMemory), 0660|IPC_CREAT|IPC_EXCL);
    void *SharePnt = NULL; // void 指针没有特定的类型，因此它可以指向任何类型的数据
    if(shmid == -1){
        shmid = shmget(KEY, sizeof(struct SharingMemory), 0660|IPC_CREAT);
        SharePnt = shmat(shmid, NULL, 0); //shmaddr 是NULL，系统将自动选择一个合适的地址
        share = (struct SharingMemory *) SharePnt;
    }
    else{
        printf("Run producer to create and init shm first.\n");
        exit(0);
    }
    
    int f, e, mtx;
    char msg[1024];
    while(1){
        mtx = semctl(semid, 0, GETVAL, 0);
        f = semctl(semid, 1, GETVAL, 0);
        e = semctl(semid, 2, GETVAL, 0);
        printf("Consumer pid: %d, mutex: %d, full: %d, empty: %d\n", getpid(), mtx, f, e);
        printf("in: %d, out: %d\n", share->in, share->out);
        printf("Press your operation ID: 1. consumer a product; 2. exit without delete; 3. delete Sharing Momery and exit\n");
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
            P(semid, 1);
            P(semid, 0);
            strcpy(msg, share->memory[share->out++]);
            share->out %= N;
            V(semid, 0);
            V(semid, 2);
            mtx = semctl(semid, 0, GETVAL, 0);
            f = semctl(semid, 1, GETVAL, 0);
            e = semctl(semid, 2, GETVAL, 0);
            printf("message: %s\n", msg);
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