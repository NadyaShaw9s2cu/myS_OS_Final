/*
在Linux系统下，编制一个Shell命令解析器。
具备的功能：
改换工作目录  cd
显示当前目录信息  ls
复制普通文件和目录文件 cp
多命令之间的管道通信 |
输入输出重定向 >
编辑文件 edit
编译文件 compile
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <sys/ipc.h>
const int maxPath = 1024;
const int maxValue = 10;

char WorkPath[maxPath];
char MyCmdHead[maxPath];
char MyCmd[maxPath];
int CmdLength;
int myargc;
char myargv[maxValue][maxPath];

enum OPER{
   QUIT,CD, LS, CP, PIPE, RED, EDIT, COMPILE, WRONG
};

void mycd() {
    if(myargc != 2) {
        puts("Wrong Command!");
        return ;
    }else{
        if(strcmp(myargv[1], "..") == 0) {
			int pt = 0;
			int wlen = strlen(WorkPath);
			for(int i = 0; i < wlen; i ++) {
				if( WorkPath[i] == '/') pt = i;
			}
			WorkPath[pt+1] = '\0';
		}else if(myargv[1][0] != '/'){
			strcat(WorkPath, myargv[1]);
		}

		if(chdir(WorkPath) != 0) {
			perror("CD Error: ");
			return ;
        }
        memset(MyCmdHead, '\0', sizeof(MyCmd));
        strcat(MyCmdHead, "user@linux:");
        strcat(MyCmdHead, WorkPath);
        strcat(MyCmdHead, "$ ");
    }
}

void myls() {
    sprintf(myargv[0], "%s/", WorkPath);
    strcat(myargv[0], "ls");
    if(myargc == 1) {
        strcpy(myargv[1], ".");
        myargc = 2;
    }else if(myargc == 2 && strcmp(myargv[1], "-l") == 0) {
        strcpy(myargv[2], myargv[1]);
        strcpy(myargv[1], ".");
        myargc = 3;
    }
    char* mycmd[] = {NULL, NULL, NULL, NULL};
    if(myargc <= 3) {
        for(int i = 0; i <myargc; i ++) {
            mycmd[i] = myargv[i];
        }
    }else {
        puts("Wrong Command!");
        return ;
    }
    pid_t mypid;
    if( (mypid = fork()) < 0) {
        perror("shell fork ls error: ");
        return ;
    }
    if( mypid == 0 ) {
        if( execv("ls", mycmd) <= 0 ) {
            perror("ls exec error: ");
            return ;
        }
    }
    while(wait(0) != -1);
}
void mycp() {
    sprintf(myargv[0], "%s/", WorkPath);
    strcat(myargv[0], "cp");
    if(myargc != 3) {
        puts("Wrong Command!");
        return ;
    }
    char* mycmd[] = {myargv[0], myargv[1], myargv[2], NULL};

    pid_t mypid;
    if( (mypid = fork()) < 0) {
        perror("shell fork cp error: ");
        return ;
    }
    if( mypid == 0 ) {
        if( execv("cp", mycmd) <= 0 ) {
            perror("cp exec error: ");
            return ;
        }
    }
    while(wait(0) != -1);
}
void myPipe() {
    puts("mPipe");
}
void myRed(){
    puts("myRed");
}
void myEdit(){
    puts("myEdit");
}
void myCompile(){
    puts("myCompile");
}
void clearAll() {
    puts("clearAll");
}
void wrongCmd(){
    puts("wrongCmd");
    clearAll();
}

void myInitialize(){
    getcwd(WorkPath, maxPath);
    memset(MyCmdHead, '\0', sizeof(MyCmd));
    strcat(MyCmdHead, "user@linux:");
    strcat(MyCmdHead, WorkPath);
    strcat(MyCmdHead, "$ ");
}
//分割cd, ls, cp, edit, compile对应的参数字符串, 返回对应的操作的编号
int splitCmdA() {
    int cnt = 0;
    int pt = 0;

    for(int i = 0; i <= CmdLength; i ++) {
        if(MyCmd[i] == ' ' || MyCmd[i] == '\0') {
            myargv[cnt][pt ++] = '\0';
            cnt ++;
            pt = 0;
        }else{
            myargv[cnt][pt++] = MyCmd[i];
        }
    }
    myargc = cnt;

    if(strcmp(myargv[0], "cd") == 0) return CD;
    else if(strcmp(myargv[0], "ls") == 0) return LS;
    else if(strcmp(myargv[0], "cp") == 0) return CP;
    else if(strcmp(myargv[0], "edit") == 0) return EDIT;
    else if(strcmp(myargv[0], "compile") == 0) return COMPILE;
    else return WRONG;
}
//分割|, >对应的参数字符串
int splitCmdB() {
    int cnt = 0;
    int pt = 0;
    int prep = -2;

    int ret = 0;
    for(int i = 0; i <= CmdLength; i ++){
        if(MyCmd[i] == '>' || MyCmd[i] == '|' || MyCmd[i] == '\0') {
            if(MyCmd[i] == '>') ret = RED;
            if(MyCmd[i] == '|') ret = PIPE;
            if(prep == i-1) return WRONG;
            prep = i;
            myargv[cnt][pt++] = '\0';
            cnt ++;
            pt = 0;
        }else if(MyCmd[i] == ' '){
            return WRONG;
        }else{
            myargv[cnt][pt++] = MyCmd[i];
        }
    }
    myargc = cnt;
    return ret;
}
//处理用户输入的参数并确定符号类型
int getOperatorStyle() {
    CmdLength = strlen(MyCmd);
    if(CmdLength == 1 && MyCmd[0] == 'q') return QUIT;

    bool pipeExist = false, redExist = false;
    for(int i = 0; i < CmdLength; i ++) {
        if(MyCmd[i] == '|') pipeExist = true;
        if(MyCmd[i] == '>') redExist = true;
    }

    if(pipeExist && redExist) return WRONG;

    if(pipeExist || redExist) {
        return splitCmdB();
    }else{
        return splitCmdA();
    }

}

int main(int argc, char* argv[]) {
    myInitialize();
    while(true) {
        printf("%s", MyCmdHead);
        gets(MyCmd);
        int opStyle = getOperatorStyle();

        switch(opStyle){
            case CD: mycd();break;
            case LS: myls();break;
            case CP: mycp();break;
            case PIPE: myPipe();break;
            case RED: myRed();break;
            case EDIT: myEdit();break;
            case COMPILE: myCompile();break;
            case WRONG: wrongCmd();break;
            case QUIT: clearAll();break;
            default: break;
        }
    }
}
