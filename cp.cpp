#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>
char BUFFER[1024]; 
void CopyFile(char *fsource,char *ftarget)
{
	int fd = open(fsource,0);   
	int fdr;   
	struct stat statbuf;    
	int wordbit;
	lstat(fsource,&statbuf);

	if(S_ISLNK(statbuf.st_mode))
	{
		int rest=readlink(fsource,BUFFER,1023);
		BUFFER[rest]='\0';
		symlink(BUFFER,ftarget);
	}
	else
	{
		fdr = creat(ftarget,statbuf.st_mode);   
		while((wordbit = read(fd,BUFFER,1024)) > 0)  
		{
			write(fdr,BUFFER,wordbit);  
		}
		close(fdr);
	}
	close(fd); 
}
void Mycp(char *fsource,char *ftarget)
{
	char source[512];
	char target[512];
	struct stat statbuf;

	DIR *dir;
	struct dirent * entry;
	strcpy(source,fsource);
	strcpy(target,ftarget);
	dir = opendir(source);   
	while((entry = readdir(dir)) != NULL)  

	{
		if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0)   
			continue;
		if(entry->d_type == 4)
		{
			strcat(source,"/");
			strcat(source,entry->d_name);
			strcat(target,"/");
			strcat(target,entry->d_name);
			lstat(source,&statbuf);   

			if(S_ISLNK(statbuf.st_mode))
			{
				int rest=readlink(source,BUFFER,1023);
				BUFFER[rest]='\0';
				symlink(BUFFER,target);
			}
			else {mkdir(target,statbuf.st_mode);  
				Mycp(source,target);}
			strcpy(source,fsource);
			strcpy(target,ftarget);
		}
		else   
		{
			strcat(source,"/");
			strcat(source,entry->d_name);
			strcat(target,"/");
			strcat(target,entry->d_name);
			CopyFile(source,target);
			strcpy(source,fsource);
			strcpy(target,ftarget);
		}
	}
}

int main(int argc,char *argv[])
{
	struct stat statbuf;     //stat
	DIR * dir;             //DIR
	if(argc != 3)          
	{
		printf("ARGC ERROR!\n");
	}
	else
	{
		if((open(argv[1],0))==-1)
		{
			printf("Source is not legal.\n");    
			return 0; 
		}
		if(opendir(argv[1])!=NULL)
		{
			lstat(argv[1],&statbuf);

			if(S_ISLNK(statbuf.st_mode))
			{
				int rest=readlink(argv[1],BUFFER,1023);
				BUFFER[rest]='\0';
				symlink(BUFFER,argv[2]);

			}	
			else{
				mkdir(argv[2],statbuf.st_mode);  
				Mycp(argv[1],argv[2]);  } 
		}
		else 
		{
			CopyFile(argv[1],argv[2]);
		}
	}
	printf("Copy Finished.\n");
	return 0;
}
