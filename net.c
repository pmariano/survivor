#include <SDL_net.h>
#include <math.h>
#include "app.h"

#define PACKET_SIZE 1400
#define TIME_SLICE 200 // ms

void net_init(App *app)
{
	app->net.packet = SDLNet_AllocPacket(PACKET_SIZE);
	if(!app->net.packet) {
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(1);
	}
}

void net_setup(App *app, char *host, int port)
{
	app->net.udpsock=SDLNet_UDP_Open(port);
	if(!app->net.udpsock) {
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		exit(1);
	}

	if(SDLNet_ResolveHost(&app->net.address, host, port)) {
		printf("SDLNet_ResolveIP: %s\n", SDLNet_GetError());
		exit(1);
	}

	int channel = SDLNet_UDP_Bind(app->net.udpsock, -1, &app->net.address);
	if(channel==-1) {
		printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		exit(1);
	}
}

typedef struct {
	int time;
	int dist;
	int prio;
	int index;
} PrioQueue;

typedef struct {
	unsigned char x:10;
	unsigned char y:10;
	unsigned char index:10;
	unsigned char type:4; // enemy type or weapon held
} MsgPosItem;

typedef struct {
	
	unsigned char x:10;
	unsigned char y:10;
	unsigned char index:10; // 0,1=players; 2+=enemies
	unsigned char type:4; // enemy type or weapon held
} MsgPos;

typedef struct {
	unsigned char keysym;
	unsigned char type:1; // 0=keydown, 1=keyup
} MsgEvt;

enum {
	MSG_EVT = 0,
	MSG_POS = 1,
};

typedef struct {
	char type:1; // 0=evt, 1=pos
	char count:7; //
	union {
		MsgPos pos;
		MsgEvt evt;
	};
} Msg;

static inline int dist(Body *a, Body *b)
{
	return sqrt(pow(a->pos.x-b->pos.x,2)+pow(a->pos.y-b->pos.y,2));
}

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

static inline int dist_players(App *app, Body *body)
{
	return min(
			dist(&app->game.player1.body, body),
			dist(&app->game.player2.body, body)
	);
}

static int prio_cmp(const void *a, const void *b)
{
	PrioQueue *aa = (PrioQueue *)a;
	PrioQueue *bb = (PrioQueue *)b;
	return aa->prio - bb->prio;
}

void net_exchange(App *app)
{
	PrioQueue prio[2+ENEMY_COUNT];
	int i;
	if(app->net.role == NET_STANDALONE)
		return;

	if(app->net.role == NET_SERVER) {
		// populate priority queue (sort by exchange_time and proximity)
		int max_dist = sqrt(pow(mapWidth,2)+pow(mapHeight,2));

		prio[0].time = 0;
		prio[0].dist = 0;

		prio[1].time = 0;
		prio[1].dist = 0;

		for(i=0;i<ENEMY_COUNT;i++) {
			prio[2+i].time = app->net.exchange_time[i];
			prio[2+i].dist = dist_players(app, &app->game.enemies[i].body);
		}

		for(i=0;i<2+ENEMY_COUNT;i++) {
			prio[i].prio = prio[i].time/TIME_SLICE + prio[i].dist*TIME_SLICE/max_dist;
			prio[i].index = i;
		}

		qsort(prio, 2+ENEMY_COUNT, sizeof(PrioQueue), prio_cmp);

		// fill a packet
		
		i=0;
		app->net.packet.len=0;
		Msg *msg = (Msg*)app->net.packet->data;
		while(app->net.packet.len+sizeof(Msg) < app->net.packet.maxlen && i < 2+ENEMY_COUNT) {
			msg->type = MSG_POS;
			msg++;
			app->net.packet.len+=sizeof(Msg);
		}
	} else {
		for(i=0; i< app->net.evt_queue_count; i++) {
			msg->keysym = app->net.evt_queue[i];
			
		}
		
		app->net.evt_queue_count = 0;
	}

	// send a packet
	app->net.packet->channel=channel;

	int numsent = SDLNet_UDP_Send(app->net.udpsock, app->net.packet->channel, app->net.packet);
	if(!numsent) {
		printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
		//exit(1);
	}

	// receive all pending events and queue on sdl
	while(SDLNet_UDP_Recv(app->net.udpsock, app->net.packet)) {
		Msg *msg = (Msg *)app->net.packet->data;
		while(app->net.packet->len>0) {
			switch(msg->type) {
				case MSG_EVT:
					msg->keysym = 
					// TODO change pressed state (SDL does not change?)
				break;
				case MSG_POS:
				break;
			}
			msg++;
			app->net.packet->len-=sizeof(Msg);
		}
	}
}

void net_terminate(App *app) {
	SDLNet_FreePacket(app->net.packet);
	SDLNet_UDP_Close(app->net.udpsock);
}

