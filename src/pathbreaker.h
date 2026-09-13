


#ifndef PATHBREAKER_H
#define PATHBREAKER_H
#include "raylib.h"
#include "consts.h"

#define PB_MOVE_SPEED 3
#define PB_ENT_AMT 6 // 3*2
#define PB_ENT_TRAIL_AMT 4 // per ent

#define PB_WIDTH 1000
#define PB_HEIGHT 900

#define PB_H_WIDTH (PB_WIDTH/2)
#define PB_H_HEIGHT (PB_HEIGHT/2)
#define PB_ENT_RAD 25

typedef struct PBEnt{
    Vector2 position;
    Vector2 trail[PB_ENT_TRAIL_AMT];
    int8_t trailIndex;
    bool alive;
}PBEnt;

PBEnt pbEnts[PB_ENT_AMT] = {0};
int8_t selectedIndex = -1;
void initPathBreaker(){
	
	//this is slop. fix later 2 bi better
	Vector2 center = (Vector2){(PB_H_WIDTH),(PB_H_HEIGHT)};
	int spX = (PB_WIDTH/3);
	int spY = ((PB_H_HEIGHT)+325);
	Vector2 totalSize = (Vector2){spX * 2.0, spY * 1.0};
    Vector2 startOffset = Vector2Subtract(center, Vector2Scale(totalSize,0.5f));
    
    int index = 0;
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 3; x++) {
			pbEnts[index].position = Vector2Add(startOffset, (Vector2){x*spX, y*spY});
			pbEnts[index].trail[0] = pbEnts[index].position;
			pbEnts[index].trail[1] = pbEnts[index].position;
			pbEnts[index].trail[2] = pbEnts[index].position;
			pbEnts[index].trail[3] = pbEnts[index].position;
			pbEnts[index].alive = true;
            index++;
        }
    }
}
bool doLerp = false;
int lerpIndex = 0;
int lerpTimer = 0;
float lerpDist = 0;
int entReadyCount = 0;
void tryLerp(const int tInd){
	bool shouldLerpMoar = false;
	for(int i = 0; i < PB_ENT_AMT; i++){
		if(tInd < 0 || tInd > 2) continue;
		pbEnts[i].position = Vector2Lerp(pbEnts[i].trail[lerpIndex], pbEnts[i].trail[lerpIndex+1], lerpDist);
	}
}
float getInAngle(Vector2 testPos){
	int ang = GetRandomValue(0,360);
	if(testPos.x < 0){
		ang = 0;
	}
	if(testPos.x > 1000){
		ang = 180;
	}
	if(testPos.y < 0){
		ang = 90;
	}
	if(testPos.y > 900){
		ang = 270;
	}
	return (M_PI/180.0f) * ang;
}
void aiMove(){
	for(int i = 0; i < 3; i++){
			pbEnts[i].trail[0] = pbEnts[i].position;
		for(int j = 1; j < 4; j++){
			//int ang = GetRandomValue(0,360);
			float ang = getInAngle(pbEnts[i].trail[j-1]);
			pbEnts[i].trail[j].x = pbEnts[i].trail[j-1].x + (cos(ang) * 100.0f);
			pbEnts[i].trail[j].y = pbEnts[i].trail[j-1].y + (sin(ang) * 100.0f);
		}
	}
}

Color testColors[4] = {WHITE, ORANGE, YELLOW, BLACK};

const Rectangle redGoal = (Rectangle){PB_H_WIDTH-200,0,400,75};
const Rectangle blueGoal = (Rectangle){PB_H_WIDTH-200,PB_HEIGHT-75,400,75};

//replace slop with half loop for team
void checkCollision(int checkId){
	for(int i = 0; i < PB_ENT_AMT; i++){		
		if(i == checkId) continue;
		if(!pbEnts[i].alive) continue;
		int teamA = (checkId < 3) ? 1 : 2;
		int teamB = (i < 3) ? 1 : 2;
		if(teamA == teamB) continue;
		
		if(CheckCollisionCircles(pbEnts[checkId].position, PB_ENT_RAD, pbEnts[i].position, PB_ENT_RAD)){ //if circle collide, kill both
			pbEnts[checkId].alive = false;
			pbEnts[i].alive = false;
		}
		if(teamA == 1 && CheckCollisionCircleRec(pbEnts[checkId].position, PB_ENT_RAD, blueGoal)){
			pbEnts[checkId].alive = false;
		}
		if(teamA == 2 && CheckCollisionCircleRec(pbEnts[checkId].position, PB_ENT_RAD, redGoal)){
			pbEnts[checkId].alive = false;
		}
		for(int j = 0; j < PB_ENT_TRAIL_AMT-1; j++){  //if line collide,
			if(doLerp && CheckCollisionCircleLine(pbEnts[checkId].position, PB_ENT_RAD, pbEnts[i].trail[j],pbEnts[i].trail[j+1])){
				pbEnts[checkId].alive = false;
			}
		}
	}
}

void updatePathBreaker(){
	if(lerpIndex != -1){
		tryLerp(lerpIndex);
	}
	if(doLerp){
		if(lerpDist < 1){
			lerpDist+=.01f;
		}else{
			lerpDist = 0;
			lerpIndex++;
			if(lerpIndex >= PB_ENT_TRAIL_AMT){
				doLerp = false;
			};
		}
	}
	
	if(selectedIndex != -1){
		Vector2 mPos = GetMousePosition();
		int lastIndex = pbEnts[selectedIndex].trailIndex;
		Vector2 previousPoint = pbEnts[selectedIndex].trail[lastIndex - 1];

		if (Vector2DistanceSqr(previousPoint, pbEnts[selectedIndex].trail[lastIndex]) > 200) {
			Vector2 direction = { mPos.x - previousPoint.x, mPos.y - previousPoint.y };
			float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
			if (length > 0.0f) {
				direction.x /= length;
				direction.y /= length;
				
				pbEnts[selectedIndex].trail[pbEnts[selectedIndex].trailIndex].x = previousPoint.x + (direction.x * 100.0f);
				pbEnts[selectedIndex].trail[pbEnts[selectedIndex].trailIndex].y = previousPoint.y + (direction.y * 100.0f);
			}
		}
		else{
			pbEnts[selectedIndex].trail[pbEnts[selectedIndex].trailIndex] = GetMousePosition();
		}
	}
	
	if(IsMouseButtonPressed(0)){
		if(selectedIndex != -1){ //if ent selected
			//pbEnts[selectedIndex].trail[pbEnts[selectedIndex].trailIndex] = GetMousePosition();
			pbEnts[selectedIndex].trailIndex++;
			if(pbEnts[selectedIndex].trailIndex >= PB_ENT_TRAIL_AMT){ //if trail finished setting
				entReadyCount++;
				selectedIndex = -1;
				//pbEnts[selectedIndex].trailIndex = 0;
				if(entReadyCount > 2){	//if all 3 ready	
					aiMove();
					entReadyCount = 0;								
					lerpDist = 0;
					lerpIndex = 0;
					doLerp = true;
				}
			}
		}else{	//query for ent selection			
			for(int i = 0; i < 3; i++){
				if(CheckCollisionPointCircle(GetMousePosition(), pbEnts[i+3].position, PB_ENT_RAD)){
					selectedIndex = i+3;
					pbEnts[selectedIndex].trail[0] = pbEnts[selectedIndex].position;
					pbEnts[selectedIndex].trailIndex= 1;
				}
			}
		}
	}
	
	ClearBackground(GRAY);		
	DrawRectangleRec(redGoal,(Color){100,0,0,255});
	DrawRectangleRec(blueGoal,(Color){0,0,100,255});
	for(int i = 0; i < PB_ENT_AMT; i++){
		if(!pbEnts[i].alive) continue;
		checkCollision(i);
		
		Color teamColor = (i < 3) ? RED : BLUE;
		Color renderColor = (selectedIndex == i) ? YELLOW : teamColor;
		DrawCircleV(pbEnts[i].position,PB_ENT_RAD,renderColor);
		//for(int j = 0; j < pbEnts[i].trailIndex; j++){
		for(int j = 0; j < PB_ENT_TRAIL_AMT; j++){
			if (j < PB_ENT_TRAIL_AMT-1) DrawLineV(pbEnts[i].trail[j],pbEnts[i].trail[j+1], teamColor);
			DrawCircleV(pbEnts[i].trail[j],5,testColors[j]);
		}
	}
	DrawLine(0,PB_H_HEIGHT,PB_WIDTH,PB_H_HEIGHT,BLACK);
	DrawLine(PB_H_WIDTH,0,PB_H_WIDTH,PB_HEIGHT,BLACK);
	if (selectedIndex != -1) DrawText(TextFormat("%i",pbEnts[selectedIndex].trailIndex), 10,10, 30, BLACK);
}

#endif




