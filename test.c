#include<stdio.h>
#include<sys/types.h>
#include<linux/unistd.h>
#include<sys/syscall.h>
#include<stdlib.h>
#include<fcntl.h>

#include "test.h"

FILE *fp;

int cg_open_n(int *fd, int file_num)
{
    int index;
    char str[20] = {};
    for (index = 0; index < file_num; index++) {
        sprintf(str, "%d", index);
        fd[index] = syscall(SYS_CG_OPEN, str, O_RDWR);
        if (fd[index] < 0) {
            fprintf(fp, "Failed to alloc fd\n");
            break;
        } else {
            fprintf(fp, "%s %d fd = %d, fd number = %d\n",
                    __func__,__LINE__,fd[index],index+1);
        }
    }
    return index;
}

int cg_close_n(int *fd, int index)
{
    int ret;
    int fail = 0;
    for (int i = 0; i < index; i++) {
        if ((ret = syscall(SYS_CG_CLOSE, fd[i])) < 0) {
            fail = 1;
            fprintf(fp, "Close file error with fd %d\n", fd[i]);
        }
    }
    return fail;
}

void log_init()
{
    fp = fopen("log", "w+");
}

void log_clear()
{
    fclose(fp);
}

int main(void)
{
    int ret, file_num = 2*MAX_FD;
    int fd[file_num], index;
    char str[20] = {};
    int fail;

    /*
     * Init the log file.
     */
    log_init();

    /*
     * Show that fd capacity has been limited to MAX_FD.
     */
    printf("[TEST] cg_max_fd : ");
    if ((ret = syscall(SYS_MAX_FD, MAX_FD)) != 0) {
        printf("FAIL.\n");
        exit(1);
    }
    printf("PASS. Maximum fd number is now %d\n", MAX_FD);

    /*
     * Create test files. Open the test files with regular open()
     * to show its fd resource has no limit.
     */
    create_file();

    /*
     * Open the test files with cg_open().
     * On success, when opened file number exceeds MAX_FD, cg_open()
     * will fail.
     */
    printf("[TEST] cg_open : ");
    index = cg_open_n(fd, file_num);
    if (index == MAX_FD)
        printf("PASS.\n");
    else
        printf("FAIL.\n");

    /*
     * Reclaim fd resource using cg_close().
     */
    printf("[TEST] cg_close : ");
    fail = cg_close_n(fd, index);
    if (fail) 
        printf("FAIL.\n");
    else
        printf("PASS.\n");

    /*
     * Try cg_open() again.
     * On success, we will open the file "0" and get a new fd. And
     * do not forget to reclaim this new fd.
     */
    printf("[TEST] Alloc fd after reclaiming : ");
    index = cg_open_n(fd, 1);
    if (fd[0] < 0) {
        printf("FAIL.\n");
        fprintf(fp, "Failed to alloc fd\n");
    } else {
        printf("PASS.\n");
        fprintf(fp, "%s %d fd = %d, fd number = %d\n",
                __func__,__LINE__,fd[0],1);
        fail = cg_close_n(fd, 1);
    }
        
    /*
     * Test that bad fd does not claim fd resource, and that
     * cg_close does not reclaim any fd when close is failed.
     */
    printf("[TEST] Bad fd : ");
    sprintf(str, "%d", 1020);
    if ((ret = syscall(SYS_CG_OPEN, str, O_RDWR)) >= 0) {
        printf("FAIL.\n");
        fprintf(fp, "Opened a file that not exists!\n");
        syscall(SYS_CG_CLOSE, ret);
        goto bad_close;
    }
    index = cg_open_n(fd, file_num);
    fail = cg_close_n(fd, index);
    if (index == MAX_FD)
        printf("PASS.\n");
    else
        printf("FAIL.\n");

bad_close:   
    printf("[TEST] Bad close : ");
    if ((ret = syscall(SYS_CG_CLOSE, -1)) >= 0) {
        printf("FAIL.\n");
        fprintf(fp, "Close a file that not exists!\n");
        goto normal_return;
    }
    index = cg_open_n(fd, file_num);
    fail = cg_close_n(fd, index);
    if (index == MAX_FD)
        printf("PASS.\n");
    else
        printf("FAIL.\n");
    
normal_return:
    /*
     * Close the log file.
     */
    log_clear();
    return 0;
}
