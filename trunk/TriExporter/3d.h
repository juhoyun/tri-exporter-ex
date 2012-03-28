#pragma once

#include "arcball.h"
#include "StuffFile.h"
#include "TriFile.h"

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
#define SAFE_RELEASE(p)            { if (p) { (p)->Release();    (p)=NULL; } }

class CDXMessageLoop : public CMessageLoop {
	
	BOOL OnIdle(int nIdleCount) {
		return !CMessageLoop::OnIdle(nIdleCount);
	}
	
};

typedef struct
{
	D3DMATERIAL9 material;
	LPDIRECT3DTEXTURE9 pTexture;
} CMaterial;

class C3d : public CWindowImpl<C3d>
{
public:
	LPDIRECT3D9						g_pD3D;
	LPDIRECT3DDEVICE9				g_pd3dDevice;
	LPDIRECT3DVERTEXBUFFER9			g_pVB ;
	vector<LPDIRECT3DINDEXBUFFER9>	g_pIB;
	vector<CMaterial>				g_pMaterial;
	ArcBall m_abArcBall;
	float distance;
	float alight;
	UINT_PTR time;
	DWORD vcount;
	std::vector<DWORD> fcount;
	bool loaded;
	int m_lMousex;
	int m_lMousey;
	bool rotation;
	D3DXVECTOR3 vCenter;
	D3DXMATRIXA16 matWorld;
	D3DLIGHT9 light;
	FLOAT vRadius;
	BEGIN_MSG_MAP(C3d)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnDblClick)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void ClearTextures();
	void ClearIndexes();
	void TextureChange(const StuffFile &sf, const vector<int> &textures, const vector<Diffuse> &diffuseColors);
	void Open(const TriFile &tfile);
	C3d();
	void SwapTexture(int x, int y);
	void InitD3D();
	void Cleanup();
	void Reset();
	void SwapFillMode();
	void SetupMatrices();
	void SetupLights();
	BOOL SubclassWindow(HWND hWnd);
	void Render();
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};