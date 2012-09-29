#pragma once

#include <DXUT.h>
#include <vector>
#include <cstdint>
#include "T3d.h"
#define MAX_MESH_INSTANCES 50
//This class ecapsulates the D3D11 resources needed for a mesh

struct MeshInstanceType
{
	D3DXMATRIX Transformation;
};
class Mesh
{
public:
	//Create mesh object and set filenames, but don't create D3D11 resources
	//This constructor should be called from within InitApp().
	Mesh(const char* filename_t3d,           //filename of t3d file for mesh geometry
         const char* filename_ntx_diffuse,   //filename of ntx file for diffuse texture
         const char* filename_ntx_specular,  //filename of ntx file for specular texture
         const char* filename_ntx_glow,     //filename of ntx file for glow texture
		 const char* filename_ntx_normal);
	//Currently does nothing.
	//This destructor should be called from within DeinitApp().
	~Mesh(void);

	//Creates the required D3D11 resources from the given input files.
	//This function should be called from within OnD3D11CreateDevice().
	HRESULT CreateResources(ID3D11Device* pd3dDevice);

	//Releases all D3D11 resources of the mesh.
	//This destructor should be called from within OnD3D11DestroyDevice().
	void ReleaseResources();

	//Simple getters for all resources
	ID3D11Buffer*               GetVertexBuffer() {return m_VertexBuffer;       }
	ID3D11Buffer*               GetIndexBuffer()  {return m_IndexBuffer;        }
	ID3D11Buffer*               GetInstanceBuffer()  {return m_InstanceBuffer;        }
	DXGI_FORMAT                 GetIndexFormat()  {return DXGI_FORMAT_R32_UINT; }
	int                         GetIndexCount()   {return m_IndexCount;         }
	int                         GetVertexCount()  {return m_VertexCount;         }
	ID3D11Texture2D*            GetDiffuseTex()   {return m_DiffuseTex;         }
	ID3D11ShaderResourceView*   GetDiffuseSRV()   {return m_DiffuseSRV;         }
	ID3D11ShaderResourceView*   GetSpecularSRV()  {return m_SpecularSRV;        }
	ID3D11Texture2D*            GetSpecularTex()  {return m_SpecularTex;        }
	ID3D11Texture2D*	        GetGlowTex()      {return m_GlowTex;            }
	ID3D11ShaderResourceView*   GetGlowSRV()      {return m_GlowSRV;            }
	ID3D11Texture2D*	        GetNormalTex()    {return m_NormalTex;          }
	ID3D11ShaderResourceView*   GetNormalSRV()    {return m_NormalSRV;          }
	D3DXVECTOR3*				GetOriginPoint()  {return &m_centerVertex;      }
	float						GetMeshRadius()	  {return m_sphereRadius;		}
	int							GetInstanceCount(){return m_InstanceCount;		}
	static HRESULT				CreateInstanceLayout(ID3D11Device* pd3dDevice, 
	ID3DX11EffectPass* pass, ID3D11InputLayout** t3dInputLayout);
	int							AddInstance(D3DXMATRIX* tranformation);
	void						ResetInstances();
	std::vector<MeshInstanceType> GetInstancesMatrix() {return std::vector<MeshInstanceType>(m_MeshInstanceMatrices.begin(),m_MeshInstanceMatrices.begin()+m_InstanceCount);}
private:
	//Reads the complete file given by "path" byte-wise into "data".
	// (this function is similar to the one from PtfGenerator)
	static HRESULT LoadFile(const char * filename, std::vector<uint8_t>& data);
	std::vector<D3DXVECTOR3> readVerticesFromStream(std::vector<T3dVertex>* in);
private:
	D3DXMATRIX*					m_pMeshInstanceList;
	//D3DXMATRIX					m_MeshInstanceMatrices;
	std::vector<MeshInstanceType>		m_MeshInstanceMatrices;

	//Filenames
	std::string					m_FilenameT3d;
	std::string					m_FilenameNtxDiffuse;
	std::string					m_FilenameNtxSpecular;
	std::string					m_FilenameNtxGlow;
	std::string					m_FilenameNtxNormal;

	//Mesh geometry information
	ID3D11Buffer*               m_VertexBuffer;
	ID3D11Buffer*               m_IndexBuffer;
	ID3D11Buffer*               m_InstanceBuffer;
	int                         m_IndexCount; //number of single indices in m_IndexBuffer (needed for DrawIndexed())
	int							m_InstanceCount;
	int							m_VertexCount;
	D3DXVECTOR3					m_centerVertex;
	float						m_sphereRadius;

	//Mesh textures and corresponding shader resource views
	ID3D11Texture2D*            m_DiffuseTex;
	ID3D11ShaderResourceView*   m_DiffuseSRV;
	ID3D11Texture2D*            m_SpecularTex;
	ID3D11ShaderResourceView*   m_SpecularSRV;
	ID3D11Texture2D*	        m_GlowTex;
	ID3D11ShaderResourceView*   m_GlowSRV;
	ID3D11Texture2D*            m_NormalTex;
	ID3D11ShaderResourceView*   m_NormalSRV;
};