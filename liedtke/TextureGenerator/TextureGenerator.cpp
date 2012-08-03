#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <cstdint>
#include "SimpleImage.h"
#include <vector>
#include <ppl.h>

#define saturate(x) ((x >1)?1:(x<0)?0:x)

struct Vector{
	float x;
	float y;
	float z;
	Vector(float x, float y, float z){
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector normalized(){
		float length = std::sqrt(x*x+y*y+z*z);
		return Vector(x/length, y/length, z/length);
	}
};

struct Color{
	BYTE r;
	BYTE g;
	BYTE b;
	Color(BYTE r, BYTE g, BYTE b){
		this->r = r;
		this->g = g;
		this->b = b;
	}

	Color operator*(int i){
		Color rgb(r*i, g*i, b*i);
		return rgb;
	}
	Color operator*(float i){
		Color rgb((BYTE)(r*i), (BYTE)(g*i), (BYTE)(b*i));
		return rgb;
	}

	Color operator+(Color c){
		Color rgb(r+c.r, g+c.g, b+c.b);
	return rgb;
	}
};

int resolution;
std::vector<float> height;

inline float getHeightOfField(int x, int y){
	if(x < 0)
		x = 0;
	else if(x >= resolution)
		x = resolution-1;
	if(y < 0)
		y = 0;
	else if(y >= resolution)
		y = resolution-1;
	return height[y*resolution + x];
}

void calcAlphas(float height, float slope, float& alpha1, float& alpha2, float& alpha3) { 
	//alpha1 = (1-height) *acos(1-(slope*6))*1.5f; 
	alpha1 = saturate(acos(1-(1-height) *slope*8)*1.25f); 
	alpha2 = height; 
	alpha3 = saturate(acos(1-height*slope*8)*0.66f);
}

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

int main(int argc, char * argv[]) 
{
	puts("Texture/normalmap started");
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
	resolution = atoi(argv[2]);
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
		puts("LoadBlock failed\n");
		puts(filename_in);
		system("pause");
		return -1;
	}
	height = std::vector<float>(resolution*resolution);
	//convert raw heightfield to floats in range [0;1]
	Concurrency::parallel_for(size_t(0), size_t(resolution*resolution), [&](size_t(i))
	//for (int i=0; i<resolution*resolution; i++)
	{
		height[i] = (float)height_raw[i] / (float)UINT16_MAX;
	});
	//writeArray(height);

	//Load input textures
	//TODO: use textures of your liking from /external/textures or 
	//      use your own textures (do not commit textures from external
	//      to your own repository)
 	SimpleImage tex_flat_low  ("../../external/textures/gras15.jpg");
	SimpleImage tex_steep_low ("../../external/textures/Holz-34.jpg");
	SimpleImage tex_flat_high ("../../external/textures/ground02.jpg");
	SimpleImage tex_steep_high("../../external/textures/rock5.jpg");

	//Create output images
	SimpleImage img_color(resolution, resolution);
	SimpleImage img_normal(resolution, resolution);
	float scale = (float)(resolution-1.f)/(2*(1 << (sizeof(UINT16)*8))-1.f); //identisch mit den beiden unteren Werten
	//float scale2 = (float)(resolution-1.f)/((1 << (sizeof(UINT16)*8+1))-2.f);
	//float scale = (1.f/(std::powf(2,16)-1.f))/(2/((resolution-1.f)));
	scale *= UINT16_MAX;
	scale /= 12.f;
	//Iterate over height field
	Concurrency::parallel_for(size_t(0), size_t(resolution), [&](size_t y)
	{
		for (int x=0; x<resolution; x++)
		{
			//Vector n = Vector(-(resolution*(getHeightOfField(x+1,y) - getHeightOfField(x-1,y))/2.f), -(resolution*(getHeightOfField(x,y+1) - getHeightOfField(x,y-1))/2.f), 1.f);
			Vector n(scale*(getHeightOfField(x-1,y) - getHeightOfField(x+1,y)), (scale*(getHeightOfField(x,y-1) - getHeightOfField(x,y+1))), 1.f);
			//default normal pointing upward
			Vector normalized = n.normalized();
			//map floats in range [-1;1] to bytes in range [0;255]
			Color normal((BYTE)(((normalized.x+1.0f))/2.0f * 255.0f),(BYTE)(((normalized.y+1.0f))/2.0f * 255.0f),(BYTE)(((normalized.z+1.0f))/2.0f * 255.0f));

			//set all normals in normal image to default normal
			img_normal.setPixel(x, y, normal.r, normal.g, normal.b);


			//um die Alphas habe ich mich noch nicht gekümmert im Assigment 4 //ToDo
			float alpha1;//Erde
			float alpha2;//Geröll
			float alpha3;//Felsen
			//Gras ist = 1
			calcAlphas(getHeightOfField(x,y), 1-normalized.z, alpha1, alpha2, alpha3);

			//for a start, we simply use one of our textures for the color image
			Color lowFlat(0,0,0); //c0
			int x_seemless = x % tex_flat_low.getWidth();  //seemless texturing
			int y_seemless = y % tex_flat_low.getHeight(); //seemless texturing
			tex_flat_low.getPixel(x_seemless, y_seemless, lowFlat.r, lowFlat.g, lowFlat.b);
			//Nun erde drüber blenden
			Color lowSteep(0,0,0);//c1
			x_seemless = x %tex_steep_low.getWidth();
			y_seemless = y %tex_steep_low.getHeight();
			tex_steep_low.getPixel(x_seemless, y_seemless, lowSteep.r,lowSteep.g,lowSteep.b);
			Color highFlat(0,0,0);//c2
			x_seemless = x %tex_flat_high.getWidth();
			y_seemless = y %tex_flat_high.getHeight();
			tex_flat_high.getPixel(x_seemless, y_seemless, highFlat.r, highFlat.g, highFlat.b);
			Color highSteep(0,0,0);//c3
			x_seemless = x %tex_steep_high.getWidth();
			y_seemless = y %tex_steep_high.getHeight();
			tex_steep_high.getPixel(x_seemless, y_seemless, highSteep.r,highSteep.g,highSteep.b);

			//blending
			//a3*c3 + (a2*c2 + ((1-a1)*c0+a1*c1)*(1-a2)*(1-a3)
			Color C3 = highSteep*alpha3 + (highFlat*alpha2 + (lowFlat*(1-alpha1)+lowSteep*alpha1)*(1-alpha2))*(1-alpha3);


			img_color.setPixel(x, y, C3.r, C3.g, C3.b);
		}
	});

	//write color image
	success = img_color.save(filename_out_color);
	std::cout << "saved image to " << filename_out_color << std::endl;
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
	puts("Texture/Diffusemap generated");
	return 0;
}

