#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>
#include <sys/inotify.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))
  
void otsl()
{
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];
    
    FILE *fp;
    char text1[50] = "The file was created.\n";
    char text2[50] = "The file was deleted.\n";
    char text3[50] = "The file was modified.\n";
    fp = fopen("example.txt", "a");

    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, "/home/stud126798/source/",
        IN_MODIFY | IN_CREATE | IN_DELETE);
    length = read(fd, buffer, BUF_LEN);

    if (length < 0) {
        perror("read");
    }
    
    while (i < length) {
        struct inotify_event *event =
            (struct inotify_event *) &buffer[i];
        if (event->len) {
            if (event->mask & IN_DELETE) {
                fprintf(fp, "%s %s",event->name, text2);
            } else if (event->mask & IN_CREATE) {
                fprintf(fp, "%s %s",event->name, text1);
            } else if (event->mask & IN_MODIFY) {
                fprintf(fp, "%s %s",event->name, text3);
            }
        }
        i += EVENT_SIZE + event->len;
    }

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);
    fclose(fp);
}	
   
static void skeleton_daemon()
{
    pid_t pid;
    
    pid = fork();
    
    if (pid < 0)
        exit(EXIT_FAILURE);
    
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    pid = fork();
    
    if (pid < 0)
        exit(EXIT_FAILURE);
    
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    umask(0);

    chdir("/");
    
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }
    
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}

int main(int argc, char **argv)
{	
	while(1)
	{
		otsl();
	}
    skeleton_daemon();

    //return 0;
    
    return EXIT_SUCCESS;
}
