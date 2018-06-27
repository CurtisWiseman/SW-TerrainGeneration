#include <vector>


class FGrid
{
public:
	FGrid(int width, int height);
	FGrid(int width, int height, std::vector<double> _input);
	~FGrid() = default;

	int getWidth() { return width; }
	int getHeight() { return height; }

	double getGridPoint(int x, int y);
	void setGridPoint(int index, double _val);
	void setGridPoint(int x, int y, double _val);

protected:
	std::vector<double> gridVector;
	int width;
	int height;
};