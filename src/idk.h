

#ifndef IDK_H
#define IDK_H

#include "raylib.h"
#include "consts.h"

#define IDK_GRID_CELL_SIZE 32
#define IDK_GRID_WIDTH 25
#define IDK_GRID_HEIGHT 22
#define IDK_GRID_SIZE (IDK_GRID_WIDTH*IDK_GRID_HEIGHT)
uint8_t idkGrid[IDK_GRID_SIZE] = {0};


Color idkColorPallete[5] = {
	(Color){255,0,0,255},	//red, player 0 (player)
	(Color){255,255,0,255},	//yellow player 1
	(Color){0,255,0,255},	//green player 2
	(Color){0,0,255,255},	//blue player 3
	(Color){255,0,255,255},		//wall color

};
#define GRAVITY .1

typedef struct LilDude{
    Vector2 position;
    Vector2 velocity;
    int8_t targetIndex;
    bool alive;
}LilDude;
Texture2D dudeTex;
#define LILDUDE_AMOUNT 4
LilDude dudes[LILDUDE_AMOUNT] = {0};


typedef struct Ball{
    Vector2 position;
    Vector2 velocity;
    int thrownIndex;
    bool thrown;
    bool active;
}Ball;
Ball ballTest = {0};


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



void initIdk(){
	for(int i = 0; i < IDK_GRID_SIZE; i++){
	//idkGrid[i] = GetRandomValue(0,1);
		if(i % IDK_GRID_WIDTH == 0 || i / IDK_GRID_WIDTH == 0 ||
			i % IDK_GRID_WIDTH == IDK_GRID_WIDTH-1 || i / IDK_GRID_WIDTH == IDK_GRID_HEIGHT-1){
			idkGrid[i] = 1;
		}
	}

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

	dudes[0].position = (Vector2){300,300};
	dudes[1].position = (Vector2){600,300};
	dudes[2].position = (Vector2){600,600};
	dudes[3].position = (Vector2){300,600};
	
	
	dudes[3].velocity = (Vector2){1,1};
	
	for(int i = 0; i < 4; i++){
		dudes[i].alive = true;
		dudes[i].targetIndex = -1;
	}
	
	ballTest.position = (Vector2){400,400};
	ballTest.velocity = Vector2Zero();
	ballTest.active = true;
	ballTest.thrownIndex = -1;
	
}
int collisionDetect(int dudeIndex){
	
	int addX = 0;
	int addY = 0;
	
	addX = (dudes[dudeIndex].velocity.x > 0)? 16:-16;
	addY = (dudes[dudeIndex].velocity.y > 0)? 16:-16;

	int dGridIndex = ((int)(dudes[dudeIndex].position.y+addY)/IDK_GRID_CELL_SIZE)
		*IDK_GRID_WIDTH
		+((int)(dudes[dudeIndex].position.x+addX)/IDK_GRID_CELL_SIZE);
	return (dGridIndex>= 0 || dGridIndex < IDK_GRID_SIZE)?dGridIndex:-1;    
}
int collisionResolve(){}
int animFrame = 0;
int animWait = 0;

int getGridIndexByVector(const Vector2 in){
	int x = (int)(in.x / 32);
	int y = (int)(in.y / 32);
	int index = y * IDK_GRID_WIDTH + x;
	return (index >= 0 && index < IDK_GRID_SIZE)?index:-1;
}
int getGridIndexByVectorTL(const Vector2 in){
	int x = (int)(in.x / 32);
	int y = (int)(in.y / 32);
	int index = y * IDK_GRID_WIDTH + x;
	int tlIndex = (index-1)-IDK_GRID_WIDTH;
	return (tlIndex >= 0 && tlIndex < IDK_GRID_SIZE)?tlIndex:-1;
}

float getAngleBetweenVecs(Vector2 v1, Vector2 v2){
	float x = v2.x - v1.x;
	float y = v2.y - v1.y;
	return atan2(y,x);
}
int getClosestDude(int requestDude){
	int closestDude = -1;
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
void throwBallAt(int ent1, Vector2 target, Ball* b){
	float angle = getAngleBetweenVecs(dudes[ent1].position, target);	
	b->velocity = Vector2Scale((Vector2){cos(angle),sin(angle)},2);
	b->position = Vector2Add(b->position,Vector2Scale(b->velocity,10));
	b->thrown = true;
}
void respawnBall(){
	ballTest.position = (Vector2){GetRandomValue(100,600), GetRandomValue(100,600)};
	ballTest.thrown = false;
	ballTest.thrownIndex = -1;
	ballTest.velocity = Vector2Zero();
}

int getRandomDude(int requester){
	int ranIndex = GetRandomValue(0,3);
	for(int i = 0; i < 4; i++){
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

int ballUpdateFrame = 0;
int ranTarget = -1;
void updateIdk(){

	ClearBackground(BLACK);		
	
	if(IsMouseButtonDown(0)){
		Vector2 pos = GetMousePosition();
		int index = ((int)pos.y/IDK_GRID_CELL_SIZE)*IDK_GRID_WIDTH+((int)pos.x/IDK_GRID_CELL_SIZE);
		if(index >= 0 && index < IDK_GRID_SIZE-1){
			//idkGrid[index] = 1;
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
	if(IsMouseButtonPressed(0)){
		if(ballTest.thrownIndex != -1){		
			throwBallAt(0, GetMousePosition(),&ballTest);	
			//float angle = getAngleBetweenVecs(ballTest.position, GetMousePosition());
			//ballTest.velocity = Vector2Scale((Vector2){cos(angle),sin(angle)},2);
			//ballTest.position = Vector2Add(ballTest.position,Vector2Scale(ballTest.velocity,10));
			//ballTest.thrown = true;
		}
	}
	
	
	//DUDES 3 (BLUE);
	for(int i = 0; i < IDK_GRID_SIZE; i++){
		if(idkGrid[i] == 0) continue;
		int gridX = (i % IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16;
		int gridY = (i/ IDK_GRID_WIDTH)*IDK_GRID_CELL_SIZE+16;
		Vector2 cCenter = (Vector2){gridX, gridY};
		if(CheckCollisionRecs((Rectangle){dudes[3].position.x-16,dudes[3].position.y-16,32,32},(Rectangle){gridX-16,gridY-16,32,32})){
			float vecX = dudes[3].position.x - gridX;
			float vecY = dudes[3].position.y - gridY;
				
			if( vecX*vecX>vecY*vecY ){
				if (vecX > 0){
					dudes[3].position.x = gridX + IDK_GRID_CELL_SIZE;
					dudes[3].velocity.x = -dudes[3].velocity.x;
				}
				else  if (vecX < 0){
					dudes[3].position.x = gridX - 32;
					dudes[3].velocity.x = -dudes[3].velocity.x;
				}
			}
			else if ( vecY*vecY>vecX*vecX ){
				if(vecY > 0){
					//player.y = tile.y + tile.sizeY;	
					
					dudes[3].position.y = gridY + IDK_GRID_CELL_SIZE;
					dudes[3].velocity.y = -dudes[3].velocity.y;
				}
				else if(vecY < 0){
					dudes[3].position.y = gridY - 32;
					dudes[3].velocity.y = -dudes[3].velocity.y;
				}
			}
				
		}
	}
	//DUDE 2 (GREEN)
	if(ballTest.thrownIndex == -1){
		float ang = getAngleBetweenVecs(dudes[2].position, ballTest.position);
		dudes[2].velocity = (Vector2){cos(ang),sin(ang)};
	}else{
		if(dudes[2].targetIndex == -1){
			dudes[2].targetIndex = getRandomDude(2);
		}
		if(dudes[2].targetIndex != -1){
			float ang = getAngleBetweenVecs(dudes[2].position, dudes[dudes[2].targetIndex].position);
			dudes[2].velocity = (Vector2){cos(ang),sin(ang)};	
		}else{
			dudes[2].velocity = Vector2Zero();
		}
	}
	if(dudes[2].targetIndex != -1 && !dudes[dudes[2].targetIndex].alive) dudes[2].targetIndex = -1;
	
	if(animWait < 6){		
		animWait++;
	}else{	
		animWait = 0;
		animFrame = (animFrame == 0)?1:0;
	}
	
	//DUDES
	for(int i = 0; i < LILDUDE_AMOUNT; i++){
		dudes[i].position = Vector2Add(dudes[i].position, dudes[i].velocity);
		if(CheckCollisionCircles(dudes[i].position, 16,ballTest.position, 8)){
			if(ballTest.thrownIndex == -1 || ballTest.thrownIndex == i){
				ballTest.thrownIndex = i;
			}else{
				dudes[i].alive = false;
			}
		}
		if(ballTest.thrownIndex == i && !ballTest.thrown && Vector2DistanceSqr(dudes[i].position, dudes[dudes[i].targetIndex].position) < 10000){
			throwBallAt(i, dudes[dudes[i].targetIndex].position, &ballTest);
		}
		if(!dudes[i].alive) continue;
		uint8_t af = (dudes[i].velocity.x != 0 && dudes[i].velocity.y != 0)?animFrame:0;
		DrawTexturePro(dudeTex, 
			(Rectangle){af*32,0,32,32},
			(Rectangle){dudes[i].position.x-16,dudes[i].position.y-16,32,32},
			Vector2Zero(),
			0,
			idkColorPallete[i]);
		DrawCircleV(dudes[i].position, 2, WHITE);
	}
	
	if(ballTest.thrown && ballUpdateFrame < 300){
		ballUpdateFrame++;
		if(ballUpdateFrame >= 300){
			respawnBall();
			ballUpdateFrame = 0;
		} 
	}
	
	if(ballTest.thrownIndex != -1 && !ballTest.thrown){
		//ballTest.velocity = Vector2Scale(ballTest.velocity, .995f);
		ballTest.position = dudes[ballTest.thrownIndex].position;
	}else{
		ballTest.position = Vector2Add(ballTest.position, ballTest.velocity);
	}
	DrawCircleLines(ballTest.position.x, ballTest.position.y,8,((ballTest.thrownIndex !=-1)?idkColorPallete[ballTest.thrownIndex]:WHITE));
	

	for (int i = 0; i < IDK_GRID_SIZE; i++) {
		if(idkGrid[i] == 0) continue;
		int gridX = i % IDK_GRID_WIDTH;
		int gridY = i / IDK_GRID_WIDTH;
		DrawRectangleLines(gridX * IDK_GRID_CELL_SIZE, gridY * IDK_GRID_CELL_SIZE, IDK_GRID_CELL_SIZE, IDK_GRID_CELL_SIZE, idkColorPallete[4]);
	}
	
	int BALL_RAD = 8;
	int BALL_D = 16;
	int ballInd = getGridIndexByVectorTL(ballTest.position);	
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
		
		if(idkGrid[gridInd] != 0 && CheckCollisionRecs((Rectangle){ballTest.position.x-BALL_RAD,ballTest.position.y-BALL_RAD,BALL_D,BALL_D},(Rectangle){gridXc-16,gridYc-16,32,32})){
			float vecX = ballTest.position.x - gridXc;
			float vecY = ballTest.position.y - gridYc;
			
			float vecX2 = vecX*vecX;
			float vecY2 = vecY*vecY;
				
			if( vecX2 > vecY2 ){
				ballTest.velocity.x = -ballTest.velocity.x;
				if (vecX > 0){
					ballTest.position.x = gridXc+16+BALL_RAD;
				}
				else if (vecX < 0){
					ballTest.position.x = gridXc-16-BALL_RAD;
				}
			}
			else if(vecY2 > vecX2 ){
				ballTest.velocity.y = -ballTest.velocity.y;
				if(vecY > 0){
					ballTest.position.y = gridYc+16+BALL_RAD;
				}
				else if(vecY < 0){
					ballTest.position.y = gridYc-16-BALL_RAD;
				}
			}
				
		}
	}
}
#endif
