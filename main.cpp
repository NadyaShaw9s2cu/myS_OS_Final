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
int argc;
char argv[maxValue][maxPath];
enum OPER{
   QUIT,CD, LS, CP, PIPE, RED, EDIT, COMPILE, WRONG
};
void mycd() {
    puts("mycd");
}

void myls() {
    puts("myls");
}
void mycp() {
    puts("mycp");
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

    for(int i = 0; i < CmdLength; i ++) {
        if(MyCmd[i] == ' ' || MyCmd[i] == '\0') {
            argv[cnt][pt ++] = '\0';
            cnt ++;
            pt = 0;
        }else{
            argv[cnt][pt++] = MyCmd[i];
        }
    }

    if(strcmp(argv[0], "cd") == 0) return CD;
    else if(strcmp(argv[0], "ls") == 0) return LS;
    else if(strcmp(argv[0], "cp") == 0) return CP;
    else if(strcmp(argv[0], "edit") == 0) return EDIT;
    else if(strcmp(argv[0], "compile") == 0) return COMPILE;
    else return WRONG;
}
//分割|, >对应的参数字符串
int splitCmdB() {
    int cnt = 0;
    int pt = 0;
    int prep = -2;

    int ret = 0;
    for(int i = 0; i < CmdLength; i ++){
        if(MyCmd[i] == '>' || MyCmd[i] == '|' || MyCmd[i] == '\0') {
            if(MyCmd[i] == '>') ret = RED;
            if(MyCmd[i] == '|') ret = PIPE;
            if(prep == i-1) return WRONG;
            prep = i;
            argv[cnt][pt++] = '\0';
            cnt ++;
            pt = 0;
        }else if(MyCmd[i] == ' '){
            return WRONG;
        }else{
            argv[cnt][pt++] = MyCmd[i];
        }
    }
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

int main() {
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
