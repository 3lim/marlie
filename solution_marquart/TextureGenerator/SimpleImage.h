/************************************************************************* 
	Class: SimpleImage
	Author: I. Demir
	**********************************************************************
*/

#pragma once
#include <cstdint>
#pragma warning(disable:4005)
#include <wincodec.h>
#pragma warning(default:4005)
#include "ComPtr.h"

#pragma comment(lib, "Windowscodecs.lib") 

class SimpleImage
{        
public:
	// Create an empty image
	SimpleImage(UINT width, UINT height);

	// Create Image from a image File using WIC
	SimpleImage(const char* filename);

	~SimpleImage(void);
	SimpleImage(const SimpleImage& x);
	SimpleImage& operator=(SimpleImage& x);

	// Save Image to PNG File
	bool save(const char* filename) const;

	UINT getHeight() const {
		return data->height;
	}

	UINT getWidth() const { 
		return data->width; 
	}

	// set RGB Values to pixel (x,y)
	void setPixel(UINT x, UINT y, const BYTE r, const BYTE g, const BYTE b);

	// get RGB Values from pixel (x,y)
	void getPixel(UINT x, UINT y, BYTE& r, BYTE& g, BYTE& b) const;

private:

	typedef struct {
		UINT height;
		UINT width;
		BYTE* img;
		unsigned int num_ref;
	} Data;

	Data* data;

	void unref();
	void ref(int width, int height);
	bool wic_LoadImg(const wchar_t* filenameW);
	bool wic_SaveImg(const wchar_t* filenameW) const;
};