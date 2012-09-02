#include "app.h"

void InitializePathfinder();

int FindPath (int pathfinderID,int startingX, int startingY,
			  int targetX, int targetY);
void ReadPath(int pathfinderID,int currentX,int currentY, int pixelsPerFrame);
void EndPathfinder();

#define mapWidth  26
#define mapHeight 20
#define tileSize  40 
#define numberPeople ENEMY_COUNT

extern char walkability[mapWidth][mapHeight];
extern int pathStatus[numberPeople+1];
extern int xPath[numberPeople+1];
extern int yPath[numberPeople+1];

