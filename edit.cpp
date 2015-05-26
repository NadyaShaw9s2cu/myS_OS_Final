#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
void myCopy(FILE *sfp, FILE *dfp) {
    char c;
    while((c = getc(sfp)) != EOF)
        putc(c, dfp);
}
void check(FILE* p){
    if(p == NULL) {
        printf("Could not open this file\n");
        exit(-1);
    }
}
int main(int argc, char* argv[]) {
    FILE *sfp, *dfp;

    //edit
    if(argc == 1) {
        myCopy(stdin, stdout);
    } else if(argc == 2) {
        // edit a
        sfp = fopen(argv[1], "r");
        check(sfp);
        myCopy(sfp, stdout);
    } else {
        // edit xx xx xx > yy
        if(strcmp(argv[argc-2], ">") == 0) {
            dfp = fopen(argv[argc-1], "a+");
            check(dfp);
            for(int i = 1; i < argc-2; i ++) {
                sfp = fopen(argv[i], "r");
                check(sfp);
                myCopy(sfp, dfp);
            }
        } else {
            for(int i = 1; i < argc; i ++) {
                sfp = fopen(argv[i], "r");
                check(sfp);
                myCopy(sfp, stdout);
            }
        }
    }
}
