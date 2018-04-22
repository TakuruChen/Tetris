#include <algorithm>
#include <windows.h>
#include <fstream>
#include "util.cpp"

#define MINVALUE -10000000
#define MAXVALUE  10000000

int insertLinesCount[40*40*40*40];
int totalCount;

static bool lineType[40][12]={
	{1,0,0,0,0,0,0,0,0,0,0,1},//0
	{1,0,1,1,1,1,1,1,1,1,1,1},//0-1
	{1,1,0,1,1,1,1,1,1,1,1,1},
	{1,1,1,0,1,1,1,1,1,1,1,1},
	{1,1,1,1,0,1,1,1,1,1,1,1},
	{1,1,1,1,1,0,1,1,1,1,1,1},
	{1,1,1,1,1,1,0,1,1,1,1,1},
	{1,1,1,1,1,1,1,0,1,1,1,1},
	{1,1,1,1,1,1,1,1,0,1,1,1},
	{1,1,1,1,1,1,1,1,1,0,1,1},
	{1,1,1,1,1,1,1,1,1,1,0,1},//0-10
	{1,0,0,1,1,1,1,1,1,1,1,1},//1-1
	{1,1,0,0,1,1,1,1,1,1,1,1},
	{1,1,1,0,0,1,1,1,1,1,1,1},
	{1,1,1,1,0,0,1,1,1,1,1,1},
	{1,1,1,1,1,0,0,1,1,1,1,1},
	{1,1,1,1,1,1,0,0,1,1,1,1},
	{1,1,1,1,1,1,1,0,0,1,1,1},
	{1,1,1,1,1,1,1,1,0,0,1,1},
	{1,1,1,1,1,1,1,1,1,0,0,1},//1-9
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,1,1,1,1,1,1,1,1},//2-1
	{1,1,0,0,0,1,1,1,1,1,1,1},
	{1,1,1,0,0,0,1,1,1,1,1,1},
	{1,1,1,1,0,0,0,1,1,1,1,1},
	{1,1,1,1,1,0,0,0,1,1,1,1},
	{1,1,1,1,1,1,0,0,0,1,1,1},
	{1,1,1,1,1,1,1,0,0,0,1,1},
	{1,1,1,1,1,1,1,1,0,0,0,1},//2-8
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,1,1,1,1,1,1},//3-1
	{1,1,0,0,0,0,1,1,1,1,1,1},
	{1,1,1,0,0,0,0,1,1,1,1,1},
	{1,1,1,1,0,0,0,0,1,1,1,1},
	{1,1,1,1,1,0,0,0,0,1,1,1},
	{1,1,1,1,1,1,0,0,0,0,1,1},
	{1,1,1,1,1,1,1,0,0,0,0,1},//3-7
};

mt19937 engine(time(0));

void gotoxy(int x, int y){
	COORD c;
	c.X = x; c.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),c);
}

#define PARAM_COUNT 11 

double param[]={
	3.5,//eliminate count
	-4.5,//landding height
	10,//log(21-pileHeight)
	-3.2178882868487753,//rowTransitions
	-9.348695305445199,//colTransitions
	-7.899265427351652,//numHoles
	-3.3855972247263626,//wellSums
	0,//altiDiff
	0,//wellDep
	0,//wellSum
	0,//holeHeight 
};

double boardValue(const Board& board){
	int pileHeight = 0;
	int alti[12];
	memset(alti,0,sizeof(alti));
	alti[0] = alti[11] = 20; 
	int rowTransitions = 0;
	int colTransitions = 0;
	int numHoles = 0;
	int wellSums = 0;
	int holeHeight = 0;
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
	int altiDiff = altitudeDiff(alti);
	int wellDep = wellDepth(alti);
	int wellsum = wellSum(alti);
	int accu = 0;
	for(int y=20;y>=0;y--){
		int rc = bits[y]^(bits[y]>>1);
		int cc = bits[y]^(bits[y+1]);
		int nh = (~bits[y])&accu;
		int ws = bits[y]&(~(bits[y]>>1))&(bits[y]>>2);
		rowTransitions += bitcount(rc);
		colTransitions += bitcount(cc);
		numHoles += bitcount(nh);
		wellSums += bitcount(ws);
		if(nh&&!holeHeight) holeHeight = y;
		accu |= bits[y];
	}
	return 
			+param[2]*log(21-pileHeight)
			+param[3]*rowTransitions
			+param[4]*colTransitions
			+param[5]*numHoles
			+param[6]*wellSums
			+param[7]*altiDiff
			+param[8]*wellDep
			+param[9]*wellsum
			+param[10]*holeHeight;
}



double value(const Block& block, const Board& board, int* blockCount){
	Board newboard = board; 
	place(block, newboard);
	int elimnum = eliminate(newboard,block.y + Ymin[block.t][block.o], block.y + Ymax[block.t][block.o]);
	return  boardValue(newboard)+elimnum * param[0] + param[1]*block.y;
}

int placeElim(const Block& block, Board& board){
	int i, tmpX, tmpY;
	int start = block.y+Ymin[block.t][block.o];
	int end = block.y + Ymax[block.t][block.o];
	int left[21]={};
	int right[21]={};
	for (i = 0; i < 4; i++)
	{
		tmpX = block.x + blockShape[block.t][block.o][2 * i];
		tmpY = block.y + blockShape[block.t][block.o][2 * i + 1];
		board.grid[tmpY][tmpX] = 1;
		if(left[tmpY]==0||tmpX<left[tmpY]) left[tmpY] = tmpX;
		if(tmpX>right[tmpY]) right[tmpY] = tmpX;
	}
	int count = 0;
	int lineNum[4];
	for (int i = start; i <= MAPHEIGHT; i++)
	{
		if (i > end) {
			if (!count) break;
			memmove(board.grid[i - count], board.grid[i],12*sizeof(bool)*(20-end) );
			memset(board.grid[21 - count], 0, 12 * sizeof(bool)*count);
			break;
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
			lineNum[count++]=i;
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
	int code = 0;
	for(int i=0;i<count;i++){
		code = code* 40 + left[lineNum[i]]+(right[lineNum[i]]-left[lineNum[i]])*10;
	}
	//cout<<"Code"<<code<<"               "<<endl;
	insertLinesCount[code]++;
	totalCount++;
	return count;
}

bool insertLine(Board& board, int lineCode){
	if(lineCode==0) return true;
	int height = 0;
	for(int c = lineCode; c;){
		c/=40; height++;
	}
	for(int y=20-height+1;y<=20;y++){
		for(int x=1;x<=10;x++)
			if(board.grid[y][x]) return false;
	}
	memmove(board.grid[height+1],board.grid[1],(20-height)*12*sizeof(bool));
	for(int y = height;y>=1;y--){
		memcpy(board.grid[y],lineType[lineCode%40],12*sizeof(bool));
		lineCode/=40;
	}
	return true;
}

bool randomInsert(Board& board){
	if(uniform_real_distribution<double>(0,1)(engine)>0.3) return true;
	int num = uniform_int_distribution<int>(1,totalCount-insertLinesCount[0])(engine);
	int type = 1;
	int count = 0;
	while(true){
		count+=insertLinesCount[type];
		if(count>=num)
			break;
		type++;
	}
	//cout<<insertLinesCount[0]<<"            "<<endl;
	//cout<<num<<"              "<<endl;
	//cout<<type<<"           "<<endl;
	return insertLine(board,type);
}

int perform(int &elim){
	int grid[22][12]={};
	Board board(grid);
	int blockCount[7]={}; 
	int count = 0;
	elim = 0;
	while(true){
		int opChoice[7]; int ptoOC = 0;
		int maxCount = 0, minCount = blockCount[0]+10;
		for (int i = 0; i < 7; i++)
		{
			if (blockCount[i] > maxCount)
				maxCount = blockCount[i];
			if (blockCount[i] < minCount)
				minCount = blockCount[i];
		}
		for(int i = 0; i < 7; i++)
			if(blockCount[i]<minCount+2) opChoice[ptoOC++] = i;
		uniform_int_distribution<int> dis(0,ptoOC-1);
		int choice = opChoice[dis(engine)];
		//int choice = uniform_int_distribution<int>(0,6)(engine);
		blockCount[choice]++;
		
		Block moves[40];
		int size = simpleMoves(choice, board, moves);
		if(size == 0) break;
		shuffle(moves,moves+size,engine);
		Block best; double max = MINVALUE;
		for (int i = 0; i < size;i++) {
			double se = value(moves[i], board, blockCount);
			if(se>max){
				max = se;
				best = moves[i];
			}
		}
		elim+=placeElim(best,board);
		if(!randomInsert(board)) break;
		count++;
		if(count>50){
			gotoxy(0,0);
			printField(board);
			cout<<count<<"          "<<endl;
		//	system("pause");
		}
		
	}
	gotoxy(0,0);
	printField(board);
	cout<<count<<"          "<<endl;
	return count;
}

int main(){
	ofstream out("out.txt");
	CONSOLE_CURSOR_INFO info;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&info);
	info.bVisible = false;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&info);
	int best = 0;
	for(int i=0;i<10;i++){
		int elim;
		best += perform(elim);
		cout<<i<<endl;
	}
	
	best/=10;
	cout<<best<<endl;
	system("pause");
	//best=100; 
	while(true){
		for(int i=0;i<PARAM_COUNT;i++){
			param[i]=uniform_real_distribution<double>(-10,10)(engine);
		}
		int elim;
		int count = perform(elim);
		cout<<"Count: "<<count<<"          "<<endl;
		cout<<"Eliminate: "<<elim<<"          "<<endl;
		//cout<<"Ratio: "<<insertLinesCount[0]/(double)totalCount<<"          "<<endl; 
		if(count>best){
			cout<<"Epoch: "<<1<<"          "<<endl;
			for(int i=2;i<=10;i++){
				count+=perform(elim);
				cout<<"Count: "<<count/i<<"          "<<endl;
				cout<<"Eliminate: "<<elim<<"          "<<endl;
				//cout<<"Ratio: "<<insertLinesCount[0]/(double)totalCount<<"          "<<endl; 
				cout<<"Epoch: "<<i<<"          "<<endl;
			}
			if(count/10>best){
				//best = count/10;
				out<<count/10<<endl;
				//out<<"Ratio: "<<insertLinesCount[0]/(double)totalCount<<endl; 
				for(int i=0;i<PARAM_COUNT;i++) out<<param[i]<<','<<endl;
				out<<endl;
			}
			system("cls");
		}	
	}
}
