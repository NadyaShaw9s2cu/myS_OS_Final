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
#include <fcntl.h>
const int maxWordLen = 65536;
const int maxPath = 1024;
const int maxValue = 32;

char WordBufPool[maxWordLen];
char WorkPath[maxPath];
char MyCmdHead[maxPath];
char MyCmd[maxPath];
int CmdLength;
int myargc;
char myargv[maxValue][maxPath];

enum OPER {
    QUIT,CD, LS, CP, PIPE, REDIN, REDOUT, EDIT, COMPILE, WRONG
};

void mycd() {
    if(myargc != 2) {
        puts("Wrong Command!");
        return ;
    } else {
        if(strcmp(myargv[1], "..") == 0) {
            int pt = 0;
            int wlen = strlen(WorkPath);
            for(int i = 0; i < wlen; i ++) {
                if( WorkPath[i] == '/') pt = i;
            }
            WorkPath[pt] = '\0';
        } else if(myargv[1][0] != '/') {
            strcat(WorkPath, "/");
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
    } else if(myargc == 2 && strcmp(myargv[1], "-l") == 0) {
        strcpy(myargv[2], myargv[1]);
        strcpy(myargv[1], ".");
        myargc = 3;
    }
    char* mycmd[] = {NULL, NULL, NULL, NULL};
    if(myargc <= 3) {
        for(int i = 0; i <myargc; i ++) {
            mycmd[i] = myargv[i];
        }
    } else {
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
//progA|progB
void myPipe() {
    char buf[maxPath];
    sprintf(buf, "%s/", WorkPath);
    strcat(buf, "pipe");
    char* mycmd[] = {buf, myargv[0], myargv[1],NULL};

    pid_t mypid;
    if( (mypid = fork()) < 0) {
        perror("pipe fork error: ");
        return ;
    }
    if( mypid == 0 ) {
        if( execv("pipe", mycmd) <= 0 ) {
            perror("pipe exec error: ");
            return ;
        }
    }
    while(wait(0) != -1);
}

//redirect input
void myRedIn() {
    if(myargc != 2) {
        puts("Command Error.");
        return ;
    }
    pid_t pid;
    if((pid = fork()) == -1) {
        perror("Redirect fork error:");
        exit(-1);
    }

    if(pid == 0) {

        int fd = open(myargv[1], O_RDONLY);
        int newfd = dup2(fd, 0);
        close(fd);
        if(newfd != 0) {
            puts("Could not duplicate.");
            return ;
        }

        char* mycmd[]= {myargv[0], NULL};
        if( execv(myargv[0], mycmd) < 0) {
            perror("Redirect execlp error:");
            exit(-1);
        }
    }

    if(pid) while(wait(0) != -1) ;
}
void myRedOut() {
    if(myargc != 2) {
        puts("Command Error.");
        return ;
    }
    pid_t pid;

    if((pid = fork()) == -1) {
        perror("Redirect fork error:");
        exit(-1);
    }

    if(pid == 0) {
        close(1);
        int fd = creat(myargv[1], 0644);
        if(fd < 0) {
            perror("Create Pipe Error:");
            return ;
        }
        char* mycmd[]= {myargv[0], NULL};
        if( execv(myargv[0], mycmd) < 0) {
            perror("Redirect execlp error:");
            return ;
        }
    }
    if(pid)while(wait(0) != -1);
}
void myEdit() {
    sprintf(myargv[0], "%s/", WorkPath);
    strcat(myargv[0], "edit");
    char* mycmd[35];
    for(int i = 0; i < myargc; i ++)
        mycmd[i] = myargv[i];
    mycmd[myargc] = NULL;

    pid_t mypid;
    if( (mypid = fork()) < 0) {
        perror("shell fork edit error: ");
        return ;
    }

    if( mypid == 0 ) {
        if( execv("edit", mycmd) <= 0 ) {
            perror("edit exec error: ");
            return ;
        }
    }
    while(wait(NULL) != -1);
}
void myCompile() {
    puts("myCompile");
}
void clearAll() {
    puts("clearAll");
}
void wrongCmd() {
    puts("wrongCmd");
    clearAll();
}

void myInitialize() {
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
        } else {
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
    for(int i = 0; i <= CmdLength; i ++) {
        if(MyCmd[i] == '>' || MyCmd[i] == '|' || MyCmd[i] == '\0' || MyCmd[i] == '<') {
            if(MyCmd[i] == '>') ret = REDOUT;
            if(MyCmd[i] == '<') ret = REDIN;
            if(MyCmd[i] == '|') ret = PIPE;
            if(prep == i-1) return WRONG;
            prep = i;
            myargv[cnt][pt++] = '\0';
            cnt ++;
            pt = 0;
        } else if(MyCmd[i] == ' ') {
            return WRONG;
        } else {
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
    char _e[6] = "edit ";
    bool isEdit = (CmdLength >= 5?true:false);
    for(int i = 0; i < 5; i ++){
        if(MyCmd[i] != _e[i]) {
            isEdit = false;break;
        }
    }

    bool pipeExist = false, redOutExist = false, redInExist = false;
    for(int i = 0; i < CmdLength && !isEdit; i ++) {
        if(MyCmd[i] == '|') pipeExist = true;
        else if(MyCmd[i] == '>') redOutExist = true;
        else if(MyCmd[i] == '<') redInExist = true;
    }

    if((pipeExist && redInExist) || (pipeExist && redOutExist) || (redInExist && redOutExist)) return WRONG;

    if(pipeExist || redInExist || redOutExist) {
        return splitCmdB();
    } else {
        return splitCmdA();
    }

}

int main(int argc, char* argv[]) {
    myInitialize();
    while(true) {
        printf("%s", MyCmdHead);
        gets(MyCmd);
        int opStyle = getOperatorStyle();

        switch(opStyle) {
        case CD:
            mycd();
            break;
        case LS:
            myls();
            break;
        case CP:
            mycp();
            break;
        case PIPE:
            myPipe();
            break;
        case REDIN:
            myRedIn();
            break;
        case REDOUT:
            myRedOut();
            break;
        case EDIT:
            myEdit();
            break;
        case COMPILE:
            myCompile();
            break;
        case WRONG:
            wrongCmd();
            break;
        case QUIT:
            clearAll();
            break;
        default:
            break;
        }
    }
}
