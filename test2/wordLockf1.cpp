#include<iostream>
#include<stdio.h>
#include<unistd.h>

using namespace std;

int main(){
	pid_t fpid1 = fork();
	if(fpid1 == 0) {
		lockf(1, 1, 0);
		printf("I am CHILD process b.\n");
		lockf(1, 0, 0);
	}
	else {
		pid_t fpid2 = fork();
		if(fpid2 == 0) {
			lockf(1, 1, 0);
			printf("I am child process c.\n");
			lockf(1, 0, 0);
		}
		else {
			lockf(1, 1, 0);
			printf("I am PARENT process a.\n");
			lockf(1, 0, 0);
		}
	}
	return 0;
}
