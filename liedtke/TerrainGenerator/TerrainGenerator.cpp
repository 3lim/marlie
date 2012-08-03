// TerrainGenerator.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//4
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include "DiamondSquare.h"
#include "stdafx.h"
#include <stdio.h>
#include <ctime>


using namespace std;
//Deklaration
bool SaveBlock(const char * Path, const unsigned short * Block, 
	size_t Count);

//Definition
int main(int argc, char* argv[])
{
	srand(10146);
	char* path = "heightmap2.raw";
	int size = 8;
	if(argc > 4){
		size = atoi(argv[2]);
		path = argv[4];
	}
	cout << "TerrainGenerator started" << endl;
	time_t begin = time(NULL);
	DiamondSquare myField(size, 2.3f);
	myField.writeArray(false);
	time_t mid = time(NULL);
	myField.SmoothTerrain(10);
	time_t end = time(NULL);
	cout << "final heightfield generated in: " << difftime(mid,begin) << " smoothing in : " << difftime(end, mid) << endl;
	SaveBlock(path, myField.GetTerrainField(), size*size);
	//uncomment for small fields <=16
	//myField.writeArray(true);
	//getchar();
	return (0);
}
 
bool SaveBlock(const char * Path, const unsigned short * Block, 
	size_t Count) { 
		if(!Block) return false; 
		FILE * filePointer = NULL; 
		errno_t error = fopen_s(&filePointer, Path, "wb"); 
		if(error) return false; 
		fwrite(Block, sizeof(unsigned short), Count, filePointer); 
		fclose(filePointer); 
		return true; 
} 

