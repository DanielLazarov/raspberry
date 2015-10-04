#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h>  
#include <string.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#include <unistd.h>
#include <linux/kd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>


#define COMMAND_SIZE 128
/*-------Command---------------

    Example command: "01000"

    1 = Pressed Key
    0 = Released Key
    
    -Key indexes in command-   
        0 => 'Right arrow'
        1 => 'Up arrow'
        2 => 'Left arrow'
        3 => 'Down arrow'
        4 => 'Esc'
*/

static struct termios tty_attr_old;
static int old_keyboard_mode;


int setupKeyboard(void);
void restoreKeyboard(void);
void readKeyboard(void);
void sig_handler(int signo);

void  CommandReader(void);                /* child process prototype  */
void  CommandSender(void);               /* parent process prototype */

char *command;
int main(int argc, char **argv)
{

    key_t key;
    int shmid;
    int mode;

    if((shmid = shmget(key, COMMAND_SIZE, 0644 | IPC_CREAT)) == -1)
    {
        perror("Failed shmget");
        exit(1);
    }
    command = shmat(shmid, (void *)0, 0);
    if(command == (char *)(-1))
    {
        perror("Failed shmat");
        exit(1);
    }
    command[0] = '0';
    command[1] = '0';
    command[2] = '0';
    command[3] = '0';
    command[4] = '0';
    command[5] = '\0';
    
    pid_t  pid;

    pid = fork();
    if (pid == 0) 
    {
        CommandSender();
    } 
    else
    {
        CommandReader();
    }
}

void  CommandReader()
{
    signal(SIGIO, &sig_handler); //TODO  Try sigaction() it is better
    fcntl(STDIN_FILENO, F_SETOWN, getpid());//Sets PID which will receive the SIGIOs
    int oflags = fcntl(STDIN_FILENO, F_GETFL);//Save old flags
    fcntl(STDIN_FILENO, F_SETFL, oflags | FASYNC);//Add async flag

    if(setupKeyboard())
    {
	    write(1, "Terminal in RAW mode\n", 21);
    }   
    else
    {
        write(1, "RAW mode failed\n", 15);
    }
    
    while(1)
    {
        sleep(1);
    }
    
}

void  CommandSender()
{

    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    
    char ip[16];
    int fh;
    int fs;

    //Get the raspberry's IP address from external file
    fh = open("IP", O_RDONLY);
    fs = lseek(fh, 0, SEEK_END);
    lseek(fh, 0, 0);
    read(fh, ip, fs);
    ip[fs] = '\0';
    
    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(ip);//External config?
    servaddr.sin_port=htons(32000);

    while (1)
    {
        usleep(50000);
        sendto(sockfd,command,COMMAND_SIZE,0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
    }
}

int setupKeyboard()
{
    struct termios tty_attr;
    int flags;

    /* make stdin non-blocking */
    flags = fcntl(0, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(0, F_SETFL, flags);

    /* save old keyboard mode */
    if (ioctl(0, KDGKBMODE, &old_keyboard_mode) < 0) 
    {
	    return 0;
    }

    tcgetattr(0, &tty_attr_old);

    /* turn off buffering, echo and key processing */
    tty_attr = tty_attr_old;
    tty_attr.c_lflag &= ~(ICANON | ECHO | ISIG);
    tty_attr.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
    tcsetattr(0, TCSANOW, &tty_attr);

    ioctl(0, KDSKBMODE, K_RAW);

    return 1;
}

void restoreKeyboard()
{
    tcsetattr(0, TCSAFLUSH, &tty_attr_old);
    ioctl(0, KDSKBMODE, old_keyboard_mode);
    write(1, "Terminal RESTORED\n", 18);
}

void readKeyboard()
{
    unsigned char buf[1];
    
    while(read(0, &buf[0], 1) > 0)
	{
        switch (buf[0]) 
        { 
            //Right arrow
            case 0x4d:
                command[0] = '1';//write(1, "R press\n", 8);
                break;
            case 0xcd:
                command[0] = '0';//write(1, "R released\n", 11);
                break;
            
            //Up arrow
            case 0x48:
                command[1] = '1';//write(1, "U press\n", 8);
                break;
            case 0xc8:
                command[1] = '0';//write(1, "U released\n", 11);
                break;
           
            //Left arrow
            case 0x4b:
                command[2] = '1';//write(1, "L press\n", 8);
                break;
            case 0xcb:
                command[2] = '0';//write(1, "L released\n", 11);
                break;
           
            //Down arrow   
            case 0x50:
                command[3] = '1';//write(1, "D press\n", 8);
                break;
            case 0xd0:
                command[3] = '0';//write(1, "D released\n", 11);
                break;
            
            //Esc
	        case 0x01:
                command[4] = '1';//write(1, "Esc pressed\n", 12);
                restoreKeyboard();
                break;
            case 0x81:
                command[4] = '0';//write(1, "Esc released\n", 13);
                break;

        }
    }
}

void sig_handler(int signo)
{
    char buf[1];
    if (signo == SIGIO) //ASSERTION That the correct signal is received.
    {
        readKeyboard();
    }
}
