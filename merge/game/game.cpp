#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <cstdint>
#include <time.h>
#include "resource.h"


#include "DXUT/Core/dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include <alg.h>
#include "Effects11/Inc/d3dx11effect.h"
#include <boost/lexical_cast.hpp>
#include "ntx/NTX.h"

#include "MeshRenderer.h"
#include "Mesh.h"
#include "T3d.h"
#include "ObjectTransformation.h"
#include "TerrainObject.h"
#include "EnemyTransformation.h"
#include "EnemyInstance.h"
#include "SpriteRenderer.h"
#include "ProjectileType.h"
#include "WeaponType.h"
#include "debug.h"
#include "Particle.h"
#include "ParticleEffect.h"
#include "Skybox.h"
#include "Macros.h"

// Convenience macros for safe effect variable retrieval
#define random(a, b, outP){ outP = ((float)rand()/RAND_MAX)*(max(a,b)-min(a,b))+min(a,b); }
#if defined(DEBUG)
const std::string TARGETNAME = "Debug";
#else
const std::string TARGETNAME = "Release";
#endif

#define null NULL
// Help macros
#define DEG2RAD( a ) ( (a) * D3DX_PI / 180.f )

using namespace std;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
//Skybox
struct SKYBOX_VERTEX
{
	D3DXVECTOR4 pos;
};

enum TEXTPOSITION
{
	TEXTBOX,
	LEFT,
	RIGHT
};

struct DISPLAYTEXT
{
	wstring Message;
	float alpha;
	float lifeTime;
	TEXTPOSITION pos;
	D3DXCOLOR color;
};

// Camera
struct CAMERAPARAMS {
	float   m_Fovy; //Sichtfeld?
	float   m_Aspect;
	float   m_NearPlane;
	float   m_FarPlane;
}                                       g_CameraParams;
float                                   g_CameraMoveScaler = 1000.f;
float                                   g_CameraRotateScaler = 0.01f;
CFirstPersonCamera                      g_Camera;               // A first person camera
bool									g_CameraMoving;
bool									g_GamePause;
float									g_SunSpeed = 0.4f;
static int								g_PlayerPoints = 0;
bool									g_drawShadows = true;

// User Interface
CDXUTDialogResourceManager              g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                         g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*                        g_TxtHelper = NULL;
CDXUTTextHelper*						g_leftText = NULL;
CDXUTTextHelper*						g_rightText = NULL;
CDXUTDialog                             g_HUD;                  // dialog for standard controls
CDXUTDialog                             g_SampleUI;             // dialog for sample specific controls

// A D3DX rendering effect
ID3DX11Effect*                          g_Effect = NULL; // The whole rendering effect
ID3DX11EffectTechnique*                 g_Technique = NULL; // One technique to render the effect
ID3DX11EffectPass*                      g_Pass0 = NULL; // One rendering pass of the technique
ID3DX11EffectMatrixVariable*            g_WorldEV = NULL; // World matrix effect variable
ID3DX11EffectMatrixVariable*            g_WorldViewProjectionEV = NULL; // WorldViewProjection matrix effect variable
ID3DX11EffectMatrixVariable*            g_invWorldViewProjectionEV = NULL; // WorldViewProjection matrix effect variable
ID3DX11EffectShaderResourceVariable*    g_DiffuseEV = NULL; // Effect variable for the diffuse color texture
ID3DX11EffectVectorVariable*            g_LightDirEV = NULL; // Light direction in object space
ID3DX11EffectMatrixVariable*			g_LightWorldViewProjMatrixEV = NULL;
ID3DX11EffectMatrixVariable*			g_ProjEV = NULL;
// BEGIN: Assignment 4.2.1 
ID3DX11EffectShaderResourceVariable*	g_HeightEV = NULL; 
ID3DX11EffectShaderResourceVariable*	g_NormalEV = NULL; 
ID3DX11EffectScalarVariable*			g_TerrainResEV = NULL;
ID3DX11EffectScalarVariable*			g_TerrainQuadResEV = NULL; 

CHAR									g_SkyboxPath[MAX_PATH] = {'\0'};
bool									g_UseSkybox = false;

//5.3.1
ID3DX11EffectScalarVariable*			g_isCameraObecjtEV = NULL;
ID3DX11EffectShaderResourceVariable*	g_SpecularEV = NULL;
ID3DX11EffectShaderResourceVariable*	g_GlowEV = NULL;
ID3DX11EffectMatrixVariable*			g_WorldViewEV = NULL;
ID3DX11EffectMatrixVariable*			g_WorldViewNormalsEV = NULL;
ID3DX11EffectVectorVariable*			g_LightDirViewEV = NULL;
ID3DX11EffectMatrixVariable*			g_LightPorjektionMatrixEV = NULL;
ID3DX11EffectMatrixVariable*			g_LightViewMatrixEV = NULL;
ID3DX11EffectPass*						g_Pass1_Mesh = NULL;

ID3DX11EffectTechnique*					g_Shadow = NULL;
ID3DX11EffectTechnique*					g_BillboardTechnique = NULL;
ID3DX11EffectPass*						g_Pass_ShadowTerrain = NULL;
ID3DX11EffectPass*						g_Pass_ShadowMesh = NULL;
ID3D11DepthStencilView*					g_shadowDSV = NULL;
ID3D11Texture2D*						g_ShadowMap = NULL;
ID3D11ShaderResourceView*				g_ShadowMapSRV = NULL;
ID3DX11EffectShaderResourceVariable*	g_ShadowMapEV = NULL;



// Background color
D3DXVECTOR4                             g_ClearColor;
//5.2.5
MeshRenderer* g_MeshRenderer = NULL;
ID3D11InputLayout* g_MeshInputLayout = NULL;
//Mesh* g_CockpitMesh = NULL;
//map<string, Mesh*> g_Meshes;
map<string, EnemyTransformation*> g_EnemyTyp;	
vector<ObjectTransformation> g_ObjectTransformations;
map<string, int> g_ObjectReferences;
vector<TerrainObject> g_TerrainObjects;
list<EnemyInstance> g_EnemyInstances;
vector<string> g_EnemyTypeNames;
//SpriteRenderer
SpriteRenderer*			g_SpriteRenderer = NULL;
vector<pair<string, int>>			spriteFileNames;
vector<SpriteVertex>		g_SpritesToRender;
map<string, ProjectileType>  g_ProjectileTypes;
list<Particle> g_Particles;
vector<WeaponType>		g_WeaponTypes;
list<DISPLAYTEXT> g_DisplayTEXTBOX;
list<DISPLAYTEXT> g_DisplayTextLeft;
list<DISPLAYTEXT> g_DisplayTextRight;
int g_TerrainVertexCount;
Skybox* g_SkyboxRenderer;

//map<string, ParticleEffect> g_ParticleEffects;
//list<ParticleEffect> g_ActiveEffects;

//SpawnParameters
double g_LastSpawn = 0;
float g_MinHeight;
float g_MaxHeight;
float g_MinCircle = 0.7f;
float g_MaxCircle = 1.4f;
float g_SpawnIntervall;

//Player
bool p_Fire1;
bool p_Fire2;

//OnKey
bool Omove = false;
bool ORotate = false;
bool OScale = false;
int Oindex = 2;
bool useDeveloperFeatures = false;


// Terrain meta information
struct PtfHeader{
	int16_t magicNumber; 	// Must be 0x00DA
	int16_t version;		// Must be 1
	int32_t heightSize;	    // Height data size
	int32_t colorSize;	    // Color data size
	int32_t normalSize;	    // Normal data size
}                                       g_TerrainHeader; 
char                                    g_TerrainPath[MAX_PATH] = { '\0' };
int                                     g_TerrainResolution;
int                                     g_TerrainNumVertices = 3;
int                                     g_TerrainNumTriangles = 1;
float                                   g_TerrainWidth = 1000.0f;
float                                   g_TerrainDepth = 1000.0f;
float                                   g_TerrainHeight = 400.0f;
bool                                    g_TerrainSpinning = true;
float                                   g_TerrainSpinSpeed = 0.0f;
float									g_BoundingBox = 0.f;
string									g_ResourcesPath;
D3DXMATRIX								g_ViewProj;
D3DXMATRIX                              g_TerrainWorld; // object- to world-space transformation
D3DXMATRIX								g_MeshWorld;
D3DXVECTOR3								g_CameraPos;
D3DXVECTOR3								g_CameraLookAt;
string									g_SolutionDir;
SpriteVertex							pSun;
SpriteVertex							g_Radar;
float									pow2Border;


	D3DXMATRIX worldViewNormals;
	D3DXMATRIX mTrans, mScale, mRot;
	D3DXMATRIX inverseTerrainWorldDir;
	D3DXVECTOR4 lightDirObj;
	D3DXMATRIX lightProjektionMatrix;
	D3DXMATRIX lightViewMatrix;
	D3DXVECTOR3 lightAt(0,0,0);
	D3DXVECTOR3 lightEye;
	D3DXVECTOR3 lightUp(0,1,0);
	D3DXMATRIX lightViewProjMatrix;
	D3DXMATRIX inversView;
	D3DXVECTOR4 lightDirView;
	D3D11_TEXTURE2D_DESC shadowMap_desc;
	D3D11_VIEWPORT cameraVP;
	D3D11_VIEWPORT shadowVP;
	ID3D11Buffer* vbs[] = { NULL, };
	ID3D11DepthStencilView* pDSV;
	ID3D11RenderTargetView* pRTV;

// Terrain rendering resources
// BEGIN: Assignment 4.2.2 
ID3D11Buffer*							g_TerrainHeightBuf = NULL; 
ID3D11ShaderResourceView*				g_TerrainHeightSRV = NULL; 
ID3D11Texture2D*						g_TerrainNormalTex = NULL; 
ID3D11ShaderResourceView*				g_TerrainNormalSRV = NULL; 
// END: Assignment 4.2.2
ID3D11Texture2D*                        g_TerrainDiffuseTex = NULL; // The terrain's material color for diffuse lighting
ID3D11ShaderResourceView*               g_TerrainDiffuseSRV = NULL; // Describes the structure of the diffuse texture to the shader stages

// Scene information
D3DXVECTOR4                             g_LightDir;
D3DXVECTOR3								g_LightColor;
ID3DX11EffectVectorVariable*			g_LightColorEV;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLESPIN          4
#define IDC_RELOAD_SHADERS		101
#define IDC_RELOAD_Config		102
#define IDC_PAUSE				103

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSide1Down, bool bSide2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext); 
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext );
HRESULT LoadFile(const char * filename, std::vector<uint8_t>& data);


void InitApp();
void DeinitApp();
void RenderText();

void ReleaseShader();
HRESULT ReloadShader(ID3D11Device* pd3dDevice);
HRESULT ReloadConfig(ID3D11Device* pd3dDevice);
float getHeightAtPoint(float x, float y);
inline float getHeightAtPoint(int TerrainX, int TerrainY);
D3DXVECTOR3 getNormalAtPoint(float x, float y);
void placeTerrainObject(TerrainObject*);
void AutomaticPositioning();
void loadConfig(bool reload);

bool VertexSort(SpriteVertex& a, SpriteVertex& b)
{
	return (double)D3DXVec3Dot(&a.Position, g_Camera.GetWorldAhead()) > (double)D3DXVec3Dot(&b.Position, g_Camera.GetWorldAhead());
}

//=====================================================================================
/*
|| ::DESCRIPTION::
|| This function will convert a WCHAR string to a CHAR string.
||
|| Param 1 :: Pointer to a buffer that will contain the converted string. Ensure this
|| buffer is large enough; if not, buffer overrun errors will occur.
|| Param 2 :: Constant pointer to a source WCHAR string to be converted to CHAR
*/
void wtoc(CHAR* Dest, const WCHAR* Source)
{
	int i = 0;

	while(Source[i] != '\0')
	{
		Dest[i] = (CHAR)Source[i];
		++i;
	}
}

//=====================================================================================
/*
|| ::DESCRIPTION::
|| This function will convert a CHAR string to a WCHAR string.
||
|| Param 1 :: Pointer to a buffer that will contain the converted string. Ensure this
|| buffer is large enough; if not, buffer overrun errors will occur.
|| Param 2 :: Constant pointer to a source CHAR string to be converted to WCHAR
*/
void ctow(WCHAR* Dest, const CHAR* Source)
{
	int i = 0;

	while(Source[i] != '\0')
	{
		Dest[i] = (WCHAR)Source[i];
		++i;
	}
}
CHAR wide_to_narrow(WCHAR w)
{
	// simple typecast
	// works because UNICODE incorporates ASCII into itself
	return CHAR(w);
}


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	wstring ws = wstring(lpCmdLine);
	string t = "";
	t.resize(ws.length());
	transform(ws.begin(), ws.end(), t.begin(), wide_to_narrow);
	g_SolutionDir = string(t);
	g_SolutionDir = g_SolutionDir.substr(g_SolutionDir.find("-dir=\"")+6);
	g_SolutionDir = g_SolutionDir.substr(0, g_SolutionDir.find_first_of("\""));

	// Old Direct3D Documentation:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > Windows DirectX Graphics Documentation

	// DXUT Documentaion:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > DirectX Documentation for C++ : The DirectX Software Development Kit > Programming Guide > DXUT


	// New Direct3D Documentaion (just for reference, use old documentation to find explanations):
	// http://msdn.microsoft.com/en-us/library/windows/desktop/hh309466%28v=vs.85%29.aspx



	// Set DXUT callbacks
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse);
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	InitApp();
	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true );
	DXUTCreateWindow( L"Shooting the evil aliens by Sven Liedtke@owned by c++" ); // You may change the title

	DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 720 );

	AutomaticPositioning();
	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown();
	DeinitApp();

	return DXUTGetExitCode();
}


void loadConfig(bool reload = false)
{
	HRESULT hr;
	WCHAR path[MAX_PATH];
	char name[MAX_PATH] = {'\0'}, T3dPath[MAX_PATH]={'\0'}, DiffusePath[MAX_PATH] = {'\0'}, SpecularPath[MAX_PATH] = {'\0'}, GlowPath[MAX_PATH] = {'\0'};
	float scale, rotX, rotY, rotZ, transX, transY, transZ, offset, spacing, speed, damage, cooldown, mass, radius;
	bool useNormal;
	int maxCount, textureIndex;
	int hitpoints, units;
	float sphere;
	string effect;
	string file_str, input_str, output_str, it, args, TargetName;
	//		TargetName = MY_TARGETNAME;
	// Parse the config file


	V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.cfg"));
	ifstream stream(path);
	if(!stream) {
		MessageBoxA (NULL, "Fatal Error: game.cfg not found.",
			"Missing file", MB_ICONERROR | MB_OK);
		return;
	}

	std::string var;
	while(!stream.eof())
	{
		stream >> var;
		float x,y,z;
		if (var.empty() || (var[0] == '#')){
			std::string comment;
			std::getline(stream, comment);
			continue;
		}
		if(var.compare("Resources_Dir") == 0) stream >> g_ResourcesPath; 
		else if ( var.compare("Spinning")       ==0 ) stream >> g_TerrainSpinning;
		else if ( var.compare("SpinSpeed")      ==0 ) stream >> g_TerrainSpinSpeed;
		else if ( var.compare("BackgroundColor")==0 ) stream >> g_ClearColor.x >> g_ClearColor.y >> g_ClearColor.z >> g_ClearColor.w;
		// Begin: Assignment 3.2.1
		//liest die erweiterten Angaben aus der game.cfg aus
		else if ( var.compare("TerrainPath")      ==0 ) stream >> g_TerrainPath;
		else if ( var.compare("TerrainWidth")      ==0 ) stream >> g_TerrainWidth;
		else if ( var.compare("TerrainDepth")      ==0 ) stream >> g_TerrainDepth;
		else if ( var.compare("TerrainHeight")      ==0 ) stream >> g_TerrainHeight;
		else if ( var.compare("SkyboxTexture")	==0) {
			stream >> g_SkyboxPath;
			g_UseSkybox = true;}
		else if ( var.compare("Sun") == 0){
			float x2, y2, z2, r, g, b;
			stream >> r >> g >> b >>x >> y >> z >> x2 >> y2 >> z2;
			g_LightColor = D3DXVECTOR3(r, g, b);
			g_LightDir = D3DXVECTOR4(x2-x, y2-y, z2-z, 0);
			D3DXVec3Normalize((D3DXVECTOR3*)&g_LightDir, (D3DXVECTOR3*)&g_LightDir);
		}
		else if ( var.compare("CameraPos") == 0){ stream >>x >> offset >> z;
		g_CameraPos = D3DXVECTOR3(x, offset, z);
		}
		else if(var.compare("CameraLookAt") == 0){ stream >> x >> y >> z;
		g_CameraLookAt = D3DXVECTOR3(x,y,z);
		}
		// END: Assignment 3.2.1
		//5.1.2
		else if(var.compare("Mesh") == 0 && !reload){
			char normal[MAX_PATH] = { '\0'};
			stream >> name >> T3dPath >> DiffusePath >> SpecularPath >> GlowPath >> normal;
			MeshRenderer::g_Meshes[name] = new Mesh((g_ResourcesPath + T3dPath).c_str(), (g_ResourcesPath+DiffusePath).c_str(), (g_ResourcesPath+SpecularPath).c_str(), (g_ResourcesPath+GlowPath).c_str(), (g_ResourcesPath+normal).c_str());
		}
		else if(var.compare("CameraObject") == 0) {
			stream >> name >> scale >> rotX >> rotY >> rotZ>> transX >> transY >> transZ;
			g_ObjectTransformations.push_back(ObjectTransformation(name, true, scale,rotX, rotY, rotZ, transX, transY, transZ));
			g_ObjectReferences[name] = g_ObjectTransformations.size()-1;
		}
		else if(var.compare("WorldObject") == 0) {
			bool automaticHeight;
			stream >> name >> scale >> rotX >> rotY >> rotZ >> transX >> transY >>transZ >> automaticHeight;
			g_ObjectTransformations.push_back(ObjectTransformation(name,  false, scale,rotX, rotY, rotZ, transX, transY, transZ, automaticHeight));
			g_ObjectReferences[name] = g_ObjectTransformations.size()-1;
		}
		else if(var.compare("TerrainObject") == 0)
		{
			stream >> name >> scale >> rotX >> rotY >> rotZ >> useNormal >> spacing >> offset >> maxCount;
			g_TerrainObjects.push_back(TerrainObject(name, useNormal, scale, rotX, rotY, rotZ, spacing, offset, maxCount, MeshRenderer::g_Meshes[name]));
		}
		else if(var.compare("EnemyType") == 0){
			string meshName;

			stream >> name >> hitpoints >> units >> speed >> meshName >> scale >> rotX >> rotY >> rotZ >> transX >> transY >> transZ >> sphere >> effect;
			//g_EnemyTyp
			g_EnemyTyp[name] = new EnemyTransformation(meshName, hitpoints, units, speed, MeshRenderer::g_Meshes[meshName], scale, rotX, rotY, rotZ, transX, transY, transZ, sphere);
			if( effect.size() > 0  && effect.compare("-") != 0)
				g_EnemyTyp[name]->setDeathEffect(&ParticleEffect::g_ParticleEffects[effect]);
			g_EnemyTypeNames.push_back(name);
		}
		else if(var.compare("Spawn") == 0)
		{
			stream>>g_SpawnIntervall >> g_MinHeight >> g_MaxHeight >> g_MinCircle >> g_MaxCircle;
		}
		else if ( var.compare("SpriteTexture")	==0 )
		{
			pair<string,int> p;
			stream>>p.first>>p.second;
			p.first = g_ResourcesPath+p.first;
			spriteFileNames.push_back(p);
		} else if(var.compare("Projectile") == 0)
		{
			stream >> name >> radius >> textureIndex >> damage >> cooldown >> speed >> mass;
			g_ProjectileTypes[name] = ProjectileType(name, radius, textureIndex,speed, mass, cooldown, damage);
		} else if(var.compare("Weapon") == 0)
		{
			stream >> name >> x >> y >> z;
			stream >> var;
			vector<ProjectileType*> tmp;
			if(var.compare("{") == 0)
			{
				stream >> var;
				while(var.compare("}") != 0){
					tmp.push_back(&g_ProjectileTypes[var]);
					stream >> var;
				} 
			}
			D3DXVECTOR3* p = &g_ObjectTransformations[g_ObjectReferences[name]].getPosition();
			g_WeaponTypes.push_back(WeaponType(name, x+p->x,y+p->y,z+p->z, tmp, &g_Particles));
		} else if(var.compare("Explosion") == 0)
		{
			int index, count;
			float duration;
			stream >> name >> textureIndex >> duration >> scale >> speed >> mass >> x >> y >> z >> count;
			ParticleEffect::g_ParticleEffects[name] = ParticleEffect(name, duration, scale, textureIndex, count, D3DXVECTOR3(x,y,z), speed, mass);
			stream >> var;
			if(var.compare("{") == 0)
			{
				stream >> var;
				while(var.compare("}") != 0)
				{
					ParticleEffect::g_ParticleEffects[name].SetChildEffect(var);
					stream >> var;
				}
			}
		}
		// simple error check
		if(stream.fail() && !stream.eof()) {
			MessageBoxA (NULL, ("Warning: Failed parsing config variable "
				+ var).c_str(), "Config file error", MB_ICONERROR | MB_OK);
			exit(-1);
		}
		var.clear();
	}

	stream.close();

}
//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	ShowCursor(false);
	loadConfig();
	g_SpriteRenderer = new SpriteRenderer(spriteFileNames);
	g_MeshRenderer = new MeshRenderer();
	g_SkyboxRenderer = new Skybox(g_SkyboxPath);
	// Intialize the user interface
	g_SettingsDlg.Init( &g_DialogResourceManager );
	g_HUD.Init( &g_DialogResourceManager );
	g_SampleUI.Init( &g_DialogResourceManager );

	g_HUD.SetCallback( OnGUIEvent );
	int iY = 30;
	int iYo = 26;
	g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
	g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
	g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );

	g_HUD.AddButton (IDC_RELOAD_SHADERS, L"Reload shaders (F5)", 0, iY += 24, 170, 22, VK_F5);
	g_HUD.AddButton (IDC_RELOAD_Config, L"Reload Config File (F6)", 0, iY += 24, 170, 22, VK_F6);
	g_HUD.AddButton (IDC_PAUSE, L"Pause", 0, iY += 24, 170, 22, 'P');
	g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
	iY += 24;
	//g_SampleUI.AddCheckBox( IDC_TOGGLESPIN, L"Toggle Spinning", 0, iY += 24, 125, 22, g_TerrainSpinning );		//7.2.2
	//**********GUI*************//
	g_Radar.Opacity = 0.8f;
	g_Radar.Position = D3DXVECTOR3(0.f,-0.68f,0);
	g_Radar.Radius = .08f;
	g_Radar.TextureIndex = 5;
	g_Radar.AnimationProgress = 0;
	SpriteRenderer::g_GUISprites.push_back(g_Radar);
}

//5.2.6
void DeinitApp(){
	g_MeshRenderer->Deinit();
	for(auto it=g_EnemyTyp.begin(); it!= g_EnemyTyp.end(); it++)
		SAFE_DELETE(it->second);
	g_EnemyTyp.clear();
	SAFE_DELETE(g_SpriteRenderer);
	SAFE_DELETE(g_MeshRenderer);
	SAFE_DELETE(g_SkyboxRenderer);
	ShowCursor(true);
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	g_TxtHelper->Begin();		
	g_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_TxtHelper->SetInsertionPos( 5, 5 );
	g_TxtHelper->DrawTextLine( DXUTGetFrameStats(true)); //DXUTIsVsyncEnabled() ) );
	if(useDeveloperFeatures){
		g_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		g_TxtHelper->DrawTextLine( DXUTGetDeviceStats() );
		std::wstringstream out;
		out << Oindex << " "
			<< g_ObjectTransformations[Oindex].getName().c_str()
			<<"  Translation X: "
			<< g_ObjectTransformations[Oindex].getTranslationX() << " Y: "
			<< g_ObjectTransformations[Oindex].getTranslationY() << " Z: "
			<< g_ObjectTransformations[Oindex].getTranslationZ() << " Scale: " << g_ObjectTransformations[Oindex].getScale();

		g_TxtHelper->DrawTextLine(out.str().c_str());
	}
	wstringstream points;
	points << "Your Points: " << g_PlayerPoints;
	g_TxtHelper->DrawTextLine(points.str().c_str());
	g_TxtHelper->SetInsertionPos(5, 160);
	for(auto s = g_DisplayTEXTBOX.begin(); s != g_DisplayTEXTBOX.end();)
	{
		g_TxtHelper->SetForegroundColor(D3DXCOLOR(s->color.r, s->color.g, s->color.b, s->alpha));
		g_TxtHelper->DrawTextLine(s->Message.c_str());
		if(s->lifeTime <= 0)
			s->alpha -= 0.01f;
		if(s->alpha <= 0)
		{
			auto rem = s;
			s++;
			g_DisplayTEXTBOX.erase(rem);
		}
		else
			s++;
	}
	g_TxtHelper->SetInsertionPos(100,300);
	for(auto s = g_DisplayTextLeft.begin(); s != g_DisplayTextLeft.end();)
	{
		g_TxtHelper->SetForegroundColor(D3DXCOLOR(s->color.r, s->color.g, s->color.b, s->alpha));
		g_TxtHelper->DrawTextLine(s->Message.c_str());
		if(s->lifeTime <= 0)
			s->alpha -= 0.01f;
		if(s->alpha <= 0)
		{
			auto rem = s;
			s++;
			g_DisplayTextLeft.erase(rem);
		}
		else
			s++;
	}
	g_TxtHelper->SetInsertionPos(280,300);
	for(auto s = g_DisplayTextRight.begin(); s != g_DisplayTextRight.end();)
	{
		g_TxtHelper->SetForegroundColor(D3DXCOLOR(s->color.r, s->color.g, s->color.b, s->alpha));
		g_TxtHelper->DrawTextLine(s->Message.c_str());
		if(s->lifeTime <= 0)
			s->alpha -= 0.01f;
		if(s->alpha <= 0)
		{
			auto rem = s;
			s++;
			g_DisplayTextRight.erase(rem);
		}
		else
			s++;
	}
	if(g_DisplayTEXTBOX.size() > 4)
	{
		g_DisplayTEXTBOX.begin()->alpha -= 0.01f;
	}

	g_TxtHelper->End();
}

void pushText(const char* s, D3DXCOLOR c, TEXTPOSITION pos)
{
	wstringstream wss;
	wss << s;
	DISPLAYTEXT newEntry;
	newEntry.color = c;
	newEntry.Message = wss.str();
	newEntry.alpha = 1.0f;
	newEntry.lifeTime = 2.0f;
	newEntry.pos = pos;
	switch(pos)
	{
	case TEXTBOX:
		g_DisplayTEXTBOX.push_back(newEntry);
		break;
	case LEFT:
		g_DisplayTextLeft.push_back(newEntry);
		if(g_DisplayTextLeft.size() > 1)
			g_DisplayTextLeft.pop_front();
		break;
	case RIGHT:
		g_DisplayTextRight.push_back(newEntry);
		if(g_DisplayTextRight.size() > 1)
			g_DisplayTextRight.pop_front();
		break;
	}
}

void inline pushText(string& s, D3DXCOLOR c, TEXTPOSITION pos)
{
	pushText(s.c_str(), c, pos);
}

void inline pushText(const char* s, D3DXCOLOR c)
{
	pushText(s, c, TEXTBOX);
}
void inline pushText(const char* s)
{
	pushText(s, D3DXCOLOR(0.9f, 1.0f, 0.3f, 1.f));
}
void inline pushText(const char* s, TEXTPOSITION pos)
{
	pushText(s, D3DXCOLOR(0.9f, 1.0f, 0.3f, 1.f), pos);
}
void inline pushText(string& s)
{
	pushText(s.c_str());
}
void inline pushText(string& s, D3DXCOLOR c)
{
	pushText(s.c_str(), c);
}
void inline pushText(string& s, TEXTPOSITION pos)
{
	pushText(s.c_str(), pos);
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Specify the initial device settings
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
			( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
			pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
		{
			DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
		}

	}
	//// Enable anti aliasing
	pDeviceSettings->d3d11.sd.SampleDesc.Count = 4;
	pDeviceSettings->d3d11.sd.SampleDesc.Quality = 1;

	return true;
}

std::vector<unsigned short> terrainHeights;

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext(); // http://msdn.microsoft.com/en-us/library/ff476891%28v=vs.85%29
	V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
	V_RETURN( g_SettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );

	g_TxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );
	g_leftText = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );
	g_rightText= new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );

	V_RETURN( ReloadShader(pd3dDevice) );

	errno_t error;
	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;      

	// Initialize the camera
	//D3DXVECTOR3 Eye(g_CameraPos.x, *1.2f, g_CameraPos.y );
	//D3DXVECTOR3 At(g_CameraLookAt);
	//g_Camera.SetViewParams( &Eye, &g_CameraLookAt ); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	//g_Camera.SetScalers( g_CameraRotateScaler, g_CameraMoveScaler );

	//// Define the input layout
	//5.3.7
	//T3d::CreateT3dInputLayout(pd3dDevice, g_Pass1_Mesh, &g_MeshInputLayout);


	// BEGIN: Assignment 3.2.2
	//   // Find the terrain file
	wss.str(L""); wss << g_TerrainPath;
	V(DXUTFindDXSDKMediaFileCch( path, MAX_PATH, wss.str().c_str()));
	if (hr != S_OK) {
		ss.str();
		ss << "Could not find '" << g_TerrainPath << "'";
		MessageBoxA (NULL, ss.str().c_str(), "Missing file", MB_ICONERROR | MB_OK);
		return hr;
	}

	// open the terrain file
	FILE* file;
	error = _wfopen_s(&file, path, TEXT("rb"));
	if (file == nullptr) {
		ss.str();
		ss << "could not open '" << g_TerrainPath << "'";
		MessageBoxA(NULL, ss.str().c_str(), "file error", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// read the terrain header
	{
		auto r = fread (&g_TerrainHeader, sizeof (g_TerrainHeader), 1, file);//TODO g_t.. -> PtfH...
		if (r != 1) {
			MessageBoxA(null, "could not read the header.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}
	}

	// check the magic number
	if (g_TerrainHeader.magicNumber != 0x00da) {
		MessageBoxA (null, "the magic number is incorrect.",
			"invalid terrain file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// check the version
	if (g_TerrainHeader.version != 1) {
		MessageBoxA (null, "the header version is incorrect.",
			"invalid terrain file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// calculate the terrain resoultion from the height size
	g_TerrainResolution = (int)sqrt(g_TerrainHeader.heightSize / 2.0); // assume a square terrain
	g_TerrainNumVertices = g_TerrainResolution * g_TerrainResolution;
	g_TerrainNumTriangles = 2 * (g_TerrainResolution - 1) * (g_TerrainResolution - 1) ;
	assert((g_TerrainHeader.heightSize / 2) == g_TerrainNumVertices);
	g_TerrainVertexCount = g_TerrainNumTriangles*3;
	pow2Border = g_TerrainWidth*0.5f*(g_MaxCircle+0.2f)*g_TerrainWidth*0.5f*(g_MaxCircle+0.2f);
	g_BoundingBox = sqrt(sqrt(g_TerrainDepth*g_TerrainDepth+g_TerrainWidth*g_TerrainWidth)*sqrt(g_TerrainDepth*g_TerrainDepth+g_TerrainWidth*g_TerrainWidth)+g_TerrainHeight*g_TerrainHeight);
	// read the terrain heights
	terrainHeights.resize(g_TerrainNumVertices);
	{
		//Kontrolliert, ob die Werte in das array passen
		if (g_TerrainHeader.heightSize != (int)::fread (&terrainHeights[0], sizeof(byte), g_TerrainHeader.heightSize, file)) {
			MessageBoxA(null, "error while reading height data.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}
	// Initialize the camera
	D3DXVECTOR3 Eye(g_CameraPos.x,getHeightAtPoint(g_CameraPos.x, g_CameraPos.z)+g_CameraPos.y, g_CameraPos.z);
	//D3DXVECTOR3 At(g_CameraLookAt);
	g_Camera.SetViewParams( &Eye, &g_CameraLookAt ); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_Camera.SetScalers( g_CameraRotateScaler, g_CameraMoveScaler );

	// read the terrain color texture for diffuse lighting
	std::vector<unsigned char> terraindiffusentx;
	terraindiffusentx.resize(g_TerrainHeader.colorSize);
	{
		const auto requestedsize = terraindiffusentx.size();
		if (requestedsize != fread (&terraindiffusentx[0], sizeof(unsigned char), requestedsize, file)) {
			MessageBoxA (null, "error while reading color data.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}
	// read the terrain normals texture
	std::vector<unsigned char> terrainNormalNtx;
	terrainNormalNtx.resize(g_TerrainHeader.normalSize);
	{
		const auto requestedsize = terrainNormalNtx.size();
		if (requestedsize != fread (&terrainNormalNtx[0], sizeof(unsigned char), requestedsize, file)) {
			MessageBoxA (null, "error while reading normal data.",
				"invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}

	fclose(file);

	//end: assignment 3.2.2
	// Create the vertex buffer for the terrain
	std::vector<float> terrainVB;
	terrainVB.resize(g_TerrainNumVertices * (4 + 4 + 2)); // Position / Normal / TexCoord

	D3D11_SUBRESOURCE_DATA id;
	id.pSysMem = &terrainHeights[0];
	id.SysMemPitch = sizeof(unsigned short);
	id.SysMemSlicePitch = 0;
	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.ByteWidth = g_TerrainNumVertices * id.SysMemPitch;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;	
	pd3dDevice->CreateBuffer(&bd, &id, &g_TerrainHeightBuf);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R16_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = g_TerrainResolution * g_TerrainResolution;
	V_RETURN(pd3dDevice->CreateShaderResourceView(g_TerrainHeightBuf, &srvDesc, &g_TerrainHeightSRV));
	// END: Assignment 4.2.6

	D3D11_TEXTURE2D_DESC tex2DDesc;
	std::vector<std::vector<unsigned char>> textureData;
	std::vector<D3D11_SUBRESOURCE_DATA> subresourceData;
	bool sRgb;
	V(LoadNtx(terraindiffusentx, &tex2DDesc, textureData, subresourceData, sRgb));
	if(FAILED(hr))
		return hr;
	V(pd3dDevice->CreateTexture2D(&tex2DDesc, &subresourceData[0], &g_TerrainDiffuseTex));
	if(FAILED(hr))
		return hr;
	pd3dDevice->CreateShaderResourceView(g_TerrainDiffuseTex, NULL, &g_TerrainDiffuseSRV);  

	// END: Assignment 3.2.5
	// BEGIN: Assigment 4.2.7
	D3D11_TEXTURE2D_DESC normal2DDesc;
	std::vector<std::vector<unsigned char>> normalData;
	subresourceData.empty();
	V(LoadNtx(terrainNormalNtx, &normal2DDesc, normalData, subresourceData, sRgb));
	if(FAILED(hr))
		return hr;
	pd3dDevice->CreateTexture2D(&normal2DDesc, &subresourceData[0], &g_TerrainNormalTex);
	if(FAILED(hr))
		return hr;
	pd3dDevice->CreateShaderResourceView(g_TerrainNormalTex, NULL, &g_TerrainNormalSRV);  
	//END: Assigment 4.2.7

	//5.2.8
	//g_CockpitMesh->CreateResources(pd3dDevice);
	/*for(auto it=g_Meshes.begin(); it!=g_Meshes.end(); it++)
		it->second->CreateResources(pd3dDevice);*/
	//Nun im MeshRenderer
	g_MeshRenderer->CreateResources(pd3dDevice);

	//terrainObjects place and set to draw Ojects
	for(auto o = g_TerrainObjects.begin(); o != g_TerrainObjects.end(); o++)
		placeTerrainObject(o._Ptr);


	//Skybox
	if(g_UseSkybox){
		V(g_SkyboxRenderer->CreateResources(pd3dDevice));
	}

	pSun.Opacity = 1;
	//nun im onMove
	//pSun.Position = (D3DXVECTOR3)(g_LightDir)*g_BoundingBox*-1.f;
	pSun.Radius = 100.0f;
	pSun.TextureIndex = 0;
	pSun.AnimationProgress = 0;
	pSun.Color = D3DXCOLOR(g_LightColor.x,g_LightColor.y, g_LightColor.z, 0.4f);

	//7.2.2
	g_SpriteRenderer->CreateResources(pd3dDevice);
	for_each(g_ProjectileTypes.begin(), g_ProjectileTypes.end(), [&](pair<string, ProjectileType> i){
		i.second.m_Sprite.AnimationSize = g_SpriteRenderer->GetAnimationSize(i.second.m_Sprite.TextureIndex);
		i.second.m_Sprite.TextureIndex = g_SpriteRenderer->GetTextureOffset(i.second.m_Sprite.TextureIndex);
	});
	for_each(ParticleEffect::g_ParticleEffects.begin(), ParticleEffect::g_ParticleEffects.end(), [&](pair<string,ParticleEffect> it){
		it.second.AnimationSize = g_SpriteRenderer->GetAnimationSize(it.second.TextureIndex);
		it.second.TextureIndex = g_SpriteRenderer->GetTextureOffset(it.second.TextureIndex);
	});
	//Shadow Map

	//Create shadow map texture desc
	D3D11_TEXTURE2D_DESC shadowTex_Desc;
	shadowTex_Desc.Width = 4096;
	shadowTex_Desc.Height = 4096;
	shadowTex_Desc.MipLevels = 1;
	shadowTex_Desc.ArraySize = 1;
	shadowTex_Desc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowTex_Desc.SampleDesc.Count = 1;
	shadowTex_Desc.SampleDesc.Quality = 0;
	shadowTex_Desc.Usage = D3D11_USAGE_DEFAULT;
	shadowTex_Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowTex_Desc.CPUAccessFlags = 0;
	shadowTex_Desc.MiscFlags = 0;

	//depth stencil view desc
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSV_Desc;
	shadowDSV_Desc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSV_Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSV_Desc.Texture2D.MipSlice = 0;
	shadowDSV_Desc.Flags = 0;

	//chader resource view desc
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRV_Desc;
	shadowSRV_Desc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRV_Desc.Texture2D.MipLevels = shadowTex_Desc.MipLevels;
	shadowSRV_Desc.Texture2D.MostDetailedMip = 0;



	V(pd3dDevice->CreateTexture2D(&shadowTex_Desc, NULL, &g_ShadowMap));
	V(pd3dDevice->CreateDepthStencilView(g_ShadowMap, &shadowDSV_Desc, &g_shadowDSV));
	V(pd3dDevice->CreateShaderResourceView(g_ShadowMap, &shadowSRV_Desc, &g_ShadowMapSRV));

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	g_DialogResourceManager.OnD3D11DestroyDevice();
	g_SettingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	//5.3.7
	SAFE_RELEASE(g_MeshInputLayout);

	// Assignment 3.2.5
	SAFE_RELEASE(g_TerrainDiffuseTex);
	SAFE_RELEASE(g_TerrainDiffuseSRV);
	SAFE_RELEASE(g_TerrainNormalTex);
	SAFE_RELEASE(g_TerrainNormalSRV);
	SAFE_RELEASE(g_TerrainHeightBuf);
	SAFE_RELEASE(g_TerrainHeightSRV);

	SAFE_RELEASE(g_ShadowMap);
	SAFE_RELEASE(g_ShadowMapSRV);
	SAFE_RELEASE(g_shadowDSV);


	SAFE_DELETE( g_TxtHelper );
	SAFE_DELETE(g_leftText);
	SAFE_DELETE( g_rightText);
	ReleaseShader();

	//5.2.8
	//g_CockpitMesh->ReleaseResources();
	/*for(auto it=g_Meshes.begin(); it!=g_Meshes.end(); it++)
		it->second->ReleaseResources();*/
	g_MeshRenderer->ReleaseResources();

	//7.2.2
	g_SpriteRenderer->ReleaseResources();
	g_SkyboxRenderer->ReleaseResources();
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	// Intialize the user interface

	V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
	V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
	g_HUD.SetSize( 170, 170 );
	g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
	g_SampleUI.SetSize( 170, 300 );

	// Initialize the camera

	g_CameraParams.m_Aspect = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_CameraParams.m_Fovy = 0.785398;
	g_CameraParams.m_NearPlane = 1.f;
	g_CameraParams.m_FarPlane = 2000.f;

	g_Camera.SetProjParams(g_CameraParams.m_Fovy, g_CameraParams.m_Aspect, g_CameraParams.m_NearPlane, g_CameraParams.m_FarPlane);
	g_Camera.SetEnablePositionMovement(false);
	g_Camera.SetRotateButtons(false, false, false, true);
	g_Camera.SetScalers( g_CameraRotateScaler, g_CameraMoveScaler );
	g_Camera.SetResetCursorAfterMove(true);
	g_Camera.SetNumberOfFramesToSmoothMouseData(5);

	g_Camera.SetDrag( true );

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Loads the effect from file
// and retrieves all dependent variables
//--------------------------------------------------------------------------------------
HRESULT ReloadShader(ID3D11Device* pd3dDevice)
{
	assert(pd3dDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.fxo"));
	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pd3dDevice, &g_Effect));    
	assert(g_Effect->IsValid());

	// Obtain the effect technique
	SAFE_GET_TECHNIQUE(g_Effect, "Render", g_Technique);
	SAFE_GET_TECHNIQUE(g_Effect, "Shadow", g_Shadow);
	SAFE_GET_TECHNIQUE(g_Effect, "RenderBillboard", g_BillboardTechnique);

	// Obtain the effect pass
	SAFE_GET_PASS(g_Technique, "P0", g_Pass0);
	SAFE_GET_PASS(g_Technique, "P1_Mesh", g_Pass1_Mesh);
	SAFE_GET_PASS(g_Shadow, "P0_ShadowTerrain", g_Pass_ShadowTerrain);
	SAFE_GET_PASS(g_Shadow, "P1_ShadowMesh", g_Pass_ShadowMesh);

	// Obtain the effect variables
	SAFE_GET_RESOURCE(g_Effect, "g_Diffuse", g_DiffuseEV);
	SAFE_GET_MATRIX(g_Effect, "g_World", g_WorldEV);
	SAFE_GET_MATRIX(g_Effect, "g_WorldViewProjection", g_WorldViewProjectionEV);   
	SAFE_GET_VECTOR(g_Effect, "g_LightDir", g_LightDirEV);
	SAFE_GET_MATRIX(g_Effect, "g_LightWorldViewProjMatrix", g_LightWorldViewProjMatrixEV);
	SAFE_GET_RESOURCE(g_Effect, "g_Height", g_HeightEV);
	SAFE_GET_RESOURCE(g_Effect, "g_Normal", g_NormalEV);
	SAFE_GET_SCALAR(g_Effect, "g_TerrainRes", g_TerrainResEV);
	SAFE_GET_SCALAR(g_Effect, "g_TerrainQuadRes", g_TerrainQuadResEV);
	SAFE_GET_MATRIX(g_Effect, "g_Proj", g_ProjEV);
	//5.3.6
	SAFE_GET_RESOURCE(g_Effect, "g_Specular", g_SpecularEV);
	SAFE_GET_RESOURCE(g_Effect, "g_Glow", g_GlowEV);
	SAFE_GET_MATRIX(g_Effect, "g_WorldView", g_WorldViewEV);
	SAFE_GET_MATRIX(g_Effect, "g_WorldViewNormals", g_WorldViewNormalsEV);
	SAFE_GET_VECTOR(g_Effect, "g_LightDirView", g_LightDirViewEV);
	SAFE_GET_MATRIX(g_Effect, "g_LightProjectionMatrix", g_LightPorjektionMatrixEV);
	SAFE_GET_MATRIX(g_Effect, "g_LightViewMatrix", g_LightViewMatrixEV);
	SAFE_GET_VECTOR(g_Effect, "g_LightColor", g_LightColorEV);
	SAFE_GET_RESOURCE(g_Effect, "g_ShadowMap", g_ShadowMapEV);
	SAFE_GET_SCALAR(g_Effect, "objectOfCamera", g_isCameraObecjtEV);

	//7.2.2
	g_MeshRenderer->ReloadShader(pd3dDevice, g_Effect);
	g_SpriteRenderer->ReloadShader(pd3dDevice);
	g_SkyboxRenderer->ReloadShader(pd3dDevice);
	return S_OK;
}

HRESULT ReloadConfig(ID3D11Device* pd3dDevice)
{
	assert(pd3dDevice != NULL);

	g_EnemyInstances.clear();
	g_EnemyTyp.clear();
	g_EnemyTypeNames.clear();
	g_TerrainObjects.clear();
	g_ObjectTransformations.clear();
	g_ObjectReferences.clear();
	g_SpritesToRender.clear();
	g_Particles.clear();
	g_WeaponTypes.clear();
	g_ProjectileTypes.clear();
	ParticleEffect::g_ActiveEffects.clear();
	ParticleEffect::g_ParticleEffects.clear();

	loadConfig(true);

	AutomaticPositioning();
	for(auto o = g_TerrainObjects.begin(); o != g_TerrainObjects.end(); o++)
		placeTerrainObject(o._Ptr);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	SAFE_RELEASE( g_Effect );
	g_MeshRenderer->ReleaseShader();
	g_SpriteRenderer->ReleaseShader();
	g_SkyboxRenderer->ReleaseShader();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext )
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// Pass messages to settings dialog if its active
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
		return 0;
	}

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;
	*pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// Use the mouse weel to control the movement speed
	if(uMsg == WM_MOUSEWHEEL) {
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_CameraMoveScaler *= (1 + zDelta / 500.0f);
		if (g_CameraMoveScaler < 0.1f)
			g_CameraMoveScaler = 0.1f;
		g_Camera.SetScalers(g_CameraRotateScaler, g_CameraMoveScaler);
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	//OnKeyPress
	if(bKeyDown && useDeveloperFeatures){
		switch(nChar){
		case 107:
			Oindex = (++Oindex)%g_ObjectTransformations.size();
			break;
		case 109:
			Oindex = (--Oindex+g_ObjectTransformations.size())%g_ObjectTransformations.size();
			break;
		case 'M':
			g_CameraMoving = !g_CameraMoving;
			g_Camera.SetEnablePositionMovement(g_CameraMoving);
			break;
		case 'G':
			Omove =  true;
			ORotate = OScale = false;
			break;
		case  'R':
			Omove = OScale = false;
			ORotate = true;
			break;
		case  'S':
			Omove = ORotate = false;
			OScale = true;
			break;
		case 'O':
			g_SunSpeed += 0.1f;
			break;
		case 'I':
			g_SunSpeed-= 0.1f;
			break;
		case  13: //Enter
			Omove = ORotate = OScale = false;
			break;
		case  38: //Arrow Up
			if(Omove)
				g_ObjectTransformations[Oindex].translate(0,1,0);
			if(OScale)
				g_ObjectTransformations[Oindex].scale(0.1f);
			if(ORotate)
				g_ObjectTransformations[Oindex].rotate(0, 1,0);
			break;
		case  40: //Arrow Down
			if(Omove)
				g_ObjectTransformations[Oindex].translate(0,-1,0);
			if(OScale)
				g_ObjectTransformations[Oindex].scale(-0.1f);
			if(ORotate)
				g_ObjectTransformations[Oindex].rotate(0, -1,0);
			break;
		case  39: //Arrow Right
			if(Omove)
				g_ObjectTransformations[Oindex].translate(1,0,0);
			if(ORotate)
				g_ObjectTransformations[Oindex].rotate(1, 0,0);
			break;
		case  37: //arrow Left
			if(Omove)
				g_ObjectTransformations[Oindex].translate(-1,0,0);
			if(ORotate)
				g_ObjectTransformations[Oindex].rotate( -1,0,0);
			break;
		case  187: // minus
			if(Omove)
				g_ObjectTransformations[Oindex].translate(0,0,-1);
			if(ORotate)
				g_ObjectTransformations[Oindex].rotate( 0,0,-1);
			break;
		case  189: // minus
			if(Omove)
				g_ObjectTransformations[Oindex].translate(0,0,1);
			if(ORotate)
				g_ObjectTransformations[Oindex].rotate( 0,0,1);
			break;
		} 
	}else{ //KeyUp
		//switch(nChar){
		//}
	}
	if(bKeyDown)
	{
		switch(nChar){
		case VK_F10:
			g_drawShadows = !g_drawShadows;
			break;
		case VK_SPACE://fire of the first gun
			p_Fire1 = true;
			break;
		case VK_F11:
			useDeveloperFeatures = !useDeveloperFeatures;
			ShowCursor(useDeveloperFeatures);
			g_Camera.SetResetCursorAfterMove(!useDeveloperFeatures);
			break;
		} 
	}else{ //KeyUp
		switch(nChar){
		case VK_SPACE:
			p_Fire1 = false;
			break;
		}
	}


}

void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSide1Down, bool bSide2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext)
{
	if(bRightButtonDown)
		p_Fire2 = true;
	else
		p_Fire2 = false;
	if(bLeftButtonDown)
		p_Fire1 = true;
	else
		p_Fire1 = false;
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen(); break;
	case IDC_TOGGLEREF:
		DXUTToggleREF(); break;
	case IDC_CHANGEDEVICE:
		g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
	case IDC_TOGGLESPIN:
		g_TerrainSpinning = g_SampleUI.GetCheckBox( IDC_TOGGLESPIN )->GetChecked();
		break;
	case IDC_RELOAD_SHADERS:
		ReloadShader(DXUTGetD3D11Device ());
		break;
	case IDC_RELOAD_Config:
		ReloadConfig(DXUTGetD3D11Device());
		break;
	case IDC_PAUSE:
		g_GamePause = !g_GamePause;
		DXUTPause(g_GamePause, false);
		break;
	}
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// Update the camera's position based on user input 
	g_Camera.FrameMove( fElapsedTime );

	//Update HUD Text
	for(auto s = g_DisplayTEXTBOX.begin(); s != g_DisplayTEXTBOX.end(); s++)
		s->lifeTime -= fElapsedTime;

	D3DXMATRIX mTmp;

	// Initialize the terrain world matrix
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb206365%28v=vs.85%29.aspx

	// Set origin to (0.5, 0.5, 0)
	D3DXMatrixTranslation(&g_TerrainWorld, -0.5f, -0.5f, 0.f); // Assume: x, y and z are in [0,1]

	// Scale to terrain extents
	D3DXMatrixScaling(&mTmp, g_TerrainWidth, g_TerrainDepth, g_TerrainHeight);
	g_TerrainWorld = g_TerrainWorld * mTmp;

	// Since "up" is z-axis in object space, but y-axis in world space, we rotate around the x-axis
	D3DXMatrixRotationX( &mTmp, DEG2RAD( -90.0f ) );
	g_TerrainWorld *= mTmp;

	//if( g_TerrainSpinning ) {
	//	D3DXMatrixRotationY( &mTmp, g_TerrainSpinSpeed * DEG2RAD((float)fTime) );
	//	g_TerrainWorld *= mTmp; // Rotate around world-space "up" axis
	//}
	//4.2.10
	//g_LightDir = D3DXVECTOR4(1, 1, -1, 0); // Direction to the directional light in world space    
	//is already normalized
	//D3DXVec3Normalize((D3DXVECTOR3*)&g_LightDir, (D3DXVECTOR3*)&g_LightDir); // Normalize the light direction
	//Sun Simulaions
	D3DXMatrixRotationY(&mTmp,static_cast<float>(fElapsedTime*0.03*DEG2RAD(fTime)*g_SunSpeed));
	D3DXVec4Transform(&g_LightDir, &g_LightDir,&mTmp);
	D3DXMatrixRotationZ(&mTmp,static_cast<float>( std::abs(DEG2RAD(fTime)*fElapsedTime*0.1f*g_SunSpeed)));
	D3DXVec4Transform(&g_LightDir, &g_LightDir,&mTmp);
	//D3DXMatrixRotationX(&mTmp, -DEG2RAD((float)fTime)*fElapsedTime*0.04f*g_SunSpeed);
	//D3DXVec4Transform(&g_LightDir, &g_LightDir,&mTmp);
	pSun.Position = (D3DXVECTOR3)(g_LightDir)*g_BoundingBox*-1.f;
	//*********************************SpawnObjects	
	if(fTime-g_LastSpawn> g_SpawnIntervall)
	{
		g_LastSpawn = fTime;
		EnemyTransformation* myEnemy = g_EnemyTyp[g_EnemyTypeNames[static_cast<int>((float)rand()/RAND_MAX*g_EnemyTypeNames.size())]];
		if(myEnemy->SpawnedEnemies < myEnemy->getCountUnits()){
			myEnemy->SpawnedEnemies++;
			float radOuter = g_TerrainWidth*0.5f*g_MaxCircle;
			float radInner = g_TerrainWidth*0.5f*g_MinCircle;
			float a = ((float)rand()/RAND_MAX)*2*D3DX_PI;
			float height = (((float)rand()/RAND_MAX)*(g_MaxHeight-g_MinHeight)*g_TerrainHeight)+ g_MinHeight*g_TerrainHeight;
			D3DXVECTOR3 outerPos(radOuter*cos(a), height, radOuter*sin(a));
			a = ((float)rand()/RAND_MAX)*2*D3DX_PI;
			D3DXVECTOR3 InnerPos(radInner*cos(a), height, radInner*sin(a));
			D3DXVECTOR3 MovingDirection = InnerPos-outerPos;
			D3DXVec3Normalize(&MovingDirection, &MovingDirection);

			g_EnemyInstances.push_back(EnemyInstance(myEnemy, outerPos, MovingDirection));
			//g_EnemyInstances[1].SetDestroyEffect(&g_ParticleEffects["Death1"]);
			auto g = --g_EnemyInstances.end();
			//g->SetDestroyEffect(&ParticleEffect::g_ParticleEffects["bomb"]);
			pushText("Achtung: Ein neuer Feind ist nun in Reichweite!", D3DXCOLOR(1.0f, 0.15f, 0.f, 1.f));
		}
	}
	if(p_Fire1){
		g_WeaponTypes[1].fire(0, &g_Camera, fTime);
		//move
		g_ObjectTransformations[2].rotate(0,0,170*fElapsedTime);
		g_ObjectTransformations[2].calculateWorldMatrix();
	}
	if(p_Fire2){
		g_WeaponTypes[0].fire(0, &g_Camera, fTime);
		//move
		g_ObjectTransformations[4].translate(0,0,static_cast<float>(10*fElapsedTime*sin(fTime*25)));
		g_ObjectTransformations[4].calculateWorldMatrix();
	}


	//*********************************MovingObjects
	g_ObjectTransformations[10].rotate(0,50.f*fElapsedTime,0);
	g_ObjectTransformations[10].calculateWorldMatrix();
	for(auto ei = g_EnemyInstances.begin(); ei != g_EnemyInstances.end(); ei++)
	{
		ei->move(fElapsedTime);
	}
	for(auto pi = g_Particles.begin(); pi != g_Particles.end(); pi++)
	{
		pi->move(fElapsedTime);
	}

	bool del = false;
	//*********************************Check Collisions
	for(auto shoot = g_Particles.begin(); shoot != g_Particles.end();){
		//Collision mit dem Boden
		if(getHeightAtPoint(shoot->getPosition().x, shoot->getPosition().z) > shoot->getPosition().y)
		{

			ParticleEffect p = ParticleEffect(ParticleEffect::g_ParticleEffects["bomb"]);
			p.setVertexPosition(shoot->getPosition()+D3DXVECTOR3(0,p.getSize()*0.5f,0));
			p.setScale(1);
			ParticleEffect::g_ActiveEffects.push_back(p);

			auto shoot_rem = shoot;
			shoot++;
			g_Particles.erase(shoot_rem);
		}
		else
		{
			del = false;
			for(auto enemy = g_EnemyInstances.begin(); enemy != g_EnemyInstances.end();){
				//D3DXVECTOR3 d =(shoot->Position-enemy->getCurrentPosition());
				//float abstand = D3DXVec3Length(&(shoot->Position-enemy->getCurrentPosition()));
				//float radius =(shoot->Radius +enemy->getObject()->getSphereSize())*(shoot->Radius +enemy->getObject()->getSphereSize());//TODO: richtige Objektgröße Berechnen
				//if abstand < radius
				if(D3DXVec3Length(&(shoot->Position-enemy->getCurrentPosition())) < (shoot->Radius +enemy->getObject()->getSphereSize())*(shoot->Radius +enemy->getObject()->getSphereSize())){
					del = true;
					enemy->OnHit(&shoot.operator*());
					shoot->onHit();
					shoot->Destroy();
					auto shoot_rem = shoot;
					shoot++;
					if(enemy->getLife() <= 0)
					{
						g_PlayerPoints += enemy->getObject()->getPoints();
						enemy->Death();
						auto enemy_rem = enemy;
						enemy++;
						g_EnemyInstances.erase(enemy_rem);
					}
					g_Particles.erase(shoot_rem);
					break; //Ein Projektiel kann nur ein Gegner treffen
				}
				else
					enemy++;
			}
			if(!del)
				shoot++;
		}
	}


	//*********************************DestroyObjects
	for(auto ei = g_EnemyInstances.begin(); ei != g_EnemyInstances.end();)
	{
		if(ei->getCurrentPosition().x*ei->getCurrentPosition().x + ei->getCurrentPosition().y*ei->getCurrentPosition().y-pow2Border > 0){
			auto ei_rem = ei;
			ei++;
			ei_rem->getObject()->SpawnedEnemies--;
			g_EnemyInstances.erase(ei_rem);
		} else {
			ei->calculateWorldMatrix();
			ei++;
		}
	}
	for(auto ei = g_Particles.begin(); ei != g_Particles.end();)
	{
		if(ei->getPosition().x*ei->getPosition().x + ei->getPosition().y*ei->getPosition().y-pow2Border > 0){
			auto ei_rem = ei;
			ei++;
			//ei_rem->getObject()->SpawnedEnemies--;
			g_Particles.erase(ei_rem);
		} else
			ei++;
	}
	//Nach Destroy alle Particle zum Rendern übergeben
	g_SpritesToRender.resize(g_Particles.size());
	int i = 0;
	g_SpritesToRender.push_back(pSun);

	//*********************************DeathEffects
	for(auto it = ParticleEffect::g_ActiveEffects.begin(); it != ParticleEffect::g_ActiveEffects.end(); )
	{
		it->move(fElapsedTime);
		g_SpritesToRender.push_back(it->getSpriteToRender());//jedes bewegte Sprite soll an den Renderer Übergeben werden, der nur mit nem Vector arbeitet
		if(it->isTimeOver())
		{
			auto itrem = it;
			it++;
			ParticleEffect::g_ActiveEffects.erase(itrem);
		}
		else
			it++;
	}

	for each(auto it in g_Particles)
	{
		//g_SpritesToRender.push_back(it);
		g_SpritesToRender[i++] = (SpriteVertex)it;
	}
	sort(g_SpritesToRender.begin(), g_SpritesToRender.end(), VertexSort);
	/*if(g_SpritesToRender.size() > 2048)
	{
		g_SpritesToRender = vector<SpriteVertex>(g_SpritesToRender.begin(), g_SpritesToRender.begin()+1023);
	}
*/
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext )
{
	cout << "render begin";
	HRESULT hr;
	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.OnRender( fElapsedTime );
		return;
	}

	pDSV = DXUTGetD3D11DepthStencilView();
	pRTV = DXUTGetD3D11RenderTargetView();
	const D3DXMATRIX* view, *proj;
	// Update variables that change once per frame
	view = g_Camera.GetViewMatrix();
	proj = g_Camera.GetProjMatrix();
	g_ViewProj = (*view) * (*proj);
	//Lichtinformationen des Frames
	//lightEye = ((D3DXVECTOR3)g_LightDir)*g_BoundingBox*-0.5f;
	//lightEye = (D3DXVECTOR3)g_LightDir*g_BoundingBox*-0.5f;
	D3DXMatrixLookAtLH(&lightViewMatrix, &pSun.Position, &lightAt, &lightUp);
	lightViewProjMatrix = lightViewMatrix*lightProjektionMatrix;
	D3DXMatrixTranspose(&inverseTerrainWorldDir, &g_TerrainWorld);
	D3DXVec4Transform(&lightDirObj, &g_LightDir, &inverseTerrainWorldDir);
	D3DXVec3Normalize((D3DXVECTOR3*)&lightDirObj, (D3DXVECTOR3*)&lightDirObj); 
	D3DXVec4Transform(&lightDirView, &g_LightDir, &inversView);
	D3DXVec3Normalize((D3DXVECTOR3*)&lightDirView, (D3DXVECTOR3*)&lightDirView);
	g_LightWorldViewProjMatrixEV->SetMatrix(lightViewProjMatrix);//benötigt: ShadowTerrainVS/ShadowMeshVS/TerrainVS
	g_LightDirEV->SetFloatVector( ( float* )&lightDirObj ); //benötigt: TerrainPS
	g_LightWorldViewProjMatrixEV->SetMatrix(lightViewProjMatrix);//benötigt: MeshVS
	g_LightDirViewEV->SetFloatVector((float*)lightDirView); //benötigt: MeshPS

	g_LightColorEV->SetFloatVector(g_LightColor);//benötigt: TerrainPS
	//Objectvariables that changes
	D3DXMatrixInverse(&inversView,NULL, g_Camera.GetViewMatrix());
	D3DXMatrixTranspose(&inversView, &inversView);
	g_WorldViewProjectionEV->SetMatrix( ( float* )&g_ViewProj ); //benötigt: TerrainVS
	//D3DXMatrixInverse(&invViewProj, NULL, &g_ViewProj);
	//g_invWorldViewProjectionEV->SetMatrix((float*)&invViewProj); //benötigt: SkyboxVS
	
	//D3DXMatrixInverse(&worldViewNormals, NULL, view);
	D3DXMatrixTranspose(&worldViewNormals, &inversView);
	g_WorldViewNormalsEV->SetMatrix((float*)&worldViewNormals);//benötigt: MeshVS
	g_WorldViewEV->SetMatrix((float*)&view);//benötigt: MeshVS
	g_ProjEV->SetMatrix(*proj);


	pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);	//
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->ClearRenderTargetView( pRTV, g_ClearColor );
	g_ShadowMap->GetDesc(&shadowMap_desc);
	UINT one = 1;
	pd3dImmediateContext->RSGetViewports(&one, &cameraVP);
	shadowVP = D3D11_VIEWPORT(cameraVP);
	shadowVP.Width = static_cast<float>(shadowMap_desc.Width);
	shadowVP.Height = static_cast<float>(shadowMap_desc.Height);
	shadowVP.MinDepth = 0.f;
	shadowVP.MaxDepth = 1.f;
	pd3dImmediateContext->OMSetRenderTargets(0, 0, g_shadowDSV);
	pd3dImmediateContext->RSSetViewports(1, &shadowVP);
	pd3dImmediateContext->ClearDepthStencilView(g_shadowDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
	// Set input layout
	pd3dImmediateContext->IASetInputLayout( NULL );
	// Bind the terrain vertex buffer to the input assembler stage 
	UINT stride = 0, offset = 0;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
	// Tell the input assembler stage which primitive topology to use
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//terrain Rendering
	g_WorldEV->SetMatrix( ( float* )&g_TerrainWorld );
	g_DiffuseEV->SetResource(g_TerrainDiffuseSRV);
	g_HeightEV->SetResource(g_TerrainHeightSRV);
	g_NormalEV->SetResource(g_TerrainNormalSRV);
	g_TerrainResEV->SetInt(g_TerrainResolution); //benötigt im: ShadowTerrainVS/TerrainVS
	g_TerrainQuadResEV->SetInt(g_TerrainResolution-1);
	g_LightColorEV->SetFloatVector(g_LightColor); //benötigt: MeshPS

	// Apply the rendering pass in order to submit the necessary render state changes to the device
	g_Pass_ShadowTerrain->Apply(0 , pd3dImmediateContext);
	// Draw
	pd3dImmediateContext->Draw(static_cast<UINT>(std::powf(static_cast<float>(g_TerrainResolution-1),2.f)*6), 0);
	
	//****Shadowmap of Meshes*****///
	g_MeshRenderer->ShadowMeshes(pd3dDevice, &g_ObjectTransformations);
	g_MeshRenderer->ShadowMeshes(pd3dDevice, &g_EnemyInstances);


	//*****normal Scene Rendering*****//
	if(g_Effect == NULL) {
		g_TxtHelper->Begin();
		g_TxtHelper->SetInsertionPos( 5, 5 );
		g_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		g_TxtHelper->DrawTextLine( L"SHADER ERROR" );
		g_TxtHelper->End();
		return;
	}

	// Clear the depth stencil
	pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);	//
	pd3dImmediateContext->RSSetViewports(1, &cameraVP);
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->ClearRenderTargetView( pRTV, g_ClearColor );

	g_ShadowMapEV->SetResource(g_ShadowMapSRV);//benötigt: TerrainPS/MeshPS
	//Skybox render
	if(g_UseSkybox){
		g_SkyboxRenderer->RenderSkybox(pd3dDevice, g_Camera);
	}


	stride = 0; offset = 0;

	g_WorldEV->SetMatrix( ( float* )&g_TerrainWorld );
	V(g_DiffuseEV->SetResource(g_TerrainDiffuseSRV));//benötigt: TerrainPS
	V(g_HeightEV->SetResource(g_TerrainHeightSRV));//benötigt: TerrainPS
	V(g_NormalEV->SetResource(g_TerrainNormalSRV));//benötigt: TerrainPS
	// Apply the rendering pass in order to submit the necessary render state changes to the device
	g_Pass0->Apply(0, pd3dImmediateContext);
	// Set input layout
	pd3dImmediateContext->IASetInputLayout( NULL );
	// Bind the terrain vertex buffer to the input assembler stage 
	pd3dImmediateContext->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
	// Tell the input assembler stage which primitive topology to use
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(g_TerrainVertexCount, 0);

	g_MeshRenderer->RenderMeshes(pd3dDevice, &g_ObjectTransformations);
	g_MeshRenderer->RenderMeshes(pd3dDevice, &g_EnemyInstances);

	if(g_SpritesToRender.size() >0)
		g_SpriteRenderer->RenderSprites(pd3dDevice, g_SpritesToRender, g_Camera);
	g_SpriteRenderer->RenderGUI(pd3dDevice, g_Camera);
	stringstream ss;
	ss << "Sprites: " << g_SpritesToRender.size();
	pushText(ss.str(), LEFT);

	//render shadow map billboard
	//***************************************************************************
	if(useDeveloperFeatures)
	{
		pd3dImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);	

		g_BillboardTechnique->GetPassByIndex(0)->Apply( 0, pd3dImmediateContext );
		pd3dImmediateContext->DrawIndexed(1, 0, 0);

		//unbind shadow map as SRV
		g_Effect->GetVariableByName("shadowMap")->AsShaderResource()->SetResource( 0 );
		g_BillboardTechnique->GetPassByIndex(0)->Apply( 0, pd3dImmediateContext );
	}

	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
	if(useDeveloperFeatures)
		g_HUD.OnRender( fElapsedTime );
	g_SampleUI.OnRender( fElapsedTime );
	RenderText();
	DXUT_EndPerfEvent();

	static DWORD dwTimefirst = GetTickCount();
	if ( GetTickCount() - dwTimefirst > 5000 )
	{    
		OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
		OutputDebugString( L"\n" );
		dwTimefirst = GetTickCount();
	}
	cout << "render ende";
}

inline float getHeightAtPoint(float x, float y) {
	return getHeightAtPoint(static_cast<int>(min(g_TerrainResolution - 1, (0.5f + x / g_TerrainWidth) * g_TerrainResolution)), static_cast<int>(min(g_TerrainResolution - 1, (0.5f - y / g_TerrainDepth) * g_TerrainResolution)));
}

inline float getHeightAtPoint(int TerrainX, int TerrainY) {
	return g_TerrainHeight * terrainHeights[(int)(max(0,min(TerrainX, g_TerrainResolution-1)) + g_TerrainResolution * max(0,min(TerrainY, g_TerrainResolution-1)))] / (float)UINT16_MAX;
}

D3DXVECTOR3 getNormalAtPoint(float x, float y, size_t size = 1) {
	int midX = static_cast<int>(min(g_TerrainResolution - 1, (0.5f + x / g_TerrainWidth) * g_TerrainResolution));
	int midY = static_cast<int>(min(g_TerrainResolution - 1, (0.5f - y / g_TerrainDepth) * g_TerrainResolution));

	return D3DXVECTOR3(getHeightAtPoint(midX-size, midY) - getHeightAtPoint(midX+size, midY), getHeightAtPoint(midX, midY-size) - getHeightAtPoint(midX, midY+size), 1.f);
}
void placeTerrainObject(TerrainObject* o){
	for(int i = 0; i < o->getMaxCount(); i++)
	{
		float x = 0;random(-400.f,400.f,x);
		float z = 0;random(-400.f,400.f,z);
		o->translateTo(x, getHeightAtPoint(x,z), z);
		if(o->useNormal())
		{
			D3DXVECTOR3 n = getNormalAtPoint(x,z,1);
			o->rotate(n.x,n.y, n.z);
		}
		g_ObjectTransformations.push_back(*o);
	}
}

void AutomaticPositioning()
{
	for(auto it = g_ObjectTransformations.begin(); it != g_ObjectTransformations.end(); it++)
	{
		if(it->automaticHeight())
			it->translateTo(it->getTranslationX(), getHeightAtPoint(it->getTranslationX(), it->getTranslationZ())+ it->getTranslationY(), it->getTranslationZ());
		it->calculateWorldMatrix();
	}
}

void drawShipsOnRadar()
{
	SpriteVertex pShip;
	pShip.Opacity = 1;
	pShip.Color = D3DXCOLOR(1,0,0,1);
	pShip.Radius = 0.01f;
	pShip.AnimationProgress = 0;
	pShip.TextureIndex = 0;
	vector<SpriteVertex> radarShips;

	for_each(g_EnemyInstances.begin(), g_EnemyInstances.end(), [&](EnemyInstance it){
		float x = it.getCurrentPosition().x/(g_TerrainWidth*0.5f*g_MaxCircle)*g_Radar.Radius;
		float y = it.getCurrentPosition().x/(g_TerrainWidth*0.5f*g_MaxCircle)*g_Radar.Radius;
			pShip.Position = D3DXVECTOR3(x,y,0);
			radarShips.push_back(SpriteVertex(pShip));
	});
}

HRESULT LoadFile(const char * filename, std::vector<uint8_t>& data)
{
	FILE* filePointer = NULL;
	errno_t error = fopen_s(&filePointer, filename, "rb");
	if (error) 	{ return E_INVALIDARG; }
	fseek(filePointer, 0, SEEK_END);
	long bytesize = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);
	data.resize(bytesize);
	fread(&data[0], 1, bytesize, filePointer);
	fclose(filePointer);
	return S_OK;
}

