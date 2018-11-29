#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#include <linux/pid.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/list.h>

#define MAX_PAYLOAD 8192
//#define MAX_MSGSIZE 1024
int stringlength(char *s);
void rece_msg(struct sk_buff *skb);
void DFS(struct task_struct *task,int count);
void indentcount(int count);
void stringfact(char msgfromapp[100]); // This is used for analyzing string.
char token[4]= {}; //c or s or p.
char number[20]= {}; //app's pid.
char temp_str[100]= {}; // for loop line 165
pid_t numberpid;
void* out_payload;
void reverse(char* begin, char* end); //to reverse words in parents msg.
void reverseWords(char* s);
void sendnlmsg(char *message,int dstPID);
// token[0]='\0';
// number[0]='\0';
int indent_size=0;
char indent[1024]= {};

struct pid* cur_pid=NULL;//I will change "current->pid" to the real pid I get.
struct task_struct* cur_task=NULL;//Get task_struct!!!!!

static struct sock *netlink_socket = NULL;

void indentcount(int count)
{
    int i;
    char *indentline = "	";
    memset(indent, 0, sizeof(indent));
    for (i = 0; i < count; i++) {
        strcat(indent,indentline);
    }
    //return indent;
}

void reverseWords(char* s)
{
    char* word_begin = s;
    char* temp = s; //word boundry.

    while (*temp) {
        temp++;
        if (*temp == '\0') {
            reverse(word_begin, temp - 1);
        } else if (*temp == '\n') {
            reverse(word_begin, temp - 1);
            word_begin = temp + 1;
        }
    }

    reverse(s, temp - 1);
}

void reverse(char* begin, char* end)
{
    char temp;
    while (begin < end) {
        temp = *begin;
        *begin++ = *end;
        *end-- = temp;
    }
}

void DFS(struct task_struct *task, int count)
{
    struct task_struct *childrens;
    struct list_head *list=NULL;
    int c;//record indent_size for now.
    int i;//for loop.
    c=count+1;
    list_for_each(list, &task->children) {
        childrens = list_entry(list, struct task_struct, sibling);
        sprintf(temp_str, "%d", childrens->pid); // int to string!!!!!
        //indentcount(indent_size);
        for(i=0; i<count; i++) //segmentation fault QQ.
            strcat(out_payload,"    ");

        strcat(out_payload,childrens->comm);
        strcat(out_payload,"(");
        strcat(out_payload,temp_str);
        strcat(out_payload,") ");
        strcat(out_payload,"\n");
        DFS(childrens,c);
    }
}

void stringfact(char msgfromapp[100])
{
    int i=2;
    int j=0;
    token[0]=msgfromapp[0];

    while(msgfromapp[i]!='\0') {
        number[j]=msgfromapp[i];
        // msgfromapp++;
        // strcat(number, msgfromapp);
        i++;
        j++;
    }
    number[j]='\0';
}

int stringlength(char *s)
{
    int slen = 0;
    for(; *s; s++) {
        slen++;
    }
    return slen;
}

void rece_msg_and_send(struct sk_buff *skb)
{
    int pid, payload_len, skb_len;
    int temp_format=0;
    //int list_len=0;
    struct sk_buff *out_skb,*skb_data;
    struct nlmsghdr *nlh,*out_nlh;
    char str[100];
    //char temp_str[100]= {}; // for loop line 165
    void *payload;
    //void *parent_out_payload;
    //void *out_payload;
    struct list_head *cur_task_head;
    struct task_struct *siblings,*childrens;//To tell sibling and children, I use plural noun to name it.
    nlh = nlmsg_hdr(skb);

    //printk("received data!\n");
    payload = nlmsg_data(nlh);
    payload_len = nlmsg_len(nlh);
    printk(KERN_INFO "payload_len = %d\n", payload_len);
    printk(KERN_INFO "Recievid: %s, From: %d\n", (char *)payload, nlh->nlmsg_pid);
    out_skb = nlmsg_new(MAX_PAYLOAD, GFP_KERNEL);
    if (!out_skb)
        printk(KERN_INFO "OHHHHH no\n");
    out_nlh = nlmsg_put(out_skb, 0, 0, 0, MAX_PAYLOAD, 0);
    if (!out_nlh)
        printk(KERN_INFO "OHHHHH no!\n");
    out_payload = nlmsg_data(out_nlh);
    skb_data=skb_get(skb);

    while(skb_data->len>=NLMSG_SPACE(0)) {
        nlh=nlmsg_hdr(skb_data);
        skb_len=NLMSG_ALIGN(nlh->nlmsg_len);
        memcpy(str,NLMSG_DATA(nlh),sizeof(str));
        printk("message recieved:%s\n",str);
        pid=nlh->nlmsg_pid;
        skb_pull(skb_data,skb_len);//substract the length.
    }
    stringfact(str);//split it.
    sscanf(number, "%d", &numberpid);
    printk("after factor option:\"%s\"\r\n",token); // test.
    printk("after factor number:\"%s\"\r\n",number); // test.
    printk("after factor number(pid_t):%d\r\n",numberpid); // test.

    cur_pid = find_get_pid(numberpid);
    cur_task = pid_task(cur_pid,PIDTYPE_PID);
    siblings=cur_task;
    childrens=cur_task;

    strcpy(out_payload, "\n");
    switch(token[0]) {
    case 'c':
        printk("成功傳進c!\n"); // test.
        sprintf(temp_str, "%d", cur_task->pid);
        strcpy(out_payload,cur_task->comm);
        strcat(out_payload,"(");
        strcat(out_payload,temp_str);
        strcat(out_payload,") ");
        strcat(out_payload,"\n");
        DFS(cur_task,1);
        /*list_for_each(cur_task_head, &cur_task->children) {
            childrens = list_entry(cur_task_head, struct task_struct, sibling);
            printk("children %d %s \n", childrens->pid, childrens->comm);
            sprintf(temp_str, "%d", childrens->pid); // int to string!!!!!
            strcat(out_payload,"    ");
            strcat(out_payload,childrens->comm);
            strcat(out_payload,"(");
            strcat(out_payload,temp_str);
            strcat(out_payload,") ");
            strcat(out_payload,"\n");
        }*/
        break;
    case 's':
        printk("成功傳進s!\n"); // test.
        list_for_each(cur_task_head, &cur_task->parent->children) {//parent->children
            temp_format++;
            siblings = list_entry(cur_task_head, struct task_struct, sibling);
            if(siblings->pid==(numberpid)) // Encounter pid the same as input,skip.
                continue;
            if(temp_format==1) {
                sprintf(temp_str, "%d", siblings->pid); // first time enter in this, I will use strcpy to cover "\n."
                strcpy(out_payload,siblings->comm);
                strcat(out_payload,"(");
                strcat(out_payload,temp_str);
                strcat(out_payload,") ");
                strcat(out_payload,"\n");
            } else {
                printk("sibling %d %s \n", siblings->pid, siblings->comm);
                sprintf(temp_str, "%d", siblings->pid); // int to string!!!!!
                strcat(out_payload,siblings->comm);
                strcat(out_payload,"(");
                strcat(out_payload,temp_str);
                strcat(out_payload,") ");
                strcat(out_payload,"\n");
            }
        }
        break;
    case 'p':
        printk("成功傳進p!\n"); // test.
        if(cur_task->parent == NULL) {
            printk("No Parent\n");
        } else {
            sprintf(temp_str, "%d", cur_task->pid);
            strcpy(out_payload,cur_task->comm);
            strcat(out_payload,"(");
            strcat(out_payload,temp_str);
            strcat(out_payload,") ");
            strcat(out_payload,"\n");
            do {
                cur_task=cur_task->parent;
                sprintf(temp_str, "%d", cur_task->pid);
                strcat(out_payload,cur_task->comm);
                strcat(out_payload,"(");
                strcat(out_payload,temp_str);
                strcat(out_payload,") ");
                if(cur_task->pid!=1) // recorrect format.
                    strcat(out_payload,"\n");
                else
                    continue;

            } while(cur_task->pid!=1);
            reverseWords(out_payload);
        }
        break;
    default:
        printk("Default!\n");
        break;
    }
    nlmsg_unicast(netlink_socket, out_skb, nlh->nlmsg_pid);
}



static int netlink_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = rece_msg_and_send,
    };
    netlink_socket=netlink_kernel_create(&init_net, NETLINK_USERSOCK, &cfg);
    if(netlink_socket==NULL) {
        printk("create fails.\n");
        return -1;
    }
    printk("create OK.\n");
    return 0;
}

static int __init ksimple_init(void)
{
    printk(KERN_INFO"Initializing Netlink Socket\n");
    //out_payload = kmalloc(sizeof(MAX_PAYLOAD),GFP_KERNEL);
    netlink_init();
    return 0;
}

static void __exit ksimple_exit(void)
{
    printk(KERN_INFO"bye\n");
    //kfree(out_payload);//test
    netlink_kernel_release(netlink_socket);
}

module_init(ksimple_init);
module_exit(ksimple_exit);
MODULE_AUTHOR("Chen Hua Yan F04056154 NCKUCSIE 3rd year B class.");
MODULE_DESCRIPTION("HW1");
MODULE_LICENSE("GPL");