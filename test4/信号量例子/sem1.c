#include <stdio.h>  
#include <sys/types.h>  
#include <sys/ipc.h>  
#include <sys/sem.h> 
#include <errno.h> 

# define SEM_NUM  1
#define KEY 75 

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
	struct sembuf sops={semnum,-1,0};
        return (semop(semid,&sops,1));
}

/***对信号量数组semnum编号的信号量做V操作***/

int V(int semid, int semnum)
{
        struct sembuf sops={semnum,+1,0};
        return (semop(semid,&sops,1));
}
 
int main()
{
        int semid,ret;
        union semun arg;
        struct sembuf semop;
        int flag;

	
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
		arg.val = 0; 	        
	        ret =semctl(semid,0,SETVAL,arg);  /*对0号信号量设置初始值*/
	        if (ret < 0 )
	        {
	                perror("ctl sem error");
	                semctl(semid,0,IPC_RMID,arg);
	                return -1 ;
	        }
         }
	              
       	
	printf("sem1 excutee\n") ;

	flag = V(semid,0);
        if ( flag )
        {
            perror("V operate error") ;
            return -1 ;
        }

	return 0 ;

}