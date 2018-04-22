#ifndef NOT_STAND_ALONE_FIND

#include<array>
#include<vector>
#include<queue>
#include<cstring>
#include<iostream>
#include<random>
#include<ctime>

using namespace std;

#define MAPWIDTH 10
#define MAPHEIGHT 20
static const int blockShape[7][4][8] = {
	{ { 0,0,1,0,-1,0,-1,-1 },{ 0,0,0,1,0,-1,1,-1 },{ 0,0,-1,0,1,0,1,1 },{ 0,0,0,-1,0,1,-1,1 } },
	{ { 0,0,-1,0,1,0,1,-1 },{ 0,0,0,-1,0,1,1,1 },{ 0,0,1,0,-1,0,-1,1 },{ 0,0,0,1,0,-1,-1,-1 } },
	{ { 0,0,1,0,0,-1,-1,-1 },{ 0,0,0,1,1,0,1,-1 },{ 0,0,-1,0,0,1,1,1 },{ 0,0,0,-1,-1,0,-1,1 } },
	{ { 0,0,-1,0,0,-1,1,-1 },{ 0,0,0,-1,1,0,1,1 },{ 0,0,1,0,0,1,-1,1 },{ 0,0,0,1,-1,0,-1,-1 } },
	{ { 0,0,-1,0,0,1,1,0 },{ 0,0,0,-1,-1,0,0,1 },{ 0,0,1,0,0,-1,-1,0 },{ 0,0,0,1,1,0,0,-1 } },
	{ { 0,0,0,-1,0,1,0,2 },{ 0,0,1,0,-1,0,-2,0 },{ 0,0,0,1,0,-1,0,-2 },{ 0,0,-1,0,1,0,2,0 } },
	{ { 0,0,0,1,-1,0,-1,1 },{ 0,0,-1,0,0,-1,-1,-1 },{ 0,0,0,-1,1,-0,1,-1 },{ 0,0,1,0,0,1,1,1 } }
};// 7种形状(长L| 短L| 反z| 正z| T| 直一| 田格)，4种朝向(上左下右)，8:每相邻的两个分别为x，y

#endif

static const int effiShape[7]={4,4,2,2,4,2,1}; 
static const int Xbegin[7][4]={{2,1,2,2},{2,1,2,2},{2,1,2,2},{2,1,2,2},{2,2,2,1},{1,3,1,2},{2,2,1,1}};
static const int Xend[7][4]={{9,9,9,10},{9,9,9,10},{9,9,9,10},{9,9,9,10},{9,10,9,9},{10,9,10,8},{10,10,9,9}};
static const int Ybegin[7][4]={{2,2,1,2},{2,2,1,2},{2,2,1,2},{2,2,1,2},{1,2,2,2},{2,1,3,1},{1,2,2,1}};
static const int Yend[7][4]={{20,19,19,19},{20,19,19,19},{20,19,19,19},{20,19,19,19},{19,19,20,19},{18,20,19,20},{19,20,20,19}};
static const int Ymax[7][4] = { {0,1,1,1},{0,1,1,1},{0,1,1,1},{0,1,1,1},{1,1,0,1},{2,0,1,0},{1,0,0,1} };
static const int Ymin[7][4] = { {-1,-1,0,-1},{-1,-1,0,-1},{-1,-1,0,-1},{-1,-1,0,-1},{0,-1,-1,-1},{-1,0,-2,0},{0,-1,-1,0} };

struct Block{
	int t;   // 标记方块类型的序号 0~6
	int x;            // 旋转中心的x轴坐标
	int y;            // 旋转中心的y轴坐标
	int o;       // 标记方块的朝向 0~3
};

struct Board{
	bool grid[MAPHEIGHT + 2][MAPWIDTH + 2];
	Board(int g[][MAPWIDTH + 2]){
		for(int i=0;i<MAPHEIGHT+2;i++)
			for(int j=0;j<MAPWIDTH+2;j++)
				grid[i][j]=g[i][j];
	}
};

inline void printField(const Board& board)
{
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
		for (int x = 1; x <= MAPWIDTH; x++)
			cout << i2s[board.grid[y][x] + 2];
		cout << endl;
	}
}

//assume the range of x and y of block is right
inline bool isValid(const Block& block, const Board& board)
{
	for (int i = 0; i < 4; i++)
	{
		int tmpX = block.x + blockShape[block.t][block.o][2 * i];
		int tmpY = block.y + blockShape[block.t][block.o][2 * i + 1];
		if (//tmpX < 1 || tmpX > MAPWIDTH ||
			//tmpY < 1 || tmpY > MAPHEIGHT ||
			board.grid[tmpY][tmpX] != 0)
			return false;
	}
	return true;
}

inline Block moveDown(const Block& block){
	Block ret = block;
	--ret.y;
	return ret;
}

inline bool onGround(const Block& block, const Board& board)
{
	if (isValid(block,board) && !isValid(moveDown(block),board))
		return true;
	return false;
}

inline bool rotation(const Block& block, const Board& board, int dest)
{
	Block temp = block;
	while (true)
	{
		if (!isValid(temp, board))
			return false;
		if (temp.o == dest)
			break;
		temp.o = (temp.o + 1) % 4;
	}
	return true;
}

//assume the place is correct
inline bool place(const Block& block, Board& board)
{
	//if (!onGround(block, board))
	//	return false;
	int i, tmpX, tmpY;
	for (i = 0; i < 4; i++)
	{
		tmpX = block.x + blockShape[block.t][block.o][2 * i];
		tmpY = block.y + blockShape[block.t][block.o][2 * i + 1];
		board.grid[tmpY][tmpX] = 1;
	}
	return true;
}

inline bool checkDirectDropTo(const Block& block, const Board& board)
{
	const int x=block.x;
	int y=block.y;
	auto &def = blockShape[block.t][block.o];
	for (; y <= MAPHEIGHT; y++)
		for (int i = 0; i < 4; i++)
		{
			int _x = def[i * 2] + x, _y = def[i * 2 + 1] + y;
			if (_y > MAPHEIGHT)
				continue;
			if (_y < 1 || _x < 1 || _x > MAPWIDTH || board.grid[_y][_x])
				return false;
		}
	return true;
}

inline int eliminate(Board& board, int start = 1, int end = 20)
{
	int count = 0;
	for (int i = start; i <= MAPHEIGHT; i++)
	{
		if (i > end) {
			if (!count) return 0;
			memmove(board.grid[i - count], board.grid[i],12*sizeof(bool)*(20-end) );
			memset(board.grid[21 - count], 0, 12 * sizeof(bool)*count);
			return count;
		}
		int emptyFlag = 1;
		int fullFlag = 1;
		for (int j = 1; j <= MAPWIDTH; j++)
		{
			if (board.grid[i][j] == 0)
				fullFlag = 0;
			else
				emptyFlag = 0;
			if(!fullFlag&&!emptyFlag) break;
		}
		if (fullFlag)
		{
			for (int j = 1; j <= MAPWIDTH; j++)
			{
				board.grid[i][j] = 0;
			}
			count++;
		}
		else if (emptyFlag)
		{
			break;
		}
		else if(count){
			memcpy(board.grid[i-count],board.grid[i],12*sizeof(bool));
			memset(board.grid[i]+1,0,10*sizeof(bool));
		}
	}
	return count;
}

inline bool canPut(int blockType, const Board& board)
{
	for (int y = MAPHEIGHT; y >= 1; y--)
		for (int x = 1; x <= MAPWIDTH; x++)
			for (int o = 0; o < 4; o++)
			{
				Block block{blockType,x,y,o};
				if (isValid(block, board) && checkDirectDropTo(block, board))
					return true;
			}
	return false;
}

#define MOVEIN if(block.x >= 1 && block.x <= MAPWIDTH &&\
	 block.y >= 1 && block.y <= MAPHEIGHT &&\
	!valid[block.x][block.y][block.o]&&isValid(block, board))\
	{valid[block.x][block.y][block.o] = true; validList[end++]=block;}

inline vector<Block> allPossibleState(int blockType, const Board& board){
	bool valid[MAPWIDTH+2][MAPHEIGHT+2][4]={};
	Block validList[800];
	int begin = 0, end = 0;
	for (int y = MAPHEIGHT; y >= 1; y--)
		for (int x = 1; x <= MAPWIDTH; x++)
			for (int o = 0; o < 4; o++)
			{
				Block block{blockType,x,y,o};
				if (isValid(block, board) && checkDirectDropTo(block, board)){
					valid[block.x][block.y][block.o]=true;
					validList[end++]=block;
				}
			}
	while(begin!=end){
		Block block = validList[begin++];
		block.y--;
		MOVEIN
		block.y++; block.x--;
		MOVEIN
		block.x++; block.x++;
		MOVEIN
		block.x--; block.o=(block.o+1)%4;
		MOVEIN
	}
	vector<Block> ret;
	for (int x = 1; x <= MAPWIDTH; x++)
		for (int y = 1; y <= MAPHEIGHT; y++)
			for (int o = 0; o < 4; o++)
				if(valid[x][y][o]&&onGround(Block{blockType,x,y,o}, board))
					ret.push_back(Block{blockType,x,y,o});
	return ret;				
}

inline int simpleMoves(int blockType, const Board& board, Block* ret){
	int count = 0;
	int altitude[12]={};
	altitude[0]=altitude[MAPWIDTH+1]=MAPHEIGHT;
	for(int x=1;x<=MAPWIDTH;x++){
		for(int y=1;y<=MAPHEIGHT;y++)
			if(board.grid[y][x]) altitude[x]=y;
	}
	for(int o=0;o<effiShape[blockType];o++){
		int X[4];
		for(int i=0;i<4;++i){
			X[i]=blockShape[blockType][o][2*i]+Xbegin[blockType][o];
		}
		for(int x=Xbegin[blockType][o];x<=Xend[blockType][o];x++){
			int lowest = 0;
			for(int i=0;i<4;i++){
				int height = altitude[X[i]++];
				int y = blockShape[blockType][o][2*i+1];
				int delta = height - y + 1;
				if(delta > lowest) lowest = delta;
			}
			if(lowest<=Yend[blockType][o])
				ret[count++]=(Block{blockType,x,lowest,o});
		}
	}
	return count;				
}

//features


typedef int* Altitude; // int[12]

Altitude altitude(const Board& board, Altitude ret){
	ret[0]=ret[MAPWIDTH+1]=MAPHEIGHT;
	for(int x=1;x<=MAPWIDTH;x++){
		ret[x]=0;
		for(int y=MAPHEIGHT;y>=1;y--){
			if(board.grid[y][x]){
				ret[x]=y; break;
			}
		}
	}
	return ret;
}
/*
double landHeight(const Block& block) {
	int min = 100, max = -100;
	for (int i = 0; i < 4; i++)
	{
		int tmpY = block.y + blockShape[block.t][block.o][2 * i + 1];
		if(tmpY>max) max=tmpY;
		if(tmpY<min) min=tmpY;
	}
	return (max+min)/2.0;
}
*/
double rowTransitions(const Board& board){
	int count=0;
	for(int y=1;y<=MAPHEIGHT;y++){
		for(int x=1;x<MAPWIDTH;x++)
			if(board.grid[y][x]!=board.grid[y][x+1]) count++;
		if(!board.grid[y][1]) count++;
		if(!board.grid[y][MAPWIDTH]) count++;
	}
		
	return count;
}

double colTransitions(const Board& board){
	int count=0;
	for(int y=1;y<MAPHEIGHT;y++)
		for(int x=1;x<=MAPWIDTH;x++){
			if(board.grid[y][x]!=board.grid[y+1][x]) count++;
		}
	for(int x=1;x<=MAPWIDTH;x++)
		if(!board.grid[1][x]) count++;
	return count;
}

double numHoles(const Board& board){
	int count=0;
	for(int x=1;x<=MAPWIDTH;x++){
		int cc=0;
		for(int y=1;y<=MAPHEIGHT;y++)
			if(board.grid[y][x]){
				count+=cc;
				cc=0;
			}else ++cc;
	}
	return count;
}

double connectdHoles(const Board& board){
	int count=0;
	for(int x=1;x<=MAPWIDTH;x++){
		int cc=0;
		for(int y=1;y<=MAPHEIGHT;y++)
			if(board.grid[y][x]){
				count+=cc;
				cc=0;
			}else cc=1;
	}
	return count;
}

double wellSums(const Board& board){
	int count=0;
	for(int y=1;y<=MAPHEIGHT;y++){
		for(int x=2;x<MAPWIDTH;x++){
			if(!board.grid[y][x]&&board.grid[y][x+1]&&board.grid[y][x-1]) count++;
			
		}
		if(!board.grid[y][1]&&board.grid[y][2]) count++;
		if(!board.grid[y][MAPWIDTH]&&board.grid[y][MAPWIDTH-1]) count++;
	}		
	return count;
}

double pileHeight(const Altitude& alti){
	int max = 0;
	for(int i=1; i<=MAPWIDTH;i++)
		if(alti[i]>max) max = alti[i];
	return max;
}


double altitudeDiff(const Altitude& alti){
	int min = 100; int max = -100;
	for(int i=1;i<=MAPWIDTH;i++){
		if(alti[i]<min) min=alti[i];
		if(alti[i]>max) max=alti[i];
	}
	return max-min;
}

double wellDepth(const Altitude& alti){
	int max = 0;
	for(int i=1;i<=MAPWIDTH;++i){
		if(alti[i]<alti[i-1]&&alti[i]<alti[i+1]){
			int ld = alti[i-1]-alti[i];
			int rd = alti[i+1]-alti[i];
			int depth = (ld>rd)?rd:ld;
			if(depth>max) max=depth;
		}
	}
	return max;
}

double wellSum(const Altitude& alti){
	int total = 0;
	for(int i=1;i<=MAPWIDTH;++i){
		if(alti[i]<alti[i-1]&&alti[i]<alti[i+1]){
			int ld = alti[i-1]-alti[i];
			int rd = alti[i+1]-alti[i];
			int depth = (ld>rd)?rd:ld;
			total += depth;
		}
	}
	return total;
}

double weightedBlock(const Board& board){
	int total = 0;
	for (int i = 1; i <= MAPHEIGHT; i++)
	{
		int linesum = 0;
		for (int j = 1; j <= MAPWIDTH; j++)
		{
			linesum+=board.grid[i][j];
		}
		if (!linesum)
		{
			break;
		}else{
			total+=linesum*i;
		}
	}
	return total;
}

static const int lut[256] =
	{
  		0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
	};
inline int bitcount(unsigned x){
	//only need to count lowest 16 bits
	return lut[x&0xff]+lut[(x>>8)&0xff];
}
double value(const Board& board, const int *blockCount){
	int pileHeight = 0;
	int alti[12];
	memset(alti,0,sizeof(alti));
	alti[0] = alti[11] = 20;
	int rowTransitions = 0;
	int colTransitions = 0;
	int numHoles = 0;
	int wellSums = 0;
	int bits[23];
	bits[21] = bits[22] = 0;
	for(int y=0;y<=20;y++) bits[y] = -1;
	for(int y=1;y<=20;y++){
		bool flag = false;
		int bit = 1<<12;
		for(int x = 1;x<=10;x++){
			if(!board.grid[y][x]) bits[y]&=(~bit);
			else {
				flag = true;
				++alti[x];
			}
			bit>>=1;
		}
		if(flag) pileHeight++;
	}
	int accu = 0;
	int altiDiff = altitudeDiff(alti);
	int wellDep = wellDepth(alti);
	int wellsum = wellSum(alti);
	for(int y=20;y>=0;y--){
		int rc = bits[y]^(bits[y]>>1);
		int cc = bits[y]^(bits[y+1]);
		int nh = (~bits[y])&accu;
		int ws = bits[y]&(~(bits[y]>>1))&(bits[y]>>2);
		rowTransitions += bitcount(rc);
		colTransitions += bitcount(cc);
		numHoles += bitcount(nh);
		wellSums += bitcount(ws);
		accu |= bits[y];
	}
	return 
			0.612686*log(21-pileHeight)
			-3.3249*rowTransitions
			-7.98138*colTransitions
			-6.00207*numHoles
			-5.76848*wellSums;
}

typedef array<double,12> Param;
/*
double value(const Block& block, const Board& board, Param param){
	double ret=0;
	Board b = board;
	ret+=param[0]*landHeight(block);
	place(block,b);
	ret+=param[1]*eliminate(b);
	ret+=param[2]*rowTransitions(b);
	ret+=param[3]*colTransitions(b);
	ret+=param[4]*numHoles(b);
	ret+=param[5]*wellSums(b);
	ret+=param[6]*connectdHoles(b);
	Altitude alti = altitude(b);
	ret+=param[7]*pileHeight(alti);
	ret+=param[8]*altitudeDiff(alti);
	ret+=param[9]*wellDepth(alti);
	ret+=param[10]*wellSum(alti);
	ret+=param[11]*weightedBlock(b);
	return ret;
}*/
