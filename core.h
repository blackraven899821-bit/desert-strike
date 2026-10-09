#ifndef DS_CORE_H
#define DS_CORE_H
#include <math.h>
#include <stdint.h>
#include <string.h>
#define MAX_PLAYERS 16
#define MAP_N 32
#define CELL 3.0f
#define PI 3.14159265359f
/* Original map: western long route, central crossing, eastern covered route. */
static const char *mapRows[MAP_N]={
"################################",
"#..............##..............#",
"#..............##..............#",
"#....cc........##..............#",
"#....cc........................#",
"#..............##......cc......#",
"#..............##......cc......#",
"###...###############.....######",
"#......#.......##..............#",
"#......#.......##..............#",
"#......#....c..##.....####......#",
"#......#.......##.....#..#......#",
"#......#..............#..#......#",
"#......#####...##.....#..#......#",
"#..............##.....#..#......#",
"#..............##.....#..#......#",
"#...cc.........##..............#",
"#...cc.........##..............#",
"#......#####...#####...#########",
"#......#.......................#",
"#......#.......................#",
"#......#....c.......c..........#",
"#......#.......................#",
"#......#.......######....#######",
"#......#.......#...............#",
"#..............#...............#",
"#..............#...............#",
"######....######.....cc........#",
"#..............................#",
"#..............................#",
"#..............................#",
"################################"};
typedef struct {float x,z,yaw,pitch; int32_t active,team,hp,kills,deaths,ammo,weapon,credits; float cool,reload,respawn; uint32_t shot;} Player;
typedef struct {float forward,side,yaw,pitch; uint32_t buttons;} Input;
static int selectedMap=0;
typedef struct {Player p[MAX_PLAYERS]; int32_t score[2]; float timeLeft; uint32_t tick,mapId;} World;
static float clampf(float x,float a,float b){return x<a?a:x>b?b:x;}
/* Three arena layouts use the same connected original arena, rotated or mirrored. */
static int tile(int x,int z){if(x<0||z<0||x>=MAP_N||z>=MAP_N)return '#';int px=x,pz=z;if(selectedMap==1)x=MAP_N-1-x;else if(selectedMap==2){int t=x;x=MAP_N-1-z;z=t;}int c=mapRows[z][x];if(c=='.'&&selectedMap==1&&((px==8&&pz==20)||(px==20&&pz==20)))return 'c';if(c=='.'&&selectedMap==2&&((px==12&&pz==20)||(px==23&&pz==20)))return 'c';return c;}
static int blocked(float x,float z){float r=.33f;for(int dz=-1;dz<=1;dz+=2)for(int dx=-1;dx<=1;dx+=2){int t=tile((int)floorf((x+dx*r)/CELL),(int)floorf((z+dz*r)/CELL));if(t=='#'||t=='c')return 1;}return 0;}
static int magSize(int weapon){static const int n[]={12,30,30,5};return n[weapon&3];}
static void spawnPlayer(World *w,int id){Player *p=&w->p[id];int team=p->team;float bx=team?73.5f:7.5f,bz=team?85.5f:4.5f;int slot=id/2;float yaw=team?PI:0,x=bx+(slot%4)*3,z=bz+(slot/4)*3;if(selectedMap==1){x=96-x;yaw=-yaw;}else if(selectedMap==2){float t=x;x=z;z=96-t;yaw+=PI*.5f;}p->x=x;p->z=z;p->yaw=yaw;p->pitch=0;p->hp=100;p->ammo=magSize(p->weapon);p->cool=0;p->reload=0;p->respawn=0;}
static void addPlayer(World *w,int id){memset(&w->p[id],0,sizeof(Player));w->p[id].active=1;w->p[id].team=id%2;w->p[id].credits=800;spawnPlayer(w,id);}
static void initWorld(World *w){memset(w,0,sizeof(*w));w->timeLeft=600;w->mapId=(uint32_t)selectedMap;}
static float wallDistance(float x,float z,float dx,float dz,float maxD){for(float t=0;t<maxD;t+=.12f){int c=tile((int)floorf((x+dx*t)/CELL),(int)floorf((z+dz*t)/CELL));if(c=='#'||c=='c')return t;}return maxD;}
static int visible(Player *a,Player *b){float dx=b->x-a->x,dz=b->z-a->z,d=sqrtf(dx*dx+dz*dz);return d<.1f||wallDistance(a->x,a->z,dx/d,dz/d,d)>=d;}
static void stepPlayer(World *w,int id,Input in,float dt){Player *p=&w->p[id];if(!p->active)return;if(p->hp<=0){p->respawn-=dt;if(p->respawn<=0)spawnPlayer(w,id);return;}if(!isfinite(in.forward)||!isfinite(in.side)||!isfinite(in.yaw)||!isfinite(in.pitch))return;
p->yaw=fmodf(in.yaw,PI*2);p->pitch=clampf(in.pitch,-1.3f,1.3f);p->cool=fmaxf(0,p->cool-dt);if(p->reload>0){p->reload-=dt;if(p->reload<=0)p->ammo=magSize(p->weapon);}
static const int price[]={0,700,1800,3200};for(int wi=1;wi<=3;wi++)if((in.buttons&(8u<<(wi-1)))&&p->credits>=price[wi]){p->credits-=price[wi];p->weapon=wi;p->ammo=magSize(wi);p->reload=0;break;}
float f=clampf(in.forward,-1,1),s=clampf(in.side,-1,1),l=sqrtf(f*f+s*s);if(l>1){f/=l;s/=l;}float speed=(in.buttons&4)?3.2f:6.5f;float dx=(-s*cosf(p->yaw)+f*sinf(p->yaw))*speed*dt,dz=(f*cosf(p->yaw)+s*sinf(p->yaw))*speed*dt;if(!blocked(p->x+dx,p->z))p->x+=dx;if(!blocked(p->x,p->z+dz))p->z+=dz;
if((in.buttons&2)&&p->ammo<magSize(p->weapon)&&p->reload<=0)p->reload=1.8f;
if((in.buttons&1)&&p->cool<=0&&p->reload<=0&&p->ammo>0){static const int damage[]={34,22,34,80};static const float cadence[]={.22f,.075f,.115f,.72f};p->ammo--;p->cool=cadence[p->weapon&3];p->shot++;float vx=sinf(p->yaw)*cosf(p->pitch),vz=cosf(p->yaw)*cosf(p->pitch),vy=sinf(p->pitch),limit=wallDistance(p->x,p->z,sinf(p->yaw),cosf(p->yaw),90)/fmaxf(.01f,cosf(p->pitch));int hit=-1;float nearest=limit;for(int j=0;j<MAX_PLAYERS;j++){Player *q=&w->p[j];if(j==id||!q->active||q->hp<=0||q->team==p->team)continue;float xx=q->x-p->x,zz=q->z-p->z,t=(xx*vx+zz*vz)/(vx*vx+vz*vz);float h=1.65f+t*vy;if(t>0&&t<nearest&&h>.05f&&h<1.95f){float a=xx-t*vx,b=zz-t*vz;if(a*a+b*b<.38f*.38f){hit=j;nearest=t;}}}if(hit>=0){Player *q=&w->p[hit];q->hp-=damage[p->weapon&3];if(q->hp<=0){q->hp=0;q->deaths++;q->respawn=3;p->kills++;p->credits+=300;w->score[p->team]++;}}}
}
static void stepWorld(World *w,Input *in,float dt){w->tick++;w->timeLeft-=dt;for(int i=0;i<MAX_PLAYERS;i++)stepPlayer(w,i,in[i],dt);if(w->timeLeft<=0){w->timeLeft=600;w->score[0]=w->score[1]=0;for(int i=0;i<MAX_PLAYERS;i++)if(w->p[i].active){w->p[i].kills=w->p[i].deaths=0;spawnPlayer(w,i);}}}
#endif
