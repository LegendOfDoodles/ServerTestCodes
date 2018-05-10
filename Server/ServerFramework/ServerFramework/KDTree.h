#pragma once

#include <Windows.h>
#include <iostream>

#define Conver_Unit_To_InG(val) val*1000

#define XLEVEL 0
#define YLEVEL 1

using namespace std;

NODE g_root = { 5000,2500,NULL,NULL,NULL };
int g_iCount = 0;

struct NODE
{
	int m_x;
	int m_y;
	struct NODE* m_pLeft;
	struct NODE* m_pRight;
	struct NODE* m_pParent;
};

class KDTree
{
public:
	void SetNode(int x, int y, NODE* pNode);
	NODE* insert(int x, int y, NODE* pRoot);
	void RegionSearch(int x1, int y1, int x2, int y2, NODE* pRoot, int iLevel);
	//void InitMap(FILE* pFile, char* pc)

public:
	KDTree();
	~KDTree();
};

