

#ifndef IDK_H
#define IDK_H

#include "raylib.h"
#include "consts.h"

//grid
#define IDK_GRID_CELL_SIZE 32
#define IDK_GRID_WIDTH 25
#define IDK_GRID_HEIGHT 22
#define IDK_GRID_SIZE (IDK_GRID_WIDTH*IDK_GRID_HEIGHT)
uint8_t idkGrid[IDK_GRID_SIZE] = {0};

//animstuff
uint8_t animFrame = 0;
uint8_t animWait = 0;

//pallete
Color idkColorPallete[5] = {
	(Color){255,0,0,255},	//red, player 0 (player)
	(Color){255,255,0,255},	//yellow player 1
	(Color){0,255,0,255},	//green player 2
	(Color){0,0,255,255},	//blue player 3
	(Color){255,0,255,255},		//wall color

};

//dudes
typedef struct LilDude{
    Vector2 position;
    Vector2 velocity;
    int8_t targetIndex;		//what ent targeting
    int8_t ballTargetIndex;	//what ball targeting
    bool alive;
}LilDude;
Texture2D dudeTex;
#define LILDUDE_AMOUNT 4
LilDude dudes[LILDUDE_AMOUNT] = {0};

//balls
typedef struct Ball{
    Vector2 position;
    Vector2 velocity;
    int8_t thrownIndex;
    bool thrown;
    bool active;
}Ball;
#define IDK_BALL_AMOUNT 5
Ball balls[IDK_BALL_AMOUNT] = {0};


/*
	BALL_SEEK - behavior when no ball
	TARGET_SEARCH - behavior when ball
	ATTACK - behavior when in range
	
	RED - you
	YELLOW - camper
		- on seek he finds ball
			-prioritize specials
			- fav is bomb
		-if someone else going to get ball and he farther, leave and find another
		- on search he finds corner or hall. tuck away place so he can lob it in advantage position
	GREEN - aggressor
		- on seek he finds ball
			- prioritize normal
			- for deflect
		- on search he picks target and relentlessly tracks them down
	BLUE - dumb
		- on seek he finds ball
			-fav is tracker. only prioritize this
		- on search
	
 */ 
 

int16_t getGridIndexByVector(const Vector2 in){
	int16_t x = ((int16_t)in.x / 32);
	int16_t y = ((int16_t)in.y / 32);
	int16_t index = y * IDK_GRID_WIDTH + x;
	return (index >= 0 && index < IDK_GRID_SIZE)?index:-1;
}
int16_t getGridIndexByVectorTL(const Vector2 in){
	int16_t index = getGridIndexByVector(in);
	int16_t tlIndex = (index-1)-IDK_GRID_WIDTH;
	
	return (tlIndex >= 0 && tlIndex < IDK_GRID_SIZE)?tlIndex:-1;
}

float getAngleBetweenVecs(const Vector2 v1, const Vector2 v2){
	float x = v2.x - v1.x;
	float y = v2.y - v1.y;
	return atan2(y,x);
}
int8_t getClosestDude(const uint8_t requestDude){
	int8_t closestDude = -1;
	float minDist = 10000000;
	for(int i = 0; i < 4; i++){
		if (i == requestDude) continue;
		if(!dudes[i].alive) continue;
		float checkDist = Vector2DistanceSqr(dudes[requestDude].position, dudes[i].position);
		if(checkDist < minDist){
			closestDude = i;
			minDist = checkDist;
		}
	}
	return closestDude;
}
void throwBallAt(const uint8_t ent1, const Vector2 target, const uint8_t ballInd){
	float angle = getAngleBetweenVecs(dudes[ent1].position, target);	
	balls[ballInd].velocity = Vector2Scale((Vector2){cos(angle),sin(angle)},2);
	//balls[ballInd].position = Vector2Add(balls[ballInd].position,Vector2Scale(balls[ballInd].velocity,10));
	balls[ballInd].thrown = true;
}
void respawnBall(const uint8_t ballIndex){
	
	uint16_t ranInt = 0;
	while(idkGrid[ranInt] == 1){
		ranInt = GetRandomValue(0,IDK_GRID_SIZE-1);
	}
	
	balls[ballIndex].position = (Vector2){(ranInt%IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16,(ranInt/IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16};
	balls[ballIndex].thrown = false;
	balls[ballIndex].thrownIndex = -1;
	balls[ballIndex].velocity = Vector2Zero();
}

int8_t getRandomDude(const uint8_t requester){
	uint8_t ranIndex = GetRandomValue(0,3);
	for(uint8_t i = 0; i < 4; i++){
		if (ranIndex == requester || !dudes[ranIndex].alive){
			ranIndex++;
			if(ranIndex >= 4) ranIndex = 0;
		}else{
			printf("wot:%i\n",ranIndex);
			return ranIndex;
		}
	}
	return -1;
}
uint16_t getValidGrid(){
	int returnGridInd = 0;
	while(idkGrid[returnGridInd] == 1){
		returnGridInd = GetRandomValue(0,IDK_GRID_SIZE-1);
	}
	return returnGridInd;
}
Vector2 getValidGridPos(){
	int g = getValidGrid();
	return (Vector2){(g%IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE, (g/IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE};
}
int8_t getValidBall(const uint8_t requester){
	
	uint8_t returnInd = GetRandomValue(0,IDK_BALL_AMOUNT-1);
	uint8_t cacheInd = returnInd;
	
	for(uint8_t i = 0; i < IDK_BALL_AMOUNT; i++){
		if(balls[returnInd].thrownIndex == -1){
			return returnInd;
		}else{
			returnInd++;
			if(returnInd > IDK_BALL_AMOUNT-1) returnInd = 0;
		}
	}
	
	return -1;
}

void collideWithBalls(const uint8_t dudeInd){
	
	for(uint8_t i = 0; i < IDK_BALL_AMOUNT; i++){
		if(CheckCollisionCircles(dudes[dudeInd].position, 16,balls[i].position, 8)){
			if(balls[i].thrownIndex == -1 || balls[i].thrownIndex == dudeInd){
				balls[i].thrownIndex = dudeInd;
				dudes[dudeInd].ballTargetIndex = i;
			}else{
				dudes[dudeInd].alive = false;
			}
		}
	}
	
}
void checkDudesDist(uint8_t requester){
	uint8_t index = 0;
	for(int i =0; i < LILDUDE_AMOUNT-1; i++){
		if(index == requester) index++;
		if(Vector2Distance(dudes[requester].position, dudes[index].position) < 100){
			throwBallAt(requester, dudes[index].position, dudes[requester].ballTargetIndex);
		}
		index++;
	}
}

uint16_t getHideyHole(const uint8_t dudeInd){
	uint16_t returnIndex = 0;
	for(int i = 0; i < IDK_GRID_SIZE; i++){
		int x = i % IDK_GRID_WIDTH;
		int y = i / IDK_GRID_WIDTH;
		if(x < 1 || x > IDK_GRID_WIDTH-1 || y < 1 || y > IDK_GRID_HEIGHT-1) continue;
		
		int gl = idkGrid[i-1];
		int gr = idkGrid[i+1];
		int gt = idkGrid[i-IDK_GRID_WIDTH];
		int gb = idkGrid[i-IDK_GRID_WIDTH];
		
		if(gl + gr + gt + gb) {
			returnIndex = i;
			if(GetRandomValue(0,100) > 50) return returnIndex;
		}
	}
	return returnIndex;
}

void strategyPathFindTo(const uint8_t dudeInd, Vector2 target){
	//Vector2 gridPos = (Vector2){(gridInd%IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE, (gridInd/IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE};
	float ang = getAngleBetweenVecs(dudes[dudeInd].position, target);
	Vector2 desiredVel = (Vector2){cos(ang), sin(ang)};

	int currentInd = ((int)dudes[dudeInd].position.y / IDK_GRID_CELL_SIZE) * IDK_GRID_WIDTH + ((int)dudes[dudeInd].position.x / IDK_GRID_CELL_SIZE);
	Vector2 lookAheadPos = (Vector2){
		dudes[dudeInd].position.x + desiredVel.x * (IDK_GRID_CELL_SIZE * 0.5f),
		dudes[dudeInd].position.y + desiredVel.y * (IDK_GRID_CELL_SIZE * 0.5f)
	};
	int nextInd = ((int)lookAheadPos.y / IDK_GRID_CELL_SIZE) * IDK_GRID_WIDTH + ((int)lookAheadPos.x / IDK_GRID_CELL_SIZE);
	DrawRectangleLines((currentInd % IDK_GRID_WIDTH) * IDK_GRID_CELL_SIZE, (currentInd / IDK_GRID_WIDTH) * IDK_GRID_CELL_SIZE, 32, 32, WHITE);
	if (idkGrid[nextInd] == 1 || idkGrid[currentInd] == 1) {
		float altAng1 = ang + 90; // +90 degrees in radians (PI/2)
		float altAng2 = ang - 90; // -90 degrees in radians
		Vector2 testVel1 = (Vector2){cos(altAng1), sin(altAng1)};
		Vector2 testVel2 = (Vector2){cos(altAng2), sin(altAng2)};
		Vector2 checkPos1 = {dudes[dudeInd].position.x + testVel1.x * 16, dudes[dudeInd].position.y + testVel1.y * 16};
		int checkInd1 = ((int)checkPos1.y / IDK_GRID_CELL_SIZE) * IDK_GRID_WIDTH + ((int)checkPos1.x / IDK_GRID_CELL_SIZE);
		if (idkGrid[checkInd1] != 1) {
			dudes[dudeInd].velocity = testVel1; // Slide direction 1
		} else {
			dudes[dudeInd].velocity = testVel2; // Slide direction 2 (or fallback to zero)
		}
	} else {
		dudes[dudeInd].velocity = desiredVel;
	}
} 
void strategyBounce(uint8_t dudeInd){
	
	for(int i = 0; i < IDK_GRID_SIZE; i++){
		if(idkGrid[i] == 0) continue;
		int gridX = (i % IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16;
		int gridY = (i/ IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16;
		Vector2 cCenter = (Vector2){gridX, gridY};
		if(CheckCollisionRecs((Rectangle){dudes[3].position.x-16,dudes[3].position.y-16,32,32},(Rectangle){gridX-16,gridY-16,32,32})){
			float vecX = dudes[dudeInd].position.x - gridX;
			float vecY = dudes[dudeInd].position.y - gridY;
				
			if( vecX*vecX>vecY*vecY ){
				if (vecX > 0){
					dudes[dudeInd].position.x = gridX + IDK_GRID_CELL_SIZE;
					dudes[dudeInd].velocity.x = -dudes[dudeInd].velocity.x;
				}
				else  if (vecX < 0){
					dudes[dudeInd].position.x = gridX - 32;
					dudes[dudeInd].velocity.x = -dudes[dudeInd].velocity.x;
				}
			}
			else if ( vecY*vecY>vecX*vecX ){
				if(vecY > 0){
					dudes[dudeInd].position.y = gridY + IDK_GRID_CELL_SIZE;
					dudes[dudeInd].velocity.y = -dudes[dudeInd].velocity.y;
				}
				else if(vecY < 0){
					dudes[dudeInd].position.y = gridY - 32;
					dudes[dudeInd].velocity.y = -dudes[dudeInd].velocity.y;
				}
			}
				
		}
	}
	
}
void ballBounce(uint8_t bIndex){

	const int BALL_RAD = 8;
	const int BALL_D = 16;
	int16_t checkInd = getGridIndexByVectorTL(balls[bIndex].position);	
	int x = 0;
	int y = 0;
	
	for(int i = 0; i < 9; i++){
		int gridX = (checkInd+x) % IDK_GRID_WIDTH;
		int gridY = (checkInd+(y*IDK_GRID_WIDTH))/ IDK_GRID_WIDTH;		
		
		const int gridXc = gridX*IDK_GRID_CELL_SIZE+16;
		const int gridYc = gridY*IDK_GRID_CELL_SIZE+16;		
		x++;
		if(x > 2){
			x = 0;
			y++;
		}	
		int gridInd = gridY*IDK_GRID_WIDTH+gridX;
		//if(gridInd < 0 || gridInd >= IDK_GRID_SIZE) continue;
		if(idkGrid[gridInd] == 0) continue;
		DrawCircleV((Vector2){gridXc,gridYc},2,GREEN);
		
		if(idkGrid[gridInd] != 0 && CheckCollisionRecs((Rectangle){balls[bIndex].position.x-BALL_RAD,balls[bIndex].position.y-BALL_RAD,BALL_D,BALL_D},(Rectangle){gridXc-16,gridYc-16,32,32})){
			float vecX = balls[bIndex].position.x - gridXc;
			float vecY = balls[bIndex].position.y - gridYc;
			
			float vecX2 = vecX*vecX;
			float vecY2 = vecY*vecY;
				
			if( vecX2 > vecY2 ){
				balls[bIndex].velocity.x = -balls[bIndex].velocity.x;
				if (vecX > 0){
					balls[bIndex].position.x = gridXc+16+BALL_RAD;
				}
				else if (vecX < 0){
					balls[bIndex].position.x = gridXc-16-BALL_RAD;
				}
			}
			else if(vecY2 > vecX2 ){
				balls[bIndex].velocity.y = -balls[bIndex].velocity.y;
				if(vecY > 0){
					balls[bIndex].position.y = gridYc+16+BALL_RAD;
				}
				else if(vecY < 0){
					balls[bIndex].position.y = gridYc-16-BALL_RAD;
				}
			}
				
		}
	}

}


void strategyHide(uint8_t dudeInd){

}

void strategyWait(uint8_t dudeInd){
	dudes[dudeInd].velocity = Vector2Zero();
}


int ballUpdateFrame = 0;
int dude2stuckTimer = 0;
int dude2StuckIndex = 0;
int ranValidGrid = -1;
int dude1FollowInd;

void initIdk(){
	
	
    FILE *file = fopen("test.lebeldaba", "rb");
    if (file == NULL) {
        perror("Failed to open file for reading");
    }
    int ret = fread(idkGrid, sizeof(uint8_t), IDK_GRID_SIZE, file);
	fclose(file);
	
	//for(int i = 0; i < IDK_GRID_SIZE; i++){
		//if(i % IDK_GRID_WIDTH == 0 || i / IDK_GRID_WIDTH == 0 ||
			//i % IDK_GRID_WIDTH == IDK_GRID_WIDTH-1 || i / IDK_GRID_WIDTH == IDK_GRID_HEIGHT-1){
			//idkGrid[i] = 1;
		//}
	//}

	RenderTexture2D loadT = LoadRenderTexture(IDK_GRID_CELL_SIZE*2,IDK_GRID_CELL_SIZE);
	BeginTextureMode(loadT);
	ClearBackground(BLANK);
	//DrawRectangleLines(0,0,IDK_GRID_CELL_SIZE,IDK_GRID_CELL_SIZE, RED);
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
	
	Image fImg = LoadImageFromTexture(loadT.texture);
	UnloadRenderTexture(loadT);
	ImageFlipVertical(&fImg);
	dudeTex = LoadTextureFromImage(fImg);
	UnloadImage(fImg);

	dudes[0].position = (Vector2){128,128};
	dudes[1].position = (Vector2){32*23,128};
	dudes[2].position = (Vector2){128,600};
	dudes[3].position = (Vector2){32*23,600};
	
	dude1FollowInd = getRandomDude(1);
	
	for(int i = 0; i < 4; i++){
		dudes[i].alive = true;
		dudes[i].targetIndex = -1;
		dudes[i].ballTargetIndex = -1;
	}
	
	balls[0].position = (Vector2){300,300};
	balls[1].position = (Vector2){500,300};
	
	balls[2].position = (Vector2){300,400};
	balls[3].position = (Vector2){500,400};
	balls[4].position = (Vector2){400,350};
	for(int i = 0; i < IDK_BALL_AMOUNT; i++){
		balls[i].active = true;
		balls[i].thrownIndex = -1;
	}
	
}
void updateIdk(){

	ClearBackground(BLACK);		
	
	if(IsMouseButtonDown(0)){
		Vector2 pos = GetMousePosition();
		int index = ((int)pos.y/IDK_GRID_CELL_SIZE)*IDK_GRID_WIDTH+((int)pos.x/IDK_GRID_CELL_SIZE);
		if(index >= 0 && index < IDK_GRID_SIZE-1){
			idkGrid[index] = 1;
		}
	}
		if(IsMouseButtonDown(1)){
		Vector2 pos = GetMousePosition();
		int index = ((int)pos.y/IDK_GRID_CELL_SIZE)*IDK_GRID_WIDTH+((int)pos.x/IDK_GRID_CELL_SIZE);
		if(index >= 0 && index < IDK_GRID_SIZE-1){
			idkGrid[index] = 0;
		}
	}
	
	//DUDE 0 (player)
	dudes[0].velocity = Vector2Zero();
	if(IsKeyDown(KEY_W)){
		dudes[0].velocity.y = -1;
	}
	if(IsKeyDown(KEY_A)){
		dudes[0].velocity.x = -1;
	}
	if(IsKeyDown(KEY_S)){
		dudes[0].velocity.y = 1;
	}
	if(IsKeyDown(KEY_D)){
		dudes[0].velocity.x = 1;
	}
	if(IsKeyDown(KEY_D)){
		dudes[0].velocity.x = 1;
	}
	if(IsKeyPressed(KEY_L)){
	    FILE *file = fopen("test.lebeldaba", "wb");
		if (file == NULL) {
			perror("Failed to open file for writing");
		}	
		for (int i = 0; i < IDK_GRID_SIZE; i++) {
			fwrite(&idkGrid[i], sizeof(uint8_t), 1, file);
		}
		fclose(file);
		printf("SAVED TEST LEVEL!\n");
	}
	if(IsMouseButtonPressed(0)){
		//if(ballTest.thrownIndex != -1){		
			//throwBallAt(0, GetMousePosition(),&ballTest);	
		//}
	}


	//DUDES 1[YELLOW]
	if(dudes[1].ballTargetIndex != 1){
		dudes[1].ballTargetIndex = getValidBall(1);
	}else{
		if(balls[dudes[1].ballTargetIndex].thrownIndex != 1){
			strategyPathFindTo(1, balls[dudes[1].ballTargetIndex].position);
		}else{			
			uint16_t hidehole = getHideyHole(1);
			Vector2 hidePos = (Vector2){(hidehole%IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE, (hidehole/IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE};
			if(Vector2Distance(dudes[1].position, hidePos) > 10){
				strategyPathFindTo(1,hidePos);
			}else{
				strategyWait(1);
			}
		}
		
	}
	//END
	

	//DUDES 2 [GREEN]
	if(dudes[2].ballTargetIndex == -1){
		dudes[2].ballTargetIndex = getValidBall(2);
	}else{
		if(balls[dudes[2].ballTargetIndex].thrownIndex != 2){
			strategyPathFindTo(2, balls[dudes[2].ballTargetIndex].position);
		}else{
			strategyWait(2);
		}
	}
	//END
	
	
	//DUDES 3 (BLUE);
	if(dudes[3].ballTargetIndex != 3){
		dudes[3].ballTargetIndex = getValidBall(3);
	}else{
		if(balls[dudes[3].ballTargetIndex].thrownIndex != 3){
			strategyPathFindTo(3, balls[dudes[3].ballTargetIndex].position);
		}else{
			strategyBounce(3);
		}
	}
	//--END
	
	
	
	
	/*
	// 1. Calculate desired velocity toward target/ball
	float ang = getAngleBetweenVecs(dudes[2].position, balls[0].position);
	if(ranValidGrid != -1){
		ang = getAngleBetweenVecs(dudes[2].position, (Vector2){(ranValidGrid%IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16,(ranValidGrid/IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16});
	}
	Vector2 desiredVel = (Vector2){cos(ang), sin(ang)};

	if (balls[0].thrownIndex != -1) {
		if (dudes[2].targetIndex == -1) {
			dudes[2].targetIndex = getRandomDude(2);
		}
		if (dudes[2].targetIndex != -1) {
			ang = getAngleBetweenVecs(dudes[2].position, dudes[dudes[2].targetIndex].position);
				if(ranValidGrid != -1){
					ang = getAngleBetweenVecs(dudes[2].position, (Vector2){(ranValidGrid%IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16,(ranValidGrid/IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16});
				}
			desiredVel = (Vector2){cos(ang), sin(ang)};
		} else {
			desiredVel = Vector2Zero();
		}
	}
	int currentInd = ((int)dudes[2].position.y / IDK_GRID_CELL_SIZE) * IDK_GRID_WIDTH + ((int)dudes[2].position.x / IDK_GRID_CELL_SIZE);
	Vector2 lookAheadPos = (Vector2){
		dudes[2].position.x + desiredVel.x * (IDK_GRID_CELL_SIZE * 0.5f),
		dudes[2].position.y + desiredVel.y * (IDK_GRID_CELL_SIZE * 0.5f)
	};
	int nextInd = ((int)lookAheadPos.y / IDK_GRID_CELL_SIZE) * IDK_GRID_WIDTH + ((int)lookAheadPos.x / IDK_GRID_CELL_SIZE);
	DrawRectangleLines((currentInd % IDK_GRID_WIDTH) * IDK_GRID_CELL_SIZE, (currentInd / IDK_GRID_WIDTH) * IDK_GRID_CELL_SIZE, 32, 32, WHITE);
	if (idkGrid[nextInd] == 1 || idkGrid[currentInd] == 1) {
		float altAng1 = ang + 90; // +90 degrees in radians (PI/2)
		float altAng2 = ang - 90; // -90 degrees in radians
		Vector2 testVel1 = (Vector2){cos(altAng1), sin(altAng1)};
		Vector2 testVel2 = (Vector2){cos(altAng2), sin(altAng2)};
		Vector2 checkPos1 = {dudes[2].position.x + testVel1.x * 16, dudes[2].position.y + testVel1.y * 16};
		int checkInd1 = ((int)checkPos1.y / IDK_GRID_CELL_SIZE) * IDK_GRID_WIDTH + ((int)checkPos1.x / IDK_GRID_CELL_SIZE);
		if (idkGrid[checkInd1] != 1) {
			dudes[2].velocity = testVel1; // Slide direction 1
		} else {
			dudes[2].velocity = testVel2; // Slide direction 2 (or fallback to zero)
		}
	} else {
		// Path is clear, move normally toward target
		dudes[2].velocity = desiredVel;
	}
	if(getGridIndexByVector(dudes[2].position) != dude2StuckIndex){
		dude2StuckIndex = getGridIndexByVector(dudes[2].position);
		dude2stuckTimer = 0;
	}else{
		dude2stuckTimer++;
		if(dude2stuckTimer >300){
			printf("UNSTICK\n");
			dude2stuckTimer = 0;
			ranValidGrid = (ranValidGrid == -1)?getValidGrid():-1;
			//needsNewPos = !needsNewPos;
			//dudes[2].targetIndex = getRandomDude(2);
		}
	}
	if(dudes[2].targetIndex != -1 && !dudes[dudes[2].targetIndex].alive) dudes[2].targetIndex = -1;
	*/
	//--END
	
		
	//anim flicker for dudes
	if(animWait < 6){		
		animWait++;
	}else{	
		animWait = 0;
		animFrame = (animFrame == 0)?1:0;
	}
	
	//BALLS
	for(int i = 0; i < IDK_BALL_AMOUNT; i++){
		if(balls[i].thrownIndex != -1 && !balls[i].thrown){
			balls[i].position = dudes[balls[i].thrownIndex].position;
		}
		ballBounce(i);
		balls[i].position = Vector2Add(balls[i].position, balls[i].velocity);
		DrawCircleLines(balls[i].position.x, balls[i].position.y,8,((balls[i].thrownIndex !=-1)?idkColorPallete[balls[i].thrownIndex]:WHITE));
	}
	
	//DUDES
	for(int i = 0; i < LILDUDE_AMOUNT; i++){
		if(!dudes[i].alive) continue;
		dudes[i].position = Vector2Add(dudes[i].position, dudes[i].velocity);
		collideWithBalls(i);
		if(dudes[i].ballTargetIndex != -1 && balls[dudes[i].ballTargetIndex].thrownIndex == i && !balls[dudes[i].ballTargetIndex].thrown){
			checkDudesDist(i);
		}
		uint8_t af = (dudes[i].velocity.x != 0 || dudes[i].velocity.y != 0)?animFrame:0;
		DrawTexturePro(dudeTex, 
			(Rectangle){af*32,0,32,32},
			(Rectangle){dudes[i].position.x-16,dudes[i].position.y-16,32,32},
			Vector2Zero(),
			0,
			idkColorPallete[i]);
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
	for (int i = 0; i < IDK_GRID_SIZE; i++) {
		if(idkGrid[i] == 0) continue;
		int gridX = i % IDK_GRID_WIDTH;
		int gridY = i / IDK_GRID_WIDTH;
		DrawRectangleLines(gridX * IDK_GRID_CELL_SIZE, gridY * IDK_GRID_CELL_SIZE, IDK_GRID_CELL_SIZE, IDK_GRID_CELL_SIZE, idkColorPallete[4]);
	}
	
	/*
	//ball collide
	int BALL_RAD = 8;
	int BALL_D = 16;
	int ballInd = getGridIndexByVectorTL(balls[0].position);	
	int x = 0;
	int y = 0;
	for(int i = 0; i < 9; i++){
		int gridX = (ballInd+x) % IDK_GRID_WIDTH;
		int gridY = (ballInd+(y*IDK_GRID_WIDTH))/ IDK_GRID_WIDTH;		
		
		const int gridXc = gridX*IDK_GRID_CELL_SIZE+16;
		const int gridYc = gridY*IDK_GRID_CELL_SIZE+16;		
		x++;
		if(x > 2){
			x = 0;
			y++;
		}	
		int gridInd = gridY*IDK_GRID_WIDTH+gridX;
		//if(gridInd < 0 || gridInd >= IDK_GRID_SIZE) continue;
		if(idkGrid[gridInd] == 0) continue;
		DrawCircleV((Vector2){gridXc,gridYc},2,GREEN);
		
		if(idkGrid[gridInd] != 0 && CheckCollisionRecs((Rectangle){balls[0].position.x-BALL_RAD,balls[0].position.y-BALL_RAD,BALL_D,BALL_D},(Rectangle){gridXc-16,gridYc-16,32,32})){
			float vecX = balls[0].position.x - gridXc;
			float vecY = balls[0].position.y - gridYc;
			
			float vecX2 = vecX*vecX;
			float vecY2 = vecY*vecY;
				
			if( vecX2 > vecY2 ){
				balls[0].velocity.x = -balls[0].velocity.x;
				if (vecX > 0){
					balls[0].position.x = gridXc+16+BALL_RAD;
				}
				else if (vecX < 0){
					balls[0].position.x = gridXc-16-BALL_RAD;
				}
			}
			else if(vecY2 > vecX2 ){
				balls[0].velocity.y = -balls[0].velocity.y;
				if(vecY > 0){
					balls[0].position.y = gridYc+16+BALL_RAD;
				}
				else if(vecY < 0){
					balls[0].position.y = gridYc-16-BALL_RAD;
				}
			}
				
		}
	}
	*/
}
#endif
