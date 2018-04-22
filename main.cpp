#define LANDHEIGHT -3.66825
#define SEARCH_SIZE 11 
#define ELIMINATION_REWARD 5.02571

#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <array>
#include <vector>
#include <queue>
#include <cstring>
#include <random>
using namespace std;

#define MAPWIDTH 10
#define MAPHEIGHT 20

// 我所在队伍的颜色（0为红，1为蓝，仅表示队伍，不分先后）
int currBotColor;
int enemyColor;

// 先y后x，记录地图状态，0为空，1为以前放置，2为刚刚放置，负数为越界
// （2用于在清行后将最后一步撤销再送给对方）
int gridInfo[2][MAPHEIGHT + 2][MAPWIDTH + 2] = { 0 };

// 代表分别向对方转移的行
int trans[2][4][MAPWIDTH + 2] = { 0 };

// 转移行数
int transCount[2] = { 0 };

// 运行eliminate后的当前高度
int maxHeight[2] = { 0 };

// 总消去行数的分数之和
int elimTotal[2] = { 0 };

// 一次性消去行数对应分数
const int elimBonus[] = { 0, 1, 3, 5, 7 };

// 给对应玩家的各类块的数目总计
int typeCountForColor[2][7] = { 0 };

static const int blockShape[7][4][8] = {
	{ { 0,0,1,0,-1,0,-1,-1 },{ 0,0,0,1,0,-1,1,-1 },{ 0,0,-1,0,1,0,1,1 },{ 0,0,0,-1,0,1,-1,1 } },
	{ { 0,0,-1,0,1,0,1,-1 },{ 0,0,0,-1,0,1,1,1 },{ 0,0,1,0,-1,0,-1,1 },{ 0,0,0,1,0,-1,-1,-1 } },
	{ { 0,0,1,0,0,-1,-1,-1 },{ 0,0,0,1,1,0,1,-1 },{ 0,0,-1,0,0,1,1,1 },{ 0,0,0,-1,-1,0,-1,1 } },
	{ { 0,0,-1,0,0,-1,1,-1 },{ 0,0,0,-1,1,0,1,1 },{ 0,0,1,0,0,1,-1,1 },{ 0,0,0,1,-1,0,-1,-1 } },
	{ { 0,0,-1,0,0,1,1,0 },{ 0,0,0,-1,-1,0,0,1 },{ 0,0,1,0,0,-1,-1,0 },{ 0,0,0,1,1,0,0,-1 } },
	{ { 0,0,0,-1,0,1,0,2 },{ 0,0,1,0,-1,0,-2,0 },{ 0,0,0,1,0,-1,0,-2 },{ 0,0,-1,0,1,0,2,0 } },
	{ { 0,0,0,1,-1,0,-1,1 },{ 0,0,-1,0,0,-1,-1,-1 },{ 0,0,0,-1,1,-0,1,-1 },{ 0,0,1,0,0,1,1,1 } }
};// 7种形状(长L| 短L| 反z| 正z| T| 直一| 田格)，4种朝向(上左下右)，8:每相邻的两个分别为x，y

#define NOT_STAND_ALONE_FIND

#include "util.cpp" 

#define MINVALUE -10000000
#define MAXVALUE  10000000

mt19937 engine(time(0));

bool operator<(const Block& a,const Block& b){
	return a.y<b.y;
}

inline void sortBlocks(Block* begin,Block* end){
	Block bucket[21][40];
	int size[21]={};
	Block* p = begin;
	for(;p!=end;p++) bucket[p->y][size[p->y]++]=*p;
	for(int i=1;i<=20;i++)
		for(int j=size[i]-1;j>=0;j--)
			*begin++=bucket[i][j];
}

extern double value(const Board& board, const int *blockCount);

template<int depth>
double QO(int blockType, const Board& board, int* blockCount, double a, double b);

template<int depth>
double QS(const Block& block, const Board& board, int *blockCount, double a, double b){
	Board newboard = board; 
	place(block, newboard);
	int elimnum = eliminate(newboard,block.y+Ymin[block.t][block.o], block.y + Ymax[block.t][block.o]);
	int opChoice[7]; int ptoOC = 0;
	int maxCount = 0, minCount = 99;
	for (int i = 0; i < 7; i++)
	{
		if (blockCount[i] > maxCount)
			maxCount = blockCount[i];
		if (blockCount[i] < minCount)
			minCount = blockCount[i];
	}
	for(int i = 0; i < 7; i++)
		if(blockCount[i]<minCount+2) opChoice[ptoOC++] = i;
	//shuffle opChoice
	shuffle(opChoice,opChoice+ptoOC,engine);
	double extra = elimnum * ELIMINATION_REWARD + LANDHEIGHT*block.y;
    a -= extra; b-=extra;
	for(int i=0;i<ptoOC&&a<b; i++){
		double op = QO<depth-1>(opChoice[i], newboard, blockCount, a, b);
		if(op<b) b = op;
	}
	return b + extra;
}

template<>
double QS<0>(const Block& block, const Board& board, int *blockCount, double a, double b){
	Board newboard = board; 
	place(block, newboard);
	int elimnum = eliminate(newboard,block.y + Ymin[block.t][block.o], block.y + Ymax[block.t][block.o]);
	return value(newboard,blockCount) + elimnum * ELIMINATION_REWARD + LANDHEIGHT*block.y;
}

template<int depth>
double QO(int blockType, const Board& board, int* blockCount, double a, double b){
	blockCount[blockType]++;
	Block moves[40];
	int size = simpleMoves(blockType, board, moves);
	if(size==0) return a;
	//shuffle moves
	shuffle(moves,moves+size,engine);
#ifdef SEARCH_SIZE
	if(depth>=0){
		if(size>SEARCH_SIZE){
			sortBlocks(moves,moves+size);
			size = SEARCH_SIZE;
		}	
	} 
#endif
	for (int i = 0; i < size&&a<b;i++) {
		double se = QS<depth>(moves[i], board, blockCount, a, b);
		if(se>a) a = se;
	}
	blockCount[blockType]--;
	return a;
}

template<int depth>
Block searchS(int blockType, const Board& board, int* blockCount){
	Block moves[40];
	int size = simpleMoves(blockType, board, moves);
	if(size == 0) return searchS<-1>(blockType, board, blockCount);
	shuffle(moves,moves+size,engine);
    sortBlocks(moves,moves+size);
	Block best; double max = MINVALUE-1;
	for (int i = 0; i < size;i++) {
		double se = QS<depth>(moves[i], board, blockCount, MINVALUE, MAXVALUE);
		if(se>max){
			max = se;
			best = moves[i];
		}
	}
	return best;
}

template<>
Block searchS<-1>(int blockType, const Board& board, int* blockCount){
	vector<Block> moves = allPossibleState(blockType, board);
	Block best; double max = MINVALUE;
	for(const Block block: moves){
		double se = QS<0>(block, board, blockCount, max, MAXVALUE);
		if(se>max){
			max = se;
			best = block;
		}
	}
	return best;
}

template<int depth>
int searchO(const Board& board, int* blockCount){
	int opChoice[7]; int ptoOC = 0;
	int maxCount = 0, minCount = 99;
	for (int i = 0; i < 7; i++)
	{
		if (blockCount[i] > maxCount)
			maxCount = blockCount[i];
		if (blockCount[i] < minCount)
			minCount = blockCount[i];
	}
	for(int i = 0; i < 7; i++)
		if(blockCount[i]<minCount+2) opChoice[ptoOC++] = i;
	shuffle(opChoice,opChoice+ptoOC,engine);
	double worst = MAXVALUE;
	int ret;
	for(int i=0;i<ptoOC; i++){
		double op = QO<depth>(opChoice[i], board, blockCount, MINVALUE, worst);
		if(op<worst){
			worst = op;
			ret = opChoice[i];
		}
	}
	return ret;
}

class Tetris
{
public:
	const int blockType;   // 标记方块类型的序号 0~6
	int blockX;            // 旋转中心的x轴坐标
	int blockY;            // 旋转中心的y轴坐标
	int orientation;       // 标记方块的朝向 0~3
	const int(*shape)[8]; // 当前类型方块的形状定义

	int color;

	Tetris(int t, int color) : blockType(t), shape(blockShape[t]), color(color)
	{ }

	inline Tetris &set(int x = -1, int y = -1, int o = -1)
	{
		blockX = x == -1 ? blockX : x;
		blockY = y == -1 ? blockY : y;
		orientation = o == -1 ? orientation : o;
		return *this;
	}

	// 判断当前位置是否合法
	inline bool isValid(int x = -1, int y = -1, int o = -1)
	{
		x = x == -1 ? blockX : x;
		y = y == -1 ? blockY : y;
		o = o == -1 ? orientation : o;
		if (o < 0 || o > 3)
			return false;

		int i, tmpX, tmpY;
		for (i = 0; i < 4; i++)
		{
			tmpX = x + shape[o][2 * i];
			tmpY = y + shape[o][2 * i + 1];
			if (tmpX < 1 || tmpX > MAPWIDTH ||
				tmpY < 1 || tmpY > MAPHEIGHT ||
				gridInfo[color][tmpY][tmpX] != 0)
				return false;
		}
		return true;
	}

	// 判断是否落地
	inline bool onGround()
	{
		if (isValid() && !isValid(-1, blockY - 1))
			return true;
		return false;
	}

	// 将方块放置在场地上
	inline bool place()
	{
		if (!onGround())
			return false;

		int i, tmpX, tmpY;
		for (i = 0; i < 4; i++)
		{
			tmpX = blockX + shape[orientation][2 * i];
			tmpY = blockY + shape[orientation][2 * i + 1];
			gridInfo[color][tmpY][tmpX] = 2;
		}
		return true;
	}

	// 检查能否逆时针旋转自己到o
	inline bool rotation(int o)
	{
		if (o < 0 || o > 3)
			return false;

		if (orientation == o)
			return true;

		int fromO = orientation;
		while (true)
		{
			if (!isValid(-1, -1, fromO))
				return false;

			if (fromO == o)
				break;

			fromO = (fromO + 1) % 4;
		}
		return true;
	}
};

// 围一圈护城河
void init()
{
	int i;
	for (i = 0; i < MAPHEIGHT + 2; i++)
	{
		gridInfo[1][i][0] = gridInfo[1][i][MAPWIDTH + 1] = -2;
		gridInfo[0][i][0] = gridInfo[0][i][MAPWIDTH + 1] = -2;
	}
	for (i = 0; i < MAPWIDTH + 2; i++)
	{
		gridInfo[1][0][i] = gridInfo[1][MAPHEIGHT + 1][i] = -2;
		gridInfo[0][0][i] = gridInfo[0][MAPHEIGHT + 1][i] = -2;
	}
}

namespace Util
{

	// 检查能否从场地顶端直接落到当前位置
	inline bool checkDirectDropTo(int color, int blockType, int x, int y, int o)
	{
		auto &def = blockShape[blockType][o];
		for (; y <= MAPHEIGHT; y++)
			for (int i = 0; i < 4; i++)
			{
				int _x = def[i * 2] + x, _y = def[i * 2 + 1] + y;
				if (_y > MAPHEIGHT)
					continue;
				if (_y < 1 || _x < 1 || _x > MAPWIDTH || gridInfo[color][_y][_x])
					return false;
			}
		return true;
	}

	// 消去行
	void eliminate(int color)
	{
		int &count = transCount[color] = 0;
		int i, j, emptyFlag, fullFlag;
		maxHeight[color] = MAPHEIGHT;
		for (i = 1; i <= MAPHEIGHT; i++)
		{
			emptyFlag = 1;
			fullFlag = 1;
			for (j = 1; j <= MAPWIDTH; j++)
			{
				if (gridInfo[color][i][j] == 0)
					fullFlag = 0;
				else
					emptyFlag = 0;
			}
			if (fullFlag)
			{
				for (j = 1; j <= MAPWIDTH; j++)
				{
					// 注意这里只转移以前的块，不包括最后一次落下的块（“撤销最后一步”）
					trans[color][count][j] = gridInfo[color][i][j] == 1 ? 1 : 0;
					gridInfo[color][i][j] = 0;
				}
				count++;
			}
			else if (emptyFlag)
			{
				maxHeight[color] = i - 1;
				break;
			}
			else
				for (j = 1; j <= MAPWIDTH; j++)
				{
					gridInfo[color][i - count][j] =
						gridInfo[color][i][j] > 0 ? 1 : gridInfo[color][i][j];
					if (count)
						gridInfo[color][i][j] = 0;
				}
		}
		maxHeight[color] -= count;
		elimTotal[color] += elimBonus[count];
	}

	// 转移双方消去的行，返回-1表示继续，否则返回输者
	int transfer()
	{
		int color1 = 0, color2 = 1;
		if (transCount[color1] == 0 && transCount[color2] == 0)
			return -1;
		if (transCount[color1] == 0 || transCount[color2] == 0)
		{
			if (transCount[color1] == 0 && transCount[color2] > 0)
				swap(color1, color2);
			int h2;
			maxHeight[color2] = h2 = maxHeight[color2] + transCount[color1];
			if (h2 > MAPHEIGHT)
				return color2;
			int i, j;

			for (i = h2; i > transCount[color1]; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = gridInfo[color2][i - transCount[color1]][j];

			for (i = transCount[color1]; i > 0; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = trans[color1][i - 1][j];
			return -1;
		}
		else
		{
			int h1, h2;
			maxHeight[color1] = h1 = maxHeight[color1] + transCount[color2];//从color1处移动count1去color2
			maxHeight[color2] = h2 = maxHeight[color2] + transCount[color1];

			if (h1 > MAPHEIGHT) return color1;
			if (h2 > MAPHEIGHT) return color2;

			int i, j;
			for (i = h2; i > transCount[color1]; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = gridInfo[color2][i - transCount[color1]][j];

			for (i = transCount[color1]; i > 0; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = trans[color1][i - 1][j];

			for (i = h1; i > transCount[color2]; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color1][i][j] = gridInfo[color1][i - transCount[color2]][j];

			for (i = transCount[color2]; i > 0; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color1][i][j] = trans[color2][i - 1][j];

			return -1;
		}
	}

	// 颜色方还能否继续游戏
	inline bool canPut(int color, int blockType)
	{
		Tetris t(blockType, color);
		for (int y = MAPHEIGHT; y >= 1; y--)
			for (int x = 1; x <= MAPWIDTH; x++)
				for (int o = 0; o < 4; o++)
				{
					t.set(x, y, o);
					if (t.isValid() && checkDirectDropTo(color, blockType, x, y, o))
						return true;
				}
		return false;
	}

	// 打印场地用于调试
	inline void printField()
	{
#ifndef _BOTZONE_ONLINE
		static const char *i2s[] = {
			"~~",
			"~~",
			"  ",
			"[]",
			"##"
		};
		cout << "~~：墙，[]：块，##：新块" << endl;
		for (int y = MAPHEIGHT + 1; y >= 0; y--)
		{
			for (int x = 0; x <= MAPWIDTH + 1; x++)
				cout << i2s[gridInfo[0][y][x] + 2];
			for (int x = 0; x <= MAPWIDTH + 1; x++)
				cout << i2s[gridInfo[1][y][x] + 2];
			cout << endl;
		}
#endif
	}
}

Param Defparam = {
	-4.500158825082766, //landHeight
	3.4181268101392694, //rowEliminated
	-3.2178882868487753,//rowTransitions
	-9.348695305445199, //colTransitions
	-7.899265427351652, //numHoles
	-3.3855972247263626,//wellSums
	0,//connectdHoles
	0,//pileHeight
	0,//altitudeDiff
	-1,//wellDepth
	0,//wellSum
	0,//weightedBlock
	};

Param Attparam = {
	-4.500158825082766, //landHeight
	3.4181268101392694, //rowEliminated
	-3.2178882868487753,//rowTransitions
	-9.348695305445199, //colTransitions
	-7.899265427351652, //numHoles
	-3.3855972247263626,//wellSums
	0,//connectdHoles
	0,//pileHeight
	0,//altitudeDiff
	0,//wellDepth
	0,//wellSum
	0,//weightedBlock
	};

int main()
{
	// 加速输入
	istream::sync_with_stdio(false);
	srand(time(NULL));
	init();

	int turnID, blockType;
	int nextTypeForColor[2];
	cin >> turnID;

	// 先读入第一回合，得到自己的颜色
	// 双方的第一块肯定是一样的
	cin >> blockType >> currBotColor;
	enemyColor = 1 - currBotColor;
	nextTypeForColor[0] = blockType;
	nextTypeForColor[1] = blockType;
	typeCountForColor[0][blockType]++;
	typeCountForColor[1][blockType]++;

	// 然后分析以前每回合的输入输出，并恢复状态
	// 循环中，color 表示当前这一行是 color 的行为
	// 平台保证所有输入都是合法输入
	for (int i = 1; i < turnID; i++)
	{
		int currTypeForColor[2] = { nextTypeForColor[0], nextTypeForColor[1] };
		int x, y, o;
		// 根据这些输入输出逐渐恢复状态到当前回合

		// 先读自己的输出，也就是自己的行为
		// 自己的输出是自己的最后一步
		// 然后模拟最后一步放置块
		cin >> blockType >> x >> y >> o;

		// 我当时把上一块落到了 x y o！
		Tetris myBlock(currTypeForColor[currBotColor], currBotColor);
		myBlock.set(x, y, o).place();

		// 我给对方什么块来着？
		typeCountForColor[enemyColor][blockType]++;
		nextTypeForColor[enemyColor] = blockType;

		// 然后读自己的输入，也就是对方的行为
		// 裁判给自己的输入是对方的最后一步
		cin >> blockType >> x >> y >> o;

		// 对方当时把上一块落到了 x y o！
		Tetris enemyBlock(currTypeForColor[enemyColor], enemyColor);
		enemyBlock.set(x, y, o).place();

		// 对方给我什么块来着？
		typeCountForColor[currBotColor][blockType]++;
		nextTypeForColor[currBotColor] = blockType;

		// 检查消去
		Util::eliminate(0);
		Util::eliminate(1);

		// 进行转移
		Util::transfer();
	}


	int blockForEnemy, finalX, finalY, finalO;

	// 做出决策（你只需修改以下部分）

	// 遇事不决先输出（平台上编译不会输出）
	Util::printField();

	Board myBoard(gridInfo[currBotColor]);
	Board enemyBoard(gridInfo[enemyColor]);
	Block ans = searchS<3>(nextTypeForColor[currBotColor],myBoard,typeCountForColor[currBotColor]);
	Block ea = searchS<-1>(nextTypeForColor[enemyColor],enemyBoard,typeCountForColor[enemyColor]);
	place(ea,enemyBoard);
	eliminate(enemyBoard);
	blockForEnemy = searchO<3>(enemyBoard,typeCountForColor[enemyColor]);
	finalX = ans.x;
	finalY = ans.y;
	finalO = ans.o;
	
	// 决策结束，输出结果（你只需修改以上部分）

	cout << blockForEnemy << " " << finalX << " " << finalY << " " << finalO;

	return 0;
}
