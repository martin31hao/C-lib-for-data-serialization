/*
 * @author: Xinkai Wang
 * @contact: xinkaiw@andrew.cmu.edu
 *
 * mystub.h
 * Define utility functions for mylib, the main capabilities are:
 *     1. Convert between integer type and char array
 *     2. Convert between stat struct and char array
 *     3. Convert between dirtreenode (a library data structure) struct and char array
 */

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "dirtree.h"

/* Functions that convert various integer type into char array */
char *int_to_str(int num);
char *size_t_to_str(size_t num);
char *ssize_t_to_str(ssize_t num);
char *mode_t_to_str(mode_t num);
char *off_t_to_str(off_t num);
char *char_to_str(char* str);
char *voidptr_to_str(void *ptr);
char *statptr_to_str(struct stat *buf);
char *dirtreenode_to_str(struct dirtreenode* node);
char *dev_t_to_str(dev_t num);
char *ino_t_to_str(ino_t num);
char *nlink_t_to_str(nlink_t num);
char *uid_t_to_str(uid_t num);
char *gid_t_to_str(gid_t num);
char *blksize_t_to_str(blksize_t num);
char *blkcnt_t_to_str(blkcnt_t num);
char *time_t_to_str(time_t num);

/* Functions that convert char array into various integer type */
int ato_int(const char *str);
size_t ato_size_t(const char *str);
ssize_t ato_ssize_t(const char *str);
mode_t ato_mode_t(const char *str);
off_t ato_off_t(const char *str);
struct stat *ato_stat(char *str_stat);
struct dirtreenode *ato_dirtreenode(char *str_dirtreenode);
dev_t ato_dev_t(const char *str);
ino_t ato_ino_t(const char *str);
nlink_t ato_nlink_t(const char *str);
uid_t ato_uid_t(const char *str);
gid_t ato_gid_t(const char *str);
blksize_t ato_blksize_t(const char *str);
blkcnt_t ato_blkcnt_t(const char *str);
time_t ato_time_t(const char *str);

/* Check whether parameter type is consistent */
void check_param_type(const char *subtoken, const char *type, const char *func_name);
