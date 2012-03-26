#pragma once
/*#include <vector>

using namespace std;
#pragma pack(push, 1)*/

struct VerticesList3ds
{
	static const word id = 0x4110;
	dword size;
	short nvertices;
	vector<float> vertices;
};
struct MappingList3ds
{
	static const word id = 0x4140;
	dword size;
	word nmaps;
	vector<float> maps;
};

struct MeshMatrix3ds
{
	static const word id = 0x4160;
	static const dword size = 54;
	static const float matrix[4][3];
};

const float MeshMatrix3ds::matrix[4][3] = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.001f, 0.001f, 0.001f} };

struct Face3ds
{
	word vertex1;
	word vertex2;
	word vertex3;
	static const word flags = 0;
};

struct FacesMaterials
{
	static const word id = 0x4130;
	dword size;
	string name;
	static const byte endofstring = 0;
	word nfaces;
	vector<word> faces;
};

struct FacesList3ds
{
	static const word id = 0x4120;
	dword size;
	word nfaces;
	vector<Face3ds> faces;
	vector<FacesMaterials> materials;
};

struct TriangleObject3ds
{
	static const word id = 0x4100;
	dword size;
	VerticesList3ds verticesList;
	MappingList3ds mappingList;
	MeshMatrix3ds meshMatrix;
	FacesList3ds facesList;

};

struct MapTiling3ds //uk0xA351
{
	static const word id = 0xA351;
	static const dword size = 8;
	static const word flags = 0;
};

struct Percentage3ds
{
	static const word id = 0x0030;
	static const dword size = 8;
	static const word percentage = 100;
};

struct MapTexblur3ds //uk0xA353
{
	static const word id = 0xA353;
	static const dword size = 10;
	static const dword blurring = 0;
};

struct MapFilename3ds
{
	static const word id = 0xA300;
	dword size;
	string filename;
	static const byte endofstring = 0;
};

struct MaterialName3ds
{
	static const word id = 0xA000;
	dword size;
	string name;
	static const byte endofstring = 0;
};
struct RGB3ds
{
	static const word id =  0x0011;
	static const dword size = 9;
	static const byte red = 255;
	static const byte green = 255;
	static const byte blue = 255;
	
};
struct Shininess3ds
{
	static const word id = 0xA040;
	static const dword size = 14;
	Percentage3ds percentage;
};
struct Shin2Pct3ds
{
	static const word id = 0xA041;
	static const dword size = 14;
	Percentage3ds percentage;
};
struct Transparency3ds
{
	static const word id = 0xA050;
	static const dword size = 14;
	Percentage3ds percentage;
};
struct XPFall3ds
{
	static const word id = 0xA052;
	static const dword size = 14;
	Percentage3ds percentage;
};
struct Shading3ds
{
	static const word id = 0xA100;
	static const dword size = 8;
	static const word shading = 3; 
};
struct SelfILPCT3ds
{
	static const word id = 0xA084;
	static const dword size = 14;
	Percentage3ds percentage;
};
struct XPFallin3ds
{
	static const word id = 0xA08A;
	static const dword size = 6;
};
struct PhongSoft3ds
{
	static const word id = 0xA08C;
	static const dword size = 6;
};
struct WireSize3ds
{
	static const word id = 0xA087;
	static const dword size = 10;
	static const dword wiresize = 1065353216;
};
struct RefBlur3ds
{
	static const word id = 0xA053;
	static const dword size = 14;
	Percentage3ds percentage;
};


struct MaterialAmbient3ds
{
	static const word id = 0xA010;
	static const dword size = 15;
	RGB3ds RGB;
};

struct MaterialDiffuse3ds
{
	static const word id = 0xA020;
	static const dword size = 15;
	RGB3ds RGB;
};

struct MaterialSpecular3ds
{
	static const word id = 0xA030;
	static const dword size = 15;
	RGB3ds RGB;
};

struct ObjectBlock3ds
{
	static const word id = 0x4000;
	dword size;
	static const char name[6];
	TriangleObject3ds triangleObject;
};
const char ObjectBlock3ds::name[6] = "shape";

struct MasterScale3ds
{
	static const word id = 0x0100;
	static const dword size = 10;
	static const dword scale = 1065353216;

};

struct Texture3ds
{
	static const word id = 0xA200;
	dword size;
	Percentage3ds percentage;
	MapFilename3ds mapFilename;
	MapTiling3ds mapTiling;
	MapTexblur3ds mapTexblur;
};

struct Material3ds
{
	static const word id = 0xAFFF;
	dword size;
	MaterialName3ds materialName;
	MaterialAmbient3ds materialAmbient;
	MaterialDiffuse3ds materialDiffuse;
	MaterialSpecular3ds materialSpecular;
	Shininess3ds shininess;
	Shin2Pct3ds shin2Pct;
	Transparency3ds transparency;
	XPFall3ds XPFall;
	RefBlur3ds refBlur;
	Shading3ds shading;
	SelfILPCT3ds selfILPCT;
	XPFallin3ds XPFallin;
	PhongSoft3ds phongSoft;
	WireSize3ds wireSize;
	Texture3ds textures;
};

struct MeshVerion3ds
{
	static const word id =  0x3D3D;
	static const dword size = 10;
	static const dword version = 3;
};

struct ObjectMesh3ds
{
	static const word id =  0x3D3D;
	dword size;
	MeshVerion3ds meshVerion;
	vector<Material3ds> materials;
	MasterScale3ds masterScale;
	ObjectBlock3ds objectBlock;
};

struct FileVerion3ds
{
	static const word id =  0x0002;
	static const dword size = 10;
	static const dword version = 3;
};

struct File3ds
{
	static const word id =  0x4D4D;
	dword size;
	FileVerion3ds fileVerion;
	ObjectMesh3ds objectMesh;

};

