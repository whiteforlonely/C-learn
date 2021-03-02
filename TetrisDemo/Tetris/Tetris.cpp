
/**
* 俄罗斯方块小游戏
* 
* @Author Saturday（柯清源）
* @Date 2021-03-01
* @description 俄罗斯方块,简单的模仿俄罗斯方块游戏
* 
*/

#include <iostream>
#include <graphics.h>
#include <conio.h>
#include <malloc.h>

#define UP				72
#define DOWN			80
#define LEFT			75
#define RIGHT			77

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

#define WINDOW_WIDTH		640	// 游戏界面的宽
#define WINDOW_HIGH			480 // 游戏界面的高


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
} ModuleCube;

/*
画小方块
x,y 为小方块的中心坐标
*/
void drawCube(int x, int y)
{
	setfillcolor(BLUE);
	setfillstyle(BS_HATCHED, HS_DIAGCROSS);
	fillrectangle(x - CUBE_WIDTH / 2, y - CUBE_WIDTH / 2, x + CUBE_WIDTH / 2, y + CUBE_WIDTH / 2);
}

void clearCube(int x, int y)
{
	clearrectangle(x - CUBE_WIDTH / 2, y - CUBE_WIDTH / 2, x + CUBE_WIDTH / 2, y + CUBE_WIDTH / 2);
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
	if (moduleCube->type == MODULE_TYPE_SQUARE)
	{
		//画正方形，因为正方形是不管怎么旋转都是一样的，所以较为简单
		// 画左上方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
		// 画右上方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
		// 画左下方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		// 画右下方的小方块
		drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
	}
	else if (moduleCube->type == MODULE_TYPE_ONE)
	{
		// TODO 画一字型，有两种形态
		if (moduleCube -> direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180)
		{
			// 横着的'一'
			// 左到右第一个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			// 第二个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			// 第三个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			// 第四个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 3 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
		}
		else
		{
			// 竖着的'1'
			// 从上到下第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			// 第二个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第三个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
			// 第四个块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 3 * CUBE_WIDTH);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_STEP)
	{
		// 画台阶型，也是有两种形态
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180) {
			// 左到右上台阶，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->bottom - CUBE_WIDTH / 2 );
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->bottom - CUBE_WIDTH / 2);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
		}
		else
		{
			// 左高右底闪电，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_RESTEP)
	{
		// TODO 画反台阶，也是两种形态
		if (moduleCube->direction == DIRECTION_0 || moduleCube->direction == DIRECTION_180) {
			// 左到右下台阶，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->bottom - CUBE_WIDTH / 2);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
		else
		{
			// 左底右高闪电，从下到上，左到右，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_L)
	{
		// TODO 画L型，有四种形态
		if (moduleCube->direction == DIRECTION_0)
		{
			// 正L型
			// 从左到右，上到下，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			// 从左到右，上到下 第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			// 第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			// 从左到右，从上到下，第一块
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第二块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			// 第三块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			// 第四块
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_REL)
	{
		// 画反L型，有四种形态
		if (moduleCube->direction == DIRECTION_0)
		{
			// 从上到下， 左到右
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
	}
	else if (moduleCube->type == MODULE_TYPE_T)
	{
		// 画T型，有四种形态
		if (moduleCube->direction == DIRECTION_0)
		{
			// 从上到下， 左到右
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_90)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_180)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
		else if (moduleCube->direction == DIRECTION_270)
		{
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2, moduleCube->top + CUBE_WIDTH / 2 + 2 * CUBE_WIDTH);
			drawCube(moduleCube->left + CUBE_WIDTH / 2 + CUBE_WIDTH, moduleCube->top + CUBE_WIDTH / 2 + CUBE_WIDTH);
		}
	}
}

// 移动模块
void moveModule(ModuleCube* moduleCube, int directionKey) {
	cleanModuleCube(moduleCube);
	if (directionKey == DOWN) {
		moduleCube->gravity_y += CUBE_WIDTH;
		moduleCube->top += CUBE_WIDTH;
		moduleCube->bottom += CUBE_WIDTH;
	}
	else if (directionKey == UP)
	{
		moduleCube->gravity_y -= CUBE_WIDTH;
		moduleCube->top -= CUBE_WIDTH;
		moduleCube->bottom -= CUBE_WIDTH;
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
void rotateModule(ModuleCube* moduleCube)
{

	// 先清理掉原来的模块
	cleanModuleCube(moduleCube);
	// 这边的方案是先判断类型再判断角度，或者反过来判断也是可以的
	// 角度是直接设置的，每次旋转都是按照顺时针旋转90度
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
		moveModule(moduleCube, RIGHT);
		moveModule(moduleCube, RIGHT);
	}
	else if (moduleCube->left < 0)
	{
		// 超出了左边界一个小方块长度
		moveModule(moduleCube, RIGHT);
	}
	
	if (moduleCube->right > WINDOW_WIDTH + CUBE_WIDTH)
	{
		 // 超出了右边界两个小方块长度
		moveModule(moduleCube, LEFT);
		moveModule(moduleCube, LEFT);
	}
	else if (moduleCube->right > WINDOW_WIDTH)
	{
		// 超出了右边界一个小方块
		moveModule(moduleCube, LEFT);
	}

	// 上边界可以不用处理的
	if (moduleCube->top < -CUBE_WIDTH)
	{
		// 超出了右边界两个小方块长度
		moveModule(moduleCube, DOWN);
		moveModule(moduleCube, DOWN);
	}
	else if (moduleCube->top < 0)
	{
		// 超出了右边界一个小方块
		moveModule(moduleCube, DOWN);
	}

	// 下边界判断
	if (moduleCube->bottom > WINDOW_HIGH + CUBE_WIDTH)
	{
		// 超出了右边界两个小方块长度
		moveModule(moduleCube, UP);
		moveModule(moduleCube, UP);
	}
	else if (moduleCube->bottom > WINDOW_HIGH)
	{
		moveModule(moduleCube, UP);
	}
}

// 初始化模块
ModuleCube* initModule(int type, int direction, int gravity_x, int gravity_y)
{
	ModuleCube* moduleCube = (ModuleCube*)malloc(sizeof(ModuleCube));
	moduleCube->type = type;
	moduleCube->direction = DIRECTION_0;
	moduleCube->gravity_x = gravity_x;
	moduleCube->gravity_y = gravity_y;

	if (type == MODULE_TYPE_SQUARE) {
		moduleCube->left = gravity_x - CUBE_WIDTH;
		moduleCube->top = gravity_y - CUBE_WIDTH;
		moduleCube->right = gravity_x + CUBE_WIDTH;
		moduleCube->bottom = gravity_y + CUBE_WIDTH;
	}
	else if (type == MODULE_TYPE_ONE)
	{
		// 初始化1字形
		moduleCube->left = gravity_x - 2 * CUBE_WIDTH;
		moduleCube->top = gravity_y - CUBE_WIDTH;
		moduleCube->right = gravity_x + 2 * CUBE_WIDTH;
		moduleCube->bottom = gravity_y;
	}
	else if (type == MODULE_TYPE_L || type == MODULE_TYPE_REL)
	{
		moduleCube->left = gravity_x - CUBE_WIDTH;
		moduleCube->top = gravity_y - 2 * CUBE_WIDTH;
		moduleCube->right = gravity_x + CUBE_WIDTH;
		moduleCube->bottom = gravity_y + CUBE_WIDTH;
	}
	else if (type == MODULE_TYPE_STEP || type == MODULE_TYPE_RESTEP || type == MODULE_TYPE_T)
	{
		moduleCube->left = gravity_x - CUBE_WIDTH;
		moduleCube->top = gravity_y - CUBE_WIDTH;
		moduleCube->right = gravity_x + 2 * CUBE_WIDTH;
		moduleCube->bottom = gravity_y + CUBE_WIDTH;
	}

	for (; moduleCube->direction < direction;)
	{
		rotateModule(moduleCube);
	}

	return moduleCube;
}

int main()
{
	initgraph(WINDOW_WIDTH, WINDOW_HIGH);
	// 指针数组，专门跟踪二位数组的小方块数据
	int map[10][20] = {0};

	// IMAGE img(10, 8);
	// SetWorkingImage(&img);

	int start = 300;

	ModuleCube* module1 = initModule(MODULE_TYPE_T, DIRECTION_90, 300, 60);
	drawModule(module1);

	int key = _getch();
	/*
	* 上下左右键移动
	* K键旋转
	*/
	while (1) {
		if (key == 224) {
			key = _getch();
			if (key == LEFT) {
				//printf("LEFT\n");
				moveModule(module1, LEFT);
			}
			else if (key == RIGHT) {
				//printf("RIGHT\n");
				moveModule(module1, RIGHT);
			}
			else if (key == UP)
			{
				//printf("UP\n");
				moveModule(module1, UP);
			}
			else if (key == DOWN)
			{
				//printf("DOWN\n");
				moveModule(module1, DOWN);
			}
			drawModule(module1);
		}
		else if (key == 75) {
			printf("start to rotate -----------------<");
			rotateModule(module1);
			drawModule(module1);
		}
		key = _getch();

	}

	// 按任意键退出
	//getchar();
	closegraph();
	free(module1);
	return 0;
}
