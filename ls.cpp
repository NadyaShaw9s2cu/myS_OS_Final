#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
const int maxPath = 1024;
struct stat target;
char TargetFile[maxPath];
//展示文件
int showFile(char* filePath, char* fileName) {
    struct stat fst;
    struct passwd* pw;
    struct group* gr;
    struct tm* mtm;

    if(stat(filePath, &fst) < 0)
        return -1;

    switch(fst.st_mode & S_IFMT) {
    case S_IFREG:
        printf("-");
        break;
    case S_IFDIR:
        printf("d");
        break;
    case S_IFLNK:
        printf("l");
        break;
    case S_IFBLK:
        printf("b");
        break;
    case S_IFCHR:
        printf("c");
        break;
    case S_IFIFO:
        printf("p");
        break;
    case S_IFSOCK:
        printf("s");
        break;
    }

    for(int i = 8; i >= 0; i--) {
        if(fst.st_mode & (1 << i)) {
            switch(i%3) {
            case 2:
                printf("r");
                break;
            case 1:
                printf("w");
                break;
            case 0:
                printf("x");
                break;
            }
        } else
            printf("-");
    }

    pw = getpwuid(fst.st_uid);
    gr = getgrgid(fst.st_gid);

    printf("%2d %s %s %4ld", fst.st_nlink, pw->pw_name, gr->gr_name, fst.st_size);

    mtm = localtime(&fst.st_ctime);
    printf(" %04d-%02d-%02d %02d:%02d",mtm->tm_year + 1900, mtm->tm_mon + 1, mtm->tm_mday, mtm->tm_hour, mtm->tm_min);

    printf(" %s\n", fileName);
    return 0;
}

//展示目录
int showDir(char* dirName) {
    DIR *dir;
    struct dirent *drt;
    struct stat dst;

    char buf[maxPath];

    dir = opendir(dirName);

    while((drt = readdir(dir)) != NULL)
    {
        strcpy(buf, dirName);
        strcat(buf, "/");
        strcat(buf, drt->d_name);
        if(stat(buf, &dst)){
            return -1;
        }

        if(drt->d_name[0] != '.'){
            if(showFile(buf, drt->d_name) < 0)
                return -1;
        }
    }
    return 0;
}
int main(int argc, char* argv[]) {
    if(argc != 2) {
        perror("ls error: ");
        exit(-1);
    }
    struct stat target;
    sprintf(TargetFile, "%s", argv[1]);
    if(stat(TargetFile, &target) != 0) {
        perror("目录文件错误：");
        exit(-1);
    }

    if( (target.st_mode & S_IFMT) == S_IFDIR ) {
        if(showDir(TargetFile) < 0)
            perror("showDir Error: ");
    } else {
        if(showFile(TargetFile, TargetFile))
            perror("showFile Error: ");
    }
    return 0;
}

