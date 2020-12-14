#include <stdio.h>
#include "max.h"

int g_X = 10;
int g_Y = 20;

int main(int argc, char const *argv[])
{
    int result;

    result = max();
    printf("the max unmber is %d.", result);

    return 0;
}
