#include "KDTree.h"



KDTree::KDTree()
{
}


KDTree::~KDTree()
{
}

void KDTree::SetNode(int x, int y, NODE* pNode)
{
	pNode->m_x = x;
	pNode->m_y = y;
	pNode->m_pLeft = NULL;
	pNode->m_pRight = NULL;
}

NODE* KDTree::insert(int x, int y, NODE* pRoot)
{
	int iLevel = XLEVEL;
	NODE* pHead = pRoot;
	NODE* pParent = NULL;
	NODE* pTemp = NULL;

	pTemp = (NODE*)malloc(sizeof(NODE));
	SetNode(x, y, pTemp);

	if (pRoot == NULL)
	{
		pRoot = pTemp;
		pTemp = NULL;
		return pRoot;
	}

	while (1)
	{
		pParent = pHead;
		if (iLevel == XLEVEL)
		{
			if (x <= pHead->m_x) pHead = pHead->m_pLeft;
			else pHead = pHead->m_pRight;
		}
		else
		{
			if (y <= pHead->m_y) pHead = pHead->m_pLeft;
			else pHead = pHead->m_pRight;
		}
		if (pHead == NULL) break; //자식이 없으면 끝

		iLevel = (iLevel + 1) % 2;
	}

	if (iLevel == XLEVEL)
	{
		if (pTemp->m_x <= pParent->m_x) pParent->m_pLeft = pTemp;
		else pParent->m_pRight = pTemp;
	}
	else
	{
		if (pTemp->m_y <= pParent->m_y) pParent->m_pLeft = pTemp;
		else pParent->m_pRight = pTemp;
	}

	pTemp = NULL;
	pHead = NULL;
	pParent = NULL;

	return pRoot;
}

void KDTree::RegionSearch(int x1, int y1, int x2, int y2, NODE* pRoot, int iLevel)
{
	if (pRoot == NULL) return;

	if ((pRoot->m_x >= x1 && pRoot->m_x <= x2) &&
		(pRoot->m_y >= y1 && pRoot->m_y <= y2))
	{
		cout << pRoot->m_x << "," << pRoot->m_y << "\n";
		g_iCount++;
	}

	if (iLevel == XLEVEL)
	{
		if (pRoot->m_x >= x1)
			RegionSearch(x1, y1, x2, y2, pRoot->m_pLeft, (iLevel + 1) % 2);
		if (pRoot->m_x <= x2)
			RegionSearch(x1, y1, x2, y2, pRoot->m_pRight, (iLevel + 1) % 2);
	}
	else
	{
		if (pRoot->m_y >= y1)
			RegionSearch(x1, y1, x2, y2, pRoot->m_pLeft, (iLevel + 1) % 2);
		if (pRoot->m_y <= y2)
			RegionSearch(x1, y1, x2, y2, pRoot->m_pRight, (iLevel + 1) % 2);
	}

	return;
}


