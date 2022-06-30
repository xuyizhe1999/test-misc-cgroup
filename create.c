#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>

#include "test.h"

extern FILE *fp;

void create_file()
{
    int file_num = 2*MAX_FD;
    int fd[file_num], index;
    char str[20] = {};

    for (index = 0; index < file_num; index++) {
        sprintf(str, "%d", index);
        if ((fd[index] = open(str, O_RDWR)) < 0) 
            fd[index] = open(str, O_RDWR|O_CREAT, 0777);
        fprintf(fp, "%s %d fd = %d, fd number = %d\n",
                __func__,__LINE__,fd[index],index+1);
    }

    for (int i = 0; i < index; i++) {
        close(fd[i]);
    }
}
