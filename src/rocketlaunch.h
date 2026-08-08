


#ifndef ROCKETLAUNCH_H
#define ROCKETLAUNCH_H
#include "raylib.h"
#include "consts.h"


const float ROCKET_SPEED = 500;
const unsigned int ROCKET_DOWN = 0;
const int ROCKET_SHOOT_WAIT_TIME = 2;
#define EXPLOSION_PARTICLE_COUNT 20
#define TRAIL_PARTICLE_COUNT 20

const Color ROCKET_COLOR = (Color){0,255,0,255};
const Color HIGHLIGHT_COLOR = (Color){255,255,0,50};
const Color GRID_COLOR = (Color){255,255,255,100};
const Color DEFENSE_COLOR = (Color){200,200,200,150};
const Color PLAYER_HIT_COLOR = (Color){0,200,200,100};
const Color AI_HIT_COLOR = (Color){250,0,0,100};


int defenseCount = 0;
int attackCount = 2;
int playerLives = 10;
int aiLives = 10;
bool rocketReadyToFire = false;
double lastTime;
int count = 0;
int turnStyle = 1;
int rocketPredictHitIndex = -1;


typedef struct Rocket{
    Vector2 position;
    int8_t direction;
    float speed;
    bool active;
}Rocket;

typedef struct Star{
    Vector2 position;
    float speed;
}Star;
typedef struct ParticlePoint{
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    double timestamp;
}ParticlePoint;

Rocket rocket;
static Texture2D rocketTexture;
static Texture2D gridTexture;


ParticlePoint explosion[EXPLOSION_PARTICLE_COUNT];
ParticlePoint trail[TRAIL_PARTICLE_COUNT];

Star stars[100];
    
Sound rocketLaunch;
Sound rocketExplode;


//shareable grid
//6*18
#define RL_GRID_WIDTH (GAME_WIDTH/CELL_SIZE)
#define RL_GRID_HEIGHT (GAME_HEIGHT/CELL_SIZE)
#define RL_GRID_SIZE (RL_GRID_WIDTH * RL_GRID_HEIGHT)
int rlGrid[108]= {0};
const int CELL_SIZE = 50;
const int ROCKET_H_WIDTH = 25;
const int ROCKET_H_HEIGHT = 25;

Texture2D setupGrid(){
    RenderTexture2D gridTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    BeginTextureMode(gridTexture);
    ClearBackground(BLANK);
    for (int x = 0; x <= GAME_WIDTH; x += CELL_SIZE) {
        DrawLine(x, 0, x, GAME_HEIGHT, GRAY);
    }        
    for (int y = 0; y <= GAME_HEIGHT; y += CELL_SIZE) {
        DrawLine(0, y, GAME_WIDTH, y, GRAY);
    }
    DrawLineEx((Vector2){0,(GAME_HEIGHT/2)+1},(Vector2){GAME_WIDTH,(GAME_HEIGHT/2)+1},2.0f,RED);
    DrawLineEx((Vector2){0,(GAME_HEIGHT/2)-1},(Vector2){GAME_WIDTH,(GAME_HEIGHT/2)-1},2.0f,BLUE);
    
    EndTextureMode();
    return gridTexture.texture;
}

Rocket craftRocket(){
        Rocket r = {0};        
        return r;
}
void getRocketTexture(Texture2D * tex){
        
        int x = 25; 
        int y = 25;
        
        Vector2 points[4];
        points[0] = (Vector2){x,y + ROCKET_H_HEIGHT};
        points[1] = (Vector2){x-ROCKET_H_WIDTH,y - ROCKET_H_HEIGHT};
        points[2] = (Vector2){x,y - 10};
        points[3] = (Vector2){x+ROCKET_H_WIDTH, y - ROCKET_H_HEIGHT};

        RenderTexture2D rocketTexture = LoadRenderTexture(50, 50);
        BeginTextureMode(rocketTexture);
            ClearBackground(BLANK);
        for(int i = 0; i < 4; i++){
            if(i < 3){
                DrawLine(points[i].x,points[i].y,points[i+1].x,points[i+1].y,WHITE);
            }else{
                DrawLine(points[3].x, points[3].y, points[0].x, points[0].y, WHITE);
            }
        }
        EndTextureMode();
    *tex = rocketTexture.texture;
}

//test
int getRocketHitPrediction(int* grid, int column, int width, int height, int direction) {
    if (column < 0 || column >= width) return -1; // Safety check!

    if (direction == 1) { // Moving Down
        for (int i = column; i < (width * height); i += width) {
            if (rlGrid[i] == 1 || rlGrid[i] == 4) return i;
        }
    } else { // Moving Up
        // Start at the bottom row for this column
        for (int i = ((height - 1) * width) + column; i >= 0; i -= width) {
            if (rlGrid[i] == 1 || rlGrid[i] == 4) return i;
        }
    }
    return -1;
}

void setupRocket(int column, int dir, Rocket* r){
    r->position.x = column*50 + 25;
    r->position.y = (dir == -1? 910 : -10);
    r->active = true;
    r->direction = dir;
    r->speed = ROCKET_SPEED;
}

float GetRandomFloatRange(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}
void setupStars(Star* stars){
    for(int i = 0; i < 100; i++){
        stars[i].position = (Vector2){GetRandomFloatRange(0,GAME_WIDTH),GetRandomFloatRange(0,GAME_HEIGHT)};
        stars[i].speed = GetRandomValue(20,200);
    }
}

//explosion for rocket (on explode)
void setupExplosion(ParticlePoint* p, int x, int y){
    for(int i = 0; i < EXPLOSION_PARTICLE_COUNT; i++){
        p[i].position = (Vector2){x,y};
        Vector2 vel = (Vector2){GetRandomFloatRange(-5,5),GetRandomFloatRange(-5,5)};
        p[i].velocity = Vector2Scale(vel,100);
        p[i].lifetime = GetRandomFloatRange(0.5f,1.0f);
        p[i].timestamp = GetTime();
        if(p[i].velocity.x == 0) p[i].velocity.x = 1;
        if(p[i].velocity.y == 0) p[i].velocity.y = 1;
    }
}
//trail for rocket (init)
void setupTrail(ParticlePoint* p, int x, int y, int dir){
    for(int i = 0; i < TRAIL_PARTICLE_COUNT; i++){
        p[i].position = (Vector2){x,y};
        p[i].velocity = Vector2Zero();
        p[i].velocity.x = GetRandomFloatRange(-200,200);
        p[i].velocity.y = dir*2;
        p[i].lifetime = GetRandomFloatRange(0.5f,1.0f);
        p[i].timestamp = GetTime();
        if(p[i].velocity.x == 0) p[i].velocity.x = 1;
        if(p[i].velocity.y == 0) p[i].velocity.y = 1;
    }
}

//ai place blocks
void setupAiDefense(int* grid, int size){
    for(int i = 0; i < 8; i++){
        int index = GetRandomValue(0,(size/2)-1);
        rlGrid[index] = 4;
    }
}



//set size and color
//init vars and objects
void initRocketLaunch(){
	
	getRocketTexture(&rocketTexture);    
	rocket = craftRocket();
	gridTexture = setupGrid(); //get tex
	rocketLaunch = LoadSound("../res/audio/rocketlaunch.wav");
	rocketExplode = LoadSound("../res/audio/rocketexplode.wav");
	setupTrail(&trail[0],200,200,1);
	setupExplosion(&explosion[0],-1000,-1000);
	setupStars(&stars[0]);
	
}
void resetRocketLaunch(){
		//reset score
		//reset turns
		//reset counters and timers
}

//update
void updateRocketLaunch(double now, double delta, float frameDelta){
			
	ClearBackground(BLACK);
  
	Vector2 mousePos = GetMousePosition();
	mousePos.x = ((int)mousePos.x / 50);
	mousePos.y = ((int)mousePos.y / 50);
	
	//rocket
	int angle = rocket.direction == -1? 0 : 180;
	rocket.position.y += (rocket.speed * rocket.direction) * frameDelta;
	Rectangle sourceRec = { 0.0f, 0.0f, (float)rocketTexture.width, (float)rocketTexture.height };
	Rectangle destRec = { rocket.position.x, rocket.position.y, (float)rocketTexture.width, (float)rocketTexture.height };
	Vector2 origin = { (float)rocketTexture.width / 2.0f, (float)rocketTexture.height / 2.0f };
	if (rocket.active) DrawTexturePro(rocketTexture, sourceRec, destRec, origin, angle, ROCKET_COLOR);

	if(rocketPredictHitIndex != -1 && rocket.active){
		
		// Calculate the base Y coordinate of the cell
		int cellY = (rocketPredictHitIndex / RL_GRID_WIDTH) * CELL_SIZE;
		
		// If moving DOWN (1), offset is 0. If moving UP (-1), offset is CELL_SIZE.
		// Mathematical way: (1 - rocket.dirY) / 2 * CELL_SIZE
		// Or more simply: (rocket.dirY == -1) ? CELL_SIZE : 0;
		int edgeOffset = (rocket.direction == -1) ? CELL_SIZE : 0;
		int targetEdgeY = cellY + edgeOffset;
		
		// Use the same directional logic to trigger the hit
		if(rocket.position.y * rocket.direction >= targetEdgeY * rocket.direction){
			rocket.active = false;
			rocket.speed = 0;
			
			rlGrid[rocketPredictHitIndex] = (rlGrid[rocketPredictHitIndex] == 1 ? 2 : 3);
			rocketReadyToFire = true;
			setupExplosion(&explosion[0], rocket.position.x, rocket.position.y);
			StopSound(rocketLaunch);
			PlaySound(rocketExplode);
		}
	}
	
	
	switch(turnStyle){
		case 0:
		
			if(defenseCount == 0){
				setupAiDefense(&rlGrid[0],RL_GRID_SIZE);
				defenseCount = 8;
			}
		
			if(IsMouseButtonPressed(0) && attackCount > 0 && !rocket.active){
				
				int gridX = (int)mousePos.x;
				int gridY = (int)mousePos.y;
				
				if (gridX < 0) gridX = 0;
				if (gridX >= RL_GRID_WIDTH) gridX = RL_GRID_WIDTH - 1;
				if (gridY < 0) gridY = 0;
				if (gridY >= RL_GRID_HEIGHT) gridY = RL_GRID_HEIGHT - 1;
				
				int index = (int)(mousePos.y*RL_GRID_WIDTH+mousePos.x);
				if (index >= RL_GRID_SIZE/2 && rlGrid[index] != 1){
				
					lastTime = GetTime();
					rocketReadyToFire = false;
					setupRocket(mousePos.x,-1,&rocket);
					rocketPredictHitIndex = getRocketHitPrediction(rlGrid, mousePos.x, RL_GRID_WIDTH, RL_GRID_HEIGHT, -1);
					attackCount--;
					PlaySound(rocketLaunch);
				
				}
			}
			if(rocket.active && rocket.position.y + (25*rocket.direction) < 0){
				aiLives--;
				rocketReadyToFire = true;
				rocket.active = false;
				rocket.speed = 0;
				setupExplosion(&explosion[0], rocket.position.x, rocket.position.y);
				StopSound(rocketLaunch);
				PlaySound(rocketExplode);
			}
			DrawRectangle(mousePos.x*50, 0,50,GAME_HEIGHT, HIGHLIGHT_COLOR);                        // Draw a color-filled rectangle
			break;
		case 1:
			
			if(IsMouseButtonPressed(0) && defenseCount < 8){
				int index = (int)(mousePos.y*RL_GRID_WIDTH+mousePos.x);
				if (index >= RL_GRID_SIZE/2 && rlGrid[index] != 1){
					rlGrid[index] =1;
					defenseCount++;
				}
			}
					
	
			if(delta > ROCKET_SHOOT_WAIT_TIME){
				if(!rocket.active && defenseCount == 8 && attackCount > 0){	
					lastTime = GetTime();
					rocketReadyToFire = false;
					int shotIndex = GetRandomValue(0,5); 
					setupRocket(shotIndex,1,&rocket);
					rocketPredictHitIndex = getRocketHitPrediction(rlGrid, shotIndex, RL_GRID_WIDTH, RL_GRID_HEIGHT, 1);
					attackCount--;
					PlaySound(rocketLaunch);
				}
			}
			
			if(rocket.active && rocket.position.y + (25*rocket.direction) >= GAME_HEIGHT){
				playerLives--;
				rocketReadyToFire = true;
				rocket.active = false;
				rocket.speed = 0;
				setupExplosion(&explosion[0], rocket.position.x, rocket.position.y);
				StopSound(rocketLaunch);
				PlaySound(rocketExplode);
			}
			
			DrawRectangleLines(mousePos.x*50, mousePos.y*50,50,50, BLUE);                        // Draw a color-filled rectangle
			break;
	
	}

	
	if(!rocket.active && attackCount == 0 && defenseCount == 8){
		memset(rlGrid, 0, sizeof(rlGrid));
		defenseCount = 0;
		attackCount = 2;
		turnStyle = (turnStyle == 0? 1:0);
	}
	
	//stars
	for(int i = 0; i < 100; i++){
		//stars[i].position.x = (int)(stars[i].position.x + stars[i].speed) % GAME_WIDTH;
		float newX = stars[i].position.x + (stars[i].speed * frameDelta);
		stars[i].position.x = fmodf(newX, (float)GAME_WIDTH);
		DrawRectangle(stars[i].position.x, stars[i].position.y, 2,2,WHITE);
	}

	//draw color squares (defense, hit, etc)
	for (int i = 0; i < RL_GRID_SIZE; i++) {
		int gridX = i % RL_GRID_WIDTH;
		int gridY = i / RL_GRID_WIDTH;
		
		switch (rlGrid[i]){
			case 1:
				DrawRectangle(gridX*CELL_SIZE,gridY*CELL_SIZE, CELL_SIZE, CELL_SIZE, DEFENSE_COLOR);
				break;
			case 2:
				DrawRectangle(gridX*CELL_SIZE,gridY*CELL_SIZE, CELL_SIZE, CELL_SIZE, PLAYER_HIT_COLOR);
				break;
			case 3:
				DrawRectangle(gridX*CELL_SIZE,gridY*CELL_SIZE, CELL_SIZE, CELL_SIZE, AI_HIT_COLOR);
				break;
			default: break;
		}
	}

	for(int i = 0; i < EXPLOSION_PARTICLE_COUNT; i++){
		explosion[i].position = Vector2Add(explosion[i].position, Vector2Scale(explosion[i].velocity, frameDelta));
		if(now - explosion[i].timestamp > explosion[i].lifetime){
			explosion[i].velocity = Vector2Zero();
		}
		if(explosion[i].velocity.x != 0) DrawRectangle(explosion[i].position.x, explosion[i].position.y, 5,5,ROCKET_COLOR);
	}
	
	trail[count].position = (Vector2){rocket.position.x,rocket.position.y-(30*rocket.direction)};
	count = (count + 1) % TRAIL_PARTICLE_COUNT;
	
	if(rocket.speed != 0){
		for(int i = 0; i < TRAIL_PARTICLE_COUNT; i++){
			trail[i].position = Vector2Add(trail[i].position, Vector2Scale(trail[i].velocity, frameDelta));
			DrawRectangle(trail[i].position.x, trail[i].position.y, 5,5,WHITE);
		}
	}
	
	DrawTexture(gridTexture, 0, 0, GRID_COLOR);
	DrawText(TextFormat("LIVES: %i", aiLives), 10,10, 30, WHITE);
	DrawText(TextFormat("att: %i", attackCount), 10,50, 20, WHITE);
	DrawText(TextFormat("def: %i", defenseCount), 10,70, 20, WHITE);
	DrawText(TextFormat("t: %f", (delta)), 10,90, 20, WHITE);
	DrawText(TextFormat("indeX: %i", rocketPredictHitIndex), 10,110, 20, WHITE);
	DrawText(TextFormat("LIVES: %i", playerLives), 10,860, 30, WHITE);
}

#endif




