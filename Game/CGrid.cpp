#include "CGrid.h"

CGrid::CGrid(int x, int y)
{
	width = x;
	height = y;

	gridVector.reserve(x*y);
	for (int i = 0; i < x*y; i++)
	{
		gridVector.push_back(DirectX::SimpleMath::Color(0.0f,0.0f,0.0f,1.0f));
	}
}

CGrid::CGrid(int x, int y, std::vector<DirectX::SimpleMath::Color> _input)
{
	width = x;
	height = y;

	gridVector.reserve(x*y);
	for (int i = 0; i < x*y; i++)
	{
		gridVector.push_back(_input[i]);
	}
}

DirectX::SimpleMath::Color CGrid::getGridPoint(int x, int y)
{
	//Ensure point is within grid, else return max value
	if (x >= 0 && x < width && y >= 0 && y < height)
	{
		return gridVector[(y*width) + x];
	}
	else
	{
		return DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void CGrid::setGridPoint(int index, DirectX::SimpleMath::Color _val)
{
	gridVector[index] = _val;
}

void CGrid::setGridPoint(int x, int y, DirectX::SimpleMath::Color _val)
{
	gridVector[(y*width) + x] = _val;
}