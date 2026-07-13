#pragma once
#include <nds.h>

typedef struct Character{
    s16 charX, charY;
    u8 charSpeed;
    int velocityX, velocityY;
    int frameCount;
    int frame;
    int animTimer;
    int health;
    bool isGrounded;
    bool isMoving;
    bool facingLeft;
    bool safetyBounce;
}Character;

typedef struct Coin{
    s16 x, y;
    bool collected;
    bool created;
}Coin;

typedef struct Door{
    s16 x, y, width, height;
}Door;

typedef struct Enemy{
    s16 x, y;
    int vx;
    int minX, maxX;
    int frame;
    int animTimer;
    bool created;
    bool facingLeft;
}Enemy;
