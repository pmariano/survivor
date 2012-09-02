#include "app.h"

void InitializePathfinder();

int FindPath (int pathfinderID,int startingX, int startingY,
			  int targetX, int targetY);
int ReadPath(int pathfinderID,int currentX,int currentY, int pixelsPerFrame);
void EndPathfinder();

#define mapWidth  26
#define mapHeight 20
#define tileSize  40
#define numberPeople ENEMY_COUNT
#define notfinished 0
#define notStarted 0// path-related constants
#define found 1
#define nonexistent 2
#define walkable  0
#define unwalkable 1// walkability array constants

extern char walkability[mapWidth][mapHeight];
extern int pathStatus[numberPeople+1];
extern int xPath[numberPeople+1];
extern int yPath[numberPeople+1];

