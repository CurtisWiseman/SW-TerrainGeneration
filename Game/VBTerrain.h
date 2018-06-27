#ifndef _VB_TERRAIN_H_
#define _VB_TERRAIN_H_
#include "vertex.h"
#include "ObjectList.h"

class FGrid;
class CGrid;

//=================================================================
//procedurally generate a VBGO terrain
//side be divided in to _size * _size squares (2 triangles per square)
//=================================================================

class VBTerrain : public VBGO
{
public:
	VBTerrain();
	virtual ~VBTerrain() {};

	//initialise the Veretx and Index buffers for the cube
	void init(ID3D11Device* _GD);
	void perlinify();

protected:
	//this is to allow custom versions of this which create the basic terrain and then distort it
	void loadJsonVals();
	virtual void transform();
	void loadMap();
	int flipBitmap(unsigned char* image, int bytes_per_line, int height);
	void applyColours();
	void erosion(FGrid* _input);
	void waterRise(FGrid* _input, CGrid* _c_input);

	static void writeBitmapToFile(
		BYTE* pBitmapBits,
		LONG lWidth,
		LONG lHeight,
		WORD wBitsPerPixel,
		const unsigned long& padding_size,
		LPCTSTR lpszFileName);

	static std::unique_ptr<BYTE[]> createBuffer(
		unsigned long& padding,
		BYTE* pmatrix,
		const int& width,
		const int& height);

	enum ErosionType
	{
		YMINIMUM,
		YMAXIMUM,
		YMEAN
	};
	ErosionType e_type = YMEAN;
	int smoothing_size = 4;
	int seed = 81;
	int p_octaves;
	double p_persistence;
	std::string m_filename;
	int m_width;
	int m_height;
	int num_verts;
	int neighbourArea;
	double snowHeight;
	double waterHeight;
	int waterPasses;
	//WORD* indices;
	std::vector<WORD> indices_vec;
	//myVertex* m_vertices;
	std::vector<myVertex> vertices_vec;
	//float* height_map_values = nullptr;
	FGrid* height_map_values = nullptr;
	CGrid* height_map_colours = nullptr;
};

#endif