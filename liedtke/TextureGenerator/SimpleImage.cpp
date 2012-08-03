#include "SimpleImage.h"
#include <wincodec.h>
#include <wincodecsdk.h>
#pragma comment(lib, "WindowsCodecs.lib")
void SimpleImage::unref() {
	if(!data)
		return;

	if (--(data->num_ref)==0) {
		delete[] data->img;
		delete data;
	}
}

void SimpleImage::ref(int width, int height) {
	data = new Data;
	data->num_ref = 1;
	data->height = height;
	data->width = width;
	data->img = new BYTE[height*width*3];
}

bool SimpleImage::wic_LoadImg(const wchar_t* filenameW) {
		

	ComPtr<IWICStream>				stream;
	ComPtr<IWICBitmapDecoder>		decoder;
	ComPtr<IWICImagingFactory>		factory;
	ComPtr<IWICBitmapFrameDecode>	frame;
	ComPtr<IWICFormatConverter>		reader;
	
	auto hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&factory)
		);
		
	if (SUCCEEDED(hr)) {
		hr = factory->CreateStream(&stream);
	} else {
		return false;
	}

	// Initialize the stream
	if (SUCCEEDED(hr)) {
		stream->InitializeFromFilename(filenameW, GENERIC_READ);
	} else {
		return false;
	}
		
	// Create a decoder for the stream.
	if (SUCCEEDED(hr)) {
		hr = factory->CreateDecoderFromStream(
			stream,							// The stream to use to create the decoder
			NULL,							// Do not prefer a particular vendor
			WICDecodeMetadataCacheOnLoad,	// Cache metadata when needed
			&decoder);						// Pointer to the decoder
	} else {
		return false;
	}
		
	// Retrieve the first bitmap frame.
	if (SUCCEEDED(hr)) {
		hr = decoder->GetFrame(0, &frame);
	} else {
		return false;
	}


	if (SUCCEEDED(hr)) {
		hr = factory->CreateFormatConverter (&reader);
		reader->Initialize (frame,
			GUID_WICPixelFormat24bppBGR,
			WICBitmapDitherTypeNone,
			nullptr,
			0.0,
			WICBitmapPaletteTypeCustom);
	} else {
		return false;
	}

	UINT width = 0, height = 0;

	if (SUCCEEDED(hr)) {
		hr = frame->GetSize (&width, &height);
	} else {
		return false;
	}

	if (data->img)
		delete[] data->img;
	data->img = new BYTE[width * height * 3];
	data->width = width;
	data->height = height;

	WICRect rect;
	rect.Height = height;
	rect.Width = width;
	rect.X = 0;
	rect.Y = 0;

	const UINT stride = width * sizeof (unsigned char) * 3;

	const auto result = reader->CopyPixels (&rect, stride, stride * rect.Height, data->img);
		
	if (FAILED(result)) {
		return false;
	}

	return true;

}

bool SimpleImage::wic_SaveImg(const wchar_t* filenameW) const {
	ComPtr<IWICStream>				stream;
	ComPtr<IWICBitmapEncoder>		Encoder;
	ComPtr<IWICImagingFactory>		factory;
	ComPtr<IWICBitmapFrameEncode>	frame;
	ComPtr<::IPropertyBag2>			bag;
	
	auto hr = ::CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&factory)
		);
		
	if (SUCCEEDED(hr)) {
		hr = factory->CreateStream(&stream);
	} else {
		return false;
	}

	// Initialize the stream
	if (SUCCEEDED(hr)) {
		stream->InitializeFromFilename (filenameW, GENERIC_WRITE);
	} else {
		return false;
	}
		
	// Create a encoder for the stream.
	if (SUCCEEDED(hr)) {
		hr = factory->CreateEncoder(
			GUID_ContainerFormatPng,		// Format		
			NULL,							// Do not prefer a particular vendor
			&Encoder);						// Pointer to the encoder
	} else {
		return false;
	}
		
		
	if (SUCCEEDED(hr)) {
		hr = Encoder->Initialize(stream, WICBitmapEncoderNoCache);
	} else {
		return false;
	}

	if (SUCCEEDED(hr)) {
		hr = Encoder->CreateNewFrame(&frame, 0);
	} else {
		return false;
	}

	UINT width = data->width, height = data->height;

	if(SUCCEEDED(hr)){
		frame->Initialize(0);
	}else{
		return false;
	}

	if (SUCCEEDED(hr)) {
		hr = frame->SetSize (width, height);
	} else {
		return false;
	}

	if (SUCCEEDED(hr)) {
		WICPixelFormatGUID format = GUID_WICPixelFormat24bppRGB;
		hr = frame->SetPixelFormat(&format);
	} else {
		return false;
	}

	if(SUCCEEDED(hr)){
		const UINT stride = width * sizeof (unsigned char) * 3;
		hr = frame->WritePixels(height, stride, stride*height, data->img);
	} else {
		return false;
	}

	if (SUCCEEDED(hr)){
		hr = frame->Commit();
	}    

	if (SUCCEEDED(hr)){
		hr = Encoder->Commit();
	}

	return true;
	

}

SimpleImage::SimpleImage(UINT width, UINT height) {
	ref(width, height);		
	memset(data->img, 0, sizeof(BYTE) * 3 * width * height);
}

SimpleImage::SimpleImage(const char* filename) {
	ref(0,0);
	int len = strlen(filename) + 1;
	int lenW = MultiByteToWideChar(CP_ACP, 0, filename, len, NULL, 0);
	wchar_t* filenameW = new wchar_t[lenW];
	MultiByteToWideChar(CP_ACP, 0, filename, len, filenameW, lenW);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool success = wic_LoadImg(filenameW);
	delete[] filenameW;
	CoUninitialize();
	if(!success) {
		unref();
		throw "Could not load the image";
	}			
}

SimpleImage::~SimpleImage(void) {
	unref();
}

SimpleImage::SimpleImage(const SimpleImage& x) {
	data = x.data;
	++(data->num_ref);
}

SimpleImage& SimpleImage::operator=(SimpleImage& x) {
	if(&x==this)
		return *this;
	unref();
	data = x.data;
	++(data->num_ref);
	return *this;
}

bool SimpleImage::save(const char* filename) const {
	int len = strlen(filename) + 1;
	int lenW = MultiByteToWideChar(CP_ACP, 0, filename, len, NULL, 0);
	wchar_t* filenameW = new wchar_t[lenW];
	MultiByteToWideChar(CP_ACP, 0, filename, len, filenameW, lenW);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool success = wic_SaveImg(filenameW);
	delete[] filenameW;
	CoUninitialize();
	return success;
}

void SimpleImage::setPixel(UINT x, UINT y, const BYTE r, const BYTE g, const BYTE b) {
	unsigned int offset = 3 * (x + y * data->width);
	data->img[offset] = b;
	data->img[offset + 1] = g;
	data->img[offset + 2] = r;
}

void SimpleImage::getPixel(UINT x, UINT y, BYTE& r, BYTE& g, BYTE& b) const {
	unsigned int offset = 3 * (x + y * data->width);
	b = data->img[offset];
	g = data->img[offset + 1];
	r = data->img[offset + 2];
}
