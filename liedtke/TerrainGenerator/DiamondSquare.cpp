#include <cstdlib>
#include <cstdint>
#include <random>
#include "DiamondSquare.h"
#include <iostream>
#include <Windows.h>
#include <cmath>
#include <ctime>
#include <cmath>
#include <agents.h>
#include <ppl.h>

//schränk die initialisierungshöhe ein
const int maxHeight = UINT16_MAX;
//schränk das Displacement unabhänig der Iteration ein
const int minDisplacement = -UINT16_MAX;
const int maxDisplacement = UINT16_MAX;
//repräsentiert einen Punkt des HeightField mit x,y und der eigenen Höhe
//Fügt einen Punkt mit Höhendaten in das Heightfield array ein
void DiamondSquare::insertIntoField(int x, int y,int height){
	if(x < 0 || y < 0 || x >= resolution || y >= resolution)
		return;
	myFieldArray[y*resolution + x] = height;
}
void DiamondSquare::insertIntoField(int x, int y,int height, int offset){
	int newHeight = height + offset;
	if(newHeight < 0)
		newHeight =std::abs(newHeight);
	else if (newHeight > UINT16_MAX)
		newHeight = UINT16_MAX;
	insertIntoField(x,y,newHeight);

}

uint16_t DiamondSquare::getHeightOfField(int x, int y){
	if(x >= 0 && y >= 0 && x < resolution && y < resolution)
		return myFieldArray[y*resolution + x];
	else
	{
		x = min(max(x,0),resolution-1);
		y = min(max(y,0),resolution-1);
		return myFieldArray[y*resolution + x];
	}
}

int DiamondSquare::calculateCenterSquare(int height1, int height2, int height3, int height4){
	return (height1 + height2 + height3 +height4)>>2; //devide 4
}

int DiamondSquare::calculateCenterPointDiamond(int height1, int height2, int height3, int height4) {
	int count = 4;
	int pointsHeight[] = {height1, height2, height3, height4};
	//hier hoffe ich ´den Fall auszutricksen, dass ein eckpunkt außerhalb des arrays, also bei x/y = -1 liegt
	int newHeight = 0;
	for(int i = 0; i < 4; i++){
		if(pointsHeight[i] == 0)
			--count;
		newHeight += pointsHeight[i];
	}
	return newHeight/count;
	//return struct point(newX, newY, newHeight);

}

//Konstruktor
DiamondSquare::DiamondSquare(int size, float roughness)
{
	resolution = size+1;
	//legt ein genügend Großes Array für die Heightfield an
	unsigned int resPow2 = resolution*resolution;
	myFieldArray = new uint16_t[resPow2];
	for(unsigned int i = 0; i < resPow2; i++)
		myFieldArray[i] = 0;
	//GenerateField_v1(size, roughness);
	//multithreading
	GenerateField_v2(size, roughness);
}

//Destruktor
DiamondSquare::~DiamondSquare(void)
{
	delete[] myFieldArray;
	delete[] outputPointer;
}
void DiamondSquare::insertStartEdges(int size){
	//links oben 
	insertIntoField(0,0, randomize(0, static_cast<int>(maxHeight*.1f)));
	//rechts oben
	insertIntoField(size,0, randomize(0, static_cast<int>(maxHeight*.1f)));
	//links unten
	insertIntoField(0, size, randomize(0,maxHeight));
	//rechts unten
	insertIntoField(size,size, randomize(0,maxHeight));
}
/*
void DiamondSquare::GenerateField_v1(int size, float roughness)
{
	this->roughness = roughness;
	//size muss eine zweierpotenz sein! 
	float potenz = logf((float)size)/logf(2.f);
	if(potenz != (int)potenz)
		exit(-1);
	unsigned int iteration = 0;
	//Berechnung der 4 Außenpunkte
	insertStartEdges(size);

	writeArray(false);
	int squareSize = size;
	do{
		//gibt die Anzahl der Zeilen/Spalten an, die im Array durch die CenterPoints entsehen;
		int countLines = 2*iteration;
		if(iteration==0){ //spezialfall für den ersten Diamond
			countLines= 1;
		}else
			squareSize = squareSize >>1; //devide by 2
		countLines = size/squareSize;
		for(int zeile = 0; zeile < countLines; zeile++)
			for(int spalte = 0; spalte < countLines; spalte++){
				struct point leftUp(spalte*squareSize, zeile*squareSize);
				struct point rightDown(leftUp.x + squareSize,
					leftUp.y + squareSize);
				//finde die 4 Eckpunkte des Diamonds
				point p1 = getPointOfField(leftUp.x,leftUp.y);
				point p2 = getPointOfField(rightDown.x, leftUp.y);
				point p3 = getPointOfField(leftUp.x, rightDown.y);
				point p4 = getPointOfField(rightDown.x,rightDown.y);
				//DiamondCenterpoint
				writeArray(false);
				struct point centerPoint = calculateCenterPoint(struct square(p1,p2,p3,p4));
				insertIntoField(centerPoint, randVariance(minDisplacement,maxDisplacement, iteration));
				//TODO: Displacement
				point top = calculateCenterPoint(p1,p2);
				point left =calculateCenterPoint(p1,p3);
				point right = calculateCenterPoint(p2,p4);
				point bottom =calculateCenterPoint(p3,p4);

				insertIntoField(top, randVariance(iteration));
				insertIntoField(left, randVariance(iteration));
				insertIntoField(right, randVariance(iteration));
				insertIntoField(bottom, randVariance(iteration));
			}
			iteration++;
			std::cout << "iteration " << iteration << " squareSize " << squareSize<< std::endl;
	} while(squareSize >2);
	std::cout << "end DiamondSquare" << std::endl;
}
*/

void DiamondSquare::GenerateField_v2(int size, float roughness)
{
	this->roughness = roughness;
	//size muss eine zweierpotenz sein! 
	float potenz = logf((float)size)/logf(2.f);
	if(potenz != (int)potenz)
		exit(-1);
	unsigned int iteration = 0;
	//Berechnung der 4 Außenpunkte
	insertStartEdges(size);
	int squareSize = size;
	do{
		//std::vector<point> pointsOfDiamonds;
		//Solange nicht alle Felder gefüllt sind suche alle Quadrate
		int sqHalf = static_cast<int>(squareSize*0.5f);
		for(int x = 0; x < size; x += squareSize){
			for(int y = 0; y < size; y += squareSize){
				/*
				point obenLinks = getPointOfField(x,y);
				point obenRects = getPointOfField(x+squareSize, y);
				point untenLinks = getPointOfField(x,y+squareSize);
				point untenRects = getPointOfField(x+squareSize, y+squareSize);
				*/
				insertIntoField(x+sqHalf,y+sqHalf,calculateCenterSquare(getHeightOfField(x,y),getHeightOfField(x+squareSize, y), getHeightOfField(x,y+squareSize), getHeightOfField(x+squareSize, y+squareSize)), randVariance(iteration));
			}
		}
		writeArray(false);
		//Nun die Diamonds raussuchen
		squareSize = squareSize>>1; // devide by 2
		for(int x = squareSize; x < size; x += 2*squareSize){
			for(int y = squareSize; y < size; y += 2*squareSize){
				//linker Diamond vom Punk aus gesehen
				if(getHeightOfField(x-squareSize, y) ==0){ //also Wert wurde noch nicht gesetzt
				//diamond links(getPointOfField(x-2*squareSize,y), getPointOfField(x-squareSize,y-squareSize), edge, getPointOfField(x-squareSize, y+squareSize));
					insertIntoField(x-squareSize, y,calculateCenterPointDiamond(getHeightOfField(x-2*squareSize,y), getHeightOfField(x-squareSize,y-squareSize), getHeightOfField(x,y), getHeightOfField(x-squareSize, y+squareSize)), randVariance(iteration));
				}
				//oberer Diamond vom Punk aus gesehen
				if(getHeightOfField(x, y-squareSize) ==0){ //also Wert wurde noch nicht gesetzt
				//diamond oben(getPointOfField(x-squareSize,y-squareSize), getPointOfField(x,y-squareSize*2), getPointOfField(x+squareSize, y-squareSize), getHeightOfField(x,y));
					insertIntoField(x, y-squareSize, calculateCenterPointDiamond(getHeightOfField(x-squareSize,y-squareSize), getHeightOfField(x,y-squareSize*2), getHeightOfField(x+squareSize, y-squareSize), getHeightOfField(x,y)), randVariance(iteration));
				}
				//rechts Diamond vom Punk aus gesehen
				if(getHeightOfField(x+squareSize, y) ==0){ //also Wert wurde noch nicht gesetzt
				//diamond rechts(getHeightOfField(x,y), getPointOfField(x+squareSize,y-squareSize), getPointOfField(x+squareSize*2,y),  getPointOfField(x+squareSize, y+squareSize));
					insertIntoField(x+squareSize, y, calculateCenterPointDiamond(getHeightOfField(x,y), getHeightOfField(x+squareSize,y-squareSize), getHeightOfField(x+squareSize*2,y),  getHeightOfField(x+squareSize, y+squareSize)), randVariance(iteration));
				}
				//unten Diamond vom Punk aus gesehen
				if(getHeightOfField(x, y+squareSize) ==0){ //also Wert wurde noch nicht gesetzt
				//diamond unten(getPointOfField(x-squareSize,y+squareSize), edge, getPointOfField(x+squareSize,y+squareSize), getPointOfField(x, y+squareSize*2));
					insertIntoField(x, y+squareSize, calculateCenterPointDiamond(getHeightOfField(x-squareSize,y+squareSize), getHeightOfField(x,y), getHeightOfField(x+squareSize,y+squareSize), getHeightOfField(x, y+squareSize*2)), randVariance(iteration));
				}
			}
		}
		iteration++;
	} while(squareSize > 1);
	std::cout << "end DiamondSquare" << std::endl;
}

void DiamondSquare::SmoothTerrain(unsigned int faktor){
	std::cout << "Smothing " << faktor << " times" << std::endl;
	for(unsigned int i = 1; i <= faktor; i++){
		uint16_t* smoothedTerrain = new uint16_t[resolution*resolution];
		size_t size(resolution);
		Concurrency::parallel_for(size_t(0), size, [&](size_t x)
		{		
			for(int y = 0; y <  resolution; y++)
			{
				int q,w,e,r,t,z,u,i,o;
				q = getHeightOfField(x,y);
				w =  getHeightOfField(x-1,y);
				o =  getHeightOfField(x-1,y-1);
				e = getHeightOfField(x, y-1);
				r =  getHeightOfField(x+1,y-1);
				t = getHeightOfField(x+1, y);
				z = getHeightOfField(x+1, y+1);
				u = getHeightOfField(x, y +1);
				i = getHeightOfField(x-1, y+1);
	
				unsigned int flattedHeight =getHeightOfField(x,y) + getHeightOfField(x-1,y) + getHeightOfField(x-1,y-1) + getHeightOfField(x, y-1) + getHeightOfField(x+1,y-1) + getHeightOfField(x+1, y) + getHeightOfField(x+1, y+1) + getHeightOfField(x, y +1) + getHeightOfField(x-1, y+1);
				smoothedTerrain[y*resolution + x] = flattedHeight/9;
			}
		});
		delete[] myFieldArray;
		myFieldArray = smoothedTerrain;
	}
	std::cout << "End of smoothing" << std::endl;
}

void DiamondSquare::writeArray(bool writeOnRun){
	if(!writeOnRun && !DEBUG)
		return;
	int size = resolution-1;
	COORD pos;
	pos.X = 0;
	pos.Y = 1;
	SetConsoleCursorPosition(GetStdHandle( STD_OUTPUT_HANDLE), pos);
	for(int y = 0; y <= size; y++){
		for(int x = 0; x <= size; x++)
		{
			std::cout.width(6);
			std::cout << getHeightOfField(x,y) << "|";
		}
		std::cout << std::endl << std::endl;
	}
	std::getchar();
}

//gibt das erzeugte Terrain als 1d Array zurück
std::uint16_t* DiamondSquare::GetTerrainField(){
	//kürzung des Arrays von Resoultion auf die eingegebene Size
	outputPointer = new std::uint16_t[(resolution-1)*(resolution-1)]; //size = resolution -1
	for(int x = 0; x < resolution-1; x++)
	{
		for(int y = 0; y < resolution-1; y++){
			outputPointer[y*(resolution-1)+x] = getHeightOfField(x,y);
		}
	}

	return  outputPointer;
}

//Randdom funktion mit max Wert
int DiamondSquare::randomize(int max){
	return (int)std::floor(((float)std::rand()/RAND_MAX)*(max)+0.5f);
}


int DiamondSquare::randomize(int min, int max){
	return randomize(max(min,max)-min)+min;
}


int DiamondSquare::randVariance(unsigned int iteration){
	return (int)(randomize(minDisplacement,maxDisplacement)/std::powf(roughness, (float)iteration));//*1.16f));
}


int DiamondSquare::randVariance(int min, int max, unsigned int iteration){
	return (int)(randomize(0,1) * std::powf((.5f),(float)iteration)*roughness);
}

