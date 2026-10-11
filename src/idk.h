

#ifndef IDK_H
#define IDK_H

#include "raylib.h"
#include "consts.h"

//grid
#define db_GRID_CELL_SIZE 32
#define db_GRID_WIDTH 25
#define db_GRID_HEIGHT 22
#define db_GRID_SIZE (db_GRID_WIDTH*db_GRID_HEIGHT)
uint8_t db_grid[db_GRID_SIZE] = {0};

//animstuff
int db_animFrame = 0;
int db_animWait = 0;


//pallete
enum db_META_VALUES{
	db_VAL_P0,		//0 - player 0 (for spawn n stuff)
	db_VAL_P1,		//1 - player 1
	db_VAL_P2,		//2 - player 2
	db_VAL_P3,		//3 - player 3
	db_VAL_WALL,	//4 - wall
	db_VAL_BALL,	//5 - wall spawns
	db_VAL_EMPTY = 255
};
Color db_colorPallete[6] = {
	(Color){255,0,0,255},	//red, player 0 (player)
	(Color){255,255,0,255},	//yellow player 1
	(Color){0,255,0,255},	//green player 2
	(Color){0,0,255,255},	//blue player 3
	(Color){255,0,255,255},		//wall color
	(Color){255,255,255,255},		//ball color (n spawn)
};

//dudes
typedef struct db_Dude{
    Vector2 position;
    Vector2 velocity;
    int8_t targetIndex;		//what ent targeting
    int8_t ballTargetIndex;	//what ball targeting
    uint8_t stratState; 	//what its doing
    int16_t tileIndex;		//position i want go to
    bool alive;
}db_Dude;
Texture2D db_dudeTex;
#define db_DUDE_AMOUNT 4
db_Dude db_dudes[db_DUDE_AMOUNT] = {0};

enum db_DUDE_STRAT{
	db_DUDE_WAIT,
	db_DUDE_BOUNCE,
	db_DUDE_PATHFIND,
	db_DUDE_HIDE
};

//balls
typedef struct db_Ball{
    Vector2 position;
    Vector2 velocity;
    int8_t thrownIndex;
    bool thrown;
    bool active;
}db_Ball;
#define db_BALL_AMOUNT 5
db_Ball db_balls[db_BALL_AMOUNT] = {0};

//get index from pos
//simple util to convert vector2 to spot on grid
int16_t db_getGridIndexByVector(const Vector2 in){
	int16_t x = ((int16_t)in.x / db_GRID_CELL_SIZE);
	int16_t y = ((int16_t)in.y / db_GRID_CELL_SIZE);
	int16_t index = y * db_GRID_WIDTH + x;
	return (index >= 0 && index < db_GRID_SIZE)?index:-1;
}

//tl version. used in collsions so as not to taverse entire grid
int16_t db_getGridIndexByVectorTL(const Vector2 in){
	int16_t index = db_getGridIndexByVector(in);
	int16_t tlIndex = (index-1)-db_GRID_WIDTH;
	
	return (tlIndex >= 0 && tlIndex < db_GRID_SIZE)?tlIndex:-1;
}

//get vec pos from grid ind
//more importantly, get its center to make things easier
Vector2 db_getCentralPositionFromGridIndex(uint16_t ind){

	int x = ind % db_GRID_WIDTH;
	int y = ind / db_GRID_WIDTH;
	return (Vector2){x*db_GRID_CELL_SIZE+db_GRID_CELL_SIZE/2,y*db_GRID_CELL_SIZE+db_GRID_CELL_SIZE/2};
}

//simple angle function since I'm too dumb to figure out Raylib's implementation
float db_getAngleBetweenVecs(const Vector2 v1, const Vector2 v2){
	float x = v2.x - v1.x;
	float y = v2.y - v1.y;
	return atan2(y,x);
}

//brute force to find closest dude
int8_t db_getClosestDude(const uint8_t requestDude){
	int8_t closestDude = -1;
	float minDist = 10000000;
	for(int i = 0; i < 4; i++){
		if (i == requestDude) continue;
		if(!db_dudes[i].alive) continue;
		float checkDist = Vector2DistanceSqr(db_dudes[requestDude].position, db_dudes[i].position);
		if(checkDist < minDist){
			closestDude = i;
			minDist = checkDist;
		}
	}
	return closestDude;
}

//throw ball
void db_throwBallAt(const uint8_t ent1, const Vector2 target, const uint8_t ballInd){
	float angle = db_getAngleBetweenVecs(db_dudes[ent1].position, target);	
	db_balls[ballInd].velocity = Vector2Scale((Vector2){cos(angle),sin(angle)},2);
	db_balls[ballInd].thrown = true;
	db_dudes[ent1].ballTargetIndex = -1;
	db_dudes[ent1].stratState = db_DUDE_WAIT;
}

//respawn ball
//get random valid grid pos and reset it
void db_respawnBall(const uint8_t ballIndex){
	
	//REPLACE WITH GETVALIDGRID
	uint16_t ranInt = 0;
	while(db_grid[ranInt] == db_VAL_WALL){
		ranInt = GetRandomValue(0,db_GRID_SIZE-1);
	}
	
	db_balls[ballIndex].position = (Vector2){(ranInt%db_GRID_WIDTH)*db_GRID_CELL_SIZE+16,(ranInt/db_GRID_WIDTH)*db_GRID_CELL_SIZE+16};
	db_balls[ballIndex].thrown = false;
	db_balls[ballIndex].thrownIndex = -1;
	db_balls[ballIndex].velocity = Vector2Zero();
}

//get ran dude. returns -1 if everyone dead
//simple reduced forloop trickery to just check 3 vals instead of anything crazy
int8_t db_getRandomDude(const uint8_t requester){
	uint8_t ranIndex = GetRandomValue(0,3);
	for(uint8_t i = 0; i < 4; i++){
		if (ranIndex == requester || !db_dudes[ranIndex].alive){
			ranIndex++;
			if(ranIndex >= 4) ranIndex = 0;
		}else{
			printf("wot:%i\n",ranIndex);
			return ranIndex;
		}
	}
	return -1;
}

//get valid grid pos
//brute force to find empty area, but theoretically this shouldnt be too much an issue
uint16_t db_getValidGrid(){
	int returnGridInd = 0;
	while(db_grid[returnGridInd] == db_VAL_WALL){
		returnGridInd = GetRandomValue(0,db_GRID_SIZE-1);
	}
	return returnGridInd;
}

//do thing above but return position. i think this is obsolete
Vector2 db_getValidGridPos(){
	int g = db_getValidGrid();
	return (Vector2){(g%db_GRID_WIDTH)*db_GRID_CELL_SIZE, (g/db_GRID_WIDTH)*db_GRID_CELL_SIZE};
}

//get an open ball. can retrun -1 if nothing available
int8_t db_getValidBall(const uint8_t requester){
	
	uint8_t returnInd = GetRandomValue(0,db_BALL_AMOUNT-1);
	uint8_t cacheInd = returnInd;
	
	for(uint8_t i = 0; i < db_BALL_AMOUNT; i++){
		if(db_balls[returnInd].thrownIndex == -1){
			return returnInd;
		}else{
			returnInd++;
			if(returnInd > db_BALL_AMOUNT-1) returnInd = 0;
		}
	}
	
	return -1;
}

//brute force collision checks for each ball to dude
void db_collideWithBalls(const uint8_t dudeInd){
	
	for(uint8_t i = 0; i < db_BALL_AMOUNT; i++){
		if(CheckCollisionCircles(db_dudes[dudeInd].position, 16,db_balls[i].position, 8)){
				//if ball free						and dude ball desire is this			//OR dude already owned ball
			if((db_balls[i].thrownIndex == -1 &&  db_dudes[dudeInd].ballTargetIndex == i)|| db_balls[i].thrownIndex == dudeInd){
				db_balls[i].thrownIndex = dudeInd;
					//ball thrown		and not by me					
			}else if(db_balls[i].thrown && db_balls[i].thrownIndex != dudeInd){
				printf("dude [%i] eliminated by dude [%i]\n",dudeInd, db_balls[i].thrownIndex);
				db_dudes[dudeInd].alive = false;	//X_X
			}
		}
	}
}

//brute force full dist check of other ents
//at least do distSq so no sqrt
void db_checkDudesDist(uint8_t requester){
	uint8_t index = 0;
	for(int i =0; i < db_DUDE_AMOUNT-1; i++){
		if(index == requester || !db_dudes[index].alive) index++;
		if(Vector2Distance(db_dudes[requester].position, db_dudes[index].position) < 100){
			db_throwBallAt(requester, db_dudes[index].position, db_dudes[requester].ballTargetIndex);
		}
		index++;
	}
}

//figure out how to calc (short for calculate) hidey hole
//which is defined as position with 2 or more blocks adjacent
uint16_t db_getHideyHole(const uint8_t dudeInd){
	uint16_t returnIndex = 0;
	for(int i = 0; i < db_GRID_SIZE; i++){
		int x = i % db_GRID_WIDTH;
		int y = i / db_GRID_WIDTH;
		if(x < 1 || x > db_GRID_WIDTH-1 || y < 1 || y > db_GRID_HEIGHT-1) continue;
		
		int gl = db_grid[i-1];
		int gr = db_grid[i+1];
		int gt = db_grid[i-db_GRID_WIDTH];
		int gb = db_grid[i-db_GRID_WIDTH];
		
		if(gl + gr + gt + gb) {
			returnIndex = i;
			if(GetRandomValue(0,100) > 50) return returnIndex;
		}
	}
	return returnIndex;
}

//strat for pathfinding.
//pure AI slop and idrk what this does but it works well enough so i dont touch it.
void db_strategyPathFindTo(const uint8_t dudeInd){

	int currentInd = ((int)db_dudes[dudeInd].position.y / db_GRID_CELL_SIZE) * db_GRID_WIDTH + ((int)db_dudes[dudeInd].position.x / db_GRID_CELL_SIZE);	
	if(currentInd == db_dudes[dudeInd].tileIndex){
		db_dudes[dudeInd].stratState = db_DUDE_WAIT;
		return;
	}
	Vector2 target = (Vector2){(db_dudes[dudeInd].tileIndex%db_GRID_WIDTH)*db_GRID_CELL_SIZE+16, (db_dudes[dudeInd].tileIndex/db_GRID_WIDTH)*db_GRID_CELL_SIZE+16};
	float ang = db_getAngleBetweenVecs(db_dudes[dudeInd].position, target);
	Vector2 desiredVel = (Vector2){cos(ang), sin(ang)};

	Vector2 lookAheadPos = (Vector2){
		db_dudes[dudeInd].position.x + desiredVel.x * (db_GRID_CELL_SIZE * 0.5f),
		db_dudes[dudeInd].position.y + desiredVel.y * (db_GRID_CELL_SIZE * 0.5f)
	};
	int nextInd = ((int)lookAheadPos.y / db_GRID_CELL_SIZE) * db_GRID_WIDTH + ((int)lookAheadPos.x / db_GRID_CELL_SIZE);
	//DrawRectangleLines((currentInd % db_GRID_WIDTH) * db_GRID_CELL_SIZE, (currentInd / db_GRID_WIDTH) * db_GRID_CELL_SIZE, 32, 32, WHITE);
	if (db_grid[nextInd] == db_VAL_WALL || db_grid[currentInd] == db_VAL_WALL) {
		float altAng1 = ang + 90; // +90 degrees in radians (PI/2)
		float altAng2 = ang - 90; // -90 degrees in radians
		Vector2 testVel1 = (Vector2){cos(altAng1), sin(altAng1)};
		Vector2 testVel2 = (Vector2){cos(altAng2), sin(altAng2)};
		Vector2 checkPos1 = {db_dudes[dudeInd].position.x + testVel1.x * 16, db_dudes[dudeInd].position.y + testVel1.y * 16};
		int checkInd1 = ((int)checkPos1.y / db_GRID_CELL_SIZE) * db_GRID_WIDTH + ((int)checkPos1.x / db_GRID_CELL_SIZE);
		if (db_grid[checkInd1] != db_VAL_WALL) {
			db_dudes[dudeInd].velocity = testVel1; // Slide direction 1
		} else {
			db_dudes[dudeInd].velocity = testVel2; // Slide direction 2 (or fallback to zero)
		}
	} else {
		db_dudes[dudeInd].velocity = desiredVel;
	}
} 

//simple bounce (mainly dude 3 uses). 
//TODO: cut from full grid to tl
void db_strategyBounce(uint8_t dudeInd){
	
	for(int i = 0; i < db_GRID_SIZE; i++){
		if(db_grid[i] != db_VAL_WALL) continue;
		int gridX = (i % db_GRID_WIDTH)*db_GRID_CELL_SIZE+16;
		int gridY = (i/ db_GRID_WIDTH)*db_GRID_CELL_SIZE+16;
		Vector2 cCenter = (Vector2){gridX, gridY};
		if(CheckCollisionRecs((Rectangle){db_dudes[dudeInd].position.x-16,db_dudes[dudeInd].position.y-16,32,32},(Rectangle){gridX-16,gridY-16,32,32})){
			float vecX = db_dudes[dudeInd].position.x - gridX;
			float vecY = db_dudes[dudeInd].position.y - gridY;
				
			if( vecX*vecX>vecY*vecY ){
				if (vecX > 0){
					db_dudes[dudeInd].position.x = gridX + db_GRID_CELL_SIZE;
					db_dudes[dudeInd].velocity.x = -db_dudes[dudeInd].velocity.x;
				}
				else  if (vecX < 0){
					db_dudes[dudeInd].position.x = gridX - 32;
					db_dudes[dudeInd].velocity.x = -db_dudes[dudeInd].velocity.x;
				}
			}
			else if ( vecY*vecY>vecX*vecX ){
				if(vecY > 0){
					db_dudes[dudeInd].position.y = gridY + db_GRID_CELL_SIZE;
					db_dudes[dudeInd].velocity.y = -db_dudes[dudeInd].velocity.y;
				}
				else if(vecY < 0){
					db_dudes[dudeInd].position.y = gridY - 32;
					db_dudes[dudeInd].velocity.y = -db_dudes[dudeInd].velocity.y;
				}
			}
				
		}
	}
	
}

//hidey hole stuff
void db_strategyHide(uint8_t dudeInd){

}

//if nothing important then chill
void db_strategyWait(uint8_t dudeInd){
	db_dudes[dudeInd].velocity = Vector2Zero();
}

//figure out what each dude wants to do
void db_processStrategy(uint8_t dudeInd){
	switch (db_dudes[dudeInd].stratState){
		case db_DUDE_WAIT:
			db_strategyWait(dudeInd);
			break;
		case db_DUDE_PATHFIND:
			db_strategyPathFindTo(dudeInd);
			break;
		case db_DUDE_BOUNCE:
			db_strategyBounce(dudeInd);
			break;
		case db_DUDE_HIDE:
			db_strategyHide(dudeInd);
			break;
		default:
			break;	
	}
}

//simple ball bounce reflection
//it do be big
void db_ballBounce(uint8_t bIndex){

	const int BALL_RAD = 8;
	const int BALL_D = 16;
	int16_t checkInd = db_getGridIndexByVectorTL(db_balls[bIndex].position);	
	int x = 0;
	int y = 0;
	
	for(int i = 0; i < 9; i++){
		int gridX = (checkInd+x) % db_GRID_WIDTH;
		int gridY = (checkInd+(y*db_GRID_WIDTH))/ db_GRID_WIDTH;		
		
		const int gridXc = gridX*db_GRID_CELL_SIZE+16;
		const int gridYc = gridY*db_GRID_CELL_SIZE+16;		
		x++;
		if(x > 2){
			x = 0;
			y++;
		}	
		int gridInd = gridY*db_GRID_WIDTH+gridX;
		//if(gridInd < 0 || gridInd >= db_GRID_SIZE) continue;
		if(db_grid[gridInd] != db_VAL_WALL) continue;
		DrawCircleV((Vector2){gridXc,gridYc},2,GREEN);
		
		if(db_grid[gridInd] == db_VAL_WALL && CheckCollisionRecs((Rectangle){db_balls[bIndex].position.x-BALL_RAD,db_balls[bIndex].position.y-BALL_RAD,BALL_D,BALL_D},(Rectangle){gridXc-16,gridYc-16,32,32})){
			float vecX = db_balls[bIndex].position.x - gridXc;
			float vecY = db_balls[bIndex].position.y - gridYc;
			
			float vecX2 = vecX*vecX;
			float vecY2 = vecY*vecY;
				
			if( vecX2 > vecY2 ){
				db_balls[bIndex].velocity.x = -db_balls[bIndex].velocity.x;
				if (vecX > 0){
					db_balls[bIndex].position.x = gridXc+16+BALL_RAD;
				}
				else if (vecX < 0){
					db_balls[bIndex].position.x = gridXc-16-BALL_RAD;
				}
			}
			else if(vecY2 > vecX2 ){
				db_balls[bIndex].velocity.y = -db_balls[bIndex].velocity.y;
				if(vecY > 0){
					db_balls[bIndex].position.y = gridYc+16+BALL_RAD;
				}
				else if(vecY < 0){
					db_balls[bIndex].position.y = gridYc-16-BALL_RAD;
				}
			}
		}
	}
}

int ballUpdateFrame = 0;	//remove/update/fix
int stuckTimer = 0;			//same thing

//initialize all da stuffs
void initIdk(){
	
	//load level
	//change to handle levels 0-5
    FILE *file = fopen("test.lebeldaba", "rb");
    if (file == NULL) {
        perror("Failed to open file for reading");
    }
    int ret = fread(db_grid, sizeof(uint8_t), db_GRID_SIZE, file);
	fclose(file);
	
	
	//find spawn values to cache
	//BUT WITHER WHAT IF THERES NO SPAWNS OR TOO MANY????!??!1!?
	//idk maybe just make the levels correctly the first time
	int ballSpawnIndex = 0;
	for(int i = 0; i < db_GRID_SIZE; i++){
		switch(db_grid[i]){
			case db_VAL_P0:
				db_dudes[db_VAL_P0].position = db_getCentralPositionFromGridIndex(i);
				continue;
			case db_VAL_P1:
				db_dudes[db_VAL_P1].position = db_getCentralPositionFromGridIndex(i);
				continue;
			case db_VAL_P2:
				db_dudes[db_VAL_P2].position = db_getCentralPositionFromGridIndex(i);
				continue;
			case db_VAL_P3:
				db_dudes[db_VAL_P3].position = db_getCentralPositionFromGridIndex(i);
				continue;
			case db_VAL_BALL:
				db_balls[ballSpawnIndex].position = db_getCentralPositionFromGridIndex(i);
				ballSpawnIndex++;
				printf("spawned ball %i [%i]\n",ballSpawnIndex, db_grid[i]);
				continue;
		}
	}
	
	//draw dudes anim sprite
	//lowkenuinely dont remember the order. its like head torso arms legs
	RenderTexture2D loadT = LoadRenderTexture(db_GRID_CELL_SIZE*2,db_GRID_CELL_SIZE);
	BeginTextureMode(loadT);
	ClearBackground(BLANK);
	DrawRectangleLines(13,2,9,9, WHITE);
	DrawLine(17,11,17,20, WHITE);
	
	DrawLine(17,20,24,32, WHITE);
	DrawLine(17,20,8,32, WHITE);
	
	DrawLine(17,15,8,13, WHITE);
	DrawLine(17,15,24,13, WHITE);
	
	
	DrawRectangleLines(13+33,2,9,9, WHITE);
	DrawLine(17+32,11,17+32,20, WHITE);
	
	DrawLine(17+32,20,20+32,32, WHITE);
	DrawLine(17+32,20,12+32,32, WHITE);
	
	DrawLine(17+32,15,8+32,17, WHITE);
	DrawLine(17+32,15,24+32,17, WHITE);
	EndTextureMode();

	//ball consts
	for(int i = 0; i < db_BALL_AMOUNT; i++){
		db_balls[i].active = true;
		db_balls[i].thrownIndex = -1;
	}
	
	//set to image, flip, load as texture. may not need this
	Image fImg = LoadImageFromTexture(loadT.texture);
	UnloadRenderTexture(loadT);
	ImageFlipVertical(&fImg);
	db_dudeTex = LoadTextureFromImage(fImg);
	UnloadImage(fImg);

	
	//set dude consts
	for(int i = 0; i < db_DUDE_AMOUNT; i++){
		db_dudes[i].alive = true;
		db_dudes[i].targetIndex = -1;
		db_dudes[i].ballTargetIndex = -1;
		db_dudes[i].stratState = db_DUDE_PATHFIND;
		//dudes[i].velocity = (Vector2){1,1};
		int tInd = (((int)db_dudes[i].position.y)/db_GRID_CELL_SIZE)*db_GRID_WIDTH+((int)db_dudes[i].position.x)/db_GRID_CELL_SIZE;
		//db_dudes[i].tileIndex = db_getGridIndexByVector(db_balls[db_getValidBall(i)].position);
		db_dudes[i].tileIndex = tInd;
	}
}

int tmpTileSelect = 4;	//for map editing

//main update
void updateIdk(){

	ClearBackground(BLACK);		

	//map editing stuff
	if(IsMouseButtonDown(0)){
		Vector2 pos = GetMousePosition();
		int index = ((int)pos.y/db_GRID_CELL_SIZE)*db_GRID_WIDTH+((int)pos.x/db_GRID_CELL_SIZE);
		if(index >= 0 && index < db_GRID_SIZE){
			db_grid[index] = tmpTileSelect;
		}
	}
	if(IsMouseButtonDown(1)){
		Vector2 pos = GetMousePosition();
		int index = ((int)pos.y/db_GRID_CELL_SIZE)*db_GRID_WIDTH+((int)pos.x/db_GRID_CELL_SIZE);
		if(index >= 0 && index < db_GRID_SIZE-1){
			db_grid[index] = 255;
		}
	}
	
	//TODO: figure out how to handle stuck ents. may need to be individual?
	if(stuckTimer < 300){
		stuckTimer++;
	}else{
		stuckTimer = 0;
		for(int i = 0; i < db_DUDE_AMOUNT; i++){
			if(db_balls[db_dudes[i].ballTargetIndex].thrownIndex != i){
				//db_dudes[i].stratState = db_DUDE_PATHFIND;
				//db_dudes[i].tileIndex = db_getValidGrid();
			}
		}
	}

	if(IsKeyDown(KEY_ONE)){
	tmpTileSelect = 1;
	}
	if(IsKeyPressed(KEY_TWO)){
		tmpTileSelect = 2;
	}
	if(IsKeyPressed(KEY_THREE)){
		tmpTileSelect = 3;
	}
	if(IsKeyPressed(KEY_FOUR)){
		tmpTileSelect = 4;
	}
	if(IsKeyPressed(KEY_FIVE)){
		tmpTileSelect = 5;
	}
	if(IsKeyPressed(KEY_SIX)){
		tmpTileSelect = 6;
	}
	if(IsKeyPressed(KEY_SEVEN)){
		tmpTileSelect = 7;
	}
	if(IsKeyPressed(KEY_EIGHT)){
		tmpTileSelect = 8;
	}
	if(IsKeyPressed(KEY_NINE)){
		tmpTileSelect = 9;
	}
	if(IsKeyPressed(KEY_ZERO)){
		tmpTileSelect = 0;
	}
	
	//DUDE 0 (player)
	db_dudes[0].velocity = Vector2Zero();
	if(IsKeyDown(KEY_W)){
		db_dudes[0].velocity.y = -1;
	}
	if(IsKeyDown(KEY_A)){
		db_dudes[0].velocity.x = -1;
	}
	if(IsKeyDown(KEY_S)){
		db_dudes[0].velocity.y = 1;
	}
	if(IsKeyDown(KEY_D)){
		db_dudes[0].velocity.x = 1;
	}
	if(IsKeyDown(KEY_D)){
		db_dudes[0].velocity.x = 1;
	}
	if(IsKeyPressed(KEY_L)){
	    FILE *file = fopen("test.lebeldaba", "wb");
		if (file == NULL) {
			perror("Failed to open file for writing");
		}	
		for (int i = 0; i < db_GRID_SIZE; i++) {
			//if(db_grid[i] == 0) db_grid[i] = 255;
			fwrite(&db_grid[i], sizeof(uint8_t), 1, file);
		}
		fclose(file);
		printf("SAVED TEST LEVEL!\n");
	}
	if(IsMouseButtonPressed(0)){
		//if(ballTest.thrownIndex != -1){		
			//throwBallAt(0, GetMousePosition(),&ballTest);	
		//}
	}


	//anim flicker for dudes
	if(db_animWait < 6){		
		db_animWait++;
	}else{	
		db_animWait = 0;
		db_animFrame = (db_animFrame == 0)?1:0;
	}
	
	//BALLS
	for(int i = 0; i < db_BALL_AMOUNT; i++){
		if(db_balls[i].thrownIndex != -1 && !db_balls[i].thrown){
			db_balls[i].position = db_dudes[db_balls[i].thrownIndex].position;
		}
		db_ballBounce(i);
		db_balls[i].position = Vector2Add(db_balls[i].position, db_balls[i].velocity);
		DrawCircleLines(db_balls[i].position.x, db_balls[i].position.y,8,((db_balls[i].thrownIndex !=-1)?db_colorPallete[db_balls[i].thrownIndex]:WHITE));
	}
	
	//dude logic stuffs
	//===DUDE 1 [YELLOW]===
	//===END===
	//===DUDE 2 [GREEN]===
	//===END===
	//===DUDE 3 [BLUE]===
	if(db_dudes[db_VAL_P3].stratState == db_DUDE_WAIT){
		if(db_dudes[db_VAL_P3].ballTargetIndex == -1){
			db_dudes[db_VAL_P3].ballTargetIndex = db_getValidBall(db_VAL_P3);
		}else{
			if(db_balls[db_dudes[db_VAL_P3].ballTargetIndex].thrownIndex != db_VAL_P3){			
				db_dudes[db_VAL_P3].tileIndex = db_getGridIndexByVector(db_balls[db_dudes[db_VAL_P3].ballTargetIndex].position);
				db_dudes[db_VAL_P3].stratState = db_DUDE_PATHFIND;
			}else{
				db_dudes[db_VAL_P3].stratState = db_DUDE_BOUNCE;
			}
		}
	}
	//===END===
	
	//DUDES
	for(int i = 0; i < db_DUDE_AMOUNT; i++){
		if(!db_dudes[i].alive) continue;
		db_dudes[i].position = Vector2Add(db_dudes[i].position, db_dudes[i].velocity);
		db_collideWithBalls(i);
		if(db_dudes[i].ballTargetIndex != -1 && db_balls[db_dudes[i].ballTargetIndex].thrownIndex == i && !db_balls[db_dudes[i].ballTargetIndex].thrown){
			db_checkDudesDist(i);
		}
		db_processStrategy(i);
		int af = (db_dudes[i].velocity.x != 0 || db_dudes[i].velocity.y != 0)?db_animFrame:0;
		DrawTexturePro(db_dudeTex, 
			(Rectangle){af*32,0,32,32},
			(Rectangle){db_dudes[i].position.x-16,db_dudes[i].position.y-16,32,32},
			Vector2Zero(),
			0,
			db_colorPallete[i]);
			
		int gridX = db_dudes[i].tileIndex % db_GRID_WIDTH;
		int gridY = db_dudes[i].tileIndex / db_GRID_WIDTH;
		//DrawRectangleLines(gridX * db_GRID_CELL_SIZE, gridY * db_GRID_CELL_SIZE, db_GRID_CELL_SIZE, db_GRID_CELL_SIZE, db_colorPallete[i]);
	}
	
	//update with ball arr
	//if(balls[0].thrown && ballUpdateFrame < 300){
		//ballUpdateFrame++;
		//if(ballUpdateFrame >= 300){
			//respawnBall(0);
			//ballUpdateFrame = 0;
		//} 
	//}

	//draw grid
	for (int i = 0; i < db_GRID_SIZE; i++) {
		if(db_grid[i] == db_VAL_EMPTY) continue;
		int gridX = i % db_GRID_WIDTH;
		int gridY = i / db_GRID_WIDTH;
		DrawRectangleLines(gridX * db_GRID_CELL_SIZE, gridY * db_GRID_CELL_SIZE, db_GRID_CELL_SIZE, db_GRID_CELL_SIZE, Fade(db_colorPallete[db_grid[i]],1));
	}
	DrawText(TextFormat("Tile:%i",tmpTileSelect),10,10,20,WHITE);
}
#endif
