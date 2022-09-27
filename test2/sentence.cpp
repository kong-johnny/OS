#include<iostream>
#include<stdio.h>
#include<unistd.h>

using namespace std;

int main(){
	printf("A sentence.");
	pid_t fpid1 = fork();
	if(fpid1 == 0) printf("I am CHILD process b.\n");
	else {
		pid_t fpid2 = fork();
		if(fpid2 == 0) printf("I am child process c.\n");
		else printf("I am PARENT process a.\n");
	}
	return 0;
}
