#include "AOI_World.h"
#include <iostream>

using namespace std;


//AOI_World *AOI_World::pxWorld = nullptr;


//构造函数
AOI_World::AOI_World(int _minx, int _maxx, int _miny, int _maxy, int _xcnt, int _ycnt)
{
	this->minX = _minx;
	this->maxX = _maxx;
	this->minY = _miny;
	this->maxY = _maxy;
	this->Xcnt = _xcnt;
	this->Ycnt = _ycnt;

	this->m_grids.reserve(Xcnt * Ycnt);
	for (int i = 0; i < Xcnt * Ycnt; i++)
	{
		AOI_Grid *grid = new AOI_Grid(i);

		m_grids.push_back(grid);
	}
}



//析构函数
AOI_World::~AOI_World()
{
	//释放内存
	for (auto g : m_grids)
	{
		delete g;
	}
}


//获取周围玩家
std::list<AOI_Player*> AOI_World::GetSurPlayers(AOI_Player * _player)
{
	int row = 0;
	int col = 0;
	int index = 0;

	//所有的周围玩家返回
	list<AOI_Player*> retPlayers;

	//计算当前玩家所在格子的行
	row = (_player->GetY() - minY) / ((maxY - minY) / Ycnt);
	//计算当前玩家所在格子的列
	col = (_player->GetX() - minX) / ((maxX - minX) / Xcnt);

	pair<int, int> row_col[] = {
		make_pair(row - 1, col - 1),
		make_pair(row - 1, col),
		make_pair(row - 1, col + 1),

		make_pair(row, col - 1),
		make_pair(row, col),
		make_pair(row, col + 1),

		make_pair(row + 1, col - 1),
		make_pair(row + 1, col),
		make_pair(row + 1, col + 1)
	};

	//获取周围玩家
	for (auto it : row_col)
	{
		//判断是否合法  例如在0号格子的玩家,它的周围玩家只有右下角才有
		if (it.first < 0 || it.first >= Ycnt)
		{
			continue;
		}

		if (it.second < 0 || it.second >= Xcnt)
		{
			continue;
		}

		//计算当前格子的编号
		index = it.first * Xcnt + it.second;

		//遍历格子中的所有玩家
		for (auto p : m_grids[index]->m_players)
		{
			//周围玩家不包括自己
			if (p == _player)
			{
				continue;
			}
			retPlayers.push_back(p);
		}
	}


	return retPlayers;
}


//添加玩家
void AOI_World::AddPlayer(AOI_Player * _player)
{
	//计算当前玩家格子的编号
	int index = 0;

	index = Calculate_grid_idx(_player->GetX(), _player->GetY());

	if (index < 0 || index >= Xcnt * Ycnt)
	{
		std::cout << "格子编号非法" << std::endl;
		return;
	}

	m_grids[index]->m_players.push_back(_player);
}


//删除玩家
void AOI_World::DelPlayer(AOI_Player * _player)
{
	//计算当前玩家格子的编号
	int index = 0;

	index = Calculate_grid_idx(_player->GetX(), _player->GetY());

	//判断格子编号是否非法
	if (index < 0 || index >= Xcnt * Ycnt)
	{
		std::cout << "格子编号非法" << std::endl;
		return;
	}

	m_grids[index]->m_players.remove(_player);
}


#if 0
//获取AOI_world对象指针
AOI_World * AOI_World::GetWorld()
{
	if (nullptr == pxWorld)
	{
		pxWorld = new AOI_World(85, 410, 75, 400, 10, 20);
	}

	return pxWorld;
}
#endif



//判断玩家移动的时候是否跨格子
bool AOI_World::GridChanged(AOI_Player * _player, int _newX, int _newY)
{
	int newIndex = 0;
	int oldIndex = 0;

	//计算新的位置的格子编号
	newIndex = Calculate_grid_idx(_newX, _newY);

	//计算旧的位置的格子编号
	oldIndex = Calculate_grid_idx(_player->GetX(), _player->GetY());

	return newIndex != oldIndex;
}


//根据玩家的坐标计算玩家的格子编号
int AOI_World::Calculate_grid_idx(int x, int y)
{
	int row = 0;
	int col = 0;
	int index = 0;

	//计算玩家的行号 row = (y - minY) / ((maxY - minY) / Ycnt)
	row = (y - minY) / ((maxY - minY) / Ycnt);

	//计算玩家的列号 col = (x - minX) / ((maxX - minX) / Xcnt)
	col = (x - minX) / ((maxX - minX) / Xcnt);

	//计算玩家格子编号 index = row * Xcnt + col;
	index = row * Xcnt + col;

	return index;
}
