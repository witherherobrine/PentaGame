


#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "consts.h"
#include "rocketlaunch.h"
#include "avalanche.h"


void initMainMenu(){
	SetWindowSize(300,300);
	SetWindowTitle("Main Menu");
	CURRENT_GAME = NONE;
}
void startRocketLaunch(){
	SetWindowSize(300,900);
	SetWindowTitle("Rocket Launch");
	CURRENT_GAME = ROCKET_LAUNCH;
}
void startAvalanche(){
	SetWindowSize(500,900);
	SetWindowTitle("Avalanche");
	//initAvalanche();
	CURRENT_GAME = AVALANCHE;
}

int main(void){   
    
    //core init
    InitWindow(300,900, "Avalanche");
    InitAudioDevice();
    SetTargetFPS(120);
    
    initMainMenu();
	initRocketLaunch();
	initAvalanche();
    double delta = 0;
    double now = 0;
    bool needInit = false;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
	   if(IsKeyPressed(KEY_ONE)){
			initMainMenu();
		}
        if(IsKeyPressed(KEY_TWO)){
			startRocketLaunch();
		}
        if(IsKeyPressed(KEY_THREE)){
			startAvalanche();
		}
        now = GetTime();
        delta = now - lastTime;
        float frameDelta = GetFrameTime();
        
        BeginDrawing();
        switch(CURRENT_GAME){
			case NONE:
				ClearBackground(GRAY);
				DrawText("MAINMANU",50,50,20,WHITE);
				break;
			case ROCKET_LAUNCH:
				updateRocketLaunch(now, delta, frameDelta);
				break;
			case AVALANCHE:
				updateAvalanche();
				break;
		}
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
