#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char**argv)
{
    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    char sendline[1000];


    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr("192.168.0.100");
    servaddr.sin_port=htons(32000);

    //while (fgets(sendline, 10000,stdin) != NULL)
    //{
       // sendto(sockfd,sendline,strlen(sendline),0,
      //       (struct sockaddr *)&servaddr,sizeof(servaddr)); 
    //}
    while ((sendline[0] = getchar())!= '.')
    {
        sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr)); 
    }
}
