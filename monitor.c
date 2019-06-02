#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

void _init(int argc, char *argv[], char* env[]) {
        char* log_file = "/var/log/monitor.log";
        FILE * fp;
        unsetenv("LD_PRELOAD");

        int pid = getpid();
        int ppid = getppid();
        char* SHELL = getenv("SHELL");
        char* PWD = getenv("PWD");
        char* USER = getlogin();
        char* SSH_CONNECTION = getenv("SSH_CONNECTION");
        char* CMD0 = getenv("_");
        time_t seconds;
        seconds = time(NULL);

        if((fp = fopen(log_file, "a+"))) {
			fprintf(fp, "{\"ts\": %ld, \"event_type\": \"Process Creation\", \"ppid\": %i, \"pid\": %i, \"shell\": \"%s\", \"pwd\": \"%s\", \"user\": \"%s\", \"ssh\": \"%s\", \"last_cmd\": \"%s\", \"cmd\": \"", seconds, ppid, pid, SHELL, PWD, USER, SSH_CONNECTION, CMD0);
			for(int i=0;i<argc;i++){
					fprintf(fp, " %s", argv[i]);
			}
			fprintf(fp, "\"}\n");
			fclose(fp);
		}

}


void _fini(void) {
        FILE * fp;
        char* log_file = "/var/log/monitor.log";
        time_t seconds;
        seconds = time(NULL);
        if((fp = fopen(log_file, "a+"))) {
			fprintf(fp, "{\"ts\": %ld, \"event_type\": \"Process Terminated\", \"pid: %i\"}\n", seconds, getpid());
			fclose(fp);
		}
}



int open (const char *pathname, int flags, ...){

	static int (*func_open) (const char *, int, mode_t) = NULL;
	char* log_file = "/var/log/monitor.log";
	va_list args;
	mode_t mode;
	int fd;
	time_t seconds;
	seconds = time(NULL);
	FILE * fp;

	if (!func_open)
		func_open = (int (*) (const char *, int, mode_t)) dlsym (RTLD_NEXT, "open");

	va_start (args, flags);
	mode = va_arg (args, int);
	va_end (args);

	// Don't hook logger process
	if (strcmp (pathname, log_file)){
		fd = func_open (pathname, flags, mode);
        	if((fp = fopen(log_file, "a+"))) {
			fprintf(fp, "{\"ts\": %ld, \"event_type\": \"File Access\", \"path: %s\", \"mode\": %i, \"pid: %i\"}\n", seconds, pathname, flags, getpid());
			fclose(fp);
		}
		return fd;
	}
	else {
		fd = func_open (pathname, flags, mode);
		return fd;
	}
}

int system(const char *command) {

	static int (*func_system) (const char *) = NULL;
	int retval = 0;

	char* log_file = "/var/log/monitor.log";
	time_t seconds;
	seconds = time(NULL);
	FILE * fp;

	if (! func_system)
		func_system = (int (*) (const char*)) dlsym (RTLD_NEXT, "system");

	retval = func_system (command);

	if((fp = fopen(log_file, "a+"))) {
		fprintf(fp, "{\"ts\": %ld, \"event_type\": \"System Command\", \"pid: %i\", \"cmd\": \"%s\", \"return\": %d}\n", seconds, getpid(), command, retval);
		fclose(fp);
	}
	return retval;

}

int socket(int domain, int type, int protocol)
{
	static int (*o_socket)(int,int,int) = NULL;
	char* log_file = "/var/log/monitor.log";
	time_t seconds;
	seconds = time(NULL);
	FILE * fp;

	if(!o_socket)
		o_socket = dlsym(RTLD_NEXT, "socket");

	if((fp = fopen(log_file, "a+"))) {
		fprintf(fp, "{\"ts\": %ld, \"event_type\": \"Socket Activity\", \"pid: %i\", \"domain\": %i, \"type\": %i, \"proto\": %i}\n", seconds, getpid(), domain, type, protocol);
		fclose(fp);
	}
    return o_socket(domain,type,protocol);
}


int  connect(int  sockfd,  const  struct sockaddr *serv_addr, socklen_t addrlen){
	static int (*connect_real)(int, const  struct sockaddr*, socklen_t)=NULL;
	const unsigned char *c;
	int port,ok=1;
	char* log_file = "/var/log/monitor.log";
	time_t seconds;
	seconds = time(NULL);
	FILE * fp;

	if (!connect_real) connect_real=dlsym(RTLD_NEXT,"connect");

	if (serv_addr->sa_family==AF_INET6) return EACCES;

	if (serv_addr->sa_family==AF_INET){
		c=serv_addr->sa_data;
		port=256*c[0]+c[1];
		c+=2;
		ok=0;

		if((fp = fopen(log_file, "a+"))) {
			fprintf(fp, "{\"ts\": %ld, \"event_type\": \"Network Connection\", \"pid: %i\", \"ip\": %d.%d.%d.%d, \"port\": %d}\n", seconds, getpid(), (int)(*c),(int)(*(c+1)),(int)(*(c+2)),(int)(*(c+3)), port);
			fclose(fp);
		}
	}
	return connect_real(sockfd,serv_addr,addrlen);
}

