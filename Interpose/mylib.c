/*
 * @author: Xinkai Wang
 * @contact: xinkaiw@andrew.cmu.edu
 *
 * mylib.c
 * The main functionality is to do marshalling and unmarshalling data on different system calls.
 * The mylib works as a share library to be preloaded before executing the client application
 * so that system calls can be overwritten with mylib to achieve serialization and deserialiazation
 *
 * Supported system calls:
 * open, close, read, write, lseek, xstat, unlink, getdirentries
 * As well as two self-define functions:
 * getdirtree, freedirtree
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
#include <errno.h>
#include "mystub.h"

#define MAXMSGLEN 2000 /* Maximum length of receiving message from server */
#define MAXMSHLEN 2000 /* Maximum length of the marshall message */
#define INTSIZE 13 /* Size of char representation of int */
#define ULISIZE 26 /* Size of char representation of unsigned long */
#define LONGSIZE 26 /* Size of char representation of long */
#define FD_OFFSET 100000 /* Starting offset of lib-created file descriptors */

char connection_buf[MAXMSGLEN+1]; /* Connection buffer to receive message from server */
struct dirtreenode *ret_dirtreenode; /* ptr to dirtreenode returned from getdirtree */
char marshallMsg[MAXMSHLEN]; /* Message buffer when doing marshalling */
int firstConnect = 1; /* Var to denote whether it is the first connection to server */


char *serverip; /* server ip address */
char *serverport; /* server port */
unsigned short port; /* port number in integer */
int sockfd; /* socket file descriptor */
int rv; /* return value of receiving messages */
struct sockaddr_in srv;

char *get_ret_content(char *ret_val);

/*
 * Client-side marshalling of system calls
 * The main idea is to:
 * 1. Append method name at front
 * 2. Append the method name with a '|' character
 * 3. Append argv (created by caller)
 * 4. Don't forget to append '\0' to terminate char array
 */
char* marshalling_method(const char* func_name, char *argv, int len) {
	int i;
    
	memset(marshallMsg, 0, sizeof(marshallMsg));
    
	/* append function name to marshallMsg */
	strcat(marshallMsg, func_name);

	strcat(marshallMsg, "|");
	
	char *traverse = marshallMsg;
	while (*traverse != '\0')	traverse++;

	for (i = 0; i < len; i++) {
		traverse[i] = argv[i];
	}
	traverse[i] = '\0';

	return marshallMsg;
}

int (*orig_close)(int fd); /* Original close system call function ptr */

/*
 * Set up socket connection and send marshalling message to server
 * as well as receive marshalling message from the server
 * @param:
 *    msg: message to send to server
 *    len: length of the message.
 *         This is necessary because there may be '\0' in the message, such as read and write
 * @return: the marshalling message returned by server
 */
char *connect_to_server(char* msg, int len) {

	if (firstConnect == 1) {
        firstConnect = 0;
		
        // Get environment variable indicating the ip address of the server
        serverip = getenv("server15440");
        if (serverip); printf("Got environment variable server15440: %s\n", serverip);
        else {
            serverip = "127.0.0.1";
        }

        // Get environment variable indicating the port of the server
        serverport = getenv("serverport15440");
        if (serverport) fprintf(stderr, "Got environment variable serverport15440: %s\n", serverport);
        else {
            fprintf(stderr, "Environment variable serverport15440 not found.  Using 15440\n");
            serverport = "15440";
        }
        port = (unsigned short)atoi(serverport);

        // Create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);       // TCP/IP socket
        if (sockfd < 0) {
			err(1, 0);                        // in case of error
            firstConnect = 1;
        }

        // setup address structure to point to server
        memset(&srv, 0, sizeof(srv));                   // clear it first
        srv.sin_family = AF_INET;                       // IP family
        srv.sin_addr.s_addr = inet_addr(serverip);      // IP address of server
        srv.sin_port = htons(port);                     // server port
        // actually connect to the server
        rv = connect(sockfd, (struct sockaddr*)&srv, sizeof(struct sockaddr));
        if (rv < 0) {
            err(1, 0);
			firstConnect = 1;
		}
    }

	// send message to server
	send(sockfd, msg, len + 1, 0);

	memset(connection_buf, 0, sizeof(connection_buf));

	rv = recv(sockfd, connection_buf, MAXMSGLEN, 0);
	if (rv < 0) err(1, 0);
	connection_buf[rv] = 0;

	return connection_buf;
}

// The following line declares a function pointer with the same prototype as the open function.  
//int (*orig_open)(const char *pathname, int flags, ...);  // mode_t mode is needed when flags includes O_CREAT

// This is our replacement for the open function from libc.
/*
 * open system call with data serialization and deserialization
 * @param:
 *    pathname: path name of a file
 *    flags: open options like O_RDONLY, O_WRONLY, etc
 *    mode: this parameter is needed if flag is O_CREAT
 * @return:
 *    file descriptor if acquired successfully from server, -1 if failed
 */
int open(const char *pathname, int flags, ...) {
    mode_t m=0;
    if (flags & O_CREAT) {
        va_list a;
        va_start(a, flags);
        m = va_arg(a, mode_t);
        va_end(a);
    }
    // we just print a message, then call through to the original open function (from libc)
    fprintf(stderr, "mylib: open called for path %s\n", pathname);
    
    /* allocate 300 bytes for serialization */
    char *argv = (char*)malloc(300 * sizeof(char));

    /* parameters are separated from a '|' character */
    strcpy(argv, int_to_str(flags));
    strcat(argv, "|");
    strcat(argv, mode_t_to_str(m));
    strcat(argv, "|");
    strcat(argv, pathname);
	
    char* msg = marshalling_method("open", argv, strlen(argv));
    char *ret_val = connect_to_server(msg, strlen(msg));
    ret_val = get_ret_content(ret_val);
    
    /* if return value starts with a '-', which means an errno is returned */
    if (*ret_val == '-') {
        /* set the returned errno and return -1 */
        errno = -atoi(ret_val);
        fprintf(stderr, "errno: %d\n", errno);
        return -1;
    }
    fprintf(stderr, "open fd: %d\n", atoi(ret_val));
    
    /*
     * if success, return the file descriptor starting from the FD_OFFSET
     * The FD_OFFSET is to discriminate the library fd to fd acquired from the system itself
     */
    return atoi(ret_val) + FD_OFFSET;
}

/*
 * close system call with data serialization and deserialization
 * @param:
 *    fd: file descriptor
 * @return:
 *    0 if succeed, -1 if not
 */
int close(int fd) {
	fprintf(stderr, "mylib: close called for fd: %d\n", fd);
    /*
     * if the fd is smaller than the fd provided by our mylib,
     * then the original close should be called instead
     */
	if (fd < FD_OFFSET) {
		return orig_close(fd);
	}
	char *argv;
    
    /* allocate 12 bytes for serialization */
	argv = (char*)malloc(12 * sizeof(char));

	strcpy(argv, int_to_str(fd));
	
	char* msg = marshalling_method("close", argv, strlen(argv));
	char *ret_val = connect_to_server(msg, strlen(msg));
	ret_val = get_ret_content(ret_val);
    
	if (*ret_val == '-') {
		errno = -atoi(ret_val);
		fprintf(stderr, "errno: %d\n", errno);
		return -1;
	}
	return atoi(ret_val);
}

ssize_t (*orig_read)(int fd, void *buf, size_t count);

/*
 * read system call with data serialization and deserialization
 * @param:
 *    fd: file descriptor
 *    buf: buffer in which data is read to
 *    count: maximum number of data to read
 * @return:
 *    bytes of data is read if succeed, -1 if not
 */
ssize_t read(int fd, void *buf, size_t count) {
	fprintf(stderr, "mylib: read called for fd: %d\n", fd);
    
    /*
     * if the fd is smaller than the fd provided by our mylib,
     * then the original read should be called instead
     */
	if (fd < FD_OFFSET) {
		return orig_read(fd, buf, count);
	}
    
    /* allocate 30 bytes for serialization */
	char *argv = (char *)malloc((30) * sizeof(char));

	strcpy(argv, int_to_str(fd));
	strcat(argv, "|");
	strcat(argv, size_t_to_str(count));

	char *msg = marshalling_method("read", argv, strlen(argv));
	char *ret_val = connect_to_server(msg, strlen(msg));

    /*
     * the return message format is
     * EITHER
     * "number of bytes read | contents read"
     * OR
     * "the negative length of errno | errno"
     */
	char *number = ret_val;

	while (*ret_val != '|') {
		ret_val++;
	}
	*ret_val = '\0';
	ret_val++;
	
	ssize_t byte_read = ato_ssize_t(number);
    
	if (*number == '-') {
		errno = -atoi(ret_val);
		fprintf(stderr, "read errno: %d\n", errno);
		return -1;
	}
	memcpy((char *)buf, ret_val, byte_read);
	return byte_read;
}

ssize_t (*orig_write)(int fd, void *buf, size_t count);

/*
 * write system call with data serialization and deserialization
 * @param:
 *    fd: file descriptor
 *    buf: buffer in which data is write from
 *    count: maximum number of data to write
 * @return:
 *    bytes of data is write if succeed, -1 if not
 */
ssize_t write(int fd, void *buf, size_t count) {
	fprintf(stderr, "mylib: write called for fd: %d\n", fd);
    
    /*
     * if the fd is smaller than the fd provided by our mylib,
     * then the original write should be called instead
     */
	if (fd < FD_OFFSET) {
		return orig_write(fd, buf, count);
	}
    
	char *argv = (char *)malloc((count + 20) * sizeof(char));
	fprintf(stderr, "write count: %d\n", (int)count);

	strcpy(argv, int_to_str(fd));
	strcat(argv, "|");
	strcat(argv, size_t_to_str(count));
	strcat(argv, "|");

	int tmpLen = strlen(argv);
		
	char *traverse = argv;
	while (*traverse != '\0') traverse++;
	
	memcpy(traverse, buf, count);
	
	char *msg = marshalling_method("write",  argv, tmpLen + count);
	char *ret_val = connect_to_server(msg, tmpLen + count + 6);
    
    /*
     * the return message format is
     * EITHER
     * "number of bytes write"
     * OR
     * "negative errno"
     */
    
	ret_val = get_ret_content(ret_val);

	if (*ret_val == '-') {
		errno = -atoi(ret_val);
		fprintf(stderr, "errno: %d\n", errno);
		return -1;
	}
    
	free(argv);
	return ato_ssize_t(ret_val);
}

off_t (*orig_lseek)(int fd, off_t offset, int whence);

/*
 * lseek system call with data serialization and deserialization
 * @param:
 *    fd: file descriptor
 *    offset: offset from which the file is point to
 *    whence: SEEK_SET, SEEK_CUR, SEEK_END
 * @return:
 *    resulting offset location from the beginning of the file, -1 if error
 */
off_t lseek(int fd, off_t offset, int whence) {
    fprintf(stderr, "mylib: lseek called for fd: %d\n", fd);
    /*
     * if the fd is smaller than the fd provided by our mylib,
     * then the original write should be called instead
     */
	if (fd < FD_OFFSET) {
		return orig_lseek(fd, offset, whence);
	}
	
	char *argv = (char *)malloc(40 * sizeof(char));

	strcpy(argv, int_to_str(fd));
	strcat(argv, "|");
	strcat(argv, off_t_to_str(offset));
	strcat(argv, "|");
	strcat(argv, int_to_str(whence));
	
	char* msg = marshalling_method("lseek", argv, strlen(argv));
	char *ret_val = connect_to_server(msg, strlen(msg));
	ret_val = get_ret_content(ret_val);
	if (*ret_val == '-') {
		errno = -atoi(ret_val);
		fprintf(stderr, "errno: %d\n", errno);
		return -1;
	}
	free(argv);
	return ato_off_t(ret_val);
}

/*
 * __xstat system call with data serialization and deserialization
 * @param:
 *    ver: version number
 *    path: file path to get file stat info
 *    stat_buf: stat_buf to store the file info
 * @return:
 *    0 if succeed, -1 if error
 */
int __xstat(int ver, const char *path, struct stat *stat_buf) {
	fprintf(stderr, "mylib: stat called for path: %s\n", path);

	char *argv = (char *)malloc(1000 * sizeof(char));
	strcpy(argv, int_to_str(ver));
	strcat(argv, "|");
	strcat(argv, path);
	strcat(argv, "|");
	strcat(argv, statptr_to_str(stat_buf));
	
	char* msg = marshalling_method("__xstat", argv, strlen(argv));
	char *ret_val = connect_to_server(msg, strlen(msg));
	ret_val = get_ret_content(ret_val);
    
	if (*ret_val == '-') {
		errno = -atoi(ret_val);
		fprintf(stderr, "errno: %d\n", errno);
		return -1;
	}
	free(argv);
	return atoi(ret_val);
}

/*
 * unlink system call with data serialization and deserialization
 * @param:
 *    pathname: file path to unlink
 * @return:
 *    0 if succeed, -1 if error
 */
int unlink(const char *pathname) {
	fprintf(stderr, "mylib: unlink called for path: %s\n", pathname);
	char *argv = (char *)malloc(1000 * sizeof(char));

	strcpy(argv, pathname);
	
	char* msg = marshalling_method("unlink", argv, strlen(argv));
	char *ret_val = connect_to_server(msg, strlen(msg));
	ret_val = get_ret_content(ret_val);
    
	if (*ret_val == '-') {
		errno = -atoi(ret_val);
		fprintf(stderr, "errno: %d\n", errno);
		return -1;
	}
	free(argv);
	return atoi(ret_val);
}

ssize_t (*orig_getdirentries)(int fd, char *buf, size_t nbytes, off_t *basep);

/*
 * getdirentries system call with data serialization and deserialization
 * @param:
 *    fd: file descriptor
 *    buf: buffer of the directory to read
 *    nbytes: up to nbytes will be transferred
 *    basep: write the position of the block read in to the loc pointed by basep
 * @return:
 *    0 if succeed, -1 if error
 */
ssize_t getdirentries(int fd, char *buf, size_t nbytes, off_t *basep) {
    fprintf(stderr, "mylib: getdirentries called for fd: %d\n", fd);
	if (fd < FD_OFFSET) {
		return orig_getdirentries(fd, buf, nbytes, basep);
	}
    
	char *argv = (char *)malloc(40 * sizeof(char));

	strcpy(argv, int_to_str(fd));
	strcat(argv, "|");
	strcat(argv, size_t_to_str(nbytes));
	strcat(argv, "|");
	strcat(argv, off_t_to_str(*basep));
	
	char* msg = marshalling_method("getdirentries", argv, strlen(argv));
	char *ret_val = connect_to_server(msg, strlen(msg));
	fprintf(stderr, "client: getdiren ret_val :%s\n", ret_val);

	char *number = ret_val;
	while (*number != '|') {
		number++;
	}
	*number = '\0';
	number++;

	ssize_t ret_num = ato_ssize_t(ret_val);
	if (*number == '-') {
		errno = -atoi(number);
		fprintf(stderr, "errno: %d\n", errno);
		return -1;
	}
	
	int i;
	for (i = 0; i < ret_num; i++) {
		if (number[i] != '|') // TODO: Will '|' cause bugs
			buf[i] = number[i];
		else	buf[i] = '\0';
	}

	free(argv);
	return ret_num;
}

/*
 * getdirtree function with data serialization and deserialization
 * @param:
 *    path: path of the directory to get its directory tree
 * @return:
 *    dirtreenode structure which contains information of the directory
 */
struct dirtreenode* getdirtree(const char *path) {
	fprintf(stderr, "mylib: getdirtree called for path: %s\n", path);

	char *argv = (char *)malloc(1000 * sizeof(char));
	strcpy(argv, path);
	
	char* msg = marshalling_method("getdirtree", argv, strlen(argv));
	char *ret_val = connect_to_server(msg, strlen(msg));
	ret_val = get_ret_content(ret_val);
	if (*ret_val == '-') {
		errno = -atoi(ret_val);
		fprintf(stderr, "errno: %d\n", errno);
		return NULL;
	}
	ret_dirtreenode = ato_dirtreenode(ret_val);
	free(argv);
	return ret_dirtreenode;
}

void (*orig_freedirtree)(struct dirtreenode* dt);

/*
 * freedirtree function with data serialization and deserialization
 * @param:
 *    dt: directory tree node to free
 * @return:
 */
void freedirtree(struct dirtreenode* dt) {
	fprintf(stderr, "mylib: freedirtree called\n");
    /*
     * No need to rpc this function, because server will call freedirtree after getdirtree
     * Therefore, freedirtree from local is enough
     */
	orig_freedirtree(dt);
	return; 
}

/* This function is automatically called when program is started */
void _init(void) {
	/* set function pointer orig_xxx to point to the original xxx function */
	orig_close = dlsym(RTLD_NEXT, "close");
	orig_read = dlsym(RTLD_NEXT, "read");
	orig_write = dlsym(RTLD_NEXT, "write");
	orig_lseek = dlsym(RTLD_NEXT, "lseek");
	orig_getdirentries = dlsym(RTLD_NEXT, "getdirentries");
	orig_freedirtree = dlsym(RTLD_NEXT, "freedirtree");
}

/*
 * Get content from the return value
 * Return value is composed of:
 * length of the content | content
 * @param: 
 *    ret_val: char array of the return value
 * @return: 
 *    ptr to the content
 */
char *get_ret_content(char *ret_val) {
	while (*ret_val != '|')	ret_val++;
	ret_val++;
	return ret_val;
}
