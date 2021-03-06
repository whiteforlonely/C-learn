﻿
/**
* 俄罗斯方块小游戏
* 
* @Author Saturday（柯清源）
* @Date 2021-03-01
* @description 俄罗斯方块,简单的模仿俄罗斯方块游戏
* 
*/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <graphics.h>
#include <conio.h>
#include <malloc.h>
#include <windows.h>
#include <process.h>
#include <atlstr.h>
#include <mutex>

#define UP				72
#define DOWN			80
#define LEFT			75
#define RIGHT			77

#define ROTATE			75
#define RESTART			83
#define ESC				27

#define CUBE_WIDTH		20	// 定义单个方块大小

#define DIRECTION_0		0
#define DIRECTION_90	1
#define DIRECTION_180	2
#define DIRECTION_270	3

#define MODULE_TYPE_SQUARE	1
#define MODULE_TYPE_ONE		2
#define MODULE_TYPE_STEP	3
#define MODULE_TYPE_RESTEP	4
#define MODULE_TYPE_L		5
#define MODULE_TYPE_REL		6
#define MODULE_TYPE_T		7

#define WINDOW_WIDTH		200	// 游戏界面的宽
#define WINDOW_HIGH			480 // 游戏界面的高

#define arraySizeRaw		23
#define arraySizeColumn		10

typedef struct
{
	int type;		// 模块类型	1.正方形；2.一型；3.台阶型；4.L型；5.反L型；6.T型
	int direction;	// 模块角度方向 0=0度；1=90度；2=180度；3=270度

	int gravity_x;	// 重心坐标X, 重心坐标在模块旋转的时候，是不会改变的
	int gravity_y;	// 重心坐标Y，中心左边在模块旋转的时候，是不会改变的

	int left;	// 所占据矩形空间左边位置
	int top;	// 所占据矩形空间顶部位置
	int right;	// 所占据矩形空间右边位置
	int bottom;	// 所占据矩形空间底部位置

	// 颜色值
	int r;
	int g;
	int b;		
} ModuleCube;

std::mutex m;
ModuleCube* currModuleCube, *preModuleCube;
// 当前的分
int score = 0;
// 最高得分
int maxScore = 0;
bool changed = 0;
/*
* map地图是为了辅助计算方块的各种动作和判断限制方块的各种动作，
* map地图是一个二维数组，每一个坐标位置都一一对应着游戏中方块对应的界面的一个小方块。
* map中只有0和1两个值，1代表著对应的游戏界面的位置已经画上了小方块，0代表还是空的，也就是可以移动。
*
* 1. 方块下落的时候，如何知道不能下落。
* 2. 方块左右移动的时候如何限制不能超出界限。
* 3. 判断游戏什么时候应该结束。
* 4. 判断游戏什么时候有得分。
* 5. 判断模块是否可以旋转。
*/
int map[arraySizeColumn][arraySizeRaw] = { 0 };

void drawCube(int x, int y, COLORREF color);
void clearCube(int x, int y);
int updateMap(ModuleCube* moduleCube, int map[][arraySizeRaw]);
bool checkCanMove(ModuleCube* moduleCube, int map[][arraySizeRaw], int directionKey);
void resetMap(ModuleCube* moduleCube, int map[][arraySizeRaw]);
void cleanModuleCube(ModuleCube* moduleCube);
bool checkCanRotate(ModuleCube* moduleCube, int map[][arraySizeRaw]);
void drawModule(ModuleCube* moduleCube);
void moveModule(ModuleCube* moduleCube, int directionKey, int map[][arraySizeRaw]);
void rotateModule(ModuleCube* moduleCube, int map[][arraySizeRaw]);
void initModule(ModuleCube* moduleCube, int map[][arraySizeRaw]);
void renderMap(int map[][arraySizeRaw]);
int fullLineRemove(int map[][arraySizeRaw]);
void threadAutoFall(void*);
void text(TCHAR text, int left, int top, int right, int bottom);
void restart();

/*
画小方块
x,y 为小方块的中心坐标
*/
void drawCube(int x, int y, COLORREF color)
{
	setcolor(0x000000);
	
	setfillcolor(color);
	setfillstyle(BS_SOLID);
	fillrectangle(x - CUBE_WIDTH / 2, y - CUBE_WIDTH / 2, x + CUBE_WIDTH / 2, y + CUBE_WIDTH / 2);
}

/*
清除一小个方块
*/
void clearCube(int x, int y)
{
	clearrectangle(x - CUBE_WIDTH / 2, y - CUBE_WIDTH / 2, x + CUBE_WIDTH / 2, y + CUBE_WIDTH / 2);
}

/*
更新模块地图
1：可以正常继续跑下去
0：需要生成下一个模块
-1： 游戏结束
*/
int updateMap(ModuleCube* moduleCube, int map[][arraySizeRaw])
{
	int x = moduleCube->left / CUBE_WIDTH;
	int y = moduleCube->top / CUBE_WIDTH;

	bool canGoOn = 0;

	if (moduleCube->type == MODULE_TYPE_SQUARE)
	{
		map[x][y] = 1;
		map[x + 1][y] = 1;
		map[x][y + 1] = 1;
		map[x + 1][y + 1] = 1;
		canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0;
		
	}
	else if (moduleCube->type == MODULE_TYPE_ONE)
	{
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			map[x][y] = 1;
			map[x + 1][y] = 1;
			map[x + 2][y] = 1;
			map[x + 3][y] = 1;
			canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 1] == 0 && map[x + 2][y + 1] == 0 && map[x + 3][y + 1] == 0;
		}
		else if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 1;
			map[x][y+1] = 1;
			map[x][y+2] = 1;
			map[x][y+3] = 1;
			canGoOn = map[x][y + 4] == 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_L)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			map[x][y] = 1;
			map[x][y + 1] = 1;
			map[x][y + 2] = 1;
			map[x + 1][y + 2] = 1;
			canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 3] == 0;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			map[x][y] = 1;
			map[x][y + 1] = 1;
			map[x + 1][y] = 1;
			map[x + 2][y] = 1;
			canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 1] == 0 && map[x + 2][y + 1] == 0;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			map[x][y] = 1;
			map[x + 1][y] = 1;
			map[x + 1][y + 1] = 1;
			map[x + 1][y + 2] = 1;
			canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 3] == 0;
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			map[x][y + 1] = 1;
			map[x + 1][y + 1] = 1;
			map[x + 2][y + 1] = 1;
			map[x + 2][y] = 1;
			canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_REL)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			map[x+1][y] = 1;
			map[x+1][y + 1] = 1;
			map[x+1][y + 2] = 1;
			map[x][y + 2] = 1;
			canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 3] == 0;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			map[x][y] = 1;
			map[x][y + 1] = 1;
			map[x + 1][y+1] = 1;
			map[x + 2][y+1] = 1;
			canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			map[x][y] = 1;
			map[x][y+1] = 1;
			map[x][y + 2] = 1;
			map[x + 1][y] = 1;
			canGoOn = map[x][y + 3] == 0 && map[x + 1][y +1] == 0;
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 1;
			map[x + 1][y] = 1;
			map[x + 2][y] = 1;
			map[x + 2][y + 1] = 1;
			canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 1] == 0 && map[x + 2][y + 2] == 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_STEP)
	{
		
		if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 1;
			map[x][y + 1] = 1;
			map[x + 1][y + 1] = 1;
			map[x + 1][y + 2] = 1;
			canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 3] == 0;
		}
		else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			map[x+1][y] = 1;
			map[x + 2][y] = 1;
			map[x][y + 1] = 1;
			map[x + 1][y + 1] = 1;
			canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 1] == 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_RESTEP)
	{

		if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			map[x][y+2] = 1;
			map[x][y + 1] = 1;
			map[x + 1][y + 1] = 1;
			map[x + 1][y] = 1;
			canGoOn = map[x][y + 3] == 0 && map[x + 1][y +2] == 0;
		}
		else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			map[x + 1][y] = 1;
			map[x][y] = 1;
			map[x+2][y + 1] = 1;
			map[x + 1][y + 1] = 1;
			canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_T)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			map[x][y] = 1;
			map[x + 1][y] = 1;
			map[x + 2][y] = 1;
			map[x + 1][y + 1] = 1;
			canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 1] == 0;
		}
		else if (moduleCube->direction == DIRECTION_90) {
			map[x][y + 1] = 1;
			map[x + 1][y] = 1;
			map[x + 1][y + 1] = 1;
			map[x + 1][y + 2] = 1;
			canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 3] == 0;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			map[x + 1][y] = 1;
			map[x][y + 1] = 1;
			map[x + 1][y + 1] = 1;
			map[x + 2][y + 1] = 1;
			canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 1;
			map[x][y + 1] = 1;
			map[x][y + 2] = 1;
			map[x + 1][y + 1] = 1;
			canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 2] == 0;
		}
	}

	// 最后判断下游戏是否继续
	if (canGoOn)
	{
		if (moduleCube->bottom >= arraySizeRaw * CUBE_WIDTH)
		{
			return 0;
		}
		// 可以继续往下走
		return 1;
	}
	else if (y <= 2) {
		// 不可以继续往下走，并且已经到达顶部了，游戏结束
		return -1;
	}
	else {
		// 需要重新生成另外一个模块
		return 0;
	}
}

/*
* 检查模块是否能够移动，以下情况不能再移动
* 1. 当移动到最左边
* 2. 移动到最最右边
* 3. 旁边有模块挡者
*/
bool checkCanMove(ModuleCube* moduleCube, int map[][arraySizeRaw], int directionKey) {
	int x = moduleCube->left / CUBE_WIDTH;
	int y = moduleCube->top / CUBE_WIDTH;
	bool canGoOn = 1;
	if (directionKey == LEFT)
	{
		if (x <= 0)
		{
			return 0;
		}

		if (moduleCube->type == MODULE_TYPE_SQUARE)
		{
			canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0;

		}
		else if (moduleCube->type == MODULE_TYPE_ONE)
		{
			if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x - 1][y] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0 && map[x - 1][y + 2] == 0 && map[x - 1][y + 3] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_L)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0 && map[x - 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x - 1][y] == 0 && map[x][y + 1] == 0 && map[x][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x + 1][y] == 0 && map[x - 1][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_REL)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x][y] == 0 && map[x][y + 1] == 0 && map[x - 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0 && map[x - 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x - 1][y] == 0 && map[x + 1][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_STEP)
		{

			if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0 && map[x][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y] == 0 && map[x - 1][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_RESTEP)
		{
			if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y] == 0 && map[x - 1][y + 1] == 0 && map[x - 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x - 1][y] == 0 && map[x][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_T)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x - 1][y] == 0 && map[x][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x][y] == 0 && map[x - 1][y + 1] == 0 && map[x][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y] == 0 && map[x - 1][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x - 1][y] == 0 && map[x - 1][y + 1] == 0 && map[x - 1][y + 2] == 0;
			}
		}
	}
	// 向右移动的时候，是否可行
	else if (directionKey == RIGHT)
	{
		x = moduleCube->right/CUBE_WIDTH - 1;
		if (x >= arraySizeColumn-1)
		{
			return 0;
		}

		if (moduleCube->type == MODULE_TYPE_SQUARE)
		{
			canGoOn = map[x + 1][y] == 0 && map[x + 1][y + 1] == 0;
		}
		else if (moduleCube->type == MODULE_TYPE_ONE)
		{
			if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x + 1][y] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x + 1][y] == 0 && map[x + 1][y + 1] == 0 && map[x + 1][y + 2] == 0 && map[x + 1][y + 3] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_L)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x][y] == 0 && map[x][y + 1] == 0 && map[x + 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x + 1][y] == 0 && map[x - 1][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x + 1][y] == 0 && map[x + 1][y + 1] == 0 && map[x + 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x + 1][y] == 0 && map[x + 1][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_REL)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x+1][y] == 0 && map[x+1][y + 1] == 0 && map[x + 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x - 1][y] == 0 && map[x + 1][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x + 1][y] == 0 && map[x][y + 1] == 0 && map[x][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x + 1][y] == 0 && map[x + 1][y + 1] == 0;
			}

		}
		else if (moduleCube->type == MODULE_TYPE_STEP)
		{

			if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y] == 0 && map[x + 1][y + 1] == 0 && map[x + 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x + 1][y] == 0 && map[x][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_RESTEP)
		{
			if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x+1][y] == 0 && map[x + 1][y + 1] == 0 && map[x][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y] == 0 && map[x+1][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_T)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x + 1][y] == 0 && map[x][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x+1][y] == 0 && map[x + 1][y + 1] == 0 && map[x+1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y] == 0 && map[x + 1][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y] == 0 && map[x + 1][y + 1]==0 && map[x][y + 2] == 0;
			}
		}
	}
	// 向下移动的时候，是否可行
	else if (directionKey == DOWN)
	{
		if (moduleCube->bottom/CUBE_WIDTH >= arraySizeRaw)
		{
			return 0;
		}

		if (moduleCube->type == MODULE_TYPE_SQUARE)
		{
			canGoOn = map[x][y+2] == 0 && map[x + 1][y + 2] == 0;
		}
		else if (moduleCube->type == MODULE_TYPE_ONE)
		{
			if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 1] == 0 && map[x + 2][y + 1] == 0 && map[x + 3][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y + 4] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_L)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 3] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 1] == 0 && map[x + 2][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 3] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_REL)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 3] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90)
			{
				canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 1] == 0 && map[x + 2][y + 2] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_STEP)
		{

			if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 3] == 0;
			}
			else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 1] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_RESTEP)
		{

			if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
			}
		}
		else if (moduleCube->type == MODULE_TYPE_T)
		{
			if (moduleCube->direction == DIRECTION_0)
			{
				canGoOn = map[x][y + 1] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 1] == 0;
			}
			else if (moduleCube->direction == DIRECTION_90) {
				canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 3] == 0;
			}
			else if (moduleCube->direction == DIRECTION_180)
			{
				canGoOn = map[x][y + 2] == 0 && map[x + 1][y + 2] == 0 && map[x + 2][y + 2] == 0;
			}
			else if (moduleCube->direction == DIRECTION_270)
			{
				canGoOn = map[x][y + 3] == 0 && map[x + 1][y + 2] == 0;
			}
		}
	}
	
	return canGoOn;
}

// 将对应的模块在地图中对应的位置值设置为0
void resetMap(ModuleCube* moduleCube, int map[][arraySizeRaw]) {
	int x = moduleCube->left / CUBE_WIDTH;
	int y = moduleCube->top / CUBE_WIDTH;

	if (moduleCube->type == MODULE_TYPE_SQUARE)
	{
		map[x][y] = 0;
		map[x + 1][y] = 0;
		map[x][y + 1] = 0;
		map[x + 1][y + 1] = 0;

	}
	else if (moduleCube->type == MODULE_TYPE_ONE)
	{
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			map[x][y] = 0;
			map[x + 1][y] = 0;
			map[x + 2][y] = 0;
			map[x + 3][y] = 0;
		}
		else if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 0;
			map[x][y + 1] = 0;
			map[x][y + 2] = 0;
			map[x][y + 3] = 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_L)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			map[x][y] = 0;
			map[x][y + 1] = 0;
			map[x][y + 2] = 0;
			map[x + 1][y + 2] = 0;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			map[x][y] = 0;
			map[x][y + 1] = 0;
			map[x + 1][y] = 0;
			map[x + 2][y] = 0;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			map[x][y] = 0;
			map[x + 1][y] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 1][y + 2] = 0;
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			map[x][y + 1] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 2][y + 1] = 0;
			map[x + 2][y] = 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_REL)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			map[x + 1][y] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 1][y + 2] = 0;
			map[x][y + 2] = 0;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			map[x][y] = 0;
			map[x][y + 1] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 2][y + 1] = 0;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			map[x][y] = 0;
			map[x][y + 1] = 0;
			map[x][y + 2] = 0;
			map[x + 1][y] = 0;
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 0;
			map[x + 1][y] = 0;
			map[x + 2][y] = 0;
			map[x + 2][y + 1] = 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_STEP)
	{

		if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 0;
			map[x][y + 1] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 1][y + 2] = 0;
		}
		else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			map[x + 1][y] = 0;
			map[x + 2][y] = 0;
			map[x][y + 1] = 0;
			map[x + 1][y + 1] = 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_RESTEP)
	{

		if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			map[x][y + 2] = 0;
			map[x][y + 1] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 1][y] = 0;
		}
		else if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			map[x + 1][y] = 0;
			map[x][y] = 0;
			map[x + 2][y + 1] = 0;
			map[x + 1][y + 1] = 0;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_T)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			map[x][y] = 0;
			map[x + 1][y] = 0;
			map[x + 2][y] = 0;
			map[x + 1][y + 1] = 0;
		}
		else if (moduleCube->direction == DIRECTION_90) {
			map[x][y + 1] = 0;
			map[x + 1][y] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 1][y + 2] = 0;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			map[x + 1][y] = 0;
			map[x][y + 1] = 0;
			map[x + 1][y + 1] = 0;
			map[x + 2][y + 1] = 0;
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			map[x][y] = 0;
			map[x][y + 1] = 0;
			map[x][y + 2] = 0;
			map[x + 1][y + 1] = 0;
		}
	}
}

// 清除原来的模块,一个moduleCube是由四个小方块组成的，所以清除的时候，就逐个的清理掉四个小方块。
void cleanModuleCube(ModuleCube* moduleCube)
{
	if (moduleCube->type == MODULE_TYPE_SQUARE || moduleCube->type == MODULE_TYPE_ONE)
	{
		// 因为MODULE_TYPE_SQUARE和MODULE_TYPE_ONE是完整的矩形图案，所以可以直接的一次性清理掉
		clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right, moduleCube->bottom);
	}
	// 对于L型及其他的模块，还要判断对应的角度，通过角度来确定小方块的位置，然后相应的清理掉
	else if (moduleCube->type == MODULE_TYPE_L)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right - CUBE_WIDTH, moduleCube->bottom);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->bottom- CUBE_WIDTH, moduleCube->right, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right, moduleCube->top+CUBE_WIDTH);
			clearrectangle(moduleCube->left, moduleCube->top + CUBE_WIDTH, moduleCube->left + CUBE_WIDTH, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->left + CUBE_WIDTH, moduleCube->top+CUBE_WIDTH);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top,moduleCube->right, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			clearrectangle(moduleCube->right - CUBE_WIDTH, moduleCube->top, moduleCube->right, moduleCube->bottom-CUBE_WIDTH);
			clearrectangle(moduleCube->left, moduleCube->top+CUBE_WIDTH, moduleCube->right,moduleCube->bottom);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_REL)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			clearrectangle(moduleCube->left, moduleCube->bottom-CUBE_WIDTH, moduleCube->right - CUBE_WIDTH, moduleCube->bottom);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top, moduleCube->right, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->left+CUBE_WIDTH, moduleCube->top + CUBE_WIDTH);
			clearrectangle(moduleCube->left, moduleCube->top + CUBE_WIDTH, moduleCube->right, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->left + CUBE_WIDTH, moduleCube->bottom);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top, moduleCube->right, moduleCube->top+CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right, moduleCube->bottom - CUBE_WIDTH);
			clearrectangle(moduleCube->right-CUBE_WIDTH, moduleCube->top + CUBE_WIDTH, moduleCube->right, moduleCube->bottom);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_STEP)
	{
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top, moduleCube->right, moduleCube->bottom - CUBE_WIDTH);
			clearrectangle(moduleCube->left, moduleCube->top + CUBE_WIDTH, moduleCube->right - CUBE_WIDTH, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->left + CUBE_WIDTH, moduleCube->top + 2 * CUBE_WIDTH);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH, moduleCube->right, moduleCube->bottom);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_RESTEP)
	{
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right - CUBE_WIDTH, moduleCube->bottom - CUBE_WIDTH);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH, moduleCube->right, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_90 || moduleCube->direction == DIRECTION_270)
		{
			clearrectangle(moduleCube->left, moduleCube->top + CUBE_WIDTH, moduleCube->left + CUBE_WIDTH, moduleCube->bottom);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top, moduleCube->right, moduleCube->bottom - CUBE_WIDTH);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_T)
	{
		if (moduleCube->direction == DIRECTION_0)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right, moduleCube->bottom - CUBE_WIDTH);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH, moduleCube->right - CUBE_WIDTH, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			clearrectangle(moduleCube->left, moduleCube->top + CUBE_WIDTH, moduleCube->left + CUBE_WIDTH, moduleCube->bottom - CUBE_WIDTH);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top, moduleCube->right, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top, moduleCube->right - CUBE_WIDTH, moduleCube->bottom - CUBE_WIDTH);
			clearrectangle(moduleCube->left, moduleCube->top + CUBE_WIDTH, moduleCube->right, moduleCube->bottom);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right - CUBE_WIDTH, moduleCube->bottom);
			clearrectangle(moduleCube->left + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH, moduleCube->right, moduleCube->bottom - CUBE_WIDTH);
		}
	}
	
}

/*
话各种模块，每个模块都是由四个小方块组成
type 为模块类型
1. 正方形
2. 一型
3. 台阶型（闪电型）
4. 反台阶型
5. L型
6. 反L型
7. T型

direction为各模块的角度
1. 90
2. 180
3. 270
*/
void drawModule(ModuleCube* moduleCube)
{
	COLORREF color = RGB(moduleCube->r, moduleCube->g, moduleCube->b);
	if (moduleCube->type == MODULE_TYPE_SQUARE)
	{
		//画正方形，因为正方形是不管怎么旋转都是一样的，所以较为简单
		// 画左上方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
		// 画右上方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
		// 画左下方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		// 画右下方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
	}
	else if (moduleCube->type == MODULE_TYPE_ONE)
	{
		// TODO 画一字型，有两种形态
		if (moduleCube -> direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			// 横着的'一'
			// 左到右第一个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第二个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第三个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第四个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 3 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
		}
		else
		{
			// 竖着的'1'
			// 从上到下第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第二个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第三个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
			// 第四个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 3 * CUBE_WIDTH, color);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_STEP)
	{
		// 画台阶型，也是有两种形态
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180) {
			// 左到右上台阶，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->bottom - CUBE_WIDTH / 2, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->bottom - CUBE_WIDTH / 2, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
		}
		else
		{
			// 左高右底闪电，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_RESTEP)
	{
		// TODO 画反台阶，也是两种形态
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180) {
			// 左到右下台阶，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->bottom - CUBE_WIDTH / 2, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
		else
		{
			// 左底右高闪电，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_L)
	{
		// TODO 画L型，有四种形态
		if (moduleCube->direction == DIRECTION_0)
		{
			// 正L型
			// 从左到右，上到下，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			// 从左到右，上到下 第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			// 第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			// 从左到右，从上到下，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_REL)
	{
		// 画反L型，有四种形态
		if (moduleCube->direction == DIRECTION_0)
		{
			// 从上到下， 左到右
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_T)
	{
		// 画T型，有四种形态
		if (moduleCube->direction == DIRECTION_0)
		{
			// 从上到下， 左到右
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, color);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH, color);
		}
	}
}

void drawPreModule(ModuleCube* moduleCube)
{
	int deltaX = WINDOW_WIDTH / 2 + 200;
	int deltaY = WINDOW_HIGH / 2 - 20;
	// 初始哈预览模块的位置
	moduleCube->left += deltaX;
	moduleCube->right += deltaX;
	moduleCube->top += deltaY;
	moduleCube->bottom += deltaY;

	drawModule(moduleCube);

	// 回复原来的module的位置
	moduleCube->left -= deltaX;
	moduleCube->right -= deltaX;
	moduleCube->top -= deltaY;
	moduleCube->bottom -= deltaY;
}

void cleanPreModule(ModuleCube* moduleCube)
{
	int deltaX = WINDOW_WIDTH / 2 + 200;
	int deltaY = WINDOW_HIGH / 2 - 20;
	// 初始哈预览模块的位置
	moduleCube->left += deltaX;
	moduleCube->right += deltaX;
	moduleCube->top += deltaY;
	moduleCube->bottom += deltaY;

	cleanModuleCube(moduleCube);

	// 回复原来的module的位置
	moduleCube->left -= deltaX;
	moduleCube->right -= deltaX;
	moduleCube->top -= deltaY;
	moduleCube->bottom -= deltaY;
}

// 移动模块
void moveModule(ModuleCube* moduleCube, int directionKey, int map[][arraySizeRaw]) {
	//cleanModuleCube(moduleCube);
	/*
	else if (directionKey == UP)
	{
		moduleCube->gravity_y -= CUBE_WIDTH;
		moduleCube->top -= CUBE_WIDTH;
		moduleCube->bottom -= CUBE_WIDTH;
	}*/
	if (!checkCanMove(moduleCube, map, directionKey))
	{
		// 不能再移动了
		return;
	}
	if (directionKey == DOWN) {
		moduleCube->gravity_y += CUBE_WIDTH;
		moduleCube->top += CUBE_WIDTH;
		moduleCube->bottom += CUBE_WIDTH;
	}
	else if (directionKey == LEFT)
	{
		moduleCube->gravity_x -= CUBE_WIDTH;
		moduleCube->left -= CUBE_WIDTH;
		moduleCube->right -= CUBE_WIDTH;
	}
	else if (directionKey == RIGHT)
	{
		moduleCube->gravity_x += CUBE_WIDTH;
		moduleCube->left += CUBE_WIDTH;
		moduleCube->right += CUBE_WIDTH;
	}
}

// 旋转模块
void rotateModule(ModuleCube* moduleCube, int map[][arraySizeRaw])
{

	// 先清理掉原来的模块
	/*cleanModuleCube(moduleCube);*/
	// 这边的方案是先判断类型再判断角度，或者反过来判断也是可以的
	// 角度是直接设置的，每次旋转都是按照顺时针旋转90度
	// 判断能否旋转
	//bool canRotate = checkCanRotate(moduleCube, map);
	/*if (!canRotate)
	{
		return;
	}*/

	moduleCube->direction += 1;
	moduleCube->direction %= 4;
	if (moduleCube->type == MODULE_TYPE_SQUARE)
	{
		// 旋转后没有变动
	}
	else if (moduleCube->type == MODULE_TYPE_ONE)
	{
		// 1型的旋转
		// 这边旋转的时候，重心不要改变，只改变边界
		if (moduleCube->direction == DIRECTION_0)
		{
			// 1. 一，重心在底部，重心坐标（2，1）
			moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y;
		}
		else if(moduleCube->direction == DIRECTION_90)
		{
			moduleCube->left = moduleCube->gravity_x;
			moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + 2 * CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y;
			moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else
		{
			// 1. 从1旋转到一
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x;
			moduleCube->bottom = moduleCube->gravity_y + 2 * CUBE_WIDTH;
		}
		
	}
	else if (moduleCube->type == MODULE_TYPE_L)
	{
		// 正L型的旋转,重心在拐角中心，重心坐标（1，2）
		if (moduleCube->direction == DIRECTION_0)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + 2 * CUBE_WIDTH;
		}
		else
		{
			moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_REL)
	{
		// 反L型的旋转，重心在拐角中心，重心坐标（1，2）
		if (moduleCube->direction == DIRECTION_0)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + 2 * CUBE_WIDTH;
		}
		else
		{
			moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_STEP)
	{
		// 正台阶的旋转，中心坐标（1，1）
		if (moduleCube->direction == DIRECTION_0)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + 2 * CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_RESTEP)
	{
		//  反台阶的旋转，重心坐标（1，1）
		if (moduleCube->direction == DIRECTION_0)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + 2 * CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
	}
	else if (moduleCube->type == MODULE_TYPE_T)
	{
		// T型的旋转，重心坐标（1，1）
		if (moduleCube->direction == DIRECTION_0)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + 2 * CUBE_WIDTH;
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
		else
		{
			moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
			moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
			moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
			moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
		}
	}
	/*
	* 这边注意下，其实台阶，T型的旋转都是一样的，如果原来的重心的左边是一样的话。
	*/
	// 判断是否超出了边界
	if (moduleCube->left < -CUBE_WIDTH)
	{
		// 超出了左边界两个像方块长度
		moveModule(moduleCube, RIGHT, map);
		moveModule(moduleCube, RIGHT, map);
	}
	else if (moduleCube->left < 0)
	{
		// 超出了左边界一个小方块长度
		moveModule(moduleCube, RIGHT, map);
	}
	
	if (moduleCube->right > WINDOW_WIDTH + CUBE_WIDTH)
	{
		 // 超出了右边界两个小方块长度
		moveModule(moduleCube, LEFT, map);
		moveModule(moduleCube, LEFT, map);
	}
	else if (moduleCube->right > WINDOW_WIDTH)
	{
		// 超出了右边界一个小方块
		moveModule(moduleCube, LEFT, map);
	}

	// 上边界可以不用处理的
	if (moduleCube->top < -CUBE_WIDTH)
	{
		// 超出了右边界两个小方块长度
		moveModule(moduleCube, DOWN, map);
		moveModule(moduleCube, DOWN, map);
	}
	else if (moduleCube->top < 0)
	{
		// 超出了右边界一个小方块
		moveModule(moduleCube, DOWN, map);
	}

	// 下边界判断
	if (moduleCube->bottom > WINDOW_HIGH + CUBE_WIDTH)
	{
		// 超出了右边界两个小方块长度
		moveModule(moduleCube, UP, map);
		moveModule(moduleCube, UP, map);
	}
	else if (moduleCube->bottom > WINDOW_HIGH)
	{
		moveModule(moduleCube, UP, map);
	}

}

/*
* 检查是否可以旋转
* 
* 主要的思路：
* 
*/
bool checkCanRotate(ModuleCube* moduleCube, int map[][arraySizeRaw]) {
	
	// 先把数组中的数据清0
	resetMap(moduleCube, map);
	bool canGoOn = 0;

	// 复制临时模块
	ModuleCube* tmpModule = (ModuleCube*)malloc(sizeof(ModuleCube));
	memset(tmpModule, 0x00, sizeof(ModuleCube));

	*tmpModule = *moduleCube;

	// 旋转临时模块
	rotateModule(tmpModule, map);

	int x = tmpModule->left / CUBE_WIDTH;
	int y = tmpModule->top / CUBE_WIDTH;

	// 下面是判断旋转后的模块和当前地图是否有冲突
	if (tmpModule->type == MODULE_TYPE_SQUARE)
	{
		canGoOn = 1;
	}
	else if (tmpModule->type == MODULE_TYPE_ONE)
	{
		if (tmpModule->direction == DIRECTION_0 || tmpModule->direction == DIRECTION_180)
		{
			canGoOn = map[x][y] == 0 && map[x+1][y]==0 && map[x+2][y] == 0 && map[x+3][y] == 0;
		}
		else if (tmpModule->direction == DIRECTION_90 || tmpModule->direction == DIRECTION_270)
		{
			canGoOn = map[x][y] == 0 && map[x][y + 1] == 0 && map[x][y + 2] == 0 && map[x][y + 3] == 0;
		}
	}
	else if (tmpModule->type == MODULE_TYPE_L)
	{
		if (tmpModule->direction == DIRECTION_0)
		{
			canGoOn = map[x][y] == 0 && map[x][y + 1] == 0 && map[x][y + 2] == 0 && map[x+1][y+2] == 0;
		}
		else if (tmpModule->direction == DIRECTION_90)
		{
			canGoOn = map[x][y] == 0 && map[x+1][y] == 0 && map[x+2][y] == 0 && map[x][y + 1] == 0;
		}
		else if (tmpModule->direction == DIRECTION_180)
		{
			canGoOn = map[x][y] == 0 && map[x+1][y] == 0 && map[x+1][y+1] == 0&& map[x+1][y + 2] == 0;
		}
		else if (tmpModule->direction == DIRECTION_270)
		{
			canGoOn = map[x][y+1] == 0 && map[x + 1][y + 1] == 0 && map[x+2][y+1] == 0 && map[x+2][y] == 0;
		}
	}
	else if (tmpModule->type == MODULE_TYPE_REL)
	{
		if (tmpModule->direction == DIRECTION_0)
		{
			canGoOn = map[x+1][y] == 0 && map[x+1][y + 1] == 0 && map[x+1][y + 2] == 0 && map[x][y + 2] == 0;
		}
		else if (tmpModule->direction == DIRECTION_90)
		{
			canGoOn = map[x][y] == 0 && map[x][y + 1] == 0 && map[x + 1][y+1] == 0 && map[x+2][y + 1] == 0;
		}
		else if (tmpModule->direction == DIRECTION_180)
		{
			canGoOn = map[x][y] == 0 && map[x + 1][y] == 0 && map[x][y+1] == 0 && map[x][y + 2] == 0;
		}
		else if (tmpModule->direction == DIRECTION_270)
		{
			canGoOn = map[x][y] == 0 && map[x + 1][y] == 0 && map[x + 2][y] == 0 && map[x + 2][y+1] == 0;
		}
	}
	else if (tmpModule->type == MODULE_TYPE_STEP)
	{

		if (tmpModule->direction == DIRECTION_0 || tmpModule->direction == DIRECTION_180)
		{
			canGoOn = map[x][y+1] == 0 && map[x+1][y + 1] == 0 && map[x+1][y] == 0 && map[x][y + 2] == 0;
		}
		else if (tmpModule->direction == DIRECTION_90 || tmpModule->direction == DIRECTION_270)
		{
			canGoOn = map[x][y] == 0 && map[x][y + 1] == 0 && map[x+1][y+1] == 0 && map[x+1][y+2]==0;
		}
	}
	else if (tmpModule->type == MODULE_TYPE_RESTEP)
	{
		if (tmpModule->direction == DIRECTION_0 || tmpModule->direction == DIRECTION_180)
		{
			canGoOn = map[x][y] == 0 && map[x + 1][y] == 0 && map[x+1][y+1] == 0 && map[x + 2][y + 1] == 0;
		}
		else if (tmpModule->direction == DIRECTION_90 || tmpModule->direction == DIRECTION_270)
		{
			canGoOn = map[x][y+1] == 0 && map[x][y+2] == 0 && map[x + 1][y] == 0 && map[x + 1][y + 1] == 0;
		}
	}
	else if (tmpModule->type == MODULE_TYPE_T)
	{
		if (tmpModule->direction == DIRECTION_0)
		{
			canGoOn = map[x][y] == 0 && map[x+1][y] == 0 && map[x+2][y] == 0 && map[x+1][y+1] ==0;
		}
		else if (tmpModule->direction == DIRECTION_90)
		{
			canGoOn = map[x][y+1] == 0 && map[x + 1][y] == 0 && map[x+1][y + 1] == 0&&map[x+1][y+2] == 0;
		}
		else if (tmpModule->direction == DIRECTION_180)
		{
			canGoOn = map[x+1][y] == 0 && map[x][y + 1] == 0 && map[x+1][y+1] == 0 && map[x+2][y+1] == 0;
		}
		else if (tmpModule->direction == DIRECTION_270)
		{
			canGoOn = map[x][y] == 0 && map[x][y + 1] == 0 && map[x][y + 2] == 0 && map[x+1][y+1] == 0;
		}
	}
	if (!canGoOn) 
	{
		// 还原原来的map数据
		updateMap(moduleCube, map);
	}

	free(tmpModule);
	return canGoOn;
}

// 初始化模块, map为方块对应的bool地图
void initModule(ModuleCube* moduleCube)
{
	int type = rand() % 7 + 1;
	int direction = rand() % 4;
	memset(moduleCube, 0, sizeof(ModuleCube));
	moduleCube->type = type;
	moduleCube->direction = DIRECTION_0;
	moduleCube->gravity_x = WINDOW_WIDTH/2;
	moduleCube->gravity_y = 2 * CUBE_WIDTH;
	moduleCube->r = rand() % 56 + 200;
	moduleCube->g = rand() % 56 + 200;
	moduleCube->b = rand() % 56 + 200;

	if (type == MODULE_TYPE_SQUARE) {
		moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
		moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
		moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
		moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
	}
	else if (type == MODULE_TYPE_ONE)
	{
		// 初始化1字形
		moduleCube->left = moduleCube->gravity_x - 2 * CUBE_WIDTH;
		moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
		moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
		moduleCube->bottom = moduleCube->gravity_y;
	}
	else if (type == MODULE_TYPE_L || type == MODULE_TYPE_REL)
	{
		moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
		moduleCube->top = moduleCube->gravity_y - 2 * CUBE_WIDTH;
		moduleCube->right = moduleCube->gravity_x + CUBE_WIDTH;
		moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
	}
	else if (type == MODULE_TYPE_STEP || type == MODULE_TYPE_RESTEP || type == MODULE_TYPE_T)
	{
		moduleCube->left = moduleCube->gravity_x - CUBE_WIDTH;
		moduleCube->top = moduleCube->gravity_y - CUBE_WIDTH;
		moduleCube->right = moduleCube->gravity_x + 2 * CUBE_WIDTH;
		moduleCube->bottom = moduleCube->gravity_y + CUBE_WIDTH;
	}

	for (; moduleCube->direction < direction;)
	{
		if (checkCanRotate(moduleCube, map))
		{
			rotateModule(moduleCube, map);
		}
		
	}

}

// 显示对应的地图, 为了方便测试的时候跟踪数据使用
void renderMap(int map[][arraySizeRaw]) {
	int startX = WINDOW_WIDTH + 10;
	int startY = 20;

	int i = 0;
	int j = 0;

	int width = 20;
	setcolor(WHITE);
	for (i = 0; i < arraySizeColumn; i++)
	{
		for (j = 0; j < arraySizeRaw; j++)
		{
			RECT r = { startX + width * i, startY + width * j, startX + width * (i + 1), startY + width * (j + 1) };
			if (map[i][j] == 0)
			{
				drawtext(_T("0"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			else
			{
				drawtext(_T("1"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
		}
	}
}

// 消除完整的一行或者几行,返回得分
int fullLineRemove(int map[][arraySizeRaw]) 
{
	int winRows = 0;
	int winRow[arraySizeRaw];
	for (int i = 0; i < arraySizeRaw; i++)
	{
		bool win = 1;
		for (int j = 0; j < arraySizeColumn; j++)
		{
			if (map[j][i] == 0)
			{
				win = 0;
				break;
			}
		}
		if (win)
		{
			// 标记哪一行需要消除
			winRow[winRows] = i;
			// 统计有多少行是可以消除的
			winRows++;
		}
	}

	// 消除每一行
	for (int i = 0; i < winRows; i++)
	{
		for (int j = 0; j < arraySizeColumn; j++)
		{
			int k = winRow[i];
			while (k > 0)
			{
				
				if (map[j][k - 1] == 1)
				{
					// 画模块
					if (map[j][k] == 0)
					{
						drawCube(j * CUBE_WIDTH + CUBE_WIDTH/2, k*CUBE_WIDTH + CUBE_WIDTH/2, 0xffffff);
					}
				}
				else
				{
					// 清除模块
					if (map[j][k] == 1)
					{
						clearrectangle(j * CUBE_WIDTH, k * CUBE_WIDTH, (j+1) * CUBE_WIDTH, (k+1) * CUBE_WIDTH);
					}
				}
				map[j][k] = map[j][k - 1];
				k--;
			}
		}
	}

	return winRows;
}

// 画字符
void localText(LPCTSTR txt, int left, int top, int right, int bottom)
{
	RECT gameOverRect = { left, top, right, bottom };
	drawtext(txt, &gameOverRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 更新最高分数
void updateMaxScoreText(int maxScore)
{
	setcolor(WHITE);
	char data[11];
	CString cstr = _itoa(maxScore, data, 10);
	LPCTSTR t = (LPCTSTR)cstr;
	clearrectangle(WINDOW_WIDTH + 10, 60, WINDOW_WIDTH + 400, 90);
	localText(t, WINDOW_WIDTH + 10, 60, WINDOW_WIDTH + 400, 90);
}

// 更新当前的积分
void updateCurScoreText(int curScore)
{
	setcolor(WHITE);
	char data[11];
	CString cstr = _itoa(curScore, data, 10);
	LPCTSTR t = (LPCTSTR)cstr;
	clearrectangle(WINDOW_WIDTH + 10, 100, WINDOW_WIDTH + 400, 130);
	localText(t, WINDOW_WIDTH + 10, 100, WINDOW_WIDTH + 400, 130);
}

// 自动掉落下移线程任务
void threadAutoFall(void *)
{

	// 生成预览模块
	initModule(preModuleCube);
	// 初始化复制预览模块
	*currModuleCube = *preModuleCube;
	// 重新初始化预览模块
	cleanPreModule(preModuleCube);
	initModule(preModuleCube);
	// 更新初始化模块
	updateMap(currModuleCube, map);
	// 画出预览模块
	drawPreModule(preModuleCube);

	drawModule(currModuleCube);

	int eachGameResult = updateMap(currModuleCube, map);
	while (eachGameResult != -1)
	{
		//renderMap(map);
		
		m.lock();
		if (changed)
		{
			changed = 0;
			eachGameResult = updateMap(currModuleCube, map);
			m.unlock();
			continue;
		}
		// 循环生成和掉落模块
		if (eachGameResult == 0)
		{
			// 消除和统计得分
			score += fullLineRemove(map);
			// 更新当前得分
			updateCurScoreText(score);
			// 重新初始化模块
			*currModuleCube = *preModuleCube;
			// 重新初始化预览模块
			cleanPreModule(preModuleCube);
			initModule(preModuleCube);
			// 更新初始化模块
			updateMap(currModuleCube, map);
			// 画出预览模块
			drawPreModule(preModuleCube);
		}
		// 掉落下移
		resetMap(currModuleCube, map);
		cleanModuleCube(currModuleCube);
		moveModule(currModuleCube, DOWN, map);
		drawModule(currModuleCube);
		eachGameResult = updateMap(currModuleCube, map);
		m.unlock();
		// 睡眠0.5秒
		Sleep(500);
		
	}
	setcolor(WHITE);
	RECT gameOverRect = { WINDOW_WIDTH+10,10, WINDOW_WIDTH + 360, 30 };
	drawtext(_T("GAME OVER! press 'S' key to restart"), &gameOverRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 重新开始游戏
void restart() {
	//重绘方框
	clearrectangle(0, 0, arraySizeColumn * CUBE_WIDTH + 2, arraySizeRaw * CUBE_WIDTH + 2);
	clearrectangle(WINDOW_WIDTH + 10, 10, WINDOW_WIDTH + 360, 30);
	cleanPreModule(preModuleCube);
	setcolor(0x3300ff);
	rectangle(0, 0, arraySizeColumn * CUBE_WIDTH + 2, arraySizeRaw * CUBE_WIDTH + 2);
	// 更新最高分
	if (score > maxScore)
	{
		maxScore = score;
	}
	updateMaxScoreText(maxScore);
	// 更新当前分数
	score = 0;
	updateCurScoreText(score);
	// 预制下一个模块

	// 清空地图
	for (int i = 0; i < arraySizeColumn; i++)
	{
		for (int j = 0; j < arraySizeRaw; j++)
		{
			map[i][j] = 0;
		}
	}
	_beginthread(threadAutoFall, 0, NULL);
}

int main()
{
	initgraph(WINDOW_WIDTH + 400, WINDOW_HIGH);

	// IMAGE img(10, 8);
	// SetWorkingImage(&img);
	setlinecolor(0x3300ff);
	rectangle(0, 0, arraySizeColumn * CUBE_WIDTH+1, arraySizeRaw * CUBE_WIDTH+1);
	setlinecolor(0xFFFFFF);

	currModuleCube = (ModuleCube*)malloc(sizeof(ModuleCube));
	preModuleCube = (ModuleCube*)malloc(sizeof(ModuleCube));

	updateMaxScoreText(maxScore);
	updateCurScoreText(score);

	// 启动自动掉落线程
	_beginthread(threadAutoFall, 0, NULL);
	
	int key = _getch();
	/*
	* 上下左右键移动
	* K键旋转
	*/
	while (1) {
		if (key == 224) {
			key = _getch();
			m.lock();
			cleanModuleCube(currModuleCube);
			resetMap(currModuleCube, map);
			if (key == LEFT) 
			{
				moveModule(currModuleCube, LEFT, map);
			}
			else if (key == RIGHT) 
			{
				moveModule(currModuleCube, RIGHT, map);
			}
			else if (key == DOWN)
			{
				moveModule(currModuleCube, DOWN, map);
			}
			drawModule(currModuleCube);

			updateMap(currModuleCube, map);
			//renderMap(map);
			changed = 1;
			m.unlock();
		}
		else if (key == ROTATE) {
			// K键旋转
			m.lock();
			cleanModuleCube(currModuleCube);
			resetMap(currModuleCube, map);
			rotateModule(currModuleCube, map);
			drawModule(currModuleCube);

			updateMap(currModuleCube, map);
			//renderMap(map);
			changed = 1;
			m.unlock();
		}
		else if (key == RESTART)
		{
			//重新开始游戏
			restart();
		}
		else if (key == ESC)
		{
			// 结束游戏
			return 0;
		}
		key = _getch();

	}

	free(currModuleCube);
	// 按任意键退出
	//getchar();
	closegraph();
	return 0;
}
