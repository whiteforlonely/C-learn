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

void cleanModuleCube(ModuleCube* moduleCube)
{
	clearrectangle(moduleCube->left, moduleCube->top, moduleCube->right, moduleCube->bottom);
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
	}
	else if (moduleCube->type == MODULE_TYPE_STEP)
	{
		// TODO 画台阶型，也是有两种形态
	}
	else if (moduleCube->type == MODULE_TYPE_RESTEP)
	{
		// TODO 画反台阶，也是两种形态
	}
	else if (moduleCube->type == MODULE_TYPE_L)
	{
		// TODO 画L型，有四种形态
	}
	else if (moduleCube->type == MODULE_TYPE_REL)
	{
		// TODO 画反L型，有四种形态
	}
	else if (moduleCube->type == MODULE_TYPE_T)
	{
		// TODO 画T型，有四种形态
	}
}

// 旋转模块
void rotateMoudle(ModuleCube* moduleCube)
{
	// 这边的方案是先判断类型再判断角度，或者反过来判断也是可以的
}

// 初始化模块
ModuleCube* initModule(int type, int direction, int gravity_x, int gravity_y)
{
	ModuleCube* moduleCube = (ModuleCube*)malloc(sizeof(ModuleCube));
	moduleCube->type = type;
	moduleCube->direction = direction;
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
		// TODO 初始化1字形
	}

	return moduleCube;
}

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
	drawModule(moduleCube);
}

int main()
{
	initgraph(640, 480);

	// IMAGE img(10, 8);
	// SetWorkingImage(&img);

	int start = 300;

	ModuleCube* module1 = initModule(MODULE_TYPE_SQUARE, DIRECTION_0, 300, 20);
	drawModule(module1);

	int key = _getch();
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
		}
		key = _getch();

	}

	// 按任意键退出
	//getchar();
	closegraph();
	free(module1);
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
