#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cstdint>
#include "SimpleImage.h"
#include <vector>
#include <math.h>
#include <iostream>
using namespace std;
//Load a block of data from a file
//(this is the counterpart to SaveBlock from Assignment 1)
bool LoadBlock(const char * Path, uint16_t * Block, size_t Count) {
    if(!Block) return false;
    FILE * filePointer = NULL;
    errno_t error = fopen_s(&filePointer, Path, "rb");
    if(error) return false;
    fread(Block, sizeof(uint16_t), Count, filePointer);
    fclose(filePointer);
    return true;
}

void calcAlphas(float height, float slope, float* a)
{
	//cout<<height<<"/"<<slope<<endl;
	a[0] = (1-height) * slope;
	a[1] = height;
	a[2] = height * slope;
	if(a[0]>1.f) a[0] = 1.f;
	if(a[1]>1.f) a[1] = 1.f;
	if(a[2]>1.f) a[2] = 1.f;
}

void getColorFromTexture(SimpleImage& img, int x, int y, BYTE* c)
{
	img.getPixel(x%img.getWidth(),y%img.getHeight(),c[0],c[1],c[2]);
}

void blendTextures(BYTE** tex, float* alphas, int count, BYTE* ret)
{
	ret[0] = tex[0][0];
	ret[1] = tex[0][1];
	ret[2] = tex[0][2];

	for(int i = 0; i < count-1; i++)
	{
		ret[0] = (BYTE)(alphas[i]*tex[i+1][0]+(1-alphas[i])*ret[0]);
		ret[1] = (BYTE)(alphas[i]*tex[i+1][1]+(1-alphas[i])*ret[1]);
		ret[2] = (BYTE)(alphas[i]*tex[i+1][2]+(1-alphas[i])*ret[2]);
	}
}

int main(int argc, char * argv[]) 
{
	//simple commandline validity check
	if(argc<9 || strcmp(argv[1],"-r")       != 0 || strcmp(argv[3],"-i")        != 0
              || strcmp(argv[5],"-o_color") != 0 || strcmp(argv[7],"-o_normal") != 0) 
	{
		puts("Syntax: TextureGenerator -r <Power of 2 resolution> -i <Input height field filename> "
			 "-o_color <Output color filename> -o_normal <Output normal filename>\n");
		system("pause");
		return -1;
	}

	//parse commandline arguments
	int resolution = atoi(argv[2]);
	char* filename_in         = argv[4];
	char* filename_out_color  = argv[6];
	char* filename_out_normal = argv[8];

	//Allocate raw height field
	std::vector<uint16_t> height_raw;
	height_raw.resize(resolution*resolution);

	//Load raw height field (the output from TerrainGenerator)
	//Hint: &(height_raw[0]) obtains a pointer to the internal raw C++ array of the vector
	bool success = LoadBlock(filename_in, &height_raw[0], resolution*resolution);
		
	//print warning if write failed
	if (!success)
	{
		puts("SaveBlock failed\n");
		system("pause");
		return -1;
	}

	//convert raw heightfield to floats in range [0;1]
	std::vector<float> height(resolution*resolution);
	for (int i=0; i<resolution*resolution; i++)
	{
		height[i] = (float)height_raw[i] / (float)UINT16_MAX;
	}

	//Load input textures
	//TODO: use textures of your liking from /external/textures or 
	//      use your own textures (do not commit textures from external
	//      to your own repository)
	SimpleImage tex_flat_low  ("../../external/textures/gras15.jpg");
	SimpleImage tex_steep_low ("../../external/textures/Holz-34.jpg");
	SimpleImage tex_flat_high ("../../external/textures/rock4.jpg");
	SimpleImage tex_steep_high("../../external/textures/sand08.jpg");

	//Create output images
	SimpleImage img_color(resolution, resolution);
	SimpleImage img_normal(resolution, resolution);

	cout<<"Generating texture and normals..."<<endl;
	unsigned int count=0;
	float* alphas = new float[3];
	BYTE* ctex[4] = {new BYTE[3],new BYTE[3],new BYTE[3],new BYTE[3]};
	BYTE* c = new BYTE[3];
	//Iterate over height field
	for (int y=0; y<resolution; y++)
	{
		for (int x=0; x<resolution; x++)
		{
			// TODO: Assignment 2.1.2
			float scale = (resolution-1)/131070.f;
			float normal_x = ((x>0?height[(x-1)*resolution+y]:height[x*resolution+y])-(x<resolution-1?height[(x+1)*resolution+y]:height[x*resolution+y]))*UINT16_MAX*scale;
			float normal_y = ((y>0?height[x*resolution+y-1]:height[x*resolution+y])-(y<resolution-1?height[x*resolution+y+1]:height[x*resolution+y]))*UINT16_MAX*scale;
			float normal_z = 1.f;

			float normal_amp = sqrt(normal_x*normal_x+normal_y*normal_y+1.f);
			
			normal_x /= normal_amp;
			normal_y /= normal_amp;
			normal_z /= normal_amp;

			//cout<<"("<<normal_x<<","<<normal_y<<","<<normal_z<<")"<<endl;

			//map floats in range [-1;1] to bytes in range [0;255]
			BYTE normal_r = (BYTE)((normal_x+1.0f)/2.0f * 255.0f);
			BYTE normal_g = (BYTE)((normal_y+1.0f)/2.0f * 255.0f);
			BYTE normal_b = (BYTE)((normal_z+1.0f)/2.0f * 255.0f);

			//set all normals in normal image to default normal
			img_normal.setPixel(y, x, normal_r, normal_g, normal_b);

			// TODO: Assignment 2.1.3

			// TODO: Assignment 2.1.4
			
			calcAlphas(height[x*resolution+y],1-normal_z,alphas);

			getColorFromTexture(tex_flat_low,x,y,ctex[0]);
			getColorFromTexture(tex_steep_low,x,y,ctex[1]);
			getColorFromTexture(tex_steep_high,x,y,ctex[2]);
			getColorFromTexture(tex_flat_high,x,y,ctex[3]);
			blendTextures(ctex,alphas,4,c);

			//cout<<a3<<","<<a2<<","<<a1<<endl;

			img_color.setPixel(y, x, c[0], c[1], c[2]);

			if((count++)%((resolution-1)*(resolution-1)/10)==0) cout<<(count/((resolution-1)*(resolution-1)/100))<<"%"<<endl;
		}
	}

	delete[] alphas;
	delete[] c;
	for(int i=0;i<4;i++) delete[] ctex[i];

	//write color image
	success = img_color.save(filename_out_color);
	
	//print warning if write failed
	if (!success)
	{
		puts("img_color.save failed\n");
		system("pause");
		return -1;
	}
	
	//write normal image
	success = img_normal.save(filename_out_normal);

	//print warning if write failed
	if (!success)
	{
		puts("img_normal.save failed\n");
		system("pause");
		return -1;
	}

	cout<<"Finished."<<endl;

	return 0;
}