


#ifndef AVALANCHE_H
#define AVALANCHE_H
#include "raylib.h"
#include "consts.h"


#define AV_GRID_WIDTH 10
#define AV_GRID_HEIGHT 18
#define AV_GRID_SIZE AV_GRID_WIDTH * AV_GRID_HEIGHT 
int avGrid[AV_GRID_SIZE] = {0};

//Color blockColors[] = {    
    //(Color){0,0,0,0}, //empty 0
    //(Color){0,0,0,255}, //black 1
    //(Color){255,0,0,255}, //red 2
    //(Color){0,255,0,255}, //green 3 
//};


Color blockColors[9] = {    
    (Color){0,0,0,0}, //empty 0
    (Color){0,0,0,255}, //black 1
    (Color){255,0,0,255}, //red 2
    (Color){0,255,0,255}, //green 3 
    (Color){0,0,255,255},  //blue 4
    (Color){255,255,0,255}, //yellow 5
    (Color){255,0,255,255},  //cyan 6
    (Color){0,255,255,255}, //purple 7
    (Color){127,127,127,255} //grey 8
};

static Texture2D avalancheBG;

Sound blockMatchSound;
Sound blockChainSound;
Sound blockLandSound;

int blockDropTimer = 0;
int blockDropCache = 30;
int recursiveCount = 0;


void spawnBlock(){
	int col = GetRandomValue(0,9);
	if (avGrid[col] != 0) return;
	avGrid[col] = GetRandomValue(1,(sizeof(blockColors)/sizeof(int))-1);
	blockDropCache--;
}
bool isValidCoordinate(int x, int y) {
	return x >= 0 && x < AV_GRID_WIDTH && y >= 0 && y <= AV_GRID_HEIGHT;
}

bool hasAirGap(int index){
	
	int row = index / AV_GRID_WIDTH;
	int steps = AV_GRID_HEIGHT - row;
	
	//printf("CHECKING n %i FROM r %i, i %i\n", steps,row, index);
	//if any y val down is 0, there is air gap
	for(int i = 0; i < steps; i++){
		int targetIndex = index + (i * AV_GRID_WIDTH);
		if(avGrid[targetIndex] == 0){
			return true;
		};
	}
	return false;
}
bool isFalling(int index){
	int bIndex = index+AV_GRID_WIDTH;
	return (bIndex < AV_GRID_SIZE && avGrid[bIndex] == 0);
}
void checkMatch(int index, int val){
	if(recursiveCount > 100){
		printf("RECURSION TOO BIG!!");
		exit(1);
	}
	int lIndex = index-1;
	int rIndex = index+1;
	int tIndex = index-AV_GRID_WIDTH;
	int bIndex = index+AV_GRID_WIDTH;
	
	if(isFalling(index)){
		return;
	} 
	int cacheIndex = -1;
	bool match = false;
	int currentRow = index / AV_GRID_WIDTH;
	
	if(lIndex > 0 && (lIndex / AV_GRID_WIDTH == currentRow) && !isFalling(lIndex) && val == avGrid[lIndex]){
		avGrid[lIndex] = 0;
		avGrid[index] = 0;
		cacheIndex = lIndex;
		match = true;
		recursiveCount++;
		checkMatch(cacheIndex, val);
	}
	if(rIndex < AV_GRID_SIZE && (rIndex / AV_GRID_WIDTH == currentRow) && !isFalling(rIndex) && val == avGrid[rIndex]){
		avGrid[rIndex] = 0;
		avGrid[index] = 0;
		cacheIndex = rIndex;
		match = true;
		recursiveCount++;
		checkMatch(cacheIndex, val);
	}
	if(tIndex > 0 && val == avGrid[tIndex]){
		avGrid[tIndex] = 0;
		avGrid[index] = 0;
		cacheIndex = tIndex;
		match = true;
		recursiveCount++;
		checkMatch(cacheIndex, val);
	}
	if(bIndex < AV_GRID_SIZE && val == avGrid[bIndex]){
		avGrid[bIndex] = 0;
		avGrid[index] = 0;
		cacheIndex = bIndex;
		match = true;
		recursiveCount++;
		checkMatch(cacheIndex, val);
	}
	
	if(match){	
		blockDropCache+=2;
		PlaySound(blockMatchSound);
		//printf("checking i %i v %i\n",cacheIndex, val);
		//recursiveCount++;
		//checkMatch(cacheIndex, val);
	} 
}

static Texture2D setupAvalancheBG(const int sizeX,const int sizeY){
    RenderTexture2D avalancheT = LoadRenderTexture(sizeX, sizeY);
    BeginTextureMode(avalancheT);
    
	blockMatchSound = LoadSound("../res/audio/blockstackclear.wav");
	blockChainSound = LoadSound("../res/audio/chainclear.wav");
	blockLandSound = LoadSound("../res/audio/blockland.wav");
    
    ClearBackground(WHITE);
    for (int x = 0; x <= sizeX; x += CELL_SIZE) {
        DrawLine(x, 0, x, sizeY, GRAY);
    }
    EndTextureMode();
    return avalancheT.texture;
}

void initAvalanche(){
	avalancheBG = setupAvalancheBG(500, 900);
	//for (int i = 0; i < 10; i++) {
		//avGrid[i] = 2;
	//}
}

void updateRow(const int row){
	if(row == AV_GRID_HEIGHT-1) return;
	int startIndex = row * AV_GRID_WIDTH;
	recursiveCount = 0;
	for(int i = startIndex; i < startIndex+AV_GRID_WIDTH; i++){
		if(avGrid[i] == 0) continue;
		
		checkMatch(i, avGrid[i]);
		if(recursiveCount > 1) printf("==REC %i\n",recursiveCount); 
		
		int below = i + AV_GRID_WIDTH;
		if( avGrid[below] == 0){
			avGrid[below] = avGrid[i];
			avGrid[i] = 0;
			
			if(!hasAirGap(below)){
				PlaySound(blockLandSound);
			}
		}
		
	}
}
void updateScreen(){
	for(int i = AV_GRID_HEIGHT-1; i > -1; i--){
		updateRow(i);
	}
}

int framec = 0; //frame update (replace with delta)
int rowIndex = 17;
int mouseGridIndex = -1; //actual grid index selected by mouse


void updateAvalanche(){
	
	blockDropTimer++;
	
	if(blockDropCache > 0){		
		if(blockDropTimer > 20){
			blockDropTimer = 0;
			spawnBlock();
			//avGrid[5] = 2;
			//blockDropCache--;
		}
	}
	
	
	if(IsKeyPressed(KEY_F)){
		blockDropCache++;
	}
	if(IsKeyDown(KEY_G)){
		avGrid[AV_GRID_SIZE-GetRandomValue(0,9)-1] = 0;
	}
	
	framec++;
	
	if(framec > 10){	//replace with delta and step
		updateScreen();
		framec = 0;
	}
	
	Vector2 mousePos = GetMousePosition();
	mousePos.x = ((int)mousePos.x / CELL_SIZE);
	mousePos.y = ((int)mousePos.y / CELL_SIZE);
	if(IsMouseButtonPressed(0)){
		int mouseGrab = (int)(mousePos.y*AV_GRID_WIDTH+mousePos.x);
		if(avGrid[mouseGrab] != 0){
			//mouseColorGrab = avGrid[i]; //color val
			mouseGridIndex = mouseGrab; //index THIS IS FOR ARR NOT  NOT COLUMN
			printf("grabbed x[%i] y[%i] index [%i]\n", (mouseGridIndex%AV_GRID_WIDTH),(mouseGridIndex/AV_GRID_WIDTH), mouseGridIndex);
			//columnIndex = mousePos.x;
		}
	}
	if(IsMouseButtonReleased(0) && mouseGridIndex != -1){
		int releaseIndex = (int)(mousePos.y*AV_GRID_WIDTH+(mouseGridIndex % AV_GRID_WIDTH));

		printf("x[%i] y[%i] ind[%i]\n", releaseIndex % AV_GRID_WIDTH, releaseIndex / AV_GRID_WIDTH, releaseIndex);
		if(avGrid[releaseIndex] == 0){
			
			avGrid[releaseIndex] = avGrid[mouseGridIndex];
			avGrid[mouseGridIndex] = 0;
			mouseGridIndex = -1;
		}else{
			printf("FAIL CUZ ind[%i] is val[%i]\n",releaseIndex,avGrid[releaseIndex]);
			mouseGridIndex = -1;
			//columnIndex = -1;
		}
	}
	
	DrawTexture(avalancheBG, 0, 0, WHITE);
	//DrawRectangleLines(mousePos.x*CELL_SIZE, mousePos.y*CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
	//for(int i = 0; i < 10; i++){
		//if(i % 2!= 0){
			//continue;
		//}
		//DrawRectangle(i*50,0,50,900,(Color){0,255,0,50});
	//}
	for (int i = 0; i < AV_GRID_SIZE; i++) {
		int gridX = i % AV_GRID_WIDTH;
		int gridY = i / AV_GRID_WIDTH;
		DrawRectangle(gridX * CELL_SIZE, gridY * CELL_SIZE, CELL_SIZE, CELL_SIZE, blockColors[avGrid[i]]);
		//DrawRectangleLines(gridX * CELL_SIZE, gridY * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
		//DrawText(TextFormat("%i", avGrid[i]), gridX * CELL_SIZE+25, gridY * CELL_SIZE+25, 20, ORANGE);
		DrawText(TextFormat("%i", i), gridX * CELL_SIZE+25, gridY * CELL_SIZE+25, 20, ORANGE);
		
	}	

	if(mouseGridIndex != -1){
		int x = mouseGridIndex % AV_GRID_WIDTH;
		int y = mouseGridIndex / AV_GRID_WIDTH;
		DrawRectangle(x*CELL_SIZE,y*CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
		DrawRectangle(x*CELL_SIZE, mousePos.y*CELL_SIZE, CELL_SIZE, CELL_SIZE,blockColors[avGrid[mouseGridIndex]]);
		//DrawRectangle(columnIndex*CELL_SIZE, mousePos.y*CELL_SIZE, CELL_SIZE, CELL_SIZE, Fade(blockColors[avGrid[mouseGridIndex]], 0.4f));
	}	
	
		//DrawText(TextFormat("mousegrid %i", mouseGridIndex),20,20,20,RED);
	DrawLine(0,850,AV_GRID_WIDTH*CELL_SIZE,850,BLACK);
	DrawText(TextFormat("BLOCKS: %i", blockDropCache), 30,30,20,BLACK);
	//DrawRectangleLines(0,rowIndex*CELL_SIZE,AV_GRID_WIDTH*CELL_SIZE,CELL_SIZE,BLUE);
}


#endif




