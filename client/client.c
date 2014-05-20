#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

/* where are we talking */
#define ROOMDIR "/tmp/gotrusers/"
#define UNIX_PATH_MAX 104
#define BUFLEN 2048
#define USAGE "usage: client NICKNAME"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_un sockaddr_un;
typedef struct timeval timeval;
typedef struct dirent dirent;

/* variables */
static char* nick;
int sock_fd;
static sockaddr_un receiver;

/* prototypes */
static void die(const char *message);
static int send_all(const char* message);
static int send_user(const char* message, const char* user);
int main(int argc, char* argv[]);

static void
die(const char *message) {
	fprintf(stderr, "%s\n", message);
	exit(1);
}

/* sends the message to all clients in the room */
static int
send_all(const char* message) {
	DIR *directory;
	dirent *dir;

	if(!(directory = opendir("."))) {
		perror("send_all: opendir(\".\") failed");
		return 1;
	}

	while((dir = readdir(directory))) {
		strncpy(receiver.sun_path, dir->d_name, UNIX_PATH_MAX);
		if(dir->d_type != DT_SOCK || !strcmp(dir->d_name, nick)) {
			continue;
		}
		if(sendto(sock_fd, message, strlen(message), 0, (sockaddr*) &receiver,
		          sizeof(sockaddr_un)) == -1) {
			fprintf(stderr, "Could not send message to %s: %s\n", dir->d_name,
					strerror(errno));
		}
	}

	closedir(directory);
	return 0;
}

/* sends the message to the user */
static int
send_user(const char* message, const char* user) {
	strncpy(receiver.sun_path, user, UNIX_PATH_MAX);
	if(sendto(sock_fd, message, strlen(message), 0, (sockaddr *) &receiver,
	          sizeof(sockaddr_un)) == -1) {
		perror("could not send message");
		return 1;
	}
	return 0;
}

int
main(int argc, char* argv[]) {
	struct stat finfo;
	timeval timeout = {1, 0};
	fd_set reads;
	sockaddr_un address;
	sockaddr_un recv_address;
	socklen_t recv_address_len;
	char buf[BUFLEN];
	ssize_t buf_len;

	errno = 0;

	if(argc < 2) {
		die(USAGE);
	}
	printf("entering room as %s\n", nick = argv[1]);

	if((sock_fd = socket(PF_UNIX, SOCK_DGRAM, 0)) == -1) {
		perror("main: incoming socket() failed");
		return 1;
	}

	mkdir(ROOMDIR, 0755);
	chdir(ROOMDIR);
	if(!stat(nick, &finfo)) {
		close(sock_fd);
		die("main: Nickname already in use!");
	}
	unlink(nick);

	memset(&receiver, 0, sizeof(sockaddr_un));
	receiver.sun_family = AF_UNIX;

	memset(&address, 0, sizeof(sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, nick, UNIX_PATH_MAX);

	if(bind(sock_fd, (sockaddr*)&address, sizeof(sockaddr_un)) == -1) {
		perror("main: bind() failed");
		goto fail;
	}

	while(1) {
		FD_ZERO(&reads);
		FD_SET(STDIN_FILENO, &reads);
		FD_SET(sock_fd, &reads);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		switch(select(sock_fd + 1, &reads, (fd_set*) 0, (fd_set*) 0, &timeout)) {
			default:
				if(FD_ISSET(sock_fd, &reads)) {
					recv_address_len = sizeof(sockaddr);
					buf_len = recvfrom(sock_fd, buf, BUFLEN - 1, 0, (sockaddr*)&recv_address, &recv_address_len);
					buf[buf_len] = '\0';
					fprintf(stderr, "nice massage from %s: %s", recv_address.sun_path, buf);
				}
				if(FD_ISSET(STDIN_FILENO, &reads)) {
					if(fgets(buf, BUFLEN, stdin)) {
						if(buf[0] == '/') { /* command */
							if(buf[1] == 'q') {
								close(sock_fd);
								unlink(nick);
								return 0;
							} else {
								fprintf(stderr, "unknown command: %s\n", buf);
							}
						} else {
							send_all(buf);
						}
					}
				}
				break;
			case 0: /* timeout, do nothing */
				break;
			case -1:
				perror("main: select() failed");
				goto fail;
				break;
		}
	}

fail:
	close(sock_fd);
	unlink(nick);
	return 1;
}
