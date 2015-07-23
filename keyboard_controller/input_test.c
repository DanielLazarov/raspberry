#include "unistd.h"
#include "linux/kd.h"
#include "termios.h"
#include "fcntl.h"
#include "sys/ioctl.h"
#include "signal.h"


static struct termios tty_attr_old;
static int old_keyboard_mode;

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
            //Esc
	        case 0x01:
                write(1, "Esc pressed\n", 12);
                restoreKeyboard();
                break;
            case 0x81:
                write(1, "Esc released\n", 13);
                break;

            //Up arrow
            case 0x48:
                write(1, "U press\n", 8);
                break;
            case 0xc8:
                write(1, "U released\n", 11);
                break;

            //Left arrow
            case 0x4b:
                write(1, "L press\n", 8);
                break;
            case 0xcb:
                write(1, "L released\n", 11);
                break;

            //Right arrow
            case 0x4d:
                write(1, "R press\n", 8);
                break;
            case 0xcd:
                write(1, "R released\n", 11);
                break;

            //Down arrow   
            case 0x50:
                write(1, "D press\n", 8);
                break;
            case 0xd0:
                write(1, "D released\n", 11);
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


int main(int argc, char **argv)
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

    return 0;
}
