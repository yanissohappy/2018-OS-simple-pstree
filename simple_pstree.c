#include "simple_pstree.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <malloc.h>
//#define NETLINK_TEST 17
#define MAX_PAYLOAD 8192
#define WORD 1024

int netlink_socket; //I am not sure whether the third parameter will be.
struct sockaddr_nl src_address, dest_address;//These are address of source and destination.
struct nlmsghdr *nlh = NULL;//It is a kind of a notification.(?)
struct msghdr msg;
struct iovec iov;

int main(int argc, char *argv[])
{
    int ch;
    char msgstring[WORD]; //This is msg buffer.
    char prostring[WORD]; //This is process_id's char.
    msgstring[0]='\0';
    prostring[0]='\0';
    pid_t process_id=getpid();

    sprintf(prostring, "%d", process_id); //Make pid char version.

    netlink_socket=socket(AF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);//I am not sure this usage!!
    if(netlink_socket<0)
        return -1;

    memset(&src_address, 0, sizeof(src_address)); //initialize every field in src_address and dest_address.
    src_address.nl_family = AF_NETLINK;
    src_address.nl_pid = getpid(); //its own pid.
    src_address.nl_groups = 0; //unicast.
    bind(netlink_socket, (struct sockaddr*)&src_address, sizeof(src_address)); //the nl_pid field of the sockaddr_nl can be filled with the calling process' own pid.

    memset(&dest_address, 0, sizeof(dest_address));
    dest_address.nl_family = AF_NETLINK;
    dest_address.nl_pid = 0; //for Linux Kernel
    dest_address.nl_groups = 0;

    nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    if(argc>1) { //write this part in advance.
        while ((ch = getopt(argc, argv, "::c::s::p::")) != -1) { //argument,c,s,p is optional.
            switch (ch) {
            case 'c':
                if(optarg!=NULL) {
                    //printf("-c %s\r\n", optarg);
                    strcat(msgstring,"c "); // make "c pid"
                    strcat(msgstring,optarg);
                    //printf("the message is \"%s\"\r\n",msgstring); //for test.

                } else { //default for -c
                    strcat(msgstring,"c 1");
                    //printf("the message is \"%s\"\r\n",msgstring); //for test.
                }
                break;
            case 's':
                if(optarg!=NULL) {
                    //printf("-s %s\r\n", optarg);
                    strcat(msgstring,"s ");
                    strcat(msgstring,optarg);
                    //printf("the message is \"%s\"\r\n",msgstring); //for test.

                } else { //default for -s
                    strcat(msgstring,"s ");
                    strcat(msgstring,prostring);
                    //printf("the message is \"%s\"\r\n",msgstring); //for test.
                }
                break;
            case 'p':
                if(optarg!=NULL) {
                    //printf("-p %s\r\n", optarg);
                    strcat(msgstring,"p ");
                    strcat(msgstring,optarg);
                    //printf("the message is \"%s\"\r\n",msgstring); //for test.

                } else { //default for -p
                    strcat(msgstring,"p ");
                    strcat(msgstring,prostring);
                    //printf("the message is \"%s\"\r\n",msgstring); //for test.
                }
                break;
            case '?':
                fprintf(stderr,"Usage: %s [-c]or[-s]or[-p]\n"
                        "  -c children with optional argument\n"
                        "  -s siblings with optional argument\n"
                        "  -p parents with optional argument\n"
                        ,argv[0]);
                exit(EXIT_FAILURE);
            default:
                printf("default."); //for test.
            }
        }
    } else { //It's default.(Use the same as c's default.)
        strcat(msgstring,"c 1");
        //printf("the message is \"%s\"\r\n",msgstring); //for test.
    }

    strcpy(NLMSG_DATA(nlh), msgstring);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_address;
    msg.msg_namelen = sizeof(dest_address);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1; //It only has one.

    //printf("Sending message to kernel\n");
    sendmsg(netlink_socket,&msg,0); // Hope it will work!
    //printf("Waiting for message from kernel\n");

    // memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    // recvmsg(netlink_socket, &msg, 0);
    // printf("This is what we get %s!",(char *)NLMSG_DATA(nlh));// Trying. //(char *)
    // printf("Received mesage payload: %ld|%s\n", iov.iov_len, (char *)NLMSG_DATA(nlh));

    //while(1) {
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    recvmsg(netlink_socket, &msg, 0);
    printf("%s\n", (char *)NLMSG_DATA(nlh));
    //memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    //}
    close(netlink_socket);
}
