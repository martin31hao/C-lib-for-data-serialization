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

/*
 * Server-side unmarshalling message
 * @param: 
 *    marshallMsg: marshalling message from mylib
 * @return: 
 *    Marshalling message of return value after running syscalls on server
 */
char *unmarshalling_method(char* marshallMsg) {
    // Allocate memory for function name
    char *func_name = (char *)malloc(MAXFUNCSIZE * sizeof(char));

    /* Get the function name */
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
    } else if (strcmp(func_name, "close") == 0) {
        free(func_name);
        return execute_close(marshallMsg);
    } else if (strcmp(func_name, "read") == 0) {
        free(func_name);
        return execute_read(marshallMsg);
    } else if (strcmp(func_name, "write") == 0) {
        free(func_name);
        return execute_write(marshallMsg);
    } else if (strcmp(func_name, "lseek") == 0) {
        free(func_name);
        return execute_lseek(marshallMsg);
    } else if (strcmp(func_name, "__xstat") == 0) {
        free(func_name);
        return execute_stat(marshallMsg);
    } else if (strcmp(func_name, "unlink") == 0) {
        free(func_name);
        return execute_unlink(marshallMsg);
    } else if (strcmp(func_name, "getdirentries") == 0) {
        free(func_name);
        return execute_getdirentries(marshallMsg);
    } else if (strcmp(func_name, "getdirtree") == 0) {
        free(func_name);
        return execute_getdirtree(marshallMsg);
    } else {
        printf("function %s is not supported in RPC\n", func_name);
    } // if the function name is not supported, return error string to mylib
    char *error_msg = (char *)malloc(50 * sizeof(char));
    strcpy(error_msg, "Function not supported");
    
    free(func_name);
    
    return error_msg;
}

/*
 * Unmarshall and execute open syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    fd or -errno
 */
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
    char *ret_val;
    if (openfd < 0) {
        ret_val = int_to_str(-errno);
    }
    else {
        ret_val = int_to_str(openfd);
    }
    return add_len(ret_val, 30); // return value: fd or -errno
}

/*
 * Unmarshall and execute close syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    0 or -errno
 */
char *execute_close(char* msg) {
    int fd = atoi(&msg[6]); // parameters
    fd -= FD_OFFSET;

    int closefd = close(fd);
    char *ret_val;
    if (closefd < 0) {
        ret_val = int_to_str(-errno);
    }
    else {
        ret_val = int_to_str(closefd);
    }
    return add_len(ret_val, 30); // return value: 0 or -errno
}

/*
 * Unmarshall and execute read syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    bytes_read | content
 */
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
    char *ret_val;
    if (byteread < 0) {
        ret_val = int_to_str(-errno);
        fprintf(stderr, "read errno: %d\n", errno);
        return add_neg_len(ret_val, 30);
    }
	
    ret_val = (char *)malloc((byteread + 20) * sizeof(char));
    char *len_str = ssize_t_to_str(byteread);
    strcpy(ret_val, len_str);
    free(len_str);
    ((char *)buf)[byteread] = '\0';
    strcat(ret_val, "|");
    idx = 0;
    while (ret_val[idx] != '|')	idx++;
    idx++;
    memcpy(ret_val + idx, (char *)buf, byteread);
    
    free((char*)buf);
    
    return ret_val; // return value: bytes_read|content
}

/*
 * Unmarshall and execute write syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    bytes_written OR -errno
 */
char *execute_write(char* msg) {
    int fd = 0;
    char *buf;
    size_t count = 0; // parameters
    int idx = 6;
    fd = ato_int(&msg[idx]);
    fd -= FD_OFFSET;
    while (msg[idx] != '|')	idx++;
    idx++;

    // Count first
    count = ato_size_t(&msg[idx]);
    while (msg[idx] != '|')	idx++;
    idx++;

    // Start copying content to buf
    // There may be \0 in the content, so use memcpy instead of strcpy
    buf = (char *)malloc((count + 1) * sizeof(char));
    memcpy(buf, msg + idx, count);

    ssize_t write_bytes = write(fd, buf, count);
    fprintf(stderr, "server write bytes: %d\n", (int)write_bytes);
    char *ret_val;
    if (write_bytes < 0) {
        ret_val = int_to_str(-errno);
    }
    else {
        ret_val = ssize_t_to_str(write_bytes);
    }

    free(buf);
    
    return add_len(ret_val, 30); // return value: -errno OR bytes_written
}

/*
 * Unmarshall and execute lseek syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    offset OR -errno
 */
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
    char *ret_val;
    if (ret_offset < 0) {
        ret_val = int_to_str(-errno);
    }
    else {
        ret_val = off_t_to_str(ret_offset);
    }
    return add_len(ret_val, 30); // return value: offset or -errno
}

/*
 * Unmarshall and execute __xstat syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    0 OR -errno
 */
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
    char *ret_val;
    if (stat_ret < 0) {
        ret_val = int_to_str(-errno);
    }
    else {
        ret_val = int_to_str(stat_ret);
    }
    
    free(path);
    
    return add_len(ret_val, 30); // return value: 0 or -errno
}

/*
 * Unmarshall and execute unlink syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    0 OR -errno
 */
char *execute_unlink(char* msg) {
    char *pathname = &msg[7];

    int unlink_ret = unlink(pathname);
    char *ret_val;
    if (unlink_ret < 0) {
        ret_val = int_to_str(-errno);
    }
    else {
        ret_val = int_to_str(unlink_ret);
    }
    return add_len(ret_val, 30); // return value: 0 or -errno
}

/*
 * Unmarshall and execute getdirentries syscall on server
 * Then marshall the return value and content in a char array
 * @return:
 *    bytes_transferred|contents OR -errno
 */
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

    if (ret_val < 0) {
        char *ret_str = int_to_str(-errno);
        fprintf(stderr, "getdirentries: server errno: %d\n", (int)errno);
        return add_len(ret_str, 30);
    }

    char *ret = (char *)malloc((nbytes + 30) * sizeof(char));
    char *len_str = ssize_t_to_str(ret_val);
    strcpy(ret, len_str); // get bytes transferred
    free(len_str);
    strcat(ret, "|");
    
    char *traverse = ret;
    while (*traverse != '|')	traverse++;
    traverse++;
    
    int i = 0;
    for (i = 0; i < ret_val; i++) {
        traverse[i] = buf[i];
    }
    traverse[i] = '\0';
    free(buf);
    return ret; // return value: -errno OR bytes_transferrer|contents
}

/*
 * Unmarshall and execute getdirtree function on server
 * Then marshall the return value and content in a char array
 * @return:
 *    len_of_return|contents OR -errno
 */
char *execute_getdirtree(char* msg) {
    char *path = &msg[11]; // parameter

    struct dirtreenode *ret_dirtreenode = getdirtree(path);
    if (ret_dirtreenode == NULL) {
        return add_len(int_to_str(-errno), 30);
    }
    char *ret_val = dirtreenode_to_str(ret_dirtreenode);
    int len = strlen(ret_val);
    char *ret = int_to_str(len);
    char *new_space = (char *)malloc(1000 * sizeof(char));
    strcpy(new_space, ret);
    strcat(new_space, "|");
    free(ret);
    freedirtree(ret_dirtreenode);
    char *final_val = strcat(new_space, ret_val);
    free(ret_val);
    return final_val;// return value: -errno OR len_of_return|contents
}

/*
 * Wrapper of sending message.
 * I insert 4 byte of int to denote how many bytes behind to transfer
 * Keep sending until all bytes are sent
 * @return: number of bytes sent, or -1 if error occurred
 */
int send_message(int len, char *msg, int sockfd) {
    // send message to server
    char *msgLen = (char *)malloc((4 + len) * sizeof(char));//int_to_str(len);
    memcpy(msgLen, &len, 4);
    memcpy(msgLen + 4, msg, len);
    int byte_send = 0;
    len += 4;
    
    while (byte_send < len) {
        int sd = send(sockfd, msgLen + byte_send, len - byte_send, 0);
        byte_send += sd;
    }
    free(msgLen);
    return byte_send;
}

/*
 * Wrapper of receiving message.
 * Receive the first 4 byte of int to denote how many bytes behind to be received
 * Keep receiving until all bytes have been received
 * @return: number of bytes received, or -1 if error occurred
 */
int  receive_message(int sockfd, char *connection_buf, int mallocLen) {
    int recv_byte = 0;
    memset(connection_buf, 0, mallocLen);
    
    int len;
    int first_time = 1;
    
    while (1) {
        int rv = recv(sockfd, connection_buf + recv_byte, mallocLen, 0);
        if (rv < 0) err(1, 0);
        if (rv == 0)	return 0;
        recv_byte += rv;
        if (first_time == 1) {
            first_time = 0;
            memcpy(&len, connection_buf, 4);
        }
        if (len + 4 <= recv_byte)	 break;
    }
    connection_buf[recv_byte] = 0;
    return recv_byte;
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

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        perror(0);
        exit(1);
    }
    
    while (1) {
        // wait for next client, get session socket
        sa_size = sizeof(struct sockaddr_in);
        int sessfd = accept(sockfd, (struct sockaddr *)&cli, &sa_size);
        if (sessfd < 0)	err(1, 0);
        
        // get messages and send replies to this client,
        // until it goes a way
        pid_t pid = fork();
        if (pid == 0) { // child process
            close(sockfd);
            while (1) {
                
                char *buf = (char *)malloc(MAXWRITELEN * sizeof(char));
                int crv = receive_message(sessfd, buf, MAXWRITELEN);
                if (crv == 0) {
                    free(buf);
                    close(sessfd);
                    return 0;
                }
                if (crv < 0) {
                    err(1, 0);
                }
                
                // Unmarshalling the message, and execute it
                char *ret_val = unmarshalling_method(buf + 4);
                int len = 0, lenlen = 0;
                char *start = ret_val;
                if (*start == '-') {
                    start++; // deal with read
                    lenlen++;
                }
                while (*start != '|') {
                    len = len * 10 + *start - '0';
                    start++;
                    lenlen++;
                }
                start++;
                
                send_message(len + lenlen + 1, ret_val, sessfd);
                
                free(buf);
                free(ret_val);
            }
        }else {
            close(sessfd);
        }
        
    }
    
    close(sockfd);
    
    return 0;
}

/*
 * Add length infomation to a string by append length in front of a char array
 * then separate them with a '|'
 * @param:
 *    str: char array to append length info
 *    count: number of characters to allocate the new char array
 * @return:
 *    Ptr to the new char array, len|return content OR errno
 */
char *add_len(char *str, int count) {
    int len = strlen(str);
    char *ret = (char *)malloc(count * sizeof(char));
    char *len_str = int_to_str(len);
    strcpy(ret, len_str);
    free(len_str);
    strcat(ret, "|");
    char *ret_val = strcat(ret, str);
    free(str);
    return ret_val;
}

/*
 * Add negative length infomation to a string by append length in front of a char array
 * then separate them with a '|'
 * This function is used in read syscall to distinguish read content and error number
 * @param:
 *    str: char array to append length info
 *    count: number of characters to allocate the new char array
 * @return:
 *    Ptr to the new char array, -len|errno
 */
char *add_neg_len(char *str, int count) {
    int len = strlen(str);
    char *ret = (char *)malloc(count * sizeof(char));
    char *len_str = int_to_str(-len);
    strcpy(ret, len_str);
    free(len_str);
    strcat(ret, "|");
    return strcat(ret, str);
}
