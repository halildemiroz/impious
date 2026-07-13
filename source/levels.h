#pragma once
#include <nds.h>

#define LEVEL_COUNT 4

typedef struct Level{
    s16 x, y, width, height;
}Level;

Level level[LEVEL_COUNT] = {
    {0, 144, 128, 16},
    {176, 144, 128, 16},
    {336, 112, 64, 16},
    {416, 144, 96, 16}
};

Level level2[LEVEL_COUNT] = {
    {0, 160, 96, 16},
    {144, 128, 64, 16},
    {256, 96, 64, 16},
    {368, 96, 144, 16}
};

Level level3[LEVEL_COUNT] = {
    {0, 128, 64, 16},
    {112, 128, 32, 16},
    {192, 160, 128, 16},
    {384, 112, 128, 16}
};
