#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <climits>
using namespace std;

//全局变量
int board[7][7] = { 0 }, tempBoard[10][7][7] = { 0 };
int blackcount = 2, whitecount = 2, tbcnt[10] = { 0 }, twcnt[10] = { 0 };//黑白棋子计数
int mycolor;
int maxDepth;
int start = clock();//用于卡时间
static int dir[24][2] = { { 1,1 },{ 0,1 },{ -1,1 },{ -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },
{ 2,0 },{ 2,1 },{ 2,2 },{ 1,2 },{ 0,2 },{ -1,2 },{ -2,2 },{ -2,1 },
{ -2,0 },{ -2,-1 },{ -2,-2 },{ -1,-2 },{ 0,-2 },{ 1,-2 },{ 2,-2 },{ 2,-1 } };//采用数组加一层循环的方式去走棋步

double weight[7][7] =
{ {1.00,0.96,0.88,0.80,0.88,0.96,1.00},


{0.96,0.95,0.84,0.75,0.84,0.95,0.96},


{0.88,0.84,0.88,0.70,0.88,0.84,0.88},


{0.80,0.75,0.68,0.62,0.68,0.75,0.80},


{0.88,0.84,0.88,0.70,0.88,0.84,0.88},


{0.96,0.95,0.84,0.75,0.84,0.95,0.96},


{1.00,0.96,0.88,0.80,0.88,0.96,1.00}, };//地势估值表

inline bool inMap(int x, int y)
{
	if (x < 0 || x > 6 || y < 0 || y > 6)
		return false;
	return true;
}
void cpy(int step)
{
	tbcnt[step] = blackcount;
	twcnt[step] = whitecount;
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
		{
			tempBoard[step][i][j] = board[i][j];//之前错点在于都没有用上step，数组必须多开一维才能够存储不同层的数据
		}
}

void recpy(int step)
{
	blackcount = tbcnt[step];
	whitecount = twcnt[step];
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
		{
			board[i][j] = tempBoard[step][i][j];
		}
}

bool oneCanMove(int x, int y)//具体的某一个棋是否可以移动
{
	for (int k = 0; k < 24; k++)
	{
		int tx = x + dir[k][0], ty = y + dir[k][1];
		if (!inMap(tx, ty)) continue;//这里之前注释掉了
		if (board[tx][ty] == 0) return 1;
	}
	return 0;
}

bool CanMove(int color)//某一个颜色的棋是否可以移动
{
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			if (board[i][j] == color && oneCanMove(i, j)) return 1;
		}
	}
	return 0;
}

bool JudgeEnd()//0则为结束，1则为未结束
{
	if (!CanMove(1) || !CanMove(-1)) return 0;
	//这里还有必要再遍历来数棋子个数吗？可否直接用blackcount和whitecount
	else if (!blackcount || !whitecount) return 0;
	return 1;
}

bool JudgeWin()//不可能平局，49个格子
{
	if (blackcount > whitecount) return 1;
	else return 0;
}


void move(int m, int n, int x, int y, int mycolor)//进入此函数前就保证输入的合法性
{
	int count = 0;
	if (mycolor == 1) blackcount += (abs(m - x) <= 1 && abs(n - y) <= 1);
	else if (mycolor == -1) whitecount += (abs(m - x) <= 1 && abs(n - y) <= 1);

	board[m][n] = mycolor * (abs(m - x) <= 1 && abs(n - y) <= 1);//这样就把两种情况合并起来了
	board[x][y] = mycolor;
	for (int k = 0; k < 8; k++)//实现同化
	{
		int tx = x + dir[k][0], ty = y + dir[k][1];
		if (!inMap(tx, ty))
			continue;
		if (board[tx][ty] == -mycolor)
		{
			board[tx][ty] = mycolor;
			count++;
		}
	}
	if (count != 0)
	{
		if (mycolor == 1)
		{
			blackcount += count;
			whitecount -= count;
		}
		else
		{
			blackcount -= count;
			whitecount += count;
		}
	}
	return;
}



double Benefit(int color)//估价函数//根据局面要灵活变化
{
	double win = 0;
	int cnt = 0, cnt1 = 0, cnt2 = 0;
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
		{
			if (board[i][j] == color)
				win += weight[i][j];
			
		}

	//srand((unsigned)time(0));
	//win += (rand()%10)/20.0;//加入一个0-10的随机数
	return win;
}


double Search(int color, int depth, double alpha, double beta)
{

	if (depth == maxDepth - 1 && (clock() - start >= CLOCKS_PER_SEC * 0.95))//卡时间
		return Benefit(color)-Benefit(-color);
	if (depth == maxDepth - 2 && (clock() - start >= CLOCKS_PER_SEC * 0.90))
		maxDepth-=2;

	if (depth == maxDepth)//这层到底要不要遍历？是depth+1吗
		return Benefit(color)-Benefit(-color);

	//判断终局，最后一个棋要下的下去，因为search会比现在走的更深，所以必须在这里也写一个终局判断
	if (!CanMove(color))
	{
		//如果另一种也不能走的话
		if (!CanMove(-color)) return Benefit(color)-Benefit(-color);
		//如果另一种棋可以走就继续搜索
		return Search(-color, depth + 1, alpha, beta);
	}

	double temp;
	if (color == mycolor)//max节点
	{
		alpha = INT_MIN;//这里一定不可以使用上一个节点的alpha否则
		for (int i = 0; i < 7; i++)
			for (int j = 0; j < 7; j++)//要遍历所有的位置找到最好的可能
			{
				if (board[i][j] == color)
				{
					for (int k = 0; k < 24; k++)
					{
						int tx = i + dir[k][0], ty = j + dir[k][1];
						if (!inMap(tx, ty)) continue;
						if (board[tx][ty] != 0) continue;
						cpy(depth);
						move(i, j, tx, ty, color);
						temp = Search(-color, depth + 1, alpha, beta);
						//unmove是由recpy实现的，因为只是影响了棋盘状态，所以把之前的棋盘copy过来就好啦
						recpy(depth);
						if (temp > alpha) alpha = temp;
						if (alpha >= beta)
						{
							return alpha;
						}

					}
				}
			}

		return alpha;
	}
	else if (color == -mycolor)//min节点
	{
		beta = INT_MAX;
		for (int i = 0; i < 7; i++)
			for (int j = 0; j < 7; j++)
			{
				if (board[i][j] == color)//这里是复制粘贴的想想有没有问题
				{
					for (int k = 0; k < 24; k++)
					{
						int tx = i + dir[k][0], ty = j + dir[k][1];
						if (!inMap(tx, ty)) continue;
						if (board[tx][ty] != 0) continue;
						cpy(depth);
						move(i, j, tx, ty, color);
						temp = Search(-color, depth + 1, alpha, beta);
						//unmove是由recpy实现的，因为只是影响了棋盘状态，所以把之前的棋盘copy过来就好啦
						recpy(depth);
						if (temp < beta) beta = temp;
						if (alpha >= beta)
						{
							return beta;

						}

					}
				}
			}
		return beta;
	}
}

//AI遍历整个棋盘找到最优的下法,同时兼有判断棋步是否合法的功能
void AIdecision(int color)
{
	double alpha = INT_MIN, beta = INT_MAX;
	double temp = 0.0;
	int bestm = 0, bestn = 0, bestx = 0, besty = 0;
	int i, j, k;
	for (i = 0; i < 7; i++)
		for (j = 0; j < 7; j++)
		{
			if (board[i][j] != color) continue;
			for (k = 0; k < 24; k++)
			{
				int tx = i + dir[k][0], ty = j + dir[k][1];
				if (!inMap(tx, ty)) continue;
				if (board[tx][ty] == color || board[tx][ty] == -color) continue;

				cpy(0);
				move(i, j, tx, ty, color);
				temp = Search(-color, 1, alpha, beta);//这里返回的是min节点的值
				recpy(0);
				if (temp > alpha)
				{
					alpha = temp;
					bestm = i;
					bestn = j;
					bestx = tx;
					besty = ty;
				}
			}
		}
	//if(board[bestx][besty]!=0) besty++;
	move(bestm, bestn, bestx, besty, color);
	cout << bestn << " " << bestm << " " << besty << " " << bestx;//问题出在这里了，
	return;
}

int main()
{//我还是采用0-6的方式，因为botzone上是这样的，如果玩家输入的话就--；
	//初始化棋盘

	board[0][0] = board[6][6] = 1;//black
	board[0][6] = board[6][0] = -1;//white

	int round = 0;
	cin >> round;


	int m, n, x, y;
	mycolor = -1;//如果输入的是-1则转变为黑方，否则为白方
	for (int i = 0; i < round - 1; i++)// 根据这些输入输出逐渐恢复状态到当前回合
	{
		cin >> n >> m >> y >> x;//注意botzone上面横纵坐标是相反的
		if (x >= 0)
			move(m, n, x, y, -mycolor);// 模拟对方落子，反正永远是黑棋（1）先走
		else mycolor = 1;
		cin >> n >> m >> y >> x;
		if (x >= 0)
			move(m, n, x, y, mycolor);// 模拟我方落子
	}

	//本回合输入
	cin >> n >> m >> y >> x;
	if (x >= 0)
		move(m, n, x, y, -mycolor);// 模拟对方落子
	else mycolor = 1;

	maxDepth = 6;
	//44个子之后用8步搜索
	if (blackcount + whitecount >= 44)
		maxDepth = 10;



	if (mycolor == 1)//我拿黑子
	{
		AIdecision(1);

	}
	else if (mycolor == -1)//我拿白子
	{
		AIdecision(-1);
	}
	return 0;
}
