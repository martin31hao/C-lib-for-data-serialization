/*
 * @author: Xinkai Wang
 * @contact: xinkaiw@andrew.cmu.edu
 *
 * server.c
 * Server-side functionality to do marshalling and unmarshalling data on different system calls.
 *
 * Supported system calls:
 * open, close, read, write, lseek, xstat, unlink, getdirentries
 * As well as two self-define functions:
 * getdirtree, freedirtree
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "mystub.h"
#include <pthread.h>

#define MAXMSGLEN 2000
#define MAXFUNCSIZE 30
#define MAXTHREADNUM 127
#define MAXWRITELEN 1000020
#define FD_OFFSET 100000

char *execute_open(char* msg);
char *execute_close(char* msg);
char *execute_read(char* msg);
char *execute_write(char* msg);
char *execute_lseek(char* msg);
char *execute_stat(char* msg);
char *execute_unlink(char* msg);
char *execute_getdirentries(char* msg);
char *execute_getdirtree(char* msg);

char *add_len(char *str, int count);
char *add_neg_len(char *str, int count);

pthread_t tid[MAXTHREADNUM];
int thread_idx = 0;

struct pthread_arg_t {
    int sessfd;
};

void *accept_new_client(void *arg);

/*
 * Server-side unmarshalling message
 */
char *unmarshalling_method(char* marshallMsg) {
    // Allocate memory for function name
    char *func_name = (char *)malloc(MAXFUNCSIZE * sizeof(char));

    int func_start = 0;
    int idx = 0;
    while (marshallMsg[func_start] != '|') {
        func_name[idx] = marshallMsg[func_start];
        idx++; func_start++;
    }
    func_name[idx] = '\0';

    if (strcmp(func_name, "open") == 0) {
        free(func_name);
        return execute_open(marshallMsg);
    }
    else if (strcmp(func_name, "close") == 0) {
        free(func_name);
        return execute_close(marshallMsg);
    }
    else if (strcmp(func_name, "read") == 0) {
        free(func_name);
        return execute_read(marshallMsg);
    }
    else if (strcmp(func_name, "write") == 0) {
        free(func_name);
        return execute_write(marshallMsg);
    }
    else if (strcmp(func_name, "lseek") == 0) {
        free(func_name);
        return execute_lseek(marshallMsg);
    }
    else if (strcmp(func_name, "__xstat") == 0) {
        free(func_name);
        return execute_stat(marshallMsg);
    }
    else if (strcmp(func_name, "unlink") == 0) {
        free(func_name);
        return execute_unlink(marshallMsg);
    }
    else if (strcmp(func_name, "getdirentries") == 0) {
        free(func_name);
        return execute_getdirentries(marshallMsg);
    }
    else if (strcmp(func_name, "getdirtree") == 0) {
        free(func_name);
        return execute_getdirtree(marshallMsg);
    }
    else {
        printf("function %s is not supported in RPC\n", func_name);
    }
    char *error_msg = (char *)malloc(50 * sizeof(char));
    strcpy(error_msg, "Function not supported");
    free(func_name);
    return error_msg;
}

char *execute_open(char* msg) {
    char *pathname;
    int flags;
    mode_t m; // parameters

    int idx = 5;
    flags = ato_int(&msg[idx]);
    while (msg[idx] != '|')	idx++;
    idx++;

    m = ato_mode_t(&msg[idx]);
    while (msg[idx] != '|')	idx++;
    idx++;

    // Then pathname
    pathname = &msg[idx];

    int openfd = open(pathname, flags, m);
    char *ret_val = int_to_str(openfd);
    if (openfd < 0) {
        ret_val = int_to_str(-errno);
        fprintf(stderr, "open errno: %d\n", errno);
    }
    return add_len(ret_val, 30); // return value: fd or -errno
}

char *execute_close(char* msg) {
    int fd = atoi(&msg[6]); // parameters
    fd -= FD_OFFSET;

    int closefd = close(fd);
    char *ret_val = int_to_str(closefd);
    if (closefd < 0) {
        fprintf(stderr, "close errno: %d\n", errno);
        ret_val = int_to_str(-errno);
    }
    return add_len(ret_val, 30); // return value: 0 or -errno
}

char *execute_read(char* msg) {
    int idx = 5;
    int fd;
    void* buf;
    size_t count; // parameters

    fd = ato_int(&msg[idx]);
    fd -= FD_OFFSET;
    while (msg[idx] != '|')	idx++;
    idx++;

    // Count first
    count = ato_size_t(&msg[idx]);

    buf = (char *)malloc((count + 10) * sizeof(char));

    ssize_t byteread = read(fd, buf, count);
    fprintf(stderr, "server byte read: %d\n", (int)byteread);
    char *ret_val;
    if (byteread < 0) {
        ret_val = int_to_str(-errno);
        fprintf(stderr, "read errno: %d\n", errno);
        fprintf(stderr, "server return read: %s\n", add_neg_len(ret_val, 30));
        return add_neg_len(ret_val, 30);
    }
	
    ret_val = (char *)malloc((byteread + 20) * sizeof(char));
    strcpy(ret_val, ssize_t_to_str(byteread));
    ((char *)buf)[byteread] = '\0';
    strcat(ret_val, "|");
    idx = 0;
    while (ret_val[idx] != '|')	idx++;
    idx++;
    memcpy(ret_val + idx, (char *)buf, byteread);

    free((char*)buf);

    return ret_val; // return value: bytes_read|content
}

char *execute_write(char* msg) {
    int fd = 0;
    char *buf;
    size_t count = 0; // parameters
    fprintf(stderr, "###in\n");
    int idx = 6;
    fd = ato_int(&msg[idx]);
    fd -= FD_OFFSET;
    while (msg[idx] != '|')	idx++;
    idx++;

    // Count first
    count = ato_size_t(&msg[idx]);
    while (msg[idx] != '|')	idx++;
    idx++;

    fprintf(stderr, "server write fd: %d\n", fd);
    fprintf(stderr, "server write count: %d\n", (int)count);
    // Start copying content to buf
    // There may be \0 in the content, so use memcpy instead of strcpy
    buf = (char *)malloc((count + 1) * sizeof(char));
    memcpy(buf, msg + idx, count);
    fprintf(stderr, "server write content: %s\n", buf);

    ssize_t write_bytes = write(fd, buf, count);
    fprintf(stderr, "server write bytes: %d\n", (int)write_bytes);
    char *ret_val = ssize_t_to_str(write_bytes);
    if (write_bytes < 0) {
        ret_val = int_to_str(-errno);
        fprintf(stderr, "write errno: %d\n", errno);
    }

    free(buf);
    
    return add_len(ret_val, 30); // return value: -errno OR bytes_written
}

char *execute_lseek(char* msg) {
    int fd;
    off_t offset;
    int whence; // parameters

    int idx = 6;
    fd = ato_int(&msg[idx]);
    fd -= FD_OFFSET;
    while (msg[idx] != '|')	idx++;
    idx++;

    offset = ato_off_t(&msg[idx]);
    while (msg[idx] != '|')	idx++;
    idx++;

    whence = ato_int(&msg[idx]);

    off_t ret_offset = lseek(fd, offset, whence);
    char *ret_val = off_t_to_str(ret_offset);
    if (ret_offset < 0) {
        ret_val = int_to_str(-errno);
        fprintf(stderr, "lseek server errno: %d\n", (int)errno);
    }
    return add_len(ret_val, 30); // return value: offset or -errno
}

char *execute_stat(char* msg) {
    int ver;
    char *path;
    struct stat *buf;

    int idx = 8;
    ver = ato_int(&msg[idx]);
    while (msg[idx] != '|')	idx++;
    idx++;

    path = (char *)malloc(700 * sizeof(char));
    int path_idx = 0;
    while (msg[idx] != '|')	{
        path[path_idx] = msg[idx];
        idx++; path_idx++;
    }
    path[path_idx] = '\0';
    idx++;

    buf = ato_stat(&msg[idx]);

    int stat_ret = __xstat(ver, path, buf);
    char *ret_val = int_to_str(stat_ret);
    if (stat_ret < 0) {
        ret_val = int_to_str(-errno);
        fprintf(stderr, "stat server errno: %d\n", (int)errno);
    }
    free(path);
    return add_len(ret_val, 30); // return value: 0 or -errno
}

char *execute_unlink(char* msg) {
    char *pathname = &msg[7];

    int unlink_ret = unlink(pathname);
    char *ret_val = int_to_str(unlink_ret);
    if (unlink_ret < 0) {
        ret_val = int_to_str(-errno);
        fprintf(stderr, "unlink server errno: %d\n", (int)errno);
    }
    return add_len(ret_val, 30); // return value: 0 or -errno
}

char *execute_getdirentries(char* msg) {
    int fd;
    char *buf;
    size_t nbytes;
    off_t *basep; // parameters

    int idx = 14;
    fd = ato_int(&msg[idx]);
    fd -= FD_OFFSET;
    while (msg[idx] != '|')	idx++;
    idx++;

    nbytes = ato_size_t(&msg[idx]);
    while (msg[idx] != '|')	idx++;
    idx++;

    off_t offset = ato_off_t(&msg[idx]);
    basep = &offset;
    // Start copying content to buf
    // There may be \0 in the content, so we copy them 1 by 1
    buf = (char *)malloc((nbytes + 1) * sizeof(char));

    ssize_t ret_val = getdirentries(fd, buf, nbytes, basep);
    fprintf(stderr, "ret_val in getdiren: %d\n", (int)ret_val);
    if (ret_val < 0) {
        char *ret_str = int_to_str(-errno);
        fprintf(stderr, "getdirentries: server errno: %d\n", (int)errno);
        return add_len(ret_str, 30);
    }

    char *ret = (char *)malloc((nbytes + 20) * sizeof(char));
    strcpy(ret, ssize_t_to_str(ret_val)); // get bytes transferred
    strcat(ret, "|");
    char *traverse = ret;
    while (*traverse != '|')	traverse++;
    traverse++;

    int i = 0;
    for (i = 0; i < ret_val; i++) {
        traverse[i] = buf[i];
        if (traverse[i] == '\0')	traverse[i] = '|';
    }
    traverse[i] = '\0';
    free(buf);
    return ret; // return value: -errno OR bytes_transferrer|contents
}

char *execute_getdirtree(char* msg) {
    char *path = &msg[11]; // parameter

    struct dirtreenode *ret_dirtreenode = getdirtree(path);
    if (ret_dirtreenode == NULL) {
        return add_len(int_to_str(-errno), 30);
    }
    char *ret_val = dirtreenode_to_str(ret_dirtreenode);
    int len = strlen(ret_val);
    char *ret = int_to_str(len);
    strcat(ret, "|");
    freedirtree(ret_dirtreenode);
    return strcat(ret, ret_val); // return value: -errno OR len_of_return|contents
}

int main(int argc, char **argv) {
    char *serverport;
    unsigned short port;
    int sockfd, rv;
    struct sockaddr_in srv, cli;
    socklen_t sa_size;

    // Get environment variable indicating the port of the server
    serverport = getenv("serverport15440");
    if (serverport)	port = (unsigned short)atoi(serverport);
    else	port = 15440;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)	err(1, 0);

    // Setup address structure to indicate server port
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
    srv.sin_port = htons(port);

    // bind to port
    rv = bind(sockfd, (struct sockaddr *)&srv, sizeof(struct sockaddr));
    if (rv < 0)	err(1, 0);

    // start listening for connections
    rv = listen(sockfd, 127);
    if (rv < 0)	err(1, 0);

    // main server loop, handle clients one at a time, quit after
    // 10 clients
    while (1) {
        // wait for next client, get session socket
        sa_size = sizeof(struct sockaddr_in);
        // TODO: spawn a thread to deal with concurrent clients
        int sessfd = accept(sockfd, (struct sockaddr *)&cli, &sa_size);
        if (sessfd < 0)	err(1, 0);

        struct pthread_arg_t arg;
        arg.sessfd = sessfd;
        pthread_create(&(tid[thread_idx]), NULL, accept_new_client, &arg);
        // get messages and send replies to this client,
        // until it goes a way
        /*char buf[MAXMSGLEN + 1];
         memset(buf, 0, sizeof(buf));
		rv = recv(sessfd, buf, MAXMSGLEN, 0);
		if (rv < 0)	err(1, 0);
		buf[rv] = 0; // null terminate string to print

		// Unmarshalling the message, and execute it
		char *ret_val = unmarshalling_method(buf); //TODO: return number of characters, num|string
		int len = 0, lenlen = 0;
		char *start = ret_val;
		while (*start != '|') {
			len = len * 10 + *start - '0';
			start++;
			lenlen++;
		}
		start++;

		// Send it back to client
		//printf("server reply return value of: %s\n", ret_val);
		//ret_val[strlen(ret_val)] = '\0';
		
		send(sessfd, ret_val, len + lenlen + 1, 0);

		if (rv < 0)	err(1, 0);*/
		//close(sessfd);

    }

    close(sockfd);

    return 0;
}

void *accept_new_client(void *arg) {
    // get messages and send replies to this client,
    // until it goes a way
    struct pthread_arg_t *thread_arg = (struct pthread_arg_t *)arg;
    int sessfd = thread_arg->sessfd;
    char *buf = (char *)malloc(MAXWRITELEN * sizeof(char));
    while (1) {
        memset(buf, 0, MAXWRITELEN);
        int rv = recv(sessfd, buf, MAXMSGLEN, 0);

        if (rv < 0)	err(1, 0);
        if (rv == 0)	continue;
        buf[rv] = 0; // null terminate string to print

        // Unmarshalling the message, and execute it
        char *ret_val = unmarshalling_method(buf); //TODO: return number of characters, num|string
        int len = 0, lenlen = 0;
        char *start = ret_val;
        while (*start != '|') {
            len = len * 10 + *start - '0';
            start++;
            lenlen++;
        }
        start++;

        // Send it back to client
		//ret_val[strlen(ret_val)] = '\0';
	
        send(sessfd, ret_val, len + lenlen + 1, 0);

        if (rv < 0)	err(1, 0);
        free(ret_val);
    }
    free(buf);

    return NULL;
}

char *add_len(char *str, int count) {
    int len = strlen(str);
    char *ret = (char *)malloc(count * sizeof(char));
    strcpy(ret, int_to_str(len));
    strcat(ret, "|");
    return strcat(ret, str);
}

char *add_neg_len(char *str, int count) {
    int len = strlen(str);
    char *ret = (char *)malloc(count * sizeof(char));
    strcpy(ret, int_to_str(-len));
    strcat(ret, "|");
    return strcat(ret, str);
}
