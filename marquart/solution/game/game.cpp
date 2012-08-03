#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include <map>
#include <list>
#include <random>
#include <time.h>
#include <algorithm>
#include "resource.h"

#include "DXUT/Core/dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"

#include "Effects11/Inc/d3dx11effect.h"

#include "ntx/NTX.h"
#include "Mesh.h"
#include "T3d.h"
#include "SpriteRenderer.h"

#include "debug.h"

// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );						assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect!=NULL); var = effect->GetTechniqueByName( name );						assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)   {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}

// Help macros
#define DEG2RAD( a ) ( (a) * D3DX_PI / 180.f )

using namespace std;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

// Camera
struct CAMERAPARAMS {
	float   m_Fovy;
	float   m_Aspect;
	float   m_NearPlane;
	float   m_FarPlane;
}                                       g_CameraParams;
float                                   g_CameraMoveScaler = 1000.f;
float                                   g_CameraRotateScaler = 0.01f;
CFirstPersonCamera                      g_Camera;               // A first person camera

// User Interface
CDXUTDialogResourceManager              g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                         g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*                        g_TxtHelper = NULL;
CDXUTDialog                             g_HUD;                  // dialog for standard controls
CDXUTDialog                             g_SampleUI;             // dialog for sample specific controls

// A D3DX rendering effect
ID3DX11Effect*                          g_Effect = NULL; // The whole rendering effect
// BEGIN: Assignment 4.2.1 
ID3DX11EffectShaderResourceVariable* g_HeightEV = NULL;
ID3DX11EffectShaderResourceVariable* g_NormalEV = NULL;
ID3DX11EffectScalarVariable* g_TerrainResEV = NULL;
ID3DX11EffectScalarVariable* g_TerrainQuadResEV = NULL;
// END: Assignment 4.2.1
ID3DX11EffectTechnique*                 g_Technique = NULL; // One technique to render the effect
ID3DX11EffectPass*                      g_Pass0 = NULL; // One rendering pass of the technique
ID3DX11EffectMatrixVariable*            g_WorldEV = NULL; // World matrix effect variable
ID3DX11EffectMatrixVariable*            g_WorldViewProjectionEV = NULL; // WorldViewProjection matrix effect variable
ID3DX11EffectShaderResourceVariable*    g_DiffuseEV = NULL; // Effect variable for the diffuse color texture
ID3DX11EffectVectorVariable*            g_LightDirEV = NULL; // Light direction in object space

// Background color
D3DXVECTOR4                             g_ClearColor;

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
D3DXMATRIX                              g_TerrainWorld; // object- to world-space transformation

// Terrain rendering resources
ID3D11Texture2D*                        g_TerrainDiffuseTex = NULL; // The terrain's material color for diffuse lighting
ID3D11ShaderResourceView*               g_TerrainDiffuseSRV = NULL; // Describes the structure of the diffuse texture to the shader stages
// BEGIN: Assignment 4.2.2 
ID3D11Buffer* g_TerrainHeightBuf = NULL;
ID3D11ShaderResourceView* g_TerrainHeightSRV = NULL;
ID3D11Texture2D* g_TerrainNormalTex = NULL;
ID3D11ShaderResourceView* g_TerrainNormalSRV = NULL;
// END: Assignment 4.2.2

// Scene information
D3DXVECTOR4                             g_LightDir;

// General meta data
char                                    g_DebugTexPath[MAX_PATH] = { '\0' };

// General resources
ID3D11ShaderResourceView*               g_DebugSRV = NULL;

//Mesh
ID3D11InputLayout* g_MeshInputLayout = NULL;
std::map<std::string, Mesh*> g_Meshes;

ID3DX11EffectShaderResourceVariable* g_SpecularEV = NULL; 
ID3DX11EffectShaderResourceVariable* g_GlowEV = NULL; 
ID3DX11EffectMatrixVariable* g_WorldViewEV = NULL; 
ID3DX11EffectMatrixVariable* g_WorldViewNormalsEV = NULL; 
ID3DX11EffectVectorVariable* g_LightDirViewEV = NULL; 
ID3DX11EffectPass* g_Pass1_Mesh = NULL;

SpriteRenderer* g_SpriteRenderer = NULL;
vector<SpriteVertex> g_Sprites;

//Shadow
ID3D11Texture2D* g_ShadowTex = NULL;
ID3D11ShaderResourceView* g_ShadowSRV = NULL;
ID3D11DepthStencilView* g_ShadowDSV = NULL;

struct GameObject{char type;
					string name;
					float scale;
					D3DXVECTOR3 rotation;
					D3DXVECTOR3 translation;
};

struct EnemyType{std::string name;
					int health;
					int count;
					float speed;
					GameObject mesh;
};

struct EnemyObject{std::string type;
					int hitpoints;
					D3DXVECTOR3 pos;
					D3DXVECTOR3 vel;
};

struct SpawnPrefs {float interval;
					D3DXVECTOR2 height;
					D3DXVECTOR2 x;
					D3DXVECTOR2 y;
					D3DXVECTOR2 speed;
					double lastspawned;
} g_SpawnPreferences;

struct GunPrefs {float projectileSpeed;
					float fireRate;
					unsigned int damage;
					float gravity;
					unsigned int texId;
					float texRadius;
					D3DXVECTOR3 texSpawn;
					unsigned char fireKey;
					bool fired;
					double lastFired;
					float animState;
					string name;
}; 
vector<GunPrefs> g_GunPrefs;

struct ExplPrefs {uint16_t texId;
					float durationMin;
					float durationMax;
					unsigned int particleCountMin;
					unsigned int particleCountMax;
					float particleSpeedMin;
					float particleSpeedMax;
					float particleSizeMin;
					float particleSizeMax;
					float fadeMin;
					float fadeMax;
};
map<string,ExplPrefs> g_ExplosionPrefs;

struct ExpState {string explName;
					SpriteVertex sprite;
					D3DXVECTOR3 vel;
					bool moves;
					float fadeValue;
					float duration;
};
list<ExpState> g_Explosions;

struct Projectile {D3DXVECTOR3 vel;
					float lifetime;
					unsigned int gunId;
					SpriteVertex sprite;
};
list<Projectile> g_Projectiles;

std::vector<GameObject> g_GameObjects;
std::map<std::string, EnemyType> g_EnemyTypes;
std::list<EnemyObject> g_EnemyObjects;

std::string g_SkyboxPath;
ID3D11Texture2D* g_SkyboxTex;
ID3D11ShaderResourceView* g_SkyboxSRV;
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLESPIN          4
#define IDC_RELOAD_SHADERS		101

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
						  void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
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

void InitApp();
void RenderText();

void ReleaseShader();
HRESULT ReloadShader(ID3D11Device* pd3dDevice);

void DeinitApp();
float GenRandom(float min, float max);

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

	// Old Direct3D Documentation:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > Windows DirectX Graphics Documentation

	// DXUT Documentaion:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > DirectX Documentation for C++ : The DirectX Software Development Kit > Programming Guide > DXUT


	// New Direct3D Documentaion (just for reference, use old documentation to find explanations):
	// http://msdn.microsoft.com/en-us/library/windows/desktop/hh309466%28v=vs.85%29.aspx



	// Set DXUT callbacks
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( OnKeyboard );
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
	DXUTCreateWindow( L"TODO: Insert Title Here" ); // You may change the title

	DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 720 );

	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown();
	DeinitApp();

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	HRESULT hr;
	WCHAR path[MAX_PATH];
	
	srand((unsigned int)time(0));

	// Parse the config file

	V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.cfg"));
	ifstream stream(path);
	if(!stream) {
	   MessageBoxA (NULL, "Fatal Error: game.cfg not found.",
			"Missing file", MB_ICONERROR | MB_OK);
		return;
	}

	char MeshName[MAX_PATH] = {'\0'}, T3dPath[MAX_PATH]={'\0'}, DiffusePath[MAX_PATH] = {'\0'}, SpecularPath[MAX_PATH] = {'\0'}, GlowPath[MAX_PATH] = {'\0'}, NormalPath[MAX_PATH] = {'\0'};
	vector<pair<string,int>> spriteFilenames;
	std::string var;
	while(!stream.eof())
	{
		stream >> var;

		if ( var.compare("DebugTexPath")   ==0 ) stream >> g_DebugTexPath;
		if ( var.compare("Spinning")       ==0 ) stream >> g_TerrainSpinning;
		if ( var.compare("SpinSpeed")      ==0 ) stream >> g_TerrainSpinSpeed;
		if ( var.compare("BackgroundColor")==0 ) stream >> g_ClearColor.x >> g_ClearColor.y >> g_ClearColor.z >> g_ClearColor.w;

		// BEGIN: Assignment 3.2.1

		// TODO: Parse the additional information in the config file into the variables:
		// g_TerrainPath, g_TerrainWidth, g_TerrainDepth and g_TerrainHeight
		// HINT: the operator '>>' converts the next word in 'stream' into the format of
		//       its right hand side and is really flexible

		if ( var.compare("TerrainPath")   ==0 ) stream >> g_TerrainPath;
		if ( var.compare("TerrainWidth")   ==0 ) stream >> g_TerrainWidth;
		if ( var.compare("TerrainDepth")   ==0 ) stream >> g_TerrainDepth;
		if ( var.compare("TerrainHeight")   ==0 ) stream >> g_TerrainHeight;
		if ( var.compare("Mesh") ==0 )
		{
			stream >> MeshName >> T3dPath >> DiffusePath >> SpecularPath >> GlowPath >> NormalPath;
			g_Meshes[MeshName] = new Mesh(T3dPath,DiffusePath,SpecularPath,GlowPath,NormalPath);
		}
		if ( var.compare("CockpitObject")	==0 )
		{
			GameObject c;
			c.type = 'c';
			stream >> c.name >> c.scale >> c.rotation.x >> c.rotation.y >> c.rotation.z >> c.translation.x >> c.translation.y >> c.translation.z;
			g_GameObjects.push_back(c);
		}
		if ( var.compare("GroundObject")	==0 )
		{
			GameObject c;
			c.type = 'g';
			stream >> c.name >> c.scale >> c.rotation.x >> c.rotation.y >> c.rotation.z >> c.translation.x >> c.translation.y >> c.translation.z;
			g_GameObjects.push_back(c);
		}
		if ( var.compare("EnemyType")	==0 )
		{
			EnemyType c;
			stream >> MeshName >> c.health >> c.count >> c.speed >> c.mesh.name >> c.mesh.scale >> c.mesh.rotation.x >> c.mesh.rotation.y >> c.mesh.rotation.z >> c.mesh.translation.x >> c.mesh.translation.y >> c.mesh.translation.z;
			g_EnemyTypes[MeshName] = c;
		}
		if ( var.compare("Spawn")	==0 )
		{
			stream>>g_SpawnPreferences.interval>>g_SpawnPreferences.height.x>>g_SpawnPreferences.height.y>>g_SpawnPreferences.x.x>>g_SpawnPreferences.x.y>>g_SpawnPreferences.y.x>>g_SpawnPreferences.y.y>>g_SpawnPreferences.speed.x>>g_SpawnPreferences.speed.y;
			g_SpawnPreferences.lastspawned = 0;
		}
		if ( var.compare("Skybox")	==0 )
		{
			stream>>g_SkyboxPath;
		}
		if ( var.compare("SpriteTexture")	==0 )
		{
			pair<string,int> p;
			stream>>p.first>>p.second;
			spriteFilenames.push_back(p);
		}
		if ( var.compare("Gun")	==0 )
		{
			GunPrefs gp;
			stream>>gp.projectileSpeed>>gp.fireRate>>gp.damage>>gp.gravity>>gp.texId>>gp.texRadius>>gp.texSpawn.x>>gp.texSpawn.y>>gp.texSpawn.z>>gp.fireKey>>gp.name;
			gp.fired = false;
			gp.lastFired = 0;
			gp.animState = 0;
			g_GunPrefs.push_back(gp);
		}
		if ( var.compare("Explosion")	==0 )
		{
			string name;
			ExplPrefs e;
			stream>>name>>e.texId>>e.durationMin>>e.durationMax>>e.particleCountMin>>e.particleCountMax>>e.particleSpeedMin>>e.particleSpeedMax>>e.particleSizeMin>>e.particleSizeMax>>e.fadeMin>>e.fadeMax;
			g_ExplosionPrefs[name] = e;
		}
		// END: Assignment 3.2.1
	}

	stream.close();

	g_SpriteRenderer = new SpriteRenderer(spriteFilenames);

	SpriteVertex sv;
	sv.Position = D3DXVECTOR3(0.f,120.f,0.f);
	sv.Radius=30.f;
	sv.TextureIndex=0;
	g_Sprites.push_back(sv);
	sv.Position = D3DXVECTOR3(50.f,240.f,-40.f);
	sv.Radius=50.f;
	sv.TextureIndex=1;
	g_Sprites.push_back(sv);
	sv.Position = D3DXVECTOR3(-20.f,60.f,80.f);
	sv.Radius=20.f;
	sv.TextureIndex=1;
	g_Sprites.push_back(sv);
	sv.Position = D3DXVECTOR3(200.f,90.f,0.f);
	sv.Radius=25.f;
	sv.TextureIndex=0;
	g_Sprites.push_back(sv);

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
	
	g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
	iY += 24;
	g_SampleUI.AddCheckBox( IDC_TOGGLESPIN, L"Toggle Spinning", 0, iY += 24, 125, 22, g_TerrainSpinning );   

}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	g_TxtHelper->Begin();
	g_TxtHelper->SetInsertionPos( 5, 5 );
	g_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_TxtHelper->DrawTextLine( DXUTGetFrameStats(true)); //DXUTIsVsyncEnabled() ) );
	g_TxtHelper->DrawTextLine( DXUTGetDeviceStats() );
	g_TxtHelper->End();
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

	V_RETURN( ReloadShader(pd3dDevice) );
	
	errno_t error;
	WCHAR path[MAX_PATH];
	stringstream ss;
	wstringstream wss;      
	

	// Load the debug texture and create a shader resource view
	wss.str(L""); wss << g_DebugTexPath;
	V(DXUTFindDXSDKMediaFileCch( path, MAX_PATH, wss.str().c_str()));
	if (hr != S_OK) {
		ss.str();
		ss << "Could not find '" << g_DebugTexPath << "'";
		MessageBoxA (NULL, ss.str().c_str(), "Missing file", MB_ICONERROR | MB_OK);
		return hr;
	}
	V(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, path, NULL, NULL, &g_DebugSRV, &hr));
	if (hr != S_OK) {
		ss.str();
		ss << "Could not load texture '" << g_DebugTexPath << "'";
		MessageBoxA (NULL, ss.str().c_str(), "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}

	// BEGIN: Assignment 3.2.2

	// TODO: Uncomment

	// Find the terrain file
	wss.str(L""); wss << g_TerrainPath;
	V(DXUTFindDXSDKMediaFileCch( path, MAX_PATH, wss.str().c_str()));
	if (hr != S_OK) {
		ss.str();
		ss << "Could not find '" << g_TerrainPath << "'";
		MessageBoxA (NULL, ss.str().c_str(), "Missing file", MB_ICONERROR | MB_OK);
		return hr;
	}

	// Open the terrain file
	FILE* file;
	error = _wfopen_s(&file, path, L"rb");
	if (file == nullptr) {
		ss.str();
		ss << "Could not open '" << g_TerrainPath << "'";
		MessageBoxA (NULL, ss.str().c_str(), "File error", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// Read the terrain header
	{
		auto r = fread (&g_TerrainHeader, sizeof (g_TerrainHeader), 1, file);
		if (r != 1) {
			MessageBoxA (NULL, "Could not read the header.",
				"Invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}
	}

	// Check the magic number
	if (g_TerrainHeader.magicNumber != 0x00DA) {
		MessageBoxA (NULL, "The magic number is incorrect.",
			"Invalid terrain file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// Check the version
	if (g_TerrainHeader.version != 1) {
		MessageBoxA (NULL, "The header version is incorrect.",
			"Invalid terrain file header", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	// Calculate the terrain resoultion from the height size
	g_TerrainResolution = (int)sqrt(g_TerrainHeader.heightSize / 2.0); // Assume a square terrain
	g_TerrainNumVertices = g_TerrainResolution * g_TerrainResolution;
	g_TerrainNumTriangles = 2 * (g_TerrainResolution - 1) * (g_TerrainResolution - 1) ;
	assert((g_TerrainHeader.heightSize / 2) == g_TerrainNumVertices);

	// Read the terrain heights
	std::vector<unsigned short> terrainHeights;
	terrainHeights.resize(g_TerrainNumVertices);
	{
		if (g_TerrainHeader.heightSize != (int)::fread (&terrainHeights[0], sizeof(BYTE), g_TerrainHeader.heightSize, file)) {
			MessageBoxA(NULL, "Error while reading height data.",
				"Invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}

	// Read the terrain color texture for diffuse lighting
	std::vector<unsigned char> terrainDiffuseNtx;
	terrainDiffuseNtx.resize(g_TerrainHeader.colorSize);
	{
		const auto requestedSize = terrainDiffuseNtx.size();
		if (requestedSize != fread (&terrainDiffuseNtx[0], sizeof(unsigned char), requestedSize, file)) {
			MessageBoxA (NULL, "Error while reading color data.",
				"Invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}    
	
	std::vector<unsigned char> terrainNormalNtx;
	terrainNormalNtx.resize(g_TerrainHeader.normalSize);
	{
		const auto requestedSize = terrainNormalNtx.size();
		if (requestedSize != fread (&terrainNormalNtx[0], sizeof(unsigned char), requestedSize, file)) {
			MessageBoxA (NULL, "Error while reading color data.",
				"Invalid terrain file", MB_ICONERROR | MB_OK);
			return E_FAIL;
		}    
	}

	fclose(file);

	vector<uint8_t> data;
	FILE * filePointer = NULL;
	error = fopen_s(&filePointer, g_SkyboxPath.c_str(), "rb");
	if (error) 	{ return E_INVALIDARG; }
	fseek(filePointer, 0, SEEK_END);
	long bytesize = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);
	data.resize(bytesize);
	fread(&data[0], 1, bytesize, filePointer);
	fclose(filePointer);

	// END: Assignment 3.2.2

	// Initialize the camera
	D3DXVECTOR3 Eye( 0.0f, terrainHeights[g_TerrainResolution*g_TerrainResolution/2+g_TerrainResolution/2]*1.f/UINT16_MAX * g_TerrainHeight + 40.f, 0.0f );
	D3DXVECTOR3 At( 0.5f * g_TerrainDepth, terrainHeights[g_TerrainResolution*g_TerrainResolution/2+g_TerrainResolution/2]*1.f/UINT16_MAX * g_TerrainHeight + 90.f, 0.0f );
	g_Camera.SetViewParams( &Eye, &At ); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_Camera.SetScalers( g_CameraRotateScaler, g_CameraMoveScaler );

	// BEGIN: Assignment 4.2.6
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

	

	// BEGIN: Assignment 3.2.5 

	// TODO: Create the terrain color texture

	D3D11_TEXTURE2D_DESC tex2DDesc;
	std::vector<std::vector<unsigned char>> textureData;
	std::vector<D3D11_SUBRESOURCE_DATA> subresourceData;
	bool sRgb;

	LoadNtx(terrainDiffuseNtx,&tex2DDesc,textureData,subresourceData,sRgb);
	pd3dDevice->CreateTexture2D(&tex2DDesc,&subresourceData[0],&g_TerrainDiffuseTex);
	pd3dDevice->CreateShaderResourceView(g_TerrainDiffuseTex,NULL,&g_TerrainDiffuseSRV);
	
	LoadNtx(terrainNormalNtx,&tex2DDesc,textureData,subresourceData,sRgb);
	pd3dDevice->CreateTexture2D(&tex2DDesc,&subresourceData[0],&g_TerrainNormalTex);
	pd3dDevice->CreateShaderResourceView(g_TerrainNormalTex,NULL,&g_TerrainNormalSRV);

	LoadNtx(data,&tex2DDesc,textureData,subresourceData,sRgb);
	pd3dDevice->CreateTexture2D(&tex2DDesc,&subresourceData[0],&g_SkyboxTex);
	pd3dDevice->CreateShaderResourceView(g_SkyboxTex,NULL,&g_SkyboxSRV);
	
	// END: Assignment 3.2.5

	for(auto i=g_Meshes.begin();i!=g_Meshes.end();i++)
	{
		i->second->CreateResources(pd3dDevice);
	}
	T3d::CreateT3dInputLayout(pd3dDevice,g_Pass1_Mesh,&g_MeshInputLayout);

	g_SpriteRenderer->CreateResources(pd3dDevice);

	D3D11_TEXTURE2D_DESC shadowDesc;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.Height = 2048;
	shadowDesc.Width = 2048;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;

	pd3dDevice->CreateTexture2D(&shadowDesc,NULL,&g_ShadowTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVdesc;
	shadowSRVdesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVdesc.Texture2D.MipLevels = 1;
	shadowSRVdesc.Texture2D.MostDetailedMip = 0;
	shadowSRVdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	pd3dDevice->CreateShaderResourceView(g_ShadowTex,&shadowSRVdesc,&g_ShadowSRV);

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVdesc;
	shadowDSVdesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSVdesc.Flags = 0;
	shadowDSVdesc.Texture2D.MipSlice = 0;
	shadowDSVdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	pd3dDevice->CreateDepthStencilView(g_ShadowTex,&shadowDSVdesc,&g_ShadowDSV);

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
	SAFE_RELEASE( g_DebugSRV );
	
	SAFE_RELEASE( g_TerrainNormalTex );
	SAFE_RELEASE( g_TerrainNormalSRV );
	SAFE_RELEASE( g_SkyboxTex );
	SAFE_RELEASE( g_SkyboxSRV );
	SAFE_RELEASE( g_TerrainHeightBuf );
	SAFE_RELEASE( g_TerrainHeightSRV );

	// Assignment 3.2.5
	// TODO: Release the terrain's shader resource view and texture
	SAFE_RELEASE( g_TerrainDiffuseTex );
	SAFE_RELEASE( g_TerrainDiffuseSRV );
	SAFE_RELEASE( g_MeshInputLayout );
	
	SAFE_RELEASE( g_ShadowTex );
	SAFE_RELEASE( g_ShadowSRV );
	SAFE_RELEASE( g_ShadowDSV );

	SAFE_DELETE( g_TxtHelper );
	ReleaseShader();
	for(auto i=g_Meshes.begin();i!=g_Meshes.end();i++)
	{
		i->second->ReleaseResources();
	}

	g_SpriteRenderer->ReleaseResources();
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
	g_Camera.SetRotateButtons(true, false, false);
	g_Camera.SetScalers( g_CameraRotateScaler, g_CameraMoveScaler );
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

	// Obtain the effect pass
	SAFE_GET_PASS(g_Technique, "P0", g_Pass0);
	SAFE_GET_PASS(g_Technique, "P1_Mesh", g_Pass1_Mesh);

	// Obtain the effect variables
	SAFE_GET_RESOURCE(g_Effect, "g_Diffuse", g_DiffuseEV);
	SAFE_GET_MATRIX(g_Effect, "g_World", g_WorldEV);
	SAFE_GET_MATRIX(g_Effect, "g_WorldViewProjection", g_WorldViewProjectionEV);   
	SAFE_GET_VECTOR(g_Effect, "g_LightDir", g_LightDirEV);   
	SAFE_GET_RESOURCE(g_Effect, "g_Height", g_HeightEV);
	SAFE_GET_RESOURCE(g_Effect, "g_Normal", g_NormalEV);
	SAFE_GET_SCALAR(g_Effect, "g_TerrainRes", g_TerrainResEV);
	SAFE_GET_SCALAR(g_Effect, "g_TerrainQuadRes", g_TerrainQuadResEV);
	SAFE_GET_RESOURCE(g_Effect, "g_Specular", g_SpecularEV);
	SAFE_GET_RESOURCE(g_Effect, "g_Glow", g_GlowEV);
	SAFE_GET_MATRIX(g_Effect, "g_WorldView", g_WorldViewEV);
	SAFE_GET_MATRIX(g_Effect, "g_WorldViewNormals", g_WorldViewNormalsEV);
	SAFE_GET_VECTOR(g_Effect, "g_LightDirView", g_LightDirViewEV);

	g_SpriteRenderer->ReloadShader(pd3dDevice);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	SAFE_RELEASE( g_Effect );

	g_SpriteRenderer->ReleaseShader();
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
	if ( nChar=='C' ) { g_Camera.SetEnablePositionMovement(true); }

	for(auto i = g_GunPrefs.begin();i!=g_GunPrefs.end();i++)
	{
		if(nChar==i->fireKey)
		{
			i->fired = bKeyDown;
			break;
		}
	}
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
	}
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
D3DXMATRIX worldRotation;
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// Update the camera's position based on user input 
	g_Camera.FrameMove( fElapsedTime );
	
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
	
	if( g_TerrainSpinning ) {
		D3DXMatrixRotationY( &worldRotation, g_TerrainSpinSpeed * DEG2RAD((float)fTime) );
		g_TerrainWorld *= worldRotation; // Rotate around world-space "up" axis
	}

	g_LightDir = D3DXVECTOR4(1, 1, -1, 0); // Direction to the directional light in world space    

	// Transform the light vector to terrain object space
	D3DXMATRIX invWorld;
	D3DXMatrixInverse(&invWorld, NULL, &g_TerrainWorld);
	D3DXVec4Transform(&g_LightDir, &g_LightDir, &invWorld);

	D3DXVec3Normalize((D3DXVECTOR3*)&g_LightDir, (D3DXVECTOR3*)&g_LightDir); // Normalize the light direction

	if(fTime - g_SpawnPreferences.lastspawned>=g_SpawnPreferences.interval)
	{
		EnemyObject e;
		auto eTypes = g_EnemyTypes.begin();
		for(int i=(int)GenRandom(0,(float)g_EnemyTypes.size());i>0;--i) eTypes++;
		if(eTypes==g_EnemyTypes.end()) eTypes--;
		e.type = eTypes->first;
		e.hitpoints = eTypes->second.health;
		float a = GenRandom(0.f,2.f*D3DX_PI);
		float r = GenRandom(g_TerrainWidth/2.f * g_SpawnPreferences.x.x, g_TerrainWidth/2.f * g_SpawnPreferences.x.y);
		e.pos = D3DXVECTOR3(r*std::sin(a),
							GenRandom(g_TerrainHeight * g_SpawnPreferences.height.x, g_TerrainHeight * g_SpawnPreferences.height.y),
							r*std::cos(a));
		a = GenRandom(0.f,2.f*D3DX_PI);
		r = GenRandom(g_TerrainWidth/2.f * g_SpawnPreferences.y.x, g_TerrainWidth/2.f * g_SpawnPreferences.y.y);
		D3DXVec3Normalize(&e.vel,&D3DXVECTOR3(r*std::sin(a)-e.pos.x,
							GenRandom(g_TerrainHeight * g_SpawnPreferences.height.x, g_TerrainHeight * g_SpawnPreferences.height.y)-e.pos.y,
							r*std::cos(a)-e.pos.z));
		e.vel *= eTypes->second.speed;

		g_EnemyObjects.push_back(e);
		g_SpawnPreferences.lastspawned = fTime;
	}

	for(auto i=g_EnemyObjects.begin();i!=g_EnemyObjects.end();i++)
	{
		i->vel += i->vel*(GenRandom(g_SpawnPreferences.speed.x,g_SpawnPreferences.speed.y)-1)*fElapsedTime;
		i->pos += i->vel*fElapsedTime;
	}

	for(auto i=g_GunPrefs.begin();i!=g_GunPrefs.end();i++)
	{
		if(i->fired&&fTime-i->lastFired>=i->fireRate)
		{
			Projectile p;
			D3DXVec3TransformCoord(&p.sprite.Position,&i->texSpawn,g_Camera.GetWorldMatrix());
			p.sprite.TextureIndex = g_SpriteRenderer->GetTextureOffset(i->texId);
			p.sprite.Radius = i->texRadius;
			p.lifetime = 10.f;
			D3DXVec3Normalize(&p.vel,g_Camera.GetWorldAhead());
			p.vel *= i->projectileSpeed;
			p.gunId = i - g_GunPrefs.begin();
			p.sprite.AnimationProgress = 0;
			p.sprite.AnimationSize = g_SpriteRenderer->GetAnimationSize(i->texId);
			p.sprite.Opacity = 1.f;
			g_Projectiles.push_front(p);
			i->lastFired = fTime;
			i->fired = false;
			if(i->name.compare("GatlingGun")==0)
			{
				if(i->animState<0.2f) i->animState += 1.6f * fElapsedTime;
			}
		}
		else if(fTime-i->lastFired<=0.2f)
		{
			if(i->name.compare("PlasmaGun")==0)
			{
				if(i->animState<13.f) i->animState += 60.f * fElapsedTime;
			}
		}
		else
		{
			if(i->name.compare("GatlingGun")==0)
			{
				i->animState = (i->animState<=0) ? 0 : i->animState - 0.25f * fElapsedTime;
			}
			else if(i->name.compare("PlasmaGun")==0)
			{
				i->animState = (i->animState<=0) ? 0 : i->animState - 10.f * fElapsedTime;
			}
		}
	}

	for(auto i=g_Explosions.begin();i!=g_Explosions.end();i++)
	{
		i->sprite.AnimationProgress += fElapsedTime / i->duration;
		i->sprite.Opacity = 1.f - abs(i->sprite.AnimationProgress - 0.5f) * (1.f - i->fadeValue) * 2.f;
		if(i->moves)
		{
			i->vel -= i->vel * i->sprite.AnimationProgress * fElapsedTime * 20.f;
			i->sprite.Position += i->vel * fElapsedTime;
		}
	}

	for(auto i=g_Projectiles.begin();i!=g_Projectiles.end();i++)
	{
		i->vel += D3DXVECTOR3(0.f,-9.81f,-g_GunPrefs[i->gunId].gravity)*fElapsedTime;
		i->sprite.Position += i->vel*fElapsedTime;
		i->lifetime -= fElapsedTime;
		for(auto j=g_EnemyObjects.begin();j!=g_EnemyObjects.end();j++)
		{
			D3DXVECTOR3 diff = j->pos - i->sprite.Position;
			if(D3DXVec3Length(&diff)*D3DXVec3Length(&D3DXVECTOR3(diff.z,diff.y,diff.x))<=pow(g_EnemyTypes[j->type].count-i->sprite.Radius,2))
			{
				j->hitpoints -= g_GunPrefs[i->gunId].damage;
				i->lifetime = 0;
				if(j->hitpoints<=0)
				{
					ExpState e;
					auto explTypes = g_ExplosionPrefs.begin();
					for(int k=(int)GenRandom(0,(float)g_ExplosionPrefs.size());k>0;--k) explTypes++;
					if(explTypes==g_ExplosionPrefs.end()) explTypes--;
					e.explName = explTypes->first;
					e.sprite.Position = j->pos;
					e.sprite.Radius = g_EnemyTypes[j->type].count * 2.f;
					e.sprite.TextureIndex = g_SpriteRenderer->GetTextureOffset(g_ExplosionPrefs[e.explName].texId);
					e.sprite.Opacity = 1.f;
					e.sprite.AnimationProgress = 0.f;
					e.sprite.AnimationSize = g_SpriteRenderer->GetAnimationSize(g_ExplosionPrefs[e.explName].texId);
					e.moves = false;
					e.duration = GenRandom(g_ExplosionPrefs[e.explName].durationMin,g_ExplosionPrefs[e.explName].durationMax);
					e.fadeValue = GenRandom(g_ExplosionPrefs[e.explName].fadeMin,g_ExplosionPrefs[e.explName].fadeMax);
					g_Explosions.push_front(e);
					int particleCount = (int)GenRandom((float)g_ExplosionPrefs[e.explName].particleCountMin,(float)g_ExplosionPrefs[e.explName].particleCountMax);
					for(int k=0;k<particleCount;k++)
					{
						ExpState p;
						explTypes = g_ExplosionPrefs.begin();
						for(int l=(int)GenRandom(0,(float)g_ExplosionPrefs.size());l>0;--l) explTypes++;
						if(explTypes==g_ExplosionPrefs.end()) explTypes--;
						p.explName = explTypes->first;
						p.sprite.Position = j->pos;
						p.sprite.Radius = GenRandom(g_ExplosionPrefs[e.explName].particleSizeMin,g_ExplosionPrefs[e.explName].particleSizeMax);
						p.moves = true;
						float o = GenRandom(0,D3DX_PI);
						float d = GenRandom(0,2*D3DX_PI);
						p.vel = D3DXVECTOR3(sin(o)*cos(d),sin(o)*cos(d),cos(o));
						p.vel *= GenRandom(g_ExplosionPrefs[e.explName].particleSpeedMin,g_ExplosionPrefs[e.explName].particleSpeedMax);
						p.sprite.Opacity = 1.f;
						p.sprite.AnimationProgress = 0.f;
						p.sprite.AnimationSize = g_SpriteRenderer->GetAnimationSize(g_ExplosionPrefs[p.explName].texId);
						p.sprite.TextureIndex = g_SpriteRenderer->GetTextureOffset(g_ExplosionPrefs[p.explName].texId);
						p.duration = GenRandom(g_ExplosionPrefs[p.explName].durationMin,g_ExplosionPrefs[p.explName].durationMax);
						p.fadeValue = GenRandom(g_ExplosionPrefs[p.explName].fadeMin,g_ExplosionPrefs[p.explName].fadeMax);
						g_Explosions.push_front(p);
					}
				}
			}
		}
	}
	
	g_EnemyObjects.remove_if([](EnemyObject e){return abs(e.pos.x)>g_TerrainWidth/2*g_SpawnPreferences.x.y*1.5f
													||abs(e.pos.z)>g_TerrainDepth/2*g_SpawnPreferences.x.y*1.5f
													||e.pos.y<g_TerrainHeight*g_SpawnPreferences.height.x*0.4f
													||e.hitpoints<=0;});
	g_Projectiles.remove_if([](Projectile p){return p.lifetime<=0||p.sprite.Position.y<=-2.f;});
	g_Explosions.remove_if([](ExpState e){return e.sprite.AnimationProgress>1.f;});
}

void RenderScene(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
		float fElapsedTime, void* pUserContext, const D3DXMATRIX* view, const D3DXMATRIX* proj, ID3DX11EffectTechnique* technique, bool renderCockpit)
{
	HRESULT hr;

	D3DXMATRIX inverseTerrainWorldDir;
	D3DXMatrixTranspose(&inverseTerrainWorldDir, &g_TerrainWorld);
	D3DXVECTOR4 lightDirObj;
	D3DXVec4Transform(&lightDirObj,&g_LightDir,&inverseTerrainWorldDir);
	D3DXVec3Normalize((D3DXVECTOR3*)&lightDirObj,(D3DXVECTOR3*)&lightDirObj);

	D3DXMATRIX worldViewProj = g_TerrainWorld * (*view) * (*proj);
	g_WorldEV->SetMatrix( ( float* )&g_TerrainWorld );
	g_WorldViewProjectionEV->SetMatrix( ( float* )&worldViewProj );
	g_LightDirEV->SetFloatVector( ( float* )&lightDirObj );

	pd3dImmediateContext->IASetInputLayout( NULL );
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* vbs[] = { NULL, };
	unsigned int strides[] = { 0, }, offsets[] = { 0, };
	pd3dImmediateContext->IASetVertexBuffers(0,1,vbs,strides,offsets);

	V(g_DiffuseEV->SetResource( g_TerrainDiffuseSRV));
	V(g_HeightEV->SetResource( g_TerrainHeightSRV));
	V(g_NormalEV->SetResource( g_TerrainNormalSRV));
	V(g_TerrainResEV->SetInt( g_TerrainResolution ));
	V(g_TerrainQuadResEV->SetInt( g_TerrainResolution-1 ));
	
	technique->GetPassByName("P0")->Apply(0, pd3dImmediateContext);
	pd3dImmediateContext->Draw((g_TerrainResolution-1)*(g_TerrainResolution-1)*6,0);
	
	pd3dImmediateContext->IASetInputLayout( g_MeshInputLayout );
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	D3DXVECTOR4 lightDirView;
	D3DXMATRIX cameraView;
	D3DXMatrixInverse(&cameraView,0,g_Camera.GetViewMatrix());
	D3DXMatrixTranspose(&cameraView,&cameraView);

	D3DXVec4Transform(&lightDirView,&g_LightDir,&cameraView);
	D3DXVec3Normalize((D3DXVECTOR3*)&lightDirView,(D3DXVECTOR3*)&lightDirView);

	V(g_LightDirViewEV->SetFloatVector(lightDirView));

	for(auto i = g_GameObjects.begin();i!=g_GameObjects.end();i++)
	{
		if(i->type!='g'&&!renderCockpit) continue;
		Mesh* oM = g_Meshes[i->name];
		vbs[0] = oM->GetVertexBuffer();
		strides[0] = sizeof(struct T3dVertex);
		pd3dImmediateContext->IASetVertexBuffers(0,1,vbs,strides,offsets);
		pd3dImmediateContext->IASetIndexBuffer(oM->GetIndexBuffer(),oM->GetIndexFormat(),0);
		V(g_DiffuseEV->SetResource(oM->GetDiffuseSRV()));
		V(g_SpecularEV->SetResource(oM->GetSpecularSRV()));
		V(g_GlowEV->SetResource(oM->GetGlowSRV()));
		V(g_NormalEV->SetResource(oM->GetNormalSRV()));

		D3DXMATRIX mTrans, mScale, mRotX, mRotY, mRotZ;

		if(i->name.compare("GatlingGun_top")==0)
		{
			i->rotation.z -= g_GunPrefs[0].animState;
		}

		
		if(i->name.compare("PlasmaGun_top")==0)
		{
			D3DXMatrixTranslation(&mTrans, i->translation.x, i->translation.y, i->translation.z - g_GunPrefs[1].animState);
		}
		else
		{
			D3DXMatrixTranslation(&mTrans, i->translation.x, i->translation.y, i->translation.z);
		}

		D3DXMatrixRotationX(&mRotX, i->rotation.x);
		D3DXMatrixRotationY(&mRotY, i->rotation.y);
		D3DXMatrixRotationZ(&mRotZ, i->rotation.z);
		D3DXMatrixScaling(&mScale, i->scale, i->scale, i->scale);

		D3DXMATRIX worldView = mScale * mRotX * mRotY * mRotZ * mTrans;
		worldViewProj = worldView;
		if(i->type=='g') 
		{
				if(g_TerrainSpinning) {
					worldView *= worldRotation;
					worldViewProj *= worldRotation;
				}
				worldViewProj *= (*view);
		}
		worldViewProj *= (*proj);
		if(i->type!='g') 
		{
			worldView *= *g_Camera.GetWorldMatrix();
		}
		D3DXMATRIX worldViewNormals;
		D3DXMatrixInverse(&worldViewNormals,0,&worldView);
		D3DXMatrixTranspose(&worldViewNormals,&worldViewNormals);
	
		V(g_WorldViewEV->SetMatrix(worldView));
		V(g_WorldViewProjectionEV->SetMatrix(worldViewProj));
		V(g_WorldViewNormalsEV->SetMatrix(worldViewNormals));

		technique->GetPassByName("P1_Mesh")->Apply(0, pd3dImmediateContext);
		pd3dImmediateContext->DrawIndexed(oM->GetIndexCount(),0,0);
	}

	for each(EnemyObject e in g_EnemyObjects)
	{
		GameObject mesh = g_EnemyTypes[e.type].mesh;
		Mesh* eM = g_Meshes[mesh.name];
		vbs[0] = eM->GetVertexBuffer();
		strides[0] = sizeof(struct T3dVertex);
		pd3dImmediateContext->IASetVertexBuffers(0,1,vbs,strides,offsets);
		pd3dImmediateContext->IASetIndexBuffer(eM->GetIndexBuffer(),eM->GetIndexFormat(),0);
		V(g_DiffuseEV->SetResource(eM->GetDiffuseSRV()));
		V(g_SpecularEV->SetResource(eM->GetSpecularSRV()));
		V(g_GlowEV->SetResource(eM->GetGlowSRV()));
		V(g_NormalEV->SetResource(eM->GetNormalSRV()));

		D3DXMATRIX mTrans, mScale, mRotX, mRotY, mRotZ;
		D3DXMatrixRotationX(&mRotX, mesh.rotation.x);
		D3DXMatrixRotationY(&mRotY, mesh.rotation.y);
		D3DXMatrixRotationZ(&mRotZ, mesh.rotation.z);
		D3DXMatrixTranslation(&mTrans, mesh.translation.x, mesh.translation.y, mesh.translation.z);
		D3DXMatrixScaling(&mScale, mesh.scale, mesh.scale, mesh.scale);

		D3DXVECTOR3 d = e.vel/D3DXVec3Length(&e.vel);
		D3DXMATRIX mAnim = D3DXMATRIX(d.x, d.y, d.z, 0,
									0, 1, 0, 0,
									-d.z, d.y, d.x, 0,
									e.pos.x, e.pos.y, e.pos.z, 1);

		D3DXMATRIX worldView = mScale * mRotX * mRotY * mRotZ * mTrans * mAnim;
		worldViewProj = worldView * (*view) * (*proj);

		D3DXMATRIX worldViewNormals;
		D3DXMatrixInverse(&worldViewNormals,0,&worldView);
		D3DXMatrixTranspose(&worldViewNormals,&worldViewNormals);
	
		V(g_WorldViewEV->SetMatrix(worldView));
		V(g_WorldViewProjectionEV->SetMatrix(worldViewProj));
		V(g_WorldViewNormalsEV->SetMatrix(worldViewNormals));

		technique->GetPassByName("P1_Mesh")->Apply(0, pd3dImmediateContext);
		pd3dImmediateContext->DrawIndexed(eM->GetIndexCount(),0,0);
	}
}
//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
		float fElapsedTime, void* pUserContext )
{
	HRESULT hr;

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.OnRender( fElapsedTime );
		return;
	}     

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, g_ClearColor );
	pd3dImmediateContext->ClearDepthStencilView(g_ShadowDSV,D3D11_CLEAR_DEPTH,1.f,0);
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	if(g_Effect == NULL) {
		g_TxtHelper->Begin();
		g_TxtHelper->SetInsertionPos( 5, 5 );
		g_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		g_TxtHelper->DrawTextLine( L"SHADER ERROR" );
		g_TxtHelper->End();
		return;
	}

	ID3D11RenderTargetView* oldTargets;
	ID3D11DepthStencilView* oldDSV;
	pd3dImmediateContext->OMGetRenderTargets(1,&oldTargets,&oldDSV);

	ID3D11RenderTargetView* newRTV = {NULL,};
	pd3dImmediateContext->OMSetRenderTargets(1,&newRTV,g_ShadowDSV);

	D3D11_VIEWPORT viewport;
	viewport.Height = 2048;
	viewport.Width = 2048;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	D3D11_VIEWPORT* newViewports = {&viewport,};

	D3D11_VIEWPORT oldViewport = {NULL,};
	UINT numViewports = {1,};
	pd3dImmediateContext->RSGetViewports(&numViewports,&oldViewport);
	
	pd3dImmediateContext->RSSetViewports(1,newViewports);

	D3DXVECTOR3 boundingDiff = D3DXVECTOR3(g_TerrainWidth,g_TerrainHeight,g_TerrainDepth)-D3DXVECTOR3(-g_TerrainWidth,0.f,-g_TerrainDepth);
	float boundingBoxDiagonal = D3DXVec3Length(&boundingDiff);

	D3DXMATRIX orthoProj;
	D3DXMatrixOrthoLH(&orthoProj,boundingBoxDiagonal,boundingBoxDiagonal,0.0001f,boundingBoxDiagonal);

	D3DXMATRIX mLightView;
	D3DXVECTOR4 vLightDir = g_LightDir*boundingBoxDiagonal*0.5f;
	D3DXMatrixLookAtLH(&mLightView,(D3DXVECTOR3*)&vLightDir,&D3DXVECTOR3(0.f,0.f,0.f),&D3DXVECTOR3(0.f,1.f,0.f));

	RenderScene(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&mLightView,&orthoProj,g_Effect->GetTechniqueByName("Shadow"),false);
	
	pd3dImmediateContext->RSSetViewports(1,&oldViewport);
	pd3dImmediateContext->OMSetRenderTargets(1,&oldTargets,oldDSV);
	
	pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, g_ClearColor );
	pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
			
	D3DXMATRIX const * view = g_Camera.GetViewMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	D3DXMATRIX const * proj = g_Camera.GetProjMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147302%28v=vs.85%29.aspx
	
	D3DXMATRIX viewProj = (*view) * (*proj);
	D3DXMatrixInverse(&viewProj,0,&viewProj);
	
	D3DXVECTOR3 tL;
	D3DXVec3TransformCoord(&tL, &D3DXVECTOR3(-1,1,1), &viewProj);
	D3DXVECTOR3 tR;
	D3DXVec3TransformCoord(&tR, &D3DXVECTOR3(1,1,1), &viewProj);
	D3DXVECTOR3 bL;
	D3DXVec3TransformCoord(&bL, &D3DXVECTOR3(-1,-1,1), &viewProj);
	
	D3DXVECTOR3 down = bL - tL;
	D3DXVECTOR3 right = tR - tL;

	V(g_Effect->GetVariableByName("g_Skybox")->AsShaderResource()->SetResource(g_SkyboxSRV));
	V(g_Effect->GetVariableByName("g_Eye")->AsVector()->SetFloatVector(*g_Camera.GetEyePt()));
	V(g_Effect->GetVariableByName("g_TopLeft")->AsVector()->SetFloatVector(tL));
	V(g_Effect->GetVariableByName("g_Right")->AsVector()->SetFloatVector(right));
	V(g_Effect->GetVariableByName("g_Down")->AsVector()->SetFloatVector(down));
	
	pd3dImmediateContext->IASetInputLayout( NULL ); 
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); 
	g_Effect->GetTechniqueByName("Render")->GetPassByName("P2_Skybox")->Apply(0, pd3dImmediateContext); 
	pd3dImmediateContext->Draw(4, 0);

	D3DXMATRIX lViewProj = mLightView * orthoProj;
	V(g_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(g_ShadowSRV));
	V(g_Effect->GetVariableByName("g_mLightViewProj")->AsMatrix()->SetMatrix(lViewProj));
	RenderScene(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,view,proj,g_Effect->GetTechniqueByName("Render"),true);
	
	if(g_Projectiles.size()>0||g_Explosions.size()>0)
	{
		vector<SpriteVertex> sprites;
		for each(Projectile p in g_Projectiles)
		{
			sprites.push_back(p.sprite);
		}

		for each(ExpState e in g_Explosions)
		{
			sprites.push_back(e.sprite);
		}

		sort(sprites.begin(),sprites.end(),[](SpriteVertex a,SpriteVertex b)->bool {
			double aD = D3DXVec3Dot(&a.Position,g_Camera.GetWorldAhead());
			double bD = D3DXVec3Dot(&b.Position,g_Camera.GetWorldAhead());
			return aD > bD;
		});
		g_SpriteRenderer->RenderSprites(pd3dDevice,sprites,g_Camera);
	}
	
	SAFE_RELEASE(oldDSV);
	SAFE_RELEASE(oldTargets);
	g_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(0);

	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
	g_HUD.OnRender( fElapsedTime );
	g_SampleUI.OnRender( fElapsedTime );
	RenderText();
	DXUT_EndPerfEvent();

	static DWORD dwTimefirst = GetTickCount();
	if ( GetTickCount() - dwTimefirst > 5000 )
	{    
		OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
		OutputDebugString( L"\n" );
		//wstringstream ss;
		//ss<<"("<<tL.x<<","<<tL.y<<","<<tL.z<<")";
		//OutputDebugString(ss.str().c_str());

		dwTimefirst = GetTickCount();
	}
}

void DeinitApp()
{
	for(auto i=g_Meshes.begin();i!=g_Meshes.end();i++)
	{
		SAFE_DELETE(i->second);
	}

	SAFE_DELETE(g_SpriteRenderer);
}

float GenRandom(float min, float max)
{
	return min + (rand()/(float)RAND_MAX)*(max-min);
}
