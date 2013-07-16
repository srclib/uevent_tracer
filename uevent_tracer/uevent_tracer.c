
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/mount.h>
#include <stddef.h>
#include <dirent.h>
#include <cutils/list.h>
#include <cutils/uevent.h>
#include <poll.h>
#include <signal.h>

#include <private/android_filesystem_config.h>

#define UEVENT_MSG_LEN (1024)

struct uevent {
    const char *action;
    const char *path;
    const char *subsystem;
    const char *firmware;
    const char *partition_name;
    const char *device_name;
    int partition_num;
    int major;
    int minor;
};

void die(const char *msg)
{
    perror(msg);
    exit(errno);
}

static void parse_event(const char *msg, struct uevent *uevent)
{
    uevent->action = "";
    uevent->path = "";
    uevent->subsystem = "";
    uevent->firmware = "";
    uevent->major = -1;
    uevent->minor = -1;
    uevent->partition_name = NULL;
    uevent->partition_num = -1;
    uevent->device_name = NULL;

	//currently ignoring SEQNUM
    while(*msg) {
        if(!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            uevent->action = msg;
        } else if(!strncmp(msg, "DEVPATH=", 8)) {
            msg += 8;
            uevent->path = msg;
        } else if(!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg += 10;
            uevent->subsystem = msg;
        } else if(!strncmp(msg, "FIRMWARE=", 9)) {
            msg += 9;
            uevent->firmware = msg;
        } else if(!strncmp(msg, "MAJOR=", 6)) {
            msg += 6;
            uevent->major = atoi(msg);
        } else if(!strncmp(msg, "MINOR=", 6)) {
            msg += 6;
            uevent->minor = atoi(msg);
        } else if(!strncmp(msg, "PARTN=", 6)) {
            msg += 6;
            uevent->partition_num = atoi(msg);
        } else if(!strncmp(msg, "PARTNAME=", 9)) {
            msg += 9;
            uevent->partition_name = msg;
        } else if(!strncmp(msg, "DEVNAME=", 8)) {
            msg += 8;
            uevent->device_name = msg;
        }

        // advance to after the next \0
        while(*msg++)
            ;
    }

    printf("\n{\n"
			"uevent->action=%s\n"
			"uevent->path=%s\n"
			"uevent->subsystem=%s\n"
			"uevent->firmware=%s\n"
			"uevent->major=%d\n"
			"uevent->minor=%d\n"
			"uevent->device_name=%s\n"
			"}\n"
			,uevent->action, uevent->path, uevent->subsystem,uevent->firmware, uevent->major, uevent->minor,uevent->device_name);
}



int main(int argc, char **argv, char **env)
{
    struct pollfd ufd;

    struct sockaddr_nl snl;
    int sock;

    int on = 1;              // passcred on 
	int buf_sz = 64 * 1024 ; // 64KB

	int nr = -1;

    char msg[UEVENT_MSG_LEN+2];

	struct uevent uevent_entry;

	int n;

    memset(&snl, 0, sizeof(snl));
    snl.nl_pid = getpid();
    snl.nl_family = AF_NETLINK;
    snl.nl_groups = 0xffffffff;

    if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT)) < 0)
	{
    	die("huyanwei debug create socket failed! \n");
	}

    setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &buf_sz, sizeof(buf_sz));
    setsockopt(sock, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));

    if(bind(sock, (struct sockaddr *) &snl, sizeof(snl)) < 0) 
	{
        close(sock);
        return -1;
    }

    fcntl(sock, F_SETFD, FD_CLOEXEC);
    fcntl(sock, F_SETFL, O_NONBLOCK); // non-block

    ufd.events = POLLIN;
    ufd.fd = sock;

    while(1) 
	{
        ufd.revents = 0;
        nr = poll(&ufd, 1, -1);

        if (nr <= 0)
            continue;

        if (ufd.revents == POLLIN)
        {
		    while ((n = uevent_kernel_multicast_recv(sock, msg, UEVENT_MSG_LEN)) > 0) 
			{
        		if(n >= UEVENT_MSG_LEN)   /* overflow -- discard */
		            continue;

	    	    msg[n] = '\0';
    	    	msg[n+1] = '\0';

		        parse_event(msg, &uevent_entry);
			}
		}
    }

    close(sock);

	return 0;
}
