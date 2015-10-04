/* Sample UDP server */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <wiringPi.h> 
#include <signal.h> 


void intHandler(int dummy);
void clearDigitalWite(void);
 
 
void clearDigitalWrite() 
{ 
  digitalWrite(2, LOW); 
  digitalWrite(3, LOW); 

  digitalWrite(21, LOW); 
  digitalWrite(22, LOW); 

  digitalWrite(23, LOW); 
  digitalWrite(24, LOW); 
}

void intHandler(int dummy) { 
    clearDigitalWrite();
    exit(0);
}

int main(int argc, char**argv)
{
    signal(SIGINT, intHandler);
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    char mesg[128];

    wiringPiSetup () ;
    
    pinMode (2, OUTPUT);//Left forward
    pinMode (3, OUTPUT);//Left backwards

    pinMode (23, OUTPUT);//Right forward
    pinMode (24, OUTPUT);//Right backwards 
    
    pinMode (21, OUTPUT);//Front LED
    pinMode (22, OUTPUT);//Back LED


    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(32000);
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    printf("Listening...\n");   

    while (1)
    {

        len = sizeof(cliaddr);
        recvfrom(sockfd,mesg,128,0,(struct sockaddr *)&cliaddr,&len);
        
        digitalWrite(21, HIGH);
        digitalWrite(22, HIGH);

        if(mesg[0] == '1')//Right pressed
        {
            if(mesg[1] == '1')//Up pressed
            {
                //LF
                digitalWrite(2, HIGH);
                digitalWrite(3, LOW);
                digitalWrite(23, LOW);
                digitalWrite(24, LOW);
            }
            else if(mesg[3] == '1')//Down pressed
            {
                //LB
                digitalWrite(2, LOW);
                digitalWrite(3, HIGH);
                digitalWrite(23, LOW);
                digitalWrite(24, LOW);
            }
            else
            {
                //LF, RB
                digitalWrite(2, HIGH);
                digitalWrite(3, LOW);
                digitalWrite(23, LOW);
                digitalWrite(24, HIGH);
            }
        }
        else if(mesg[2] == '1')//Left pressed
        {
            if(mesg[1] == '1')//Up pressed
            {
                //RF
                digitalWrite(2, LOW);
                digitalWrite(3, LOW);
                digitalWrite(23, HIGH);
                digitalWrite(24, LOW);
            }
            else if(mesg[3] == '1')//Down pressed
            {
                //RB
                digitalWrite(2, LOW);
                digitalWrite(3, LOW);
                digitalWrite(23, LOW);
                digitalWrite(24, HIGH);
            }
            else
            {
                //RF, LB
                digitalWrite(2, LOW);
                digitalWrite(3, HIGH);
                digitalWrite(23, HIGH);
                digitalWrite(24, LOW);
            }
        }
        else if(mesg[1] == '1')//Up pressed
        {
            //LF, RF
            digitalWrite(2, HIGH);
            digitalWrite(3, LOW);
            digitalWrite(23, HIGH);
            digitalWrite(24, LOW);
        }
        else if(mesg[3] == '1')//Down pressed
        {
            //LB, RB
            digitalWrite(2, LOW);
            digitalWrite(3, HIGH);
            digitalWrite(23, LOW);
            digitalWrite(24, HIGH);
        }
        else if(mesg[4] == '1')
        {
            clearDigitalWrite();
        }
        else
        {
            //Neutral
            digitalWrite(2, LOW);
            digitalWrite(3, LOW);
            digitalWrite(23, LOW);
            digitalWrite(24, LOW); 
        }
    }
}

