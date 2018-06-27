#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

class CGrid
{
public:
	CGrid(int width, int height);
	CGrid(int width, int height, std::vector<DirectX::SimpleMath::Color> _input);
	~CGrid() = default;

	int getWidth() { return width; }
	int getHeight() { return height; }

	DirectX::SimpleMath::Color getGridPoint(int x, int y);
	void setGridPoint(int index, DirectX::SimpleMath::Color _val);
	void setGridPoint(int x, int y, DirectX::SimpleMath::Color _val);

protected:
	std::vector<DirectX::SimpleMath::Color> gridVector;
	int width;
	int height;
};