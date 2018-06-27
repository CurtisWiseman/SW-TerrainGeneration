#include "VBTerrain.h"
#include <string>
#include <iostream>
#include <fstream>
#include "Perlin.h"
#include "FGrid.h"
#include "CGrid.h"
#include <jsoncons/json.hpp>

VBTerrain::VBTerrain()
{
	loadJsonVals();
}

void VBTerrain::loadJsonVals()
{
	using jsoncons::json;
	std::ifstream
		is("..\\Assets\\inputVars.json");
	json vals;
	is >> vals;
	for (const auto& type : vals.members())
	{
		const auto& name = type.name();
		const auto& data = type.value();

		smoothing_size = data["smoothingSize"].as_int();
		std::string j_e_type = data["eType"].as_string();
		m_filename = data["filename"].as_string();
		m_width = data["width"].as_int();
		m_height = data["height"].as_int();
		p_octaves = data["octaves"].as_int();
		p_persistence = data["persistence"].as_double();
		neighbourArea = data["colourArea"].as_int();
		snowHeight = data["snowHeight"].as_double();
		waterHeight = data["waterHeight"].as_double();
		waterPasses = data["waterPasses"].as_int();

		if (j_e_type == "SMOOTH")
		{
			e_type = YMEAN;
		}
		else if (j_e_type == "EROSION")
		{
			e_type = YMINIMUM;
		}
		else if (j_e_type == "DILATION")
		{
			e_type = YMAXIMUM;
		}
	}
}

void VBTerrain::init(ID3D11Device* GD)
{
	perlinify();
	// this is most certinaly not the most efficent way of doing most of this
	//but it does give you a very clear idea of what is actually going on
	loadMap();
	//carry out some kind of transform on these vertices to make this object more 
	applyColours();
	transform();

	//calculate the normals for the basic lighting in the base shader
	for (int i = 0; i<m_numPrims; i++)
	{
		long int V1 = 3 * i;
		long int V2 = 3 * i + 1;
		long int V3 = 3 * i + 2;

		//build normals
		Vector3 norm;
		Vector3 vec1 = vertices_vec[V1].Pos - vertices_vec[V2].Pos;
		Vector3 vec2 = vertices_vec[V3].Pos - vertices_vec[V2].Pos;
		norm = vec1.Cross(vec2);
		norm.Normalize();

		vertices_vec[V1].Norm = norm;
		vertices_vec[V2].Norm = norm;
		vertices_vec[V3].Norm = norm;
	}

	int* indicesIB = new int[indices_vec.size()];
	for (int iter = 0; iter < indices_vec.size(); iter++)
	{
		indicesIB[iter] = indices_vec[iter];
	}
	myVertex* verticesVB = new myVertex[vertices_vec.size()];
	for (int vter = 0; vter < vertices_vec.size(); vter++)
	{
		verticesVB[vter] = vertices_vec[vter];
	}


	BuildIB(GD, indicesIB);
	BuildVB(GD, num_verts, verticesVB);

	delete height_map_values;
	delete height_map_colours;
	//delete[] indices;    //this is no longer needed as this is now in the index Buffer
	delete[] indicesIB;
	//delete[] m_vertices; //this is no longer needed as this is now in the Vertex Buffer
	delete[] verticesVB;
	//delete[] height_map_values;
	//m_vertices = nullptr;
}

void VBTerrain::transform()
{
	std::cout << "Height-transforming " << (6 * m_width * m_height) << " vertices" << std::endl;
	for (int i = 0; i < 6 * m_width * m_height; i++)
	{
		//vertices_vec[i].Pos.y = GetHeightMapValue(vertices_vec[i].Pos.x, vertices_vec[i].Pos.z);
		vertices_vec[i].Pos.y = height_map_values->getGridPoint(vertices_vec[i].Pos.x, vertices_vec[i].Pos.z);
		vertices_vec[i].Color = height_map_colours->getGridPoint(vertices_vec[i].Pos.x, vertices_vec[i].Pos.z);
	}
}

void VBTerrain::applyColours()
{
	std::cout << "Colour-applying " << (6 * m_width * m_height) << " vertices" << std::endl;
	Color black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	Color blank = Color(0.0f, 0.0f, 0.0f, 0.0f);
	Color white = Color(1.0f, 1.0f, 1.0f, 1.0f);
	Color red = Color(1.0f, 0.0f, 0.0f, 1.0f);
	Color green = Color(0.0f, 1.0f, 0.0f, 1.0f);
	Color blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
	Color jesse = Color(1.0f, 1.0f, 0.0f, 1.0f);
	Color cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
	Color magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);
	Color grey = Color(0.4f, 0.4f, 0.5f, 1.0f);

	
	/*for (int i = 0; i < 6 * m_width * m_height; i++)
	{
		float vHeight = vertices_vec[i].Pos.y;

		float lerpDeltaHeight = vHeight / 255.0f;
		if (lerpDeltaHeight > 1.0f) lerpDeltaHeight = 1.0f;
		if (lerpDeltaHeight < 0.0f) lerpDeltaHeight = 0.0f;

		float lerpDeltaSnow = (vHeight - 200.0f) / 35.0f;
		if (lerpDeltaSnow > 1.0f) lerpDeltaSnow = 1.0f;
		if (lerpDeltaSnow < 0.0f) lerpDeltaSnow = 0.0f;

		float lerpDeltaWater = (vHeight - 30.0f) / 50.0f;
		if (lerpDeltaWater > 1.0f) lerpDeltaWater = 1.0f;
		if (lerpDeltaWater < 0.0f) lerpDeltaWater = 0.0f;


		Color snowLerp = XMVectorLerp(blank, white, lerpDeltaSnow);
		Color waterLerp = XMVectorLerp(blue, blank, lerpDeltaWater);
		Color totalLerp = grey + snowLerp + waterLerp;

		vertices_vec[i].Color = totalLerp;
	}*/

	double y = 256.0f;
	double* neighbours = new double[neighbourArea*neighbourArea];
	for (int i = 0; i < height_map_values->getWidth(); i++)
	{
		for (int j = 0; j < height_map_values->getHeight(); j++)
		{
			for (int si = 0; si < neighbourArea; si++)
			{
				for (int sj = 0; sj < neighbourArea; sj++)
				{
					neighbours[si*neighbourArea + sj] = 
						height_map_values->getGridPoint
						(i + (si - (neighbourArea / 2)), j + (sj - (neighbourArea / 2)));
				}
			}

			y = 0.0f;
			for (int k = 0; k < neighbourArea*neighbourArea; k++)
			{
				y += neighbours[k];
			}
			y /= neighbourArea*neighbourArea;

			if (height_map_values->getGridPoint(i, j) > (y * snowHeight))		//Snow
			{
				height_map_colours->setGridPoint(i, j, white);
			}
			else if (height_map_values->getGridPoint(i, j) < (y * waterHeight)) //Water
			{
				height_map_colours->setGridPoint(i, j, cyan);
			}
			else															//Stone
			{
				height_map_colours->setGridPoint(i, j, grey);
			}
		}
	}

	delete[] neighbours;

	waterRise(height_map_values, height_map_colours);
}

void VBTerrain::loadMap()
{

	FILE* filePtr;
	int error = 0;
	const int channels = 3;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j;
	unsigned char* bitmapImage;
	unsigned char height;

	const char* c_filename = m_filename.c_str();

	//Open up the file
	std::cout << "Opening file " << m_filename.c_str() << std::endl;
	error = fopen_s(&filePtr, c_filename, "rb");
	if (error != 0)
	{
		cout << "Error opening file" << endl;
	}

	//Read the file header
	std::cout << "Reading file header" << std::endl;
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1)
	{
		cout << "Error reading file header" << endl;
	}

	//Read the info header
	std::cout << "Reading info header" << std::endl;
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		cout << "Error reading info header" << endl;
	}

	m_width = bitmapInfoHeader.biWidth;
	m_height = bitmapInfoHeader.biHeight;

	imageSize = m_width * m_height * channels;
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		cout << "Error creating bitmap image buffer" << endl;
	}

	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	//Read in the actual data
	std::cout << "Streaming data from file" << std::endl;
	count = fread(bitmapImage, sizeof(char), imageSize, filePtr);
	if (count != imageSize)
	{
		cout << "Error streaming data from file" << endl;
	}

	flipBitmap(bitmapImage, m_width * channels, m_height);

	//close the file
	std::cout << "Closing file" << std::endl;
	error = fclose(filePtr);
	if (error != 0)
	{
		cout << "Error closing file" << endl;
	}

	//calculate number of vertices and primatives
	num_verts = 6 * m_width * m_height;
	m_numPrims = num_verts / 3;
	vertices_vec.reserve(num_verts);
	indices_vec.reserve(num_verts);

	myVertex temp;
	temp.texCoord = Vector2::One;
	//Simple initialisation
	for (int q = 0; q<num_verts; q++)
	{
		indices_vec.push_back(q);
		vertices_vec.push_back(temp);
	}

	//Create triangles
	int vert = 0;
	/*for (i = -(m_height - 1) / 2; i < (m_height - 1) / 2; i++)*/
	std::cout << "Creating vertices for " << m_filename << std::endl;
	for (i = 0; i < m_height -1; i++)
	{
		/*for (j = -(m_width - 1) / 2; j < (m_width - 1) / 2; j++)*/
		for (j = 0; j < m_width -1; j++)
		{
			double x = (double)j;
			double xPLUS1 = (double)(j + 1);
			double y = 0;
			double z = (double)i;
			double zPLUS1 = (double)(i + 1);
			Color black = Color(0.0f, 0.0f, 0.0f, 1.0f);
			Color white = Color(1.0f, 1.0f, 1.0f, 1.0f);
			Color red = Color(1.0f, 0.0f, 0.0f, 1.0f);
			Color green = Color(0.0f, 1.0f, 0.0f, 1.0f);
			Color blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
			Color jesse = Color(1.0f, 1.0f, 0.0f, 1.0f);
			Color cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
			Color magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);
			/*float lerpDeltaI = ((float)i + (0.5 * (m_height - 1.0f))) / m_height;*/
			double lerpDeltaI = (double)i / m_height;
			/*float lerpDeltaJ = ((float)j + (0.5 * (m_width - 1.0f))) / (0.5 * (m_width - 1.0f));*/
			double lerpDeltaJ = (double)j / m_width;

			Color redLerp = XMVectorLerp(black, red, lerpDeltaJ);
			Color blueLerp = XMVectorLerp(black, blue, lerpDeltaI);
			Color totalLerp = redLerp + blueLerp;

			vertices_vec[vert].Color = jesse;
			vertices_vec[vert++].Pos = Vector3(x, y, z);
			vertices_vec[vert].Color = jesse;
			vertices_vec[vert++].Pos = Vector3(x, y, zPLUS1);
			vertices_vec[vert].Color = jesse;
			vertices_vec[vert++].Pos = Vector3(xPLUS1, y, z);

			vertices_vec[vert].Color = jesse;
			vertices_vec[vert++].Pos = Vector3(xPLUS1, y, z);
			vertices_vec[vert].Color = jesse;
			vertices_vec[vert++].Pos = Vector3(x, y, zPLUS1);
			vertices_vec[vert].Color = jesse;
			vertices_vec[vert++].Pos = Vector3(xPLUS1, y, zPLUS1);
		}
	}

	//Assign bitmap values to heightmap values
	height_map_values = new FGrid(m_width, m_height);
	height_map_colours = new CGrid(m_width, m_height);
	std::cout << "Setting " << (m_width*m_height) <<" heightmap values from streamed file data" << std::endl;
	for (i = 0; i < m_width*m_height; i++)
	{
		height_map_values->setGridPoint(i, bitmapImage[(i*channels)]);
		height_map_colours->setGridPoint(i, Color(1.0f, 1.0f, 0.0f, 1.0f));
	}

	erosion(height_map_values);

	for (int i = 0; i < 6 * m_width*m_height; i++)
	{
		vertices_vec[i].Pos.y = 0;
	}
	delete[] bitmapImage;
}

int VBTerrain::flipBitmap(unsigned char* image, int bytes_per_line, int height)
{
	unsigned char* buffer;
	int index;

	if (!(buffer = (unsigned char*)malloc(bytes_per_line*height)))
	{
		return 0;
	}

	memcpy(buffer, image, bytes_per_line*height);

	for (index = 0; index < height; index++)
	{
		memcpy(&image[((height - 1) - index)*bytes_per_line], &buffer[index*bytes_per_line], bytes_per_line);
	}

	free(buffer);
	return 1;
}

void VBTerrain::perlinify()
{
	const int channels = 3;
	BYTE* bitmapImage = new BYTE[channels* m_width*m_height];
	unsigned long padding = 0;

	//Perlin noise bit
	Perlin pn(seed);
	
	unsigned int kk = 0;
	//Assign greyscale values to every pixel via Perlin
	for (unsigned int i = 0; i < m_height; i++)
	{
		for (unsigned int j = 0; j < m_width; j++)
		{
			double x = (double)j / (double)m_width;
			double y = (double)i / (double)m_height;

			double n = 20 * pn.octaveNoise(x, y, 0.8, p_octaves, p_persistence);
			n = n - floor(n);

			bitmapImage[kk] = floor(255 * n);
			bitmapImage[kk + 1] = floor(255 * n);
			bitmapImage[kk + 2] = floor(255 * n);
			kk += 3;
		}
	}

	std::cout << "Building and writing bitmap" << std::endl;
	std::unique_ptr<BYTE[]> builtBuff = std::move(createBuffer(padding, bitmapImage, m_width, m_height));

	writeBitmapToFile(builtBuff.get(), m_width, m_height, 24, padding, m_filename.c_str());
	std::cout << "Written bitmap to file" << std::endl;

	delete[] bitmapImage;
}

void VBTerrain::writeBitmapToFile(
	BYTE* pBitmapBits,
	LONG lWidth,
	LONG lHeight,
	WORD wBitsPerPixel,
	const unsigned long& padding_size,
	LPCTSTR lpszFileName)
{
	unsigned long headers_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	unsigned long pixel_data_size = lHeight * ((lWidth * (wBitsPerPixel / 8)) + padding_size);

	BITMAPINFOHEADER iHead = { 0 };
	iHead.biSize = sizeof(BITMAPINFOHEADER);
	iHead.biBitCount = wBitsPerPixel;
	iHead.biClrImportant = 0;
	iHead.biClrUsed = 0;
	iHead.biCompression = BI_RGB;
	iHead.biHeight = lHeight;
	iHead.biWidth = lWidth;
	iHead.biPlanes = 1;
	iHead.biSizeImage = pixel_data_size;

	BITMAPFILEHEADER fHead = { 0 };
	fHead.bfType = 'MB';
	fHead.bfOffBits = headers_size;
	fHead.bfSize = headers_size + pixel_data_size;

	HANDLE hFile = CreateFile(
		lpszFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (!hFile)
	{
		return;
	}

	DWORD dwWritten = 0;
	WriteFile(hFile, &fHead,
		sizeof(fHead), &dwWritten, NULL);
	WriteFile(hFile, &iHead,
		sizeof(iHead), &dwWritten, NULL);
	WriteFile(hFile, pBitmapBits,
		iHead.biSizeImage, &dwWritten, NULL);

	CloseHandle(hFile);
}

std::unique_ptr<BYTE[]> VBTerrain::createBuffer(
	unsigned long& padding,
	BYTE* pmatrix,
	const int& width,
	const int& height)
{
	padding = (4 - ((width * 3) % 4)) % 4;
	int scanlinebytes = width * 3;
	int total_scanlinebytes = scanlinebytes + padding;
	long newsize = height * total_scanlinebytes;
	std::unique_ptr<BYTE[]> newbuf(new BYTE[newsize]);

	// Fill new array with original buffer, pad remaining with zeros
	std::fill(&newbuf[0], &newbuf[newsize], 0);
	long bufpos = 0;
	long newpos = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < 3 * width; x += 3)
		{
			// Find current positions in each buffer
			bufpos = y * 3 * width + (3 * width - x);
			newpos = (height - y - 1) * total_scanlinebytes + x;

			// Flip RGB components
			newbuf[newpos] = pmatrix[bufpos + 2];
			newbuf[newpos + 1] = pmatrix[bufpos + 1];
			newbuf[newpos + 2] = pmatrix[bufpos];
		}
	}

	return newbuf;
}

void VBTerrain::erosion(FGrid* _input)
{
	FGrid* erosion = new FGrid(_input->getWidth(), _input->getHeight());

	double y = 256.0f;
	double* neighbours = new double[smoothing_size*smoothing_size];
	for (int i = 0; i < _input->getWidth(); i++)
	{
		for (int j = 0; j < _input->getHeight(); j++)
		{
			for (int si = 0; si < smoothing_size; si++)
			{
				for (int sj = 0; sj < smoothing_size; sj++)
				{
					neighbours[si*smoothing_size + sj] = _input->getGridPoint(i + (si - (smoothing_size / 2)), j + (sj - (smoothing_size / 2)));
				}
			}

			switch (e_type)
			{
			case YMINIMUM:
				y = 256.0f;
				for (int k = 0; k < smoothing_size*smoothing_size; k++)
				{
					if (neighbours[k] == -1)
					{
						neighbours[k] = 255;
					}
					if (y > neighbours[k])
					{
						y = neighbours[k];
					}
				}
				break;
			case YMAXIMUM:
				y = -1.0f;
				for (int k = 0; k < smoothing_size*smoothing_size; k++)
				{
					if (neighbours[k] == -1)
					{
						neighbours[k] = 0;
					}
					if (y < neighbours[k])
					{
						y = neighbours[k];
					}
				}
				break;
			case YMEAN:
				y = 0.0f;
				
				for (int k = 0; k < smoothing_size*smoothing_size; k++)
				{
					if (neighbours[k] == -1)
					{
						neighbours[k] = 128;
					}
					y += neighbours[k];
				}
				y /= smoothing_size*smoothing_size;
				break;
			}
			
			erosion->setGridPoint(i, j, y);
		}
	}

	*_input = *erosion;
	delete erosion;
	delete[] neighbours;
}

void VBTerrain::waterRise(FGrid* _input, CGrid* _c_input)
{
	std::cout << "Conforming water levels" << std::endl;
	double y = 256.0f;
	double neighbours[3*3];
	Color c_neighbours[3 * 3];
	int changes = 0;
	int passes = 0;

	while (passes < waterPasses)
	{
		cout << "Pass " << passes++ << std::endl;
		cout << "Last pass: " << changes << " changes" << std::endl;
		changes = 0;
		for (int i = 0; i < _input->getWidth(); i++)
		{
			for (int j = 0; j < _input->getHeight(); j++)
			{
				for (int si = 0; si < 3; si++)
				{
					for (int sj = 0; sj < 3; sj++)
					{
						neighbours[si * 3 + sj] = _input->getGridPoint(i + (si - 1), j + (sj - 1));
						c_neighbours[si * 3 + sj] = _c_input->getGridPoint(i + (si - 1), j + (sj - 1));
					}
				}

				y = _input->getGridPoint(i, j);

				if (_c_input->getGridPoint(i, j) == Color(0.0f, 1.0f, 1.0f, 1.0f))	//If this vertex is water
				{
					for (int k = 0; k < 3 * 3; k++)
					{
						if (y < neighbours[k] && c_neighbours[k] == Color(0.0f, 1.0f, 1.0f, 1.0f)) //If lower than neighbour and neighbour IS water
						{
							y = neighbours[k];													//New height = neighbour height
							_input->setGridPoint(i, j, y);									//Up to water level
							changes++;
						}
					}

					
				}
				else																//If this vertex is NOT water
				{
					for (int k = 0; k < 3 * 3; k++)
					{
						if (y < neighbours[k] && c_neighbours[k] == Color(0.0f, 1.0f, 1.0f, 1.0f))		//If lower than neighbour and neighbour IS water
						{
							y = neighbours[k];													//New height = neighbour height
							_input->setGridPoint(i, j, y);												//Up to water level
							_c_input->setGridPoint(i, j, Color(0.0f, 1.0f, 1.0f, 1.0f));				//I am become water
							changes++;
						}
					}

					
				}
			}
		}
	}
}