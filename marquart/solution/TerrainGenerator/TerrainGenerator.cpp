#include <iostream>
#include <math.h>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <cstdint>
#include <time.h>
#include <stdlib.h>
using namespace std;

bool _save(char* path, uint16_t* data, size_t count);
uint16_t* _generate(const int res, const int variance, const float displace, const bool expdisplace); 
uint16_t _displace(int val, int variance);
float _vdelta(const int i, const float displace, const int res);
uint16_t* _filter(uint16_t* in, const int res, const int gridsize, const int times=1);
	
const uint16_t VAL_MAX = 65535;

int main(int argc, char* argv[])
{	
	time_t timestart = time(NULL);
	int resc=0, pathc=0, varc=0, disc=0, filterc=0;
	bool expdisplace=true;

	for(int c=0;c<argc-1;c++)
	{
		if(strcmp(argv[c],"-r")==0) resc=c+1;//resolution
		else if(strcmp(argv[c],"-p")==0) pathc=c+1;//output-path
		else if(strcmp(argv[c],"-v")==0) varc=c+1;//start-value for displace/max height-difference
		else if(strcmp(argv[c],"-d")==0) disc=c+1;//start-value for displace-factor
		else if(strcmp(argv[c],"-e")==0) expdisplace=false;//disable exponential reduction of displacement-factor
		else if(strcmp(argv[c],"-f")==0) filterc=c+1;//filter value
	}
	
	if(resc==0) return -1;

	srand((unsigned int)time(NULL));

	uint16_t* d = _generate(atoi(argv[resc]),(varc==0)?_displace(VAL_MAX>>1,VAL_MAX>>1):atoi(argv[varc]),(disc==0)?0.6f:(float)atof(argv[disc]),expdisplace);
	uint16_t* f = filterc!=0?_filter(d,atoi(argv[resc]),(int)(atof(argv[filterc])*10-atoi(argv[filterc])*10),atoi(argv[filterc])):d;

	if(!_save((pathc!=0&&argv[pathc]!="")?argv[pathc]:"terrain_height.raw",f,atoi(argv[resc]))) cout<<"Failed to write data!"<<endl;
	delete[] d;
	if(filterc!=0) delete[] f;
	cout<<"Generated heightfield in "<<(time(NULL)-timestart)<<" seconds."<<endl;
	return 0;
}

bool _save(char* path, uint16_t* data, size_t count)
{
	if(!data) return false;
	FILE * filePointer = NULL;
	errno_t error = fopen_s(&filePointer, path, "wb");
	if(error) return false;
	fwrite(data, sizeof(uint16_t), count*count, filePointer);
	fclose(filePointer);
	return true;
}

uint16_t _displace(int val, int variance)
{
	if(variance>val) return val+variance>VAL_MAX?_displace(VAL_MAX>>1,VAL_MAX>>1):_displace((val+variance)>>1,(val+variance)>>1);
	float r = rand()/(RAND_MAX/1.f);
	int ret = (val-variance)+(int)(r*(variance*2));
	return ret>VAL_MAX?VAL_MAX:ret;
}

float* cf = new float[6];
float _vdelta(const int i, const float displace, const int res)
{
	if(i==0)
	{
		cf[0]=(float)(1.05*displace);
		cf[1]=(float)(-0.005*displace*(19*pow(res*1.f,6)-188*pow(res*1.f,5)+527*pow(res*1.f,4)+4298*pow(res*1.f,3)-21780*pow(res*1.f,2)+30072*res-11808)/(res*(55*pow(res*1.f,3)-12*pow(res*1.f,4)-120*pow(res*1.f,2)+124*res-48+pow(res*1.f,5)))); 
		cf[2]=(float)(0.005*displace*(5*pow(res*1.f,7)-452*pow(res*1.f,5)+8673*pow(res*1.f,4)-26652*pow(res*1.f,3)+19764*pow(res*1.f,2)+3168*res-1728)/(pow(res*1.f,2)*(55*pow(res*1.f,3)-12*pow(res*1.f,4)-120*pow(res*1.f,2)+124*res-48+pow(res*1.f,5)))); 
		cf[3]=(float)(-(0.01*(10*pow(res*1.f,6)-92*pow(res*1.f,5)+1209*pow(res*1.f,4)+563*pow(res*1.f,3)-10420*pow(res*1.f,2)+9036*res-432))*displace/(pow(res*1.f,2)*(pow(res*1.f,2)+8-6*res)*(pow(res*1.f,3)-6*pow(res*1.f,2)+11*res-6)));
		cf[4]=(float)((0.005*(25*pow(res*1.f,5)-267*pow(res*1.f,4)+4160*pow(res*1.f,3)-8172*pow(res*1.f,2)-576*res+1728))*displace/(pow(res*1.f,2)*(-7*pow(res*1.f,2)+pow(res*1.f,3)+14*res-8)*(pow(res*1.f,2)-5*res+6)));
		cf[5]=(float)(-0.01*displace*(432-51*pow(res*1.f,3)-1836*res+844*pow(res*1.f,2)+5*pow(res*1.f,4))/(pow(res*1.f,2)*(55*pow(res*1.f,3)-12*pow(res*1.f,4)-120*pow(res*1.f,2)+124*res-48+pow(res*1.f,5))));
	}
	float out = cf[0]+cf[1]*i+cf[2]*pow(i*1.f,2)+cf[3]*pow(i*1.f,3)+cf[4]*pow(i*1.f,4)+cf[5]*pow(i*1.f,5);
	if(i==res) delete[] cf;
	return out>0?out:0;
}

uint16_t* _generate(const int res, const int var, const float displace, const bool expdisplace)
{
	int variance = var;
	uint16_t* data = new uint16_t[(res+1)*(res+1)]; 

	data[0] = _displace(VAL_MAX>>1,VAL_MAX>>1);
	data[res] = _displace(VAL_MAX>>1,VAL_MAX>>1);
	data[res*res] = _displace(VAL_MAX>>1,VAL_MAX>>1);
	data[res*(res+1)] = _displace(VAL_MAX>>1,VAL_MAX>>1);

	int itr=0;
	for(int size=res>>1;size>=1;size=size>>1,variance=(int)(variance*(expdisplace?_vdelta(itr++,displace,(int)((log(res*1.f)/log(2*1.f))-1)):displace)))
	{
		for(int x=size;x<=res;x+=size<<1) 
		{
			for(int y=size;y<=res;y+=size<<1) 
			{
				int avg = (data[(x-size)*res+(y-size)] + data[(x-size)*res+(y+size)] + data[(x+size)*res+(y-size)] + data[(x+size)*res+(y+size)])/4;
				data[x*res+y] = _displace(avg,variance);
			}
		}

		for(int x=0;x<=res;x+=size<<1)
		{
			for(int y=0;y<=res;y+=size<<1)
			{
				if(x!=res)
				{
					int avg = (data[x*res+y] + data[(x+(size<<1))*res+y] + ((y!=res)?data[(x+(size))*res+(y+(size))]:0) + ((y!=0)?data[(x+(size))*res+(y-(size))]:0))/((y!=res&&y!=0)?4:3);
					data[(x+(size))*res+y] = _displace(avg,variance);
				}
				if(y!=res)
				{
					int avg = (data[x*res+y] + data[x*res+(y+(size<<1))] + ((x!=res)?data[(x+(size))*res+(y+(size))]:0) + ((x!=0)?data[(x-(size))*res+(y+(size))]:0))/((x!=res&&x!=0)?4:3);
					data[x*res+(y+(size))] = _displace(avg,variance);
				}
			}
		}
	}
	
	uint16_t* d = new uint16_t[res*res];
	for(int i=0;i<res;i++)
	{
		for(int j=0;j<res;j++)
		{
			d[i*res+j] = data[(i+1)*res+j+1];
		}
	}

	delete[] data;
	return d;
}

uint16_t* _filter(uint16_t* in, const int res, const int gridsize, const int times)
{
	if(gridsize%2!=1) return in;
	
	//gauss-matrix
	int* m = new int[gridsize];
	m[0] = m[gridsize-1] = 1;
	for(int i=1;i<=(gridsize-1)/2;i++) m[i] = m[gridsize-i-1] = m[i-1]<<1;
	int* g = new int[gridsize*gridsize];
	int gc = 0;
	for(int k=0;k<gridsize*gridsize;k++) gc += g[k] = m[k/gridsize] * m[k%gridsize];
	delete[] m;

	uint16_t* out = new uint16_t[res*res];

	cout<<"Filtering "<<times<<"x with "<<gridsize<<"x"<<gridsize<<"..."<<endl;
	
	int count=0;
	float maxchange = 0, maxchangebordersx = 0, maxchangebordersy = 0;

	for(int i=0;i<times;i++)
	{
		for(int x=0;x<res;x++)
		{
			for(int y=0;y<res;y++)
			{
				if(count++%(res*res*times/20)==0) cout<<((int)(count/(res*res*times*1.f)*100))<<"% ~";
				long result = 0;
				int c = 0;
				for(int x1=x-(gridsize-1)/2;x1<=x+(gridsize-1)/2;x1++)
				{
					for(int y1=y-(gridsize-1)/2;y1<=y+(gridsize-1)/2;y1++)
					{
						if(x1<0)
						{
							if(y1<0) {
								result += g[c++]*in[0];//NW
							}
							else if(y1<res) {
								result += g[c++]*in[y1];//W
							}
							else {
								result += g[c++]*in[res-1];//SW
							}
						}
						else if(x1<res)
						{
							if(y1<0) {
								result += g[c++]*in[x1*res];//N
							}
							else if(y1<res) {
								result += g[c++]*in[x1*res+y1];//Normal
							}
							else {
								result += g[c++]*in[x1*res+res-1];//S
							}
						}
						else if(x1>=res)
						{
							if(y1<0) {
								result += g[c++]*in[(res-1)*res];//NE
							}
							else if(y1<res) {
								result += g[c++]*in[(res-1)*res+y1];//E
							}
							else {
								result += g[c++]*in[(res-1)*res+res-1];//SE
							}
						}
					}
				}

				out[y*res+x] = (uint16_t)(result/gc);
				if(abs((out[y*res+x]*1.f/in[y*res+x]-1))>maxchange) maxchange=(out[y*res+x]*1.f/in[y*res+x]-1);
				if((y==0||y==res-1)&&(abs((out[y*res+x]*1.f/in[y*res+x]-1))>maxchangebordersy)) maxchangebordersy=(out[y*res+x]*1.f/in[y*res+x]-1);
				if((x==0||x==res-1)&&(abs((out[y*res+x]*1.f/in[y*res+x]-1))>maxchangebordersx)) maxchangebordersx=(out[y*res+x]*1.f/in[y*res+x]-1);
			}
		}
		cout<<"> max change -> y-borders: "<<maxchangebordersy<<"/x-borders: "<<maxchangebordersx<<"/normal: "<<maxchange<<endl;
		maxchange = maxchangebordersy = maxchangebordersx = 0;
		for(int i=0;i<res*res;i++) in[i]=out[i];
		for(int i=0;i<res;i++) {in[i] = in[res+i];in[i*res] = in[i*res+1];in[(res-1)*res + i] = in[(res-2)*res + i];in[i*res+res-1] = in[i*res+res-2];}
	}
	for(int i=0;i<res;i++) {out[i] = out[res+i];out[i*res] = out[i*res+1];out[(res-1)*res + i] = out[(res-2)*res + i];out[i*res+res-1] = out[i*res+res-2];}

	delete[] g;
	return out;
}
