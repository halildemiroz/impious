#include "nds/arm9/video.h"
#include <nds.h>
#include <nf_lib.h>
#include <filesystem.h>
#include <stdio.h>
#include "levels.h"
#include "structs.h"

#define TILE_SIZE 16
#define SCREEN_SPRITE_LIMIT 128
#define PLAYER_SPRITE_ID 0
#define ENEMY_FIRST_SPRITE_ID 10
#define PLATFORM_FIRST_SPRITE_ID 20
#define JUMP_IMPULSE 10
#define COIN_FIRST_SPRITE_ID 4
#define MAP_WIDTH 32
#define MAP_HEIGHT 16

Character c1 = {0, 0, 2, 0, 0, 4, 0, 0, 5, false, false, false, false};

Level *currentLevelData = level;
int currentLevelNum = 1;
int lastPlatformSpriteID = PLATFORM_FIRST_SPRITE_ID;
int cameraX = 0;
int collectedCoins = 0;

int totalCoinsInLevel = 0;
int totalEnemiesInLevel = 0;

Coin levelCoins[30];
Door levelDoor;
Enemy levelEnemies[30];

void renderWorld(Character *c1){
    cameraX = c1->charX - 128;
    if(cameraX < 0)
        cameraX = 0;

    NF_ScrollBg(0,3,cameraX,0);

    NF_MoveSprite(0,PLAYER_SPRITE_ID, c1->charX - cameraX, c1->charY);
    NF_MoveSprite(0,2,levelDoor.x - cameraX, levelDoor.y);

    for(int i = 0; i < totalCoinsInLevel; i++){
        if(!levelCoins[i].collected && levelCoins[i].created)
            NF_MoveSprite(0, COIN_FIRST_SPRITE_ID + i, levelCoins[i].x - cameraX, levelCoins[i].y);
    }
}

void drawHud(void){
    printf("\x1b[1;1HCoins %d/%d  \n ", collectedCoins, totalCoinsInLevel);
    printf("\x1b[3;1HHealth %d/%d   ", c1.health, 5);
}

void createPlatforms(Level *levelData, int levelCount){
    for(int mapY = 0; mapY < 32; mapY++){
        for(int mapX = 0; mapX < 64; mapX++){
            NF_SetTileOfMap(0, 3, mapX, mapY, 0);
        }
    }

    for(int i = 0; i < levelCount; i++){
        int startTileX = levelData[i].x / 8;
        int startTileY = levelData[i].y / 8;
        int tileWidth = levelData[i].width / 8;
        int tileHeight = levelData[i].height / 8;

        for(int y = 0; y < tileHeight; y++){
            for(int x = 0; x < tileWidth; x++){
                int tileID = 1 + (x % 2) + ((y % 2) * 2);
                NF_SetTileOfMap(0,3,startTileX + x, startTileY + y, tileID);
            }
        }
    }
    NF_UpdateVramMap(0,3);
}

void spawnCoins(int count){
    totalCoinsInLevel = count;
    collectedCoins = 0;

    for(int i = 0; i < totalCoinsInLevel; i++){
        levelCoins[i].collected = false;
        if (!levelCoins[i].created) {
            NF_CreateSprite(0, COIN_FIRST_SPRITE_ID + i, 3, 3, levelCoins[i].x - cameraX, levelCoins[i].y);
            NF_SpriteFrame(0, COIN_FIRST_SPRITE_ID + i, 0);
            levelCoins[i].created = true;
        } else {
            NF_MoveSprite(0, COIN_FIRST_SPRITE_ID + i, levelCoins[i].x - cameraX, levelCoins[i].y);
        }
    }
}

void spawnEnemies(int count){
    totalEnemiesInLevel = count;
    for(int i = 0; i < totalEnemiesInLevel; i++){
        levelEnemies[i].created = false;
        if(!levelEnemies[i].created){
            NF_CreateSprite(0, ENEMY_FIRST_SPRITE_ID + i, 4, 4, levelEnemies[i].x - cameraX, levelEnemies[i].y);
            levelEnemies[i].created = true;
        }
    }
}

void updateEnemies(Character *c1){
    for(int i = 0; i < totalEnemiesInLevel; i++) {
        if(!levelEnemies[i].created) continue;

        levelEnemies[i].x += levelEnemies[i].vx;
        
        if (levelEnemies[i].x <= levelEnemies[i].minX) {
            levelEnemies[i].x = levelEnemies[i].minX;
            levelEnemies[i].vx = 1; 
            levelEnemies[i].facingLeft = false;
        } else if (levelEnemies[i].x >= levelEnemies[i].maxX) {
            levelEnemies[i].x = levelEnemies[i].maxX;
            levelEnemies[i].vx = -1; 
            levelEnemies[i].facingLeft = true;
        }

        levelEnemies[i].animTimer++;
        if (levelEnemies[i].animTimer > 6) {
            levelEnemies[i].animTimer = 0;
            levelEnemies[i].frame++;
            if (levelEnemies[i].frame > 3) levelEnemies[i].frame = 0;
        }

        NF_MoveSprite(0, ENEMY_FIRST_SPRITE_ID + i, levelEnemies[i].x - cameraX, levelEnemies[i].y);
        NF_HflipSprite(0, ENEMY_FIRST_SPRITE_ID + i, levelEnemies[i].facingLeft);
        NF_SpriteFrame(0, ENEMY_FIRST_SPRITE_ID + i, levelEnemies[i].frame);

        if (c1->charX < levelEnemies[i].x + 12 &&
            c1->charX + 12 > levelEnemies[i].x &&
            c1->charY < levelEnemies[i].y + 12 &&
            c1->charY + 12 > levelEnemies[i].y) {
            
            c1->charX = 0;
            c1->charY = 0;
            c1->velocityX = 0;
            c1->velocityY = 0;
            c1->health--;
        }
    }
}

void loadLevel(int levelNum){
    for(int i = 0; i < 10; i++){
        if (i < totalCoinsInLevel && levelCoins[i].created) {
            NF_DeleteSprite(0, COIN_FIRST_SPRITE_ID + i);
            levelCoins[i].created = false;
        }
    }
    for(int i = 0; i < 10; i++){
        if(i < totalEnemiesInLevel && levelEnemies[i].created){
            NF_DeleteSprite(0, ENEMY_FIRST_SPRITE_ID + i);
            levelEnemies[i].created = false;
        }
    }
        
    currentLevelNum++;

    if(currentLevelNum > 3){
        currentLevelNum = 1;
    }

    levelDoor.width = 16;
    levelDoor.height = 16;

    if(currentLevelNum == 1){
        currentLevelData = level;
        
        totalCoinsInLevel = 2;
        levelDoor.x = 464;
        levelDoor.y = 128;
        levelCoins[0] = (Coin){100, 120, false};
        levelCoins[1] = (Coin){200, 120, false};
        
        totalEnemiesInLevel = 1;
        levelEnemies[0] = (Enemy){176, 128, 1, 176, 304 - 16, 0, 0, false, false};

    }else if(currentLevelNum == 2){
        currentLevelData = level2;
        levelDoor.x = 448;
        levelDoor.y = 80;

        totalCoinsInLevel = 3;
        levelCoins[0] = (Coin){40, 144, false};
        levelCoins[1] = (Coin){176, 112, false};
        levelCoins[2] = (Coin){384, 80, false};

        totalEnemiesInLevel = 1;
        levelEnemies[0] = (Enemy){144, 112, 1, 144, 208 - 16, 0, 0, false, false};

    }else if(currentLevelNum == 3){
        currentLevelData = level3;
        levelDoor.x = 464;
        levelDoor.y = 96;

        totalCoinsInLevel = 3;
        levelCoins[0] = (Coin){32, 112, false};
        levelCoins[1] = (Coin){120, 112, false};
        levelCoins[2] = (Coin){400, 96, false};
        
        totalEnemiesInLevel = 1;
        levelEnemies[0] = (Enemy){192, 144, 1, 192, 320 - 16, 0, 0, false, false};

    }

    spawnCoins(totalCoinsInLevel);
    spawnEnemies(totalEnemiesInLevel);
    createPlatforms(currentLevelData, LEVEL_COUNT);


    c1.charX = 0;
    c1.charY = 0;
    c1.velocityY = 0;
}

void loadNextLevel(Character *c1){
    currentLevelNum++;
    if(currentLevelNum > 3)
        currentLevelNum = 1;
    loadLevel(currentLevelNum);
}

/* ---------------- Game Mechs ---------------*/

void initGame(){
    if (!nitroFSInit(NULL)) {
        while (1) {
            swiWaitForVBlank();
        }
    }

    NF_SetRootFolder("NITROFS");
    NF_Set2D(0,0);
    NF_Set2D(1,0);

    consoleDemoInit();

    BG_PALETTE[0] = RGB15(8,8,8);
    
    NF_InitSpriteBuffers();
    NF_InitSpriteSys(0);
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(0);
    NF_InitTextSys(0);
    
    NF_LoadTiledBg("assets/block", "level_bg", 512, 256);
    NF_CreateTiledBg(0,3,"level_bg");

    NF_LoadSpriteGfx("assets/character", 0, 16, 16);
    NF_LoadSpritePal("assets/character", 0);
    NF_VramSpriteGfx(0,0,0,false);
    NF_VramSpritePal(0,0,0);

    NF_LoadSpriteGfx("assets/door", 2, 16, 16);
    NF_LoadSpritePal("assets/door", 2);
    NF_VramSpriteGfx(0,2,2,false);
    NF_VramSpritePal(0,2,2);

    NF_LoadSpriteGfx("assets/coin", 3, 16, 16);
    NF_LoadSpritePal("assets/coin", 3);
    NF_VramSpriteGfx(0,3,3,false);
    NF_VramSpritePal(0,3,3);

    NF_LoadSpriteGfx("assets/enemy", 4, 16, 16);
    NF_LoadSpritePal("assets/enemy", 4);
    NF_VramSpriteGfx(0, 4, 4, false);
    NF_VramSpritePal(0, 4, 4);

    NF_CreateSprite(0, PLAYER_SPRITE_ID, 0, 0, c1.charX, c1.charY);
    NF_CreateSprite(0, 2, 2, 2, 0, 0);
    NF_SpriteFrame(0, 2, 0);

    loadLevel(1);
    
}

/* ---------------- Game Mechs ---------------*/

/* ---------------- Character --------------- */

bool doorCollision(Character *c1, Door *door){
    bool xOverlap = (c1->charX + 16 > door->x) && (c1->charX < door->x + door->width);
    bool yOverlap = (c1->charY + 16 > door->y) && (c1->charY < door->y + door->height);

    return (xOverlap && yOverlap);
}

void coinCollision(Character *c1){
    for(int i = 0; i < totalCoinsInLevel; i++){
        if(levelCoins[i].collected) continue;

        if(c1->charX < levelCoins[i].x + 16 &&
           c1->charX + 16 > levelCoins[i].x &&
           c1->charY < levelCoins[i].y + 16 &&
           c1->charY + 16 > levelCoins[i].y) {

           levelCoins[i].collected = true;
           collectedCoins++;

           if (levelCoins[i].created) {
               NF_DeleteSprite(0, COIN_FIRST_SPRITE_ID + i);
               levelCoins[i].created = false;
           }
        }
    }
}

void collisionCharacter(Character *c1) {
    
    c1->charX += c1->velocityX;
    
    int charLeft = c1->charX;
    int charRight = c1->charX + TILE_SIZE;
    int charTop = c1->charY;
    int charBottom = c1->charY + TILE_SIZE;

    for (int i = 0; i < LEVEL_COUNT; i++) {
        int pLeft = currentLevelData[i].x;
        int pTop = currentLevelData[i].y;
        int pRight = currentLevelData[i].x + currentLevelData[i].width;
        int pBottom = currentLevelData[i].y + currentLevelData[i].height;

        if (charRight > pLeft && charLeft < pRight && charBottom > pTop && charTop < pBottom) {
            if (c1->velocityX > 0) { 
                c1->charX = pLeft - TILE_SIZE;
            } else if (c1->velocityX < 0) { 
                c1->charX = pRight;
            }
            c1->velocityX = 0;
            
            charLeft = c1->charX;
            charRight = c1->charX + TILE_SIZE;
        }
    }

    c1->velocityY += 1; 
    c1->charY += c1->velocityY;
    
    charTop = c1->charY;
    charBottom = c1->charY + TILE_SIZE;
    c1->isGrounded = false; 

    for (int i = 0; i < LEVEL_COUNT; i++) {
        int pLeft = currentLevelData[i].x;
        int pTop = currentLevelData[i].y;
        int pRight = currentLevelData[i].x + currentLevelData[i].width;
        int pBottom = currentLevelData[i].y + currentLevelData[i].height;

        if (charRight > pLeft && charLeft < pRight && charBottom > pTop && charTop < pBottom) {
            if (c1->velocityY > 0) { 
                c1->charY = pTop - TILE_SIZE;
                c1->isGrounded = true; 
            } else if (c1->velocityY < 0) {
                c1->charY = pBottom;
            }
            c1->velocityY = 0;
            
            charTop = c1->charY;
            charBottom = c1->charY + TILE_SIZE;
        }
    }
    
    int groundCheckBottom = c1->charY + TILE_SIZE + 1;
    for (int i = 0; i < LEVEL_COUNT; i++) {
        int pLeft = currentLevelData[i].x;
        int pTop = currentLevelData[i].y;
        int pRight = currentLevelData[i].x + currentLevelData[i].width;

        if (charRight > pLeft && charLeft < pRight && groundCheckBottom > pTop && c1->charY < pTop) {
            c1->isGrounded = true;
            break;
        }
    }
}

void resetCharacter(Character *c1){
    if(c1->health <= 0){
        c1->health = 5;
        currentLevelNum = 0;
        loadLevel(1);;
    }
}

void moveCharacter(Character *c1){
    scanKeys();
    
    c1->isMoving = false;
    c1->velocityX = 0;
    
    int keys = keysHeld();
    int keys_pressed = keysDown();
    int speed = c1->charSpeed;
    
    if(!c1->safetyBounce){
    if (keys & KEY_L) {
        c1->charX = 0;
        c1->charY = 0;
    }

    if (keys & KEY_B) speed *= 2;

    if (keys & KEY_RIGHT) {
        c1->velocityX = speed;
        c1->isMoving = true;
        c1->facingLeft = false;
    } else if (keys & KEY_LEFT) {
        c1->velocityX = -speed;
        c1->isMoving = true;
        c1->facingLeft = true;
    }

    if (((keys_pressed & KEY_A) || (keys_pressed & KEY_R) )&& c1->isGrounded ) {
        c1->velocityY = -JUMP_IMPULSE;
        c1->isGrounded = false;
    }
    }

    collisionCharacter(c1);

    if(c1->isGrounded)
        c1->safetyBounce = false;

    if(c1->charY > 200){
        if(!c1->safetyBounce){
            c1->charY = 180;
            c1->velocityY = -15;
            c1->safetyBounce = true;
        } else{
            c1->health--;
            c1->charX = 0;
            c1->charY = 0;
            c1->velocityX = 0;
            c1->velocityY = 0;
            c1->safetyBounce = false;
        }
    }

    coinCollision(c1);
    if(doorCollision(c1, &levelDoor)){
        if(collectedCoins == totalCoinsInLevel)
            loadNextLevel(c1);
    }
 }

void animCharacter(Character *c1){
    // Coin Animation
    static int coinAnimTimer = 0;
    static int coinFrame = 0;

    coinAnimTimer++;
    if(coinAnimTimer > 10){
        coinAnimTimer = 0;
        coinFrame = (coinFrame + 1) % 3;

        for(int i = 0; i < totalCoinsInLevel; i++){
            if(!levelCoins[i].collected && levelCoins[i].created)
            NF_SpriteFrame(0, COIN_FIRST_SPRITE_ID + i, coinFrame);
        }
    }

    // Character Animation
    NF_HflipSprite(0, PLAYER_SPRITE_ID, c1->facingLeft);

    c1->animTimer++;

    if(!c1->isGrounded){
        if(c1->frame < 8 || c1->frame > 11){
            c1->frame = 8;
            c1->animTimer = 0;
        }

        if(c1->animTimer > 4){
            c1->animTimer = 0;
            c1->frame++;

            if(c1->frame > 11)
                c1->frame = 11;
        }
    }else{

    if (!c1->isMoving) {

        if (c1->frame < 0 || c1->frame > 3) {
            c1->frame = 0;
            c1->animTimer = 0;
        }

        if (c1->animTimer > 12) {
            c1->animTimer = 0;
            c1->frame++;

            if (c1->frame > 3) {
                c1->frame = 0;
            }
        }

    } else {


        if (c1->frame < 4 || c1->frame > 7) {
            c1->frame = 4;
            c1->animTimer = 0;
        }

        if (c1->animTimer > 5) {
            c1->animTimer = 0;
            c1->frame++;

            if (c1->frame > 7) {
                c1->frame = 4;
            }
        }
    }
    }
    NF_SpriteFrame(0, PLAYER_SPRITE_ID, c1->frame);
}

/* ---------------- Character --------------- */

int main(int argc, char **argv) {
    initGame();

    while(1) {
        resetCharacter(&c1);
        moveCharacter(&c1);
        animCharacter(&c1);
        updateEnemies(&c1);
        renderWorld(&c1);
        drawHud();

        NF_SpriteOamSet(0);
        swiWaitForVBlank();
        oamUpdate(&oamMain);
    }

    return 0;
}
