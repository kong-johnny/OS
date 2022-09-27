#include<stdio.h>
#include<unistd.h>

int main(){
	pid_t fpid1 = fork();
	if(fpid1 == 0) printf("b");
	else {
		pid_t fpid2 = fork();
		if(fpid2 == 0) printf("c");
		else printf("a");
	}
	return 0;
}
