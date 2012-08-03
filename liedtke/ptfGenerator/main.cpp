#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cstdint>
#include <vector>

struct PtfHeader { 
	int16_t magicNumber; // Must be 0x00DA 
	int16_t version;     // Must be 1 
	int32_t heightSize;  // Height data size 
	int32_t colorSize;   // Color data size 
	int32_t normalSize;  // Normal data size 
}; // Sizes are always in bytes


//Reads the complete file given by "path" byte-wise into "data"
//
// Assignment 2.2.2
//TODO: you have to implement this method
void LoadFile(const char * filename, std::vector<uint8_t>& data)
{
	FILE* filePointer = NULL;
	errno_t error = fopen_s(&filePointer,filename,"rb");
	if(error != 0)
	{
		std::string errorstr("Can't open ");
		errorstr += filename;
		errorstr += " Code: ";
		char buff[100];
		strerror_s(buff, 100,error);
		errorstr += buff;
		puts(errorstr.c_str());
		exit(-1);
	}
	fseek(filePointer, 0, SEEK_END);
	size_t fileSize = ftell(filePointer);
	data.resize(fileSize);
	fseek(filePointer, 0, SEEK_SET);
	fread(&data[0], sizeof(unsigned char), fileSize, filePointer);
	fclose(filePointer);
}


//Writes a ptf-file "filename" from a given heightfield "height", 
//color map "color" and normal map "normal"
//
// Assignment 2.2.3
//TODO: you have to implement this method
void WritePtf   (char* filename, std::vector<uint8_t>& height, 
	std::vector<uint8_t>& color, std::vector<uint8_t>& normal)
{
	PtfHeader header;
	header.magicNumber = 0x00DA;
	header.version     = 1;
	header.colorSize = color.size()*sizeof(uint8_t); //= *1
	header.heightSize = height.size()*sizeof(uint8_t);
	header.normalSize = normal.size()*sizeof(uint8_t);

	FILE* filePointer = NULL;
	errno_t error = fopen_s(&filePointer, filename, "wb");

	if(error != 0)
	{
		std::string errorstr("WriePTF: Can't open ");
		errorstr += filename;
		errorstr += " Code: ";
		char buff[100];
		strerror_s(buff, 100,error);
		errorstr += buff;
		puts(errorstr.c_str());
		exit(-1);
	}
	fwrite(&header, 1, sizeof(PtfHeader), filePointer);
	fwrite(&height[0], sizeof(uint8_t),header.heightSize, filePointer);
	fwrite(&color[0], sizeof(uint8_t),header.colorSize, filePointer);
	fwrite(&normal[0], sizeof(uint8_t),header.normalSize, filePointer);

	fclose(filePointer);
}


//Reduces the resultion of the height field "height" by a factor of "factor".
//The size of "height" is changed by this method.
void ResampleHeightfield(std::vector<uint8_t>& height, int factor);

int main(int argc, char * argv[]) 
{
	//simple commandline validity check
	if(argc<9 || strcmp(argv[1],"-i_height") != 0 || strcmp(argv[3],"-i_color")  != 0
              || strcmp(argv[5],"-i_normal") != 0 || strcmp(argv[7],"-o")        != 0) 
	{
		puts("Syntax: PtfGenerator -i_height <Input height field filename> "
			                      "-i_color <Input color filename> "
								  "-i_normal <Input normal filename> "
								  "-o <Output ptf filename>\n");
		system("pause");
		return -1;
	}

	//parse commandline arguments
	char* filename_in_height  = argv[2];
	char* filename_in_color   = argv[4];
	char* filename_in_normal  = argv[6];
	char* filename_out_ptf    = argv[8];
	
	//load input files as raw byte arrays
	std::vector<uint8_t> height;
	std::vector<uint8_t> color;
	std::vector<uint8_t> normal;

	LoadFile(filename_in_height, height);
	LoadFile(filename_in_color , color );
	LoadFile(filename_in_normal, normal);

	//reduce heightfield resolution
	ResampleHeightfield(height, 4);

	//write the output ptf file
	WritePtf(filename_out_ptf, height, color, normal);    

	return 0;
}


void ResampleHeightfield(std::vector<uint8_t>& height, int factor)
{
	int resolution = (int)std::sqrt((double)(height.size()/2));
	if ( resolution < 2*factor ) return;
	int resolution_new = resolution / factor;
	
	//Do not try this at home !!!
	uint16_t* h = reinterpret_cast<uint16_t*>(&height[0]);
	for (int y=0; y<resolution_new; y++)
	{
		for (int x=0; x<resolution_new; x++)
		{
			h[y*resolution_new+x] = h[(y*factor)*resolution+(x*factor)];
		}
	}

	height.resize(height.size()/(factor*factor));
}
