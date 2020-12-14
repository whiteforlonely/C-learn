#include "max.h"

extern int g_X;
extern int g_Y;

int max()
{
    return g_X > g_Y ? g_X : g_Y;
}