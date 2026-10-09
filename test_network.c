/* POSIX adapter around the exact production server transport. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
typedef unsigned long u_long;
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "core.h"
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
static int ioctlsocket(int s,unsigned long request,unsigned long *v){int n=(int)*v;return ioctl(s,request,&n);}
#define PORT 0 /* Tests bind an ephemeral port and never interfere with a live host. */
#define MAGIC 0x44535431u
#define VERSION 3
static World world;
static Input inputs[16];
static SOCKET sock=INVALID_SOCKET;
static struct sockaddr_in serverAddr,peers[16];
static uint32_t tokens[16],sequences[16],clientToken,clientSeq;
static double lastSeen[16],lastInput[16],netNext;
static int dedicated=1;
static uint32_t random32(void){static uint32_t n=900;return ++n;}
static int samePeer(struct sockaddr_in *a,struct sockaddr_in *b){return a->sin_addr.s_addr==b->sin_addr.s_addr&&a->sin_port==b->sin_port;}
static int win_recvfrom(int s,char *b,int n,int flags,struct sockaddr *a,int *len){socklen_t size=(socklen_t)*len;int got=(int)recvfrom(s,b,n,flags,a,&size);*len=(int)size;return got;}
#define recvfrom win_recvfrom
#include "network.h"
#undef recvfrom
static void drain(int client){Snapshot s;while(recv(client,&s,sizeof(s),MSG_DONTWAIT)>0){}}
static void transmit(int client,Request *r){assert(sendto(client,r,sizeof(*r),0,(struct sockaddr*)&serverAddr,sizeof(serverAddr))==sizeof(*r));}
static Snapshot receive(int client){Snapshot s;int n=recv(client,&s,sizeof(s),0);assert(n==sizeof(s));assert(s.magic==MAGIC&&s.version==VERSION);return s;}
static int countPlayers(void){int n=0;for(int i=0;i<16;i++)n+=world.p[i].active!=0;return n;}
int main(void){
 assert(sizeof(Request)==40&&sizeof(Snapshot)==1064); /* under the usual 1500-byte MTU */
 initWorld(&world);assert(openNet(1));socklen_t len=sizeof(serverAddr);assert(!getsockname(sock,(struct sockaddr*)&serverAddr,&len));serverAddr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
 int clients[17];uint32_t session[16];
 for(int i=0;i<17;i++){clients[i]=socket(AF_INET,SOCK_DGRAM,0);assert(clients[i]>=0);struct timeval timeout={1,0};assert(!setsockopt(clients[i],SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)));Request r={0};r.magic=MAGIC;r.version=VERSION;r.type=1;transmit(clients[i],&r);serverNetwork(1);Snapshot s=receive(clients[i]);if(i<16){assert(s.type==2&&s.id==i&&s.token);session[i]=s.token;}else assert(s.type==4&&s.id==-1);drain(clients[i]);}
 assert(countPlayers()==16);
 Request r={0};r.magic=MAGIC;r.version=VERSION;r.type=3;r.token=session[0];r.seq=1;r.in.forward=1;
 transmit(clients[0],&r);serverNetwork(1.05);assert(inputs[0].forward==1);float z=world.p[0].z;stepWorld(&world,inputs,1.0f/60);assert(world.p[0].z>z);
 r.seq=2;r.token^=123;r.in.forward=-1;transmit(clients[0],&r);serverNetwork(1.06);assert(inputs[0].forward==1); /* wrong token */
 r.token=session[0];r.seq=1;transmit(clients[0],&r);serverNetwork(1.07);assert(inputs[0].forward==1); /* stale packet */
 r.seq=2;r.in.yaw=NAN;transmit(clients[0],&r);serverNetwork(1.08);assert(sequences[0]==1); /* invalid float */
 serverNetwork(1.4);assert(inputs[0].forward==0&&inputs[0].buttons==0); /* lost input stops movement */
 r.in.yaw=0;r.type=5;transmit(clients[0],&r);serverNetwork(1.41);assert(countPlayers()==15);
 drain(clients[16]);r.type=1;r.token=0;transmit(clients[16],&r);serverNetwork(1.42);Snapshot s=receive(clients[16]);assert(s.type==2&&s.id==0&&s.token!=session[0]);assert(countPlayers()==16&&inputs[0].forward==0);
 serverNetwork(12);assert(countPlayers()==0); /* disconnected slots reclaimed */
 for(int i=0;i<17;i++)close(clients[i]);stopNet();
 /* Listen host occupies one slot: fifteen remote players and a rejected sixteenth. */
 dedicated=0;initWorld(&world);addPlayer(&world,0);assert(openNet(1));len=sizeof(serverAddr);assert(!getsockname(sock,(struct sockaddr*)&serverAddr,&len));serverAddr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);netNext=0;
 for(int i=0;i<16;i++){clients[i]=socket(AF_INET,SOCK_DGRAM,0);assert(clients[i]>=0);struct timeval timeout={1,0};setsockopt(clients[i],SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));r.type=1;transmit(clients[i],&r);serverNetwork(20);s=receive(clients[i]);assert(s.type==(i<15?2:4));}
 assert(countPlayers()==16);for(int i=0;i<16;i++)close(clients[i]);stopNet();
 puts("PASS: real loopback UDP, 16-player dedicated capacity, host + 15 capacity, full rejection, authoritative movement, token checks, stale/invalid input, input timeout, disconnect, reconnect, idle expiry.");return 0;
}
