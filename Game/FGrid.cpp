#include "FGrid.h"

FGrid::FGrid(int x, int y)
{
	width = x;
	height = y;

	gridVector.reserve(x*y);
	for (int i = 0; i < x*y; i++)
	{
		gridVector.push_back(0.0f);
	}
}

FGrid::FGrid(int x, int y, std::vector<double> _input)
{
	width = x;
	height = y;

	gridVector.reserve(x*y);
	for (int i = 0; i < x*y; i++)
	{
		gridVector.push_back(_input[i]);
	}
}

double FGrid::getGridPoint(int x, int y)
{
	//Ensure point is within grid, else return max value
	if (x >= 0 && x < width && y >= 0 && y < height)
	{
		return gridVector[(y*width) + x];
	}
	else
	{
		return -1.0;
	}
}

void FGrid::setGridPoint(int index, double _val)
{
	gridVector[index] = _val;
}

void FGrid::setGridPoint(int x, int y, double _val)
{
	gridVector[(y*width) + x] = _val;
}