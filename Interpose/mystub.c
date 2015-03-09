#include "mystub.h"

#define MAXMSGLEN 2000 /* Maximum length of receiving message from server */
#define MAXMSHLEN 2000 /* Maximum length of the marshall message */
#define INTSIZE 13 /* Size of char representation of int */
#define ULISIZE 26 /* Size of char representation of unsigned long */
#define LONGSIZE 26 /* Size of char representation of long */

void check_param_type(const char *subtoken, const char *type, const char* func_name) {
        if (strcmp(subtoken, type) != 0) {
                fprintf(stderr, "%s: parameter type should be %s\n", func_name, type);
        }
}

/*
 * Convert from char array to int
 * @param: 
 *    str: char ptr to convert to
 * @return:
 *    int value of the str
 */
int ato_int(const char *str) {
        int len = strlen(str), i;
        int res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to size_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    size_t value of the str
 */
size_t ato_size_t(const char *str) {
        int len = strlen(str), i;
        size_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to ssize_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    ssize_t value of the str
 */
ssize_t ato_ssize_t(const char *str) {
        int len = strlen(str), i;
        ssize_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to mode_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    mode_t value of the str
 */
mode_t ato_mode_t(const char *str) {
        int len = strlen(str), i;
        mode_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to off_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    off_t value of the str
 */
off_t ato_off_t(const char *str) {
        int len = strlen(str), i;
        off_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to stat struct
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    stat struct ptr of the str
 */
struct stat *ato_stat(char *str_stat) {
        struct stat* param = (struct stat*)malloc(sizeof(struct stat)); // parameters

	char *traverse = str_stat;
	char *pivot = traverse;
	int len = 0;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_dev = ato_dev_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_ino = ato_ino_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_mode = ato_mode_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_nlink = ato_nlink_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_uid = ato_uid_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_gid = ato_gid_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_rdev = ato_dev_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_size = ato_off_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_blksize = ato_blksize_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_blocks = ato_blkcnt_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_atime = ato_time_t(pivot);
	pivot = traverse;

	while (*traverse != '\t') {
		traverse++;
		len++;
	}
	*traverse = '\0';
	traverse++;
	param->st_mtime = ato_time_t(pivot);
	pivot = traverse;

	param->st_ctime = ato_time_t(pivot);

        return param;
}

struct dirtreenode *ato_dirtreenode(char *str_dirtreenode) {
        int i = 0;
        struct dirtreenode *param; // parameters
	char *traverse = str_dirtreenode;
	int len = 0;
	char *num_subdirs, *pivot = traverse;

	param = (struct dirtreenode *)malloc(sizeof(struct dirtreenode));

	while (i < 2) {
		if (*traverse == '\t') {
			i++;
			if (i == 1) {
				param->name = (char *)malloc((len + 1) * sizeof(char));
				memcpy(param->name, pivot, len);
				param->name[len] = '\0';
			}
			else {
				num_subdirs = (char *)malloc((len + 1) * sizeof(char));
				memcpy(num_subdirs, pivot, len);
				num_subdirs[len] = '\0';
			}
			len = 0;
			pivot = traverse + 1;
		}
		else {
			len++;
		}
		traverse++;
	}
	traverse++;

	param->num_subdirs = atoi(num_subdirs);
	free(num_subdirs);
	if (param->num_subdirs == 0)
		param->subdirs = NULL;
	else 
		param->subdirs = (struct dirtreenode **)malloc(param->num_subdirs * sizeof(struct dirtreenode *));

	// Deal with sub directories
	int leftBrace = 1, rightBrace = 0;
	len = 0;
	pivot = traverse;
	char *subDir;
	int idx = 0;
	while(leftBrace - rightBrace > 0) {
		len++;
		if (*traverse == '(') {
			leftBrace++;
		}
		else if (*traverse == ')') {
			rightBrace++;
			if (rightBrace != 0 && leftBrace - rightBrace == 1) {
				subDir = (char *)malloc((len + 1) * sizeof(char));
				memcpy(subDir, pivot, len);
				subDir[len] = '\0';
				len = 0;
				struct dirtreenode *node = ato_dirtreenode(subDir);
				param->subdirs[idx++] = node;
				free(subDir);
				pivot = traverse + 1;
			}
		}
		traverse++;
	}

        // TODO: If errno is set, what to return?

        return param;
}

/*
 * Convert from char array to dev_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    dev_t value of the str
 */
dev_t ato_dev_t(const char *str) {
        int len = strlen(str), i;
        dev_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to ino_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    ino_t value of the str
 */
ino_t ato_ino_t(const char *str) {
        int len = strlen(str), i;
        ino_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to nlink_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    nlink_t value of the str
 */
nlink_t ato_nlink_t(const char *str) {
        int len = strlen(str), i;
        nlink_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to uid_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    uid_t value of the str
 */
uid_t ato_uid_t(const char *str) {
        int len = strlen(str), i;
        uid_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to gid_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    gid_t value of the str
 */
gid_t ato_gid_t(const char *str) {
        int len = strlen(str), i;
        gid_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to blksize_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    blksize_t value of the str
 */
blksize_t ato_blksize_t(const char *str) {
        int len = strlen(str), i;
        blksize_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to blkcnt_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    blkcnt_t value of the str
 */
blkcnt_t ato_blkcnt_t(const char *str) {
        int len = strlen(str), i;
        blkcnt_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from char array to time_t
 * @param:
 *    str: char ptr to convert to
 * @return:
 *    time_t value of the str
 */
time_t ato_time_t(const char *str) {
        int len = strlen(str), i;
        time_t res = 0;
	int st = 0;
	if (*str == '-')	st = 1;
        for (i = st; i < len && str[i] != '|'; i++)
                res = res * 10 + str[i] - '0';
	if (st == 1)	res = -res;
        return res;
}

/*
 * Convert from int to char array
 * @param:
 *    str: int to convert to
 * @return:
 *    char arry of the conversion
 */
char *int_to_str(int num) {

        int type_size = INTSIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

/*
 * Convert from size_t to char array
 * @param:
 *    str: size_t to convert to
 * @return:
 *    char arry of the conversion
 */
char *size_t_to_str(size_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;

        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        return &str[type_size-1-cnt];
}

/*
 * Convert from ssize_t to char array
 * @param:
 *    str: ssize_t to convert to
 * @return:
 *    char arry of the conversion
 */
char *ssize_t_to_str(ssize_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

/*
 * Convert from mode_t to char array
 * @param:
 *    str: mode_t to convert to
 * @return:
 *    char arry of the conversion
 */
char *mode_t_to_str(mode_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {

                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

/*
 * Convert from off_t to char array
 * @param:
 *    str: off_t to convert to
 * @return:
 *    char arry of the conversion
 */
char *off_t_to_str(off_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *char_to_str(char* str) {
        return str;
}

char *voidptr_to_str(void *ptr) {
        return (char *)ptr;
}

// TODO: need to ensure the sequence of struct is correct
char *statptr_to_str(struct stat *buf) {
        char *str = (char *)malloc(MAXMSHLEN * sizeof(char));
        //strcpy(str, "st_dev@");
        strcpy(str, dev_t_to_str(buf->st_dev));
	strcat(str, "\t");
        //strcat(str, "\tst_ino@");
        strcat(str, ino_t_to_str(buf->st_ino));
	strcat(str, "\t");
        //strcat(str, "\tst_mode@");
        strcat(str, mode_t_to_str(buf->st_mode));
	strcat(str, "\t");
        //strcat(str, "\tst_nlink@");
        strcat(str, nlink_t_to_str(buf->st_nlink));
	strcat(str, "\t");
        //strcat(str, "\tst_uid@");
        strcat(str, uid_t_to_str(buf->st_uid));
	strcat(str, "\t");
        //strcat(str, "\tst_gid@");
        strcat(str, gid_t_to_str(buf->st_gid));
	strcat(str, "\t");
        //strcat(str, "\tst_rdev@");
        strcat(str, dev_t_to_str(buf->st_rdev));
	strcat(str, "\t");
        //strcat(str, "\tst_size@");
        strcat(str, off_t_to_str(buf->st_size));
	strcat(str, "\t");
        //strcat(str, "\tst_blksize@");
        strcat(str, blksize_t_to_str(buf->st_blksize));
	strcat(str, "\t");
        //strcat(str, "\tst_blocks@");
        strcat(str, blkcnt_t_to_str(buf->st_blocks));
	strcat(str, "\t");
        //strcat(str, "\tst_atime@");
        strcat(str, time_t_to_str(buf->st_atime));
	strcat(str, "\t");
        //strcat(str, "\tst_mtime@");
        strcat(str, time_t_to_str(buf->st_mtime));
	strcat(str, "\t");
        //strcat(str, "\tst_ctime@");
        strcat(str, time_t_to_str(buf->st_ctime));
        return str;
}

char *dirtreenode_to_str(struct dirtreenode* node) {
        int i;
        char *str = (char *)malloc(MAXMSHLEN * sizeof(char));
        strcpy(str, node->name);
        strcat(str, "\t");
        strcat(str, int_to_str(node->num_subdirs));
        strcat(str, "\t(");
        for (i = 0; i < node->num_subdirs; i++) {
                strcat(str, dirtreenode_to_str(node->subdirs[i]));
        }
        strcat(str, ")");
        return str;
}

char *dev_t_to_str(dev_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *ino_t_to_str(ino_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *nlink_t_to_str(nlink_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *uid_t_to_str(uid_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *gid_t_to_str(gid_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *blksize_t_to_str(blksize_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *blkcnt_t_to_str(blkcnt_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

char *time_t_to_str(time_t num) {

        int type_size = ULISIZE;
        char *str = (char *)malloc(type_size * sizeof(char));
        str[type_size-1] = '\0';
        int cnt = 0;
        int negative = 0;

        // if num is negative, first make it positive
        if (num < 0) {
                negative = 1;
                num = -num;
        }
        if (num == 0) {
                str[type_size-2] = '0';
                return &str[type_size-2];
        }
        while (num) {
                str[type_size-2-cnt] = num % 10 + '0';
                num /= 10;
                cnt++;
        }
        // add negative number sign byte
        if (negative == 1) {
                str[type_size-2-cnt] = '-';
                cnt++;
        }
        return &str[type_size-1-cnt];
}

