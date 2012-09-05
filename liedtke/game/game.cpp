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

#include "TerrainRenderer.h"
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
#include "GameObject.h"
#include "Enemy.h"
#include "SphereCollider.h"
#include "gcProjectile.h"
#include "gcMass.h"
#include "Macros.h"
#include "ParticleSystem.h"

#include "debug.h"

using namespace std;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
////Skybox
//struct SKYBOX_VERTEX
//{
//	D3DXVECTOR4 pos;
//};

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
static int								g_PlayerPoints = 0;

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
ID3DX11EffectTechnique*					g_BillboardTechnique = NULL;

ID3D11Texture2D*						g_ShadowMap;
ID3D11ShaderResourceView*				g_ShadowMapSRV;
ID3D11DepthStencilView*					g_ShadowMapDSV;
ID3DX11EffectShaderResourceVariable*	g_ShadowMapEV;



// Background color
D3DXVECTOR4                             g_ClearColor;
//5.2.5
TerrainRenderer*		g_TerrainRenderer = NULL;
MeshRenderer*			g_MeshRenderer = NULL;
SpriteRenderer*			g_SpriteRenderer = NULL;
Skybox*					g_SkyboxRenderer = NULL;

map<string, Enemy*> g_EnemyTyp;
//enthält alle GameObjects, die dauerhaft im Spiel sind
vector<GameObject*> g_StaticGameObjects;
map<string, int> g_ObjectReferences;
vector<TerrainObject*> g_TerrainObjects;
list<Enemy*> g_EnemyInstances;
vector<string> g_EnemyTypeNames;
map<string, GameObject*>  g_ProjectileTypes;
vector<WeaponType>		g_WeaponTypes;
//SpriteRenderer
vector<pair<string, int>>			spriteFileNames;
//vector<SpriteVertex>		g_SpritesToRender;
list<GameObject*> g_Particles;
list<DISPLAYTEXT> g_DisplayTEXTBOX;
list<DISPLAYTEXT> g_DisplayTextLeft;
list<DISPLAYTEXT> g_DisplayTextRight;
std::list<ParticleSystem*> g_activeParticleSystems;
//map<string, ParticleEffect> g_ParticleEffects;
//list<ParticleEffect> g_ActiveEffects;

//SpawnParameters
double g_LastSpawn = 0;
float g_MinHeight;
float g_MaxHeight;
float g_MinCircle = 0.7f;
float g_MaxCircle = 1.4f;
float g_SpawnIntervall;
float g_SpawnCircle;
float g_TargetCircle;
float g_DestroyCircle;
//Player
bool p_Fire1;
bool p_Fire2;

//OnKey
bool Omove = false;
bool ORotate = false;
bool OScale = false;
int Oindex = 2;
bool useDeveloperFeatures = true;


CHAR									g_SkyboxPath[MAX_PATH] = {'\0'};
bool									g_UseSkybox = false;
char									g_TerrainPath[MAX_PATH] = { '\0' };
float                                   g_TerrainWidth;
float                                   g_TerrainDepth;
float                                   g_TerrainHeight;
bool                                    g_TerrainSpinning;
float                                   g_TerrainSpinSpeed;
float									g_BoundingBoxDiagonal = 0.f;
string									g_ResourcesPath;
D3DXMATRIX								g_ViewProj;
//D3DXMATRIX                              g_TerrainWorld; // object- to world-space transformation
//D3DXMATRIX								g_MeshWorld;
D3DXVECTOR3								g_CameraPos;
D3DXVECTOR3								g_CameraLookAt;
string									g_SolutionDir;
//SpriteVertex							pSun;
SpriteVertex							g_Radar;
float									pow2Border;


	//D3DXMATRIX worldViewNormals;
	//D3DXMATRIX mTrans, mScale, mRot;
	//D3DXMATRIX inverseTerrainWorldDir;
	//D3DXVECTOR4 lightDirObj;
	D3DXMATRIX lightProjektionMatrix;
	D3DXMATRIX lightViewMatrix;
	D3DXVECTOR3 lightAt(0,0,0);
	//D3DXVECTOR3 lightEye;
	D3DXVECTOR3 lightUp(0,1,0);
	D3DXMATRIX lightViewProjMatrix;
	//D3DXMATRIX inversView;
	//D3DXVECTOR4 lightDirView;
	D3D11_TEXTURE2D_DESC shadowMap_desc;
	D3D11_VIEWPORT cameraVP;
	D3D11_VIEWPORT shadowVP;
	//ID3D11Buffer* vbs[] = { NULL, };
	ID3D11DepthStencilView* pDSV;
	ID3D11RenderTargetView* pRTV;

// Terrain rendering resources
// BEGIN: Assignment 4.2.2 

// Scene information

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
HRESULT ReLoadConfig(ID3D11Device* pd3dDevice);
void placeTerrainObject(TerrainObject*);
void AutomaticPositioning();
void LoadConfig(bool reload);

bool VertexSort(SpriteVertex& a, SpriteVertex& b)
{
	return (double)D3DXVec3Dot(&a.Position, g_Camera.GetWorldAhead()) > (double)D3DXVec3Dot(&b.Position, g_Camera.GetWorldAhead());
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
	DXUTInit( true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true );

	DXUTCreateWindow( L"The M.A.R.L.I.E. Project"); // You may change the title
	DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 720 );
	
	AutomaticPositioning();
	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown();
	DeinitApp();

	return DXUTGetExitCode();
}


void LoadConfig(bool reload = false)
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
	string meshName;
	string file_str, input_str, output_str, it, args;
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
			Skybox::g_LightColor = D3DXCOLOR(r, g, b, 1);
			Skybox::g_LightDir = D3DXVECTOR4(x-x2, y-y2, z-z2, 0);
			D3DXVec3Normalize((D3DXVECTOR3*)&Skybox::g_LightDir, (D3DXVECTOR3*)&Skybox::g_LightDir);
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
			stream >> meshName >> scale >> rotX >> rotY >> rotZ>> transX >> transY >> transZ;
			g_StaticGameObjects.push_back(new GameObject(meshName,  transX, transY, transZ, scale,rotX, rotY, rotZ, 0,GameObject::CAMERA));
			g_ObjectReferences[meshName] = g_StaticGameObjects.size()-1;
		}
		else if(var.compare("WorldObject") == 0) {
			bool automaticHeight;
			stream >> meshName >> scale >> rotX >> rotY >> rotZ >> transX >> transY >>transZ >> automaticHeight;
			g_StaticGameObjects.push_back(new GameObject(meshName,  transX, transY, transZ, scale,rotX, rotY, rotZ));
			g_ObjectReferences[meshName] = g_StaticGameObjects.size()-1;
		}
		else if(var.compare("TerrainObject") == 0)
		{
			stream >> meshName >> scale >> rotX >> rotY >> rotZ >> useNormal >> spacing >> offset >> maxCount;
			g_TerrainObjects.push_back(new TerrainObject(MeshRenderer::g_Meshes[meshName], 0,0,offset, scale, rotX, rotY, rotZ, spacing, maxCount ));
		}
		else if(var.compare("EnemyType") == 0){

			stream >> name >> hitpoints >> units >> speed >> meshName >> scale >> rotX >> rotY >> rotZ >> transX >> transY >> transZ >> sphere >> effect;
			//g_EnemyTyp

			//g_EnemyTyp[name] = new Enemy(hitpoints, units, new GameObject(meshName, transX, transY, transZ, scale, rotX, rotY, rotZ, GameObject::WORLD));
			g_EnemyTyp[name] = new Enemy(hitpoints, units, meshName, transX, transY, transZ, scale, rotX, rotY, rotZ, GameObject::WORLD);
			g_EnemyTyp[name]->SetSpeed(speed);
			g_EnemyTyp[name]->AddComponent( new SphereCollider(sphere));
			if( effect.size() > 0  && effect.compare("-") != 0)
				g_EnemyTyp[name]->SetDeathEffect(&ParticleEffect::g_ParticleEffects[effect]);
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
			//g_ProjectileTypes[name] = new ProjectileType(name, radius, textureIndex,speed, mass, cooldown, damage);
			GameObject* proj = new GameObject(textureIndex, radius, 0,0,0); 
			proj->AddComponent(new SphereCollider(radius));
			proj->AddComponent(new gcProjectile(damage, speed, cooldown));
			if(mass > 0)
				proj->AddComponent(new gcMass(mass));
			g_ProjectileTypes[name] = proj;
		} else if(var.compare("Weapon") == 0)
		{
			stream >> name >> x >> y >> z;
			stream >> var;
			vector<GameObject*> tmp;
			if(var.compare("{") == 0)
			{
				stream >> var;
				while(var.compare("}") != 0){
					tmp.push_back(g_ProjectileTypes[var]);
					stream >> var;
				} 
			}
			D3DXVECTOR3* p = g_StaticGameObjects[g_ObjectReferences[name]]->GetPosition();
			g_WeaponTypes.push_back(WeaponType(name, x+p->x,y+p->y,z+p->z, tmp));
		} else if(var.compare("Explosion") == 0)
		{
			int count;
			float duration;
			stream >> name >> textureIndex >> duration >> scale >> speed >> mass >> x >> y >> z >> count;
			ParticleSystem::g_ParticleSystems[name] = new  ParticleSystem(textureIndex, 0, 0, 0, 0, GameObject::WORLD, new GameObject(textureIndex, scale, 0,0,0, duration), 0,0,0, 1, duration, true);
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
	LoadConfig();
	g_SpriteRenderer = new SpriteRenderer(spriteFileNames);
	g_MeshRenderer = new MeshRenderer();
	g_SkyboxRenderer = new Skybox(g_SkyboxPath, g_BoundingBoxDiagonal*0.7f);
	g_TerrainRenderer = new TerrainRenderer(g_TerrainPath, g_TerrainWidth, g_TerrainDepth, g_TerrainHeight, g_TerrainSpinning, g_TerrainSpinSpeed);
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
	g_TerrainRenderer->Deinit();
	g_MeshRenderer->Deinit();
	Enemy* t;
	for(auto it=g_EnemyTyp.begin(); it!= g_EnemyTyp.end(); it++)
	{
		SAFE_DELETE(it->second);
	}
	g_EnemyTyp.clear();
	for(auto it=g_EnemyInstances.begin(); it!= g_EnemyInstances.end(); it++)
		SAFE_DELETE(*it);
	g_EnemyInstances.clear();
	for(auto it = g_StaticGameObjects.begin(); it != g_StaticGameObjects.end(); it++)
		SAFE_DELETE(*it);
	g_StaticGameObjects.clear();
	for(auto it = g_TerrainObjects.begin(); it != g_TerrainObjects.end(); it++)
		SAFE_DELETE(*it);
	g_TerrainObjects.clear();
	for(auto it =g_ProjectileTypes.begin(); it != g_ProjectileTypes.end(); it++)
		SAFE_DELETE(it->second);
	for(auto it = g_Particles.begin(); it != g_Particles.end(); it++)
		SAFE_DELETE(*it);
	for(auto it = ParticleSystem::g_ParticleSystems.begin(); it != ParticleSystem::g_ParticleSystems.end(); it++)
		SAFE_DELETE(it->second);
	ParticleSystem::g_ParticleSystems.clear();
	SAFE_DELETE(g_SpriteRenderer);
	SAFE_DELETE(g_MeshRenderer);
	SAFE_DELETE(g_SkyboxRenderer);
	SAFE_DELETE(g_TerrainRenderer);
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
		if(g_StaticGameObjects.size() > Oindex)
		out << Oindex << " "
			<< (*g_StaticGameObjects[Oindex]->GetName()).c_str()
			<<"  Translation X: "
			<< g_StaticGameObjects[Oindex]->GetPosition()->x << " Y: "
			<< g_StaticGameObjects[Oindex]->GetPosition()->y << " Z: "
			<< g_StaticGameObjects[Oindex]->GetPosition()->z << " Scale: " << g_StaticGameObjects[Oindex]->GetScale();

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
	g_TerrainRenderer->CreateResources(pd3dDevice);
	float terrainWidth = (*g_TerrainRenderer->getWidth());
	float terrainDepth = (*g_TerrainRenderer->getDepth());
	float terrainHeight = (*g_TerrainRenderer->getHeight());
	//pow2Border = terrainWidth*0.5f*(g_MaxCircle+0.2f)*terrainWidth*0.5f*(g_MaxCircle+0.2f);
	D3DXVECTOR3 boundingDiff = D3DXVECTOR3(terrainWidth,terrainDepth,terrainHeight)-D3DXVECTOR3(-terrainWidth,0.f,-terrainDepth);
	D3DXVECTOR3 terrainSize(terrainWidth, terrainDepth, 0.0f);
	g_BoundingBoxDiagonal = D3DXVec3Length(&boundingDiff);
	g_SkyboxRenderer->setSunDistance(g_BoundingBoxDiagonal*0.6f);
	//pow2Border = g_BoundingBoxDiagonal*0.5f*(g_MaxCircle+0.2f);
	float terrainLenght = D3DXVec3Length(&terrainSize);
	g_SpawnCircle = terrainLenght * g_MaxCircle;
	g_TargetCircle = terrainLenght * g_MinCircle;
	g_DestroyCircle = terrainLenght * (g_MaxCircle +0.2f);
	// Initialize the camera
	D3DXVECTOR3 Eye(g_CameraPos.x,g_TerrainRenderer->getHeightAtPoint(g_CameraPos.x, g_CameraPos.z)+g_CameraPos.y, g_CameraPos.z);
	//D3DXVECTOR3 At(g_CameraLookAt);
	g_Camera.SetViewParams( &Eye, &g_CameraLookAt ); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_Camera.SetScalers( g_CameraRotateScaler, g_CameraMoveScaler );


	//end: assignment 3.2.2
	//END: Assigment 4.2.7

	//5.2.8
	//g_CockpitMesh->CreateResources(pd3dDevice);
	/*for(auto it=g_Meshes.begin(); it!=g_Meshes.end(); it++)
		it->second->CreateResources(pd3dDevice);*/
	//Nun im MeshRenderer
	g_MeshRenderer->CreateResources(pd3dDevice);

	//terrainObjects place and set to draw Ojects
	for(auto o = g_TerrainObjects.begin(); o != g_TerrainObjects.end(); o++)
{		
	placeTerrainObject(*o);
	SAFE_DELETE(*o);
	}
	g_TerrainObjects.clear();
	//Skybox
	if(g_UseSkybox){
		V(g_SkyboxRenderer->CreateResources(pd3dDevice));
	}


	//7.2.2
	g_SpriteRenderer->CreateResources(pd3dDevice);
	for_each(g_ProjectileTypes.begin(), g_ProjectileTypes.end(), [&](pair<string, GameObject*> i){
		i.second->GetSprite()->AnimationSize = g_SpriteRenderer->GetAnimationSize(i.second->GetSprite()->TextureIndex);
		i.second->GetSprite()->TextureIndex = g_SpriteRenderer->GetTextureOffset(i.second->GetSprite()->TextureIndex);
	});
	for_each(ParticleSystem::g_ParticleSystems.begin(), ParticleSystem::g_ParticleSystems.end(), [&](pair<string,ParticleSystem*> it){
		it.second->SetSpriteAnimationSize(g_SpriteRenderer->GetAnimationSize(it.second->GetTextureIndex()));
		it.second->SetTextureIndex(g_SpriteRenderer->GetTextureOffset(it.second->GetTextureIndex()));
		it.second->SetEmittedAnimationSize(g_SpriteRenderer->GetAnimationSize(it.second->GetEmittedTextureIndex()));
		it.second->SetEmittedTextureIndex(g_SpriteRenderer->GetTextureOffset(it.second->GetEmittedTextureIndex()));
	});
	//Shadow Map

	//Create shadow map texture desc
	D3D11_TEXTURE2D_DESC shadowTex_Desc;
	shadowTex_Desc.Width = 2048;
	shadowTex_Desc.Height = 2048;
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
	V(pd3dDevice->CreateDepthStencilView(g_ShadowMap, &shadowDSV_Desc, &g_ShadowMapDSV));
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

	// Assignment 3.2.5

	SAFE_RELEASE(g_ShadowMap);
	SAFE_RELEASE(g_ShadowMapSRV);
	SAFE_RELEASE(g_ShadowMapDSV);


	SAFE_DELETE( g_TxtHelper );
	SAFE_DELETE(g_leftText);
	SAFE_DELETE( g_rightText);
	ReleaseShader();

	g_TerrainRenderer->ReleaseResources();
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
	SAFE_GET_TECHNIQUE(g_Effect, "RenderBillboard", g_BillboardTechnique);
	SAFE_GET_RESOURCE(g_Effect, "g_ShadowMap", g_ShadowMapEV);

	//7.2.2
	g_TerrainRenderer->ReloadShader(pd3dDevice);
	g_MeshRenderer->ReloadShader(pd3dDevice);
	g_SpriteRenderer->ReloadShader(pd3dDevice);
	g_SkyboxRenderer->ReloadShader(pd3dDevice);
	return S_OK;
}

HRESULT ReLoadConfig(ID3D11Device* pd3dDevice)
{
	assert(pd3dDevice != NULL);

	g_EnemyInstances.clear();
	g_EnemyTyp.clear();
	g_EnemyTypeNames.clear();
	g_TerrainObjects.clear();
	g_StaticGameObjects.clear();
	g_ObjectReferences.clear();
	SpriteRenderer::g_SpritesToRender.clear();
	g_Particles.clear();
	g_WeaponTypes.clear();
	g_ProjectileTypes.clear();
	ParticleEffect::g_ActiveEffects.clear();
	ParticleEffect::g_ParticleEffects.clear();

	LoadConfig(true);

	AutomaticPositioning();
	for(auto o = g_TerrainObjects.begin(); o != g_TerrainObjects.end(); o++)
		placeTerrainObject(*o);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	SAFE_RELEASE( g_Effect );
	g_TerrainRenderer->ReleaseShader();
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
			Oindex = (++Oindex)%g_StaticGameObjects.size();
			break;
		case 109:
			Oindex = (--Oindex+g_StaticGameObjects.size())%g_StaticGameObjects.size();
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
		case  13: //Enter
			Omove = ORotate = OScale = false;
			break;
		case  38: //Arrow Up
			if(Omove)
				g_StaticGameObjects[Oindex]->Translate(0,1,0);
			if(OScale)
				g_StaticGameObjects[Oindex]->Scale(0.1f);
			if(ORotate)
				g_StaticGameObjects[Oindex]->Rotate(0, 1,0);
			break;
		case  40: //Arrow Down
			if(Omove)
				g_StaticGameObjects[Oindex]->Translate(0,-1,0);
			if(OScale)
				g_StaticGameObjects[Oindex]->Scale(-0.1f);
			if(ORotate)
				g_StaticGameObjects[Oindex]->Rotate(0, -1,0);
			break;
		case  39: //Arrow Right
			if(Omove)
				g_StaticGameObjects[Oindex]->Translate(1,0,0);
			if(ORotate)
				g_StaticGameObjects[Oindex]->Rotate(1, 0,0);
			break;
		case  37: //arrow Left
			if(Omove)
				g_StaticGameObjects[Oindex]->Translate(-1,0,0);
			if(ORotate)
				g_StaticGameObjects[Oindex]->Rotate( -1,0,0);
			break;
		case  187: // minus
			if(Omove)
				g_StaticGameObjects[Oindex]->Translate(0,0,-1);
			if(ORotate)
				g_StaticGameObjects[Oindex]->Rotate( 0,0,-1);
			break;
		case  189: // minus
			if(Omove)
				g_StaticGameObjects[Oindex]->Translate(0,0,1);
			if(ORotate)
				g_StaticGameObjects[Oindex]->Rotate( 0,0,1);
			break;
		} 
	}else{ //KeyUp
		//switch(nChar){
		//}
	}
	if(bKeyDown)
	{
		switch(nChar){
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
		ReLoadConfig(DXUTGetD3D11Device());
		break;
	case IDC_PAUSE:
		g_GamePause = !g_GamePause;
		DXUTPause(g_GamePause, false);
		break;
	}
}

//todo
list<GameObject*> g_emittedParticles;
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

	g_SkyboxRenderer->OnMove(fTime, fElapsedTime);

	D3DXMATRIX mTmp;

	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb206365%28v=vs.85%29.aspx
	g_TerrainRenderer->OnMove(fTime, fElapsedTime);

	//*********************************SpawnObjects	
	if(fTime-g_LastSpawn> g_SpawnIntervall)
	{
		if(g_EnemyTypeNames.size() > 0)
		{
		Enemy* myEnemyType = g_EnemyTyp[g_EnemyTypeNames[static_cast<int>((float)rand()/(RAND_MAX+1.f)*g_EnemyTypeNames.size())]];
		if(myEnemyType->SpawnedEnemies < myEnemyType->GetCountMaxUnits()){
			g_LastSpawn = fTime;
			//TODO: Spawn und Destroy von den Enemies besser abfragen, aktuell wird der Zähler nicht mehr dekrementiert
			myEnemyType->SpawnedEnemies++;
			Enemy* newEnemy = new Enemy(myEnemyType);
			float a = ((float)rand()/RAND_MAX)*2*D3DX_PI;
			float height = (((float)rand()/RAND_MAX)*(g_MaxHeight-g_MinHeight)*g_TerrainHeight)+ g_MinHeight*g_TerrainHeight;
			D3DXVECTOR3 outerPos(g_SpawnCircle*cos(a), height, g_SpawnCircle*sin(a));
			a = ((float)rand()/RAND_MAX)*2*D3DX_PI;
			D3DXVECTOR3 InnerPos(g_TargetCircle*cos(a), height, g_TargetCircle*sin(a));
			D3DXVECTOR3 MovingDirection = D3DXVECTOR3(0,height,0)-outerPos;
			D3DXVec3Normalize(&MovingDirection, &MovingDirection);
			newEnemy->SetMeshOrientation(MovingDirection.x, MovingDirection.y, MovingDirection.z);
			newEnemy->AddForce(myEnemyType->GetSpeed(), MovingDirection);
			newEnemy->TranslateTo(outerPos.x, outerPos.y, outerPos.z);

			g_EnemyInstances.push_back(newEnemy);
			//g_EnemyInstances[1].SetDestroyEffect(&g_ParticleEffects["Death1"]);
			//auto g = --g_EnemyInstances.end();
			//g->SetDestroyEffect(&ParticleEffect::g_ParticleEffects["bomb"]);
			pushText("Achtung: Ein neuer Feind ist nun in Reichweite!", D3DXCOLOR(1.0f, 0.15f, 0.f, 1.f));
		}
	}
	}
	if(p_Fire1){
		g_WeaponTypes[1].fire(0, &g_Camera, fTime);
		//move
		g_StaticGameObjects[2]->Rotate(0,0,170*fElapsedTime);
	}
	if(p_Fire2){
		g_WeaponTypes[0].fire(0, &g_Camera, fTime);
		//move
 		g_StaticGameObjects[4]->Translate(0,0,static_cast<float>(10*fElapsedTime*sin(fTime*25)));
	}

	//*********************************MovingObjects
	//g_StaticGameObjects[10]->Rotate(0,50.f*fElapsedTime,0);
	//g_StaticGameObjects[10]->CalculateWorldMatrix();
	for(auto ei = g_EnemyInstances.begin(); ei != g_EnemyInstances.end(); ei++)
	{
		(*ei)->OnMove(fTime, fElapsedTime);
	}
	for(auto pi = g_Particles.begin(); pi != g_Particles.end(); pi++)
	{
		(*pi)->OnMove(fTime, fElapsedTime);
	}

	bool del = false;
	//*********************************Check Collisions
	for(auto it = g_Particles.begin(); it != g_Particles.end();){
		//Collision mit dem Boden
		auto shoot = *it; 
		if(shoot->GetComponent(GameComponent::tSphereCollider)->size() == 0)
		{it++; continue;}
		float theight = g_TerrainRenderer->getHeightAtPoint(shoot->GetPosition()->x, shoot->GetPosition()->z);
		if( theight > shoot->GetPosition()->y)
		{
			ParticleSystem::g_activeParticleSystems.push_back(ParticleSystem::g_ParticleSystems["bomb"]->Clone());
			(*ParticleSystem::g_activeParticleSystems.begin())->TranslateTo(shoot->GetPosition()->x,theight,shoot->GetPosition()->z);
			(*ParticleSystem::g_activeParticleSystems.begin())->StartEmit(fTime);
			auto shoot_rem = it; 
			it++;
			SAFE_DELETE(*shoot_rem);
			g_Particles.erase(shoot_rem);
		}
		else
		{
			del = false;
			for(list<Enemy*>::iterator enemy = g_EnemyInstances.begin(); enemy != g_EnemyInstances.end();){
				//D3DXVECTOR3 d =(shoot->Position-enemy->getCurrentPosition());
				//float abstand = D3DXVec3Length(&(shoot->Position-enemy->getCurrentPosition()));
				//float radius =(shoot->Radius +enemy->getObject()->getSphereSize())*(shoot->Radius +enemy->getObject()->getSphereSize());//TODO: richtige Objektgröße Berechnen
				//if abstand < radius
 				SphereCollider* enemyCollider = static_cast<SphereCollider*>((*enemy)->GetComponent(GameComponent::tSphereCollider)->at(0));
				SphereCollider* shootCollider = static_cast<SphereCollider*>((shoot)->GetComponent(GameComponent::tSphereCollider)->at(0));
				if(D3DXVec3Length(&(*shoot->GetPosition()-*(*enemy)->GetPosition())) < (shootCollider->GetSphereRadius() +enemyCollider->GetSphereRadius())){
					del = true;
					(*enemy)->OnHit(shoot);
					shoot->OnHit(*enemy);
					//shoot->Destroy();
					auto shoot_rem = it;
					it++;
					if((*enemy)->IsDead())
					{
						g_PlayerPoints += (*enemy)->GetPoints();
						(*enemy)->OnDestroy();
						auto enemy_rem = enemy;
						enemy++;
						SAFE_DELETE(*enemy_rem);
						g_EnemyInstances.erase(enemy_rem);
					}
					SAFE_DELETE(*shoot_rem);
					g_Particles.erase(shoot_rem);
					break; //Ein Projektiel kann nur ein Gegner treffen
				}
				else
					enemy++;
			}
			if(!del)
				it++;
		}
	}


	//*********************************DestroyObjects
	for(auto ei = g_EnemyInstances.begin(); ei != g_EnemyInstances.end();)
	{
		if(D3DXVec3Length((*ei)->GetPosition())-g_DestroyCircle > 0){
			auto ei_rem = ei;
			ei++;
			//(*ei_rem)->SpawnedEnemies--;
			SAFE_DELETE(*ei_rem);
			g_EnemyInstances.erase(ei_rem);
		} else {
			(*ei)->CalculateWorldMatrix();
			ei++;
		}
	}
	for(auto ei = g_Particles.begin(); ei != g_Particles.end();)
	{
		if(D3DXVec3Length((*ei)->GetPosition())-g_DestroyCircle > 0){
			auto particle_rem = ei;
			ei++;
			SAFE_DELETE(*particle_rem);
			g_Particles.erase(particle_rem);
		} else
			ei++;
	}

	//*********************************DeathEffects
	for(auto it = ParticleEffect::g_ActiveEffects.begin(); it != ParticleEffect::g_ActiveEffects.end(); )
	{
		it->move(fElapsedTime);
		SpriteRenderer::g_SpritesToRender.push_back(it->getSpriteToRender());//jedes bewegte Sprite soll an den Renderer Übergeben werden, der nur mit nem Vector arbeitet
		if(it->isTimeOver())
		{
			auto itrem = it;
			it++;
			ParticleEffect::g_ActiveEffects.erase(itrem);
		}
		else
			it++;
	}
	//int startPositionParticleAdd = 0;
	for(auto it = ParticleSystem::g_activeParticleSystems.begin(); it != ParticleSystem::g_activeParticleSystems.end();)
	{
		(*it)->Emit(fTime);
		(*it)->OnMove(fTime, fElapsedTime);
		auto it_rem = it;
		it++;
		if((*it_rem)->CheckParticles(fTime))
		{
			SAFE_DELETE(*it_rem)
			ParticleSystem::g_activeParticleSystems.erase(it_rem);
		}
		else
		{
			if((*it_rem)->GetType() == GameObject::SPRITE)
			{
				//int size = SpriteRenderer::g_SpritesToRender.size();
				//SpriteRenderer::g_SpritesToRender.resize(size+(*it)->DisplayedParticles()->size());
				g_emittedParticles.insert(g_emittedParticles.end(),(*it_rem)->DisplayedParticles()->begin(), (*it_rem)->DisplayedParticles()->end());
				//startPositionParticleAdd+= size;
			}
		}
	}
	//Nach Destroy alle Particle zum Rendern übergeben
	SpriteRenderer::g_SpritesToRender.resize(g_Particles.size()+g_emittedParticles.size());
	int i = 0;
	SpriteRenderer::g_SpritesToRender.push_back(*g_SkyboxRenderer->getSun());
	for each(auto it in g_Particles)
	{
		//g_SpritesToRender.push_back(it);
		SpriteRenderer::g_SpritesToRender[i++] = *it->GetSprite();
	}
	for each(auto it in g_emittedParticles)
		SpriteRenderer::g_SpritesToRender[i++] = *it->GetSprite();
	g_emittedParticles.clear();
	
	sort(SpriteRenderer::g_SpritesToRender.begin(), SpriteRenderer::g_SpritesToRender.end(), VertexSort);
	
	/*if(g_SpritesToRender.size() > 2048)
	{
		g_SpritesToRender = vector<SpriteVertex>(g_SpritesToRender.begin(), g_SpritesToRender.begin()+1023);
	}
*/
	//Neuberechnung von der Worldmatrix von statischen Objekten, theorethisch nicht nötig
	//for(auto it = g_StaticGameObjects.begin(); it != g_StaticGameObjects.end(); it++)
	//{
	//	it->CalculateWorldMatrix();
	//}

}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext )
{

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.OnRender( fElapsedTime );
		return;
	}
		if(g_Effect == NULL) {
		g_TxtHelper->Begin();
		g_TxtHelper->SetInsertionPos( 5, 5 );
		g_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		g_TxtHelper->DrawTextLine( L"SHADER ERROR" );
		g_TxtHelper->End();
		return;
	}
		

	pDSV = DXUTGetD3D11DepthStencilView();
	pRTV = DXUTGetD3D11RenderTargetView();

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
	pd3dImmediateContext->OMSetRenderTargets(0, 0, g_ShadowMapDSV);
	pd3dImmediateContext->RSSetViewports(1, &shadowVP);
	pd3dImmediateContext->ClearDepthStencilView(g_ShadowMapDSV, D3D11_CLEAR_DEPTH, 1.0, 0);



	const D3DXMATRIX* view, *proj;
		D3DXVECTOR4 lightDirView;
	D3DXMATRIX cameraView;
		view = g_Camera.GetViewMatrix();
	proj = g_Camera.GetProjMatrix();
	g_ViewProj = (*view) * (*proj);
	// Update variables that change once per frame
	D3DXVECTOR3 vLightDir(Skybox::g_LightDir*g_BoundingBoxDiagonal*0.5f); // g_LightDir == normalize(vLightDir)
	D3DXMatrixOrthoLH(&lightProjektionMatrix, g_BoundingBoxDiagonal, g_BoundingBoxDiagonal, 0.0001f, g_BoundingBoxDiagonal); //ProjektionMatrix
	D3DXMatrixLookAtLH(&lightViewMatrix, &vLightDir, &lightAt, &lightUp); //ViewMatrix
	lightViewProjMatrix = lightViewMatrix*lightProjektionMatrix;

	//D3DXMatrixInverse(&cameraView,0, g_Camera.GetViewMatrix());//==cam.WorldMatrix
	D3DXMatrixTranspose(&cameraView, g_Camera.GetWorldMatrix());
	D3DXVec4Transform(&lightDirView,&Skybox::g_LightDir,&cameraView);
	D3DXVec3Normalize((D3DXVECTOR3*)&lightDirView,(D3DXVECTOR3*)&lightDirView);
	//Shadow

	//****Shadowmap of Terrain*****///
	g_TerrainRenderer->g_LightDir = &Skybox::g_LightDir;
	g_TerrainRenderer->g_ViewProj = &lightViewProjMatrix;
	g_TerrainRenderer->g_LightViewProjMatrix = &lightViewProjMatrix;

	g_TerrainRenderer->ShadowTerrain(pd3dDevice);
	//****Shadowmap of Meshes*****///
	g_MeshRenderer->g_LightDirView = &lightDirView;
	g_MeshRenderer->g_LightColor = &Skybox::g_LightColor;
	g_MeshRenderer->g_View = (D3DXMATRIX*)&lightViewMatrix;
	g_MeshRenderer->g_Proj = (D3DXMATRIX*)&lightProjektionMatrix;
	g_MeshRenderer->g_LightViewProjMatrix = &lightViewProjMatrix;

	g_MeshRenderer->ShadowMeshes(pd3dDevice, &g_StaticGameObjects);
	g_MeshRenderer->ShadowMeshes(pd3dDevice, &g_EnemyInstances);


	//*****normal Scene Rendering*****//

	// Clear the depth stencil
	pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);	//
	pd3dImmediateContext->RSSetViewports(1, &cameraVP);
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->ClearRenderTargetView( pRTV, g_ClearColor );
				//Skybox render
	if(g_UseSkybox){
		g_SkyboxRenderer->RenderSkybox(pd3dDevice, g_Camera);
	}

	// Update variables that change once per frame

	g_TerrainRenderer->g_ViewProj = &g_ViewProj;
	g_TerrainRenderer->RenderTerrain(pd3dDevice);

	g_MeshRenderer->g_View = (D3DXMATRIX*)view;
	g_MeshRenderer->g_Proj = (D3DXMATRIX*)proj;
	g_MeshRenderer->g_invView = g_Camera.GetWorldMatrix();
	g_MeshRenderer->g_ViewProj = &g_ViewProj;
	g_MeshRenderer->g_LightViewProjMatrix = &lightViewProjMatrix;

	g_MeshRenderer->RenderMeshes(pd3dDevice, &g_StaticGameObjects);
	g_MeshRenderer->RenderMeshes(pd3dDevice, &g_EnemyInstances);

	if(SpriteRenderer::g_SpritesToRender.size() >0)
		g_SpriteRenderer->RenderSprites(pd3dDevice, g_Camera);
	g_SpriteRenderer->RenderGUI(pd3dDevice, g_Camera);
	//stringstream ss;
	//ss << "Sprites: " << g_SpritesToRender.size();
	//pushText(ss.str(), LEFT);

	//render shadow map billboard
	//***************************************************************************
	if(useDeveloperFeatures)
	{
		g_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource(g_ShadowMapSRV);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);	

		g_BillboardTechnique->GetPassByIndex(0)->Apply( 0, pd3dImmediateContext );
		pd3dImmediateContext->DrawIndexed(1, 0, 0);

		//unbind shadow map as SRV
		g_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource()->SetResource( 0 );
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
}

void placeTerrainObject(TerrainObject* o){
	for(int i = 0; i < o->getMaxCount(); i++)
	{
		float x = 0;random(-400.f,400.f,x);
		float z = 0;random(-400.f,400.f,z);
		float y = g_TerrainRenderer->getHeightAtPoint(x,z);
		o->TranslateTo(x, y, z);
		//if(o->useNormal())
		//{
		//	D3DXVECTOR3 n = g_TerrainRenderer->getNormalAtPoint(x,z,1);
		//	o->rotate(n.x,n.y, n.z);
		//}
		g_StaticGameObjects.push_back(o->Clone());
	}
}

void AutomaticPositioning()
{
	for(auto it = g_StaticGameObjects.begin(); it != g_StaticGameObjects.end(); it++)
	{
		if((*it)->GetRelativePosition() == GameObject::TERRAIN)
			(*it)->TranslateTo((*it)->GetPosition()->x, g_TerrainRenderer->getHeightAtPoint((*it)->GetPosition()->x, (*it)->GetPosition()->z)+ (*it)->GetPosition()->y, (*it)->GetPosition()->z);
		(*it)->CalculateWorldMatrix();
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

	for each(auto it in g_EnemyInstances)
	{
		float x = it->GetPosition()->x/(g_TerrainWidth*0.5f*g_MaxCircle)*g_Radar.Radius;
		float y = it->GetPosition()->y/(g_TerrainWidth*0.5f*g_MaxCircle)*g_Radar.Radius;
		pShip.Position = D3DXVECTOR3(x,y,0);
		radarShips.push_back(SpriteVertex(pShip));
	}
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

