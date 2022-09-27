#include <unistd.h>
#include <stdio.h>
#include <iostream>

using namespace std;

int main(){
	pid_t fpid;
	int cnt = 0;
	fpid = fork();
	if(fpid < 0) cout << "error in fork!" << endl;
	else if(fpid == 0){ 
		cout << "I am the child process, my processs id is " << getpid() << endl;
		cnt ++;
	}
	else{
		cout << "I am the parent process, my process id is " << getpid() << endl;
		cnt ++;
	}
	cout << "total: " << cnt << endl;
	return 0;
}
