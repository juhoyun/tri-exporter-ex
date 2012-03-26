#include "stdafx.h"
#include "MainDlg.h"
#include "grannyfile.h"

enum RedParamResType
{
	REDPARAM_UNKNOWN,
	REDPARAM_FLOAT,
	REDPARAM_VEC2,
	REDPARAM_VEC3,
	REDPARAM_VEC4,
	REDPARAM_TEX2D,
	REDPARAM_TEX3D,
	REDPARAM_VARIABLE
} ;

typedef struct
{
	RedParamResType type;
	const char *typeString;
} TypeStringMap;

static const TypeStringMap RedParamsTypeStringMap[] =
{
	{REDPARAM_FLOAT,	"Tr2FloatParameter"   },
	{REDPARAM_VEC2,		"Tr2Vector2Parameter" },
	{REDPARAM_VEC3,		"Tr2Vector3Parameter" },
	{REDPARAM_VEC4,		"Tr2Vector4Parameter" },
	{REDPARAM_VARIABLE, "TriVariableParameter"}
};

static const TypeStringMap RedResourcesTypeStringMap[] = 
{
	{REDPARAM_TEX2D,	"TriTexture2DParameter"}
};

typedef struct
{
	RedParamResType type;
	int index_res;
	std::string name;
	std::string value;
} RedParameter;

typedef struct
{
	int index;
	std::string name;
	std::vector<RedParameter> parameters;
	std::vector<RedParameter> resources;
} Tr2MeshArea;

typedef struct _RedEntity
{
	int index_gr2;
	std::vector<Tr2MeshArea> meshes;
} RedEntity;

typedef struct
{
	std::string id;
	std::string value;
} RedLine;

static void ToLower(std::string &s)
{
	std::string::iterator i = s.begin();
	while(i != s.end())
	{
		if (('A' <= *i) && (*i <= 'Z'))
			*i += 'a' - 'A';
		++i;
	}
}

static int SearchStuffByPath(const StuffFile& sf, const std::string& geoResPath)
{
	std::string path = geoResPath;
	size_t pos = path.find(':');
	if (pos != std::string::npos)
		path.erase(pos, 1);
	ToLower(path);
	
	for(dword i=0; i<sf.filescount; i++)
	{
		if (sf.files[i].filename == path)
			return (int)i;
	}
	return -1;
}

static void ParseVec4Str(const std::string& s, float* f1, float* f2, float* f3, float* f4)
{
	size_t startPos = 0;
	size_t endPos = s.find(',', startPos);
	std::string s1 = s.substr(startPos, endPos-startPos);

	startPos = endPos + 1;
	endPos = s.find(',', startPos);
	std::string s2 = s.substr(startPos, endPos-startPos);

	startPos = endPos + 1;
	endPos = s.find(',', startPos);
	std::string s3 = s.substr(startPos, endPos-startPos);

	startPos = endPos + 1;
	std::string s4 = s.substr(startPos);

	*f1 = (float)atof(s1.c_str());
	*f2 = (float)atof(s2.c_str());
	*f3 = (float)atof(s3.c_str());
	*f4 = (float)atof(s4.c_str());
}

static const std::string nullStr;

static const std::string& GetParamValue(const std::vector<RedParameter>& params, const std::string& name)
{
	std::vector<RedParameter>::const_iterator i = params.begin();
	while(i != params.end())
	{
		if (i->name == name)
			return i->value;
		++i;
	}
	return nullStr;
}

LRESULT CMainDlg::OnTreeDblClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
	CTreeItem ti = m_Tree.GetSelectedItem();
	if(ti)
	{
		int out = ti.GetData()-1;
		if(out>=0)
		{
			CString f = sf.files[out].filename.c_str();
			if(!f.Right(4).Compare(".tri"))
			{
				delete file;
				file = new TriFile();
				Load(out);
			}
			else if(!f.Right(4).Compare(".dds"))
			{
				if(loaded)
				{
					Add(f.Right(f.GetLength()-f.ReverseFind('/')-1), out);
					m_p3d.TextureChange(sf, texdata);
				}
			}
			else if(!f.Right(4).Compare(".gr2"))
			{
				delete file;
				file = new GrannyTriFile();
				Load(out);
			}
			else if (!f.Right(4).Compare(".red"))
			{
				LoadRED(out);
				delete file;
				file = new GrannyTriFile();
				Load(redEntity->index_gr2);
				int missedCount = 0;
				for(int i=0; i<(int)file->header.numSurfaces; i++)
				{
					size_t m;
					for(m=0; m<redEntity->meshes.size(); m++)
					{
						if (redEntity->meshes[m].index == i)
							break;
					}
					if (m < redEntity->meshes.size())
					{
						// assuming the first one is "DiffuseMap"
						if (redEntity->meshes[m].resources[0].name == "DiffuseMap")
						{
							out = SearchStuffByPath(sf, redEntity->meshes[m].resources[0].value);
							if (out >= 0)
							{
								float r, g, b, a;
								const std::string& diffuseColorStr = GetParamValue(redEntity->meshes[m].parameters, "MaterialDiffuseColor");
								if (diffuseColorStr.size() > 0)
								{
									ParseVec4Str(diffuseColorStr, &r, &g, &b, &a);
									f = redEntity->meshes[m].resources[0].value.c_str();
									/* duplicate the missing resources with this one */
									for(int j=0; j<missedCount+1; j++)
									{
										Add(f.Right(f.GetLength()-f.ReverseFind('/')-1), out, r, g, b);
										m_p3d.TextureChange(sf, texdata);
									}
									missedCount = 0;
								}
							}
							else
								++missedCount;
						}
						else
						{
							++missedCount;
						}
					}
					else
					{
						// not found
						++missedCount;
					}
				}
			}
			else
				MessageBox("WTF?", "Error", MB_ICONERROR | MB_OK);
		}
	}
	else
		if(!loaded)
			EnableAll(FALSE);
	return TRUE;
}

void CMainDlg::Load(string &out)
{

	CString SurType;
	if(!file->LoadFile(out))
	{
		MessageBox("Model not loaded", "Error", MB_ICONERROR | MB_OK);
		return;
	}
	SurType.Format("%i", file->surfaces[0].surfaceType);
	EnableAll();
	CString title = "TriExporter";
	title += " - ";
	title += out.c_str();
	SetWindowText(title);
	m_p3d.Open(*file);
	textures.clear();
	diffuseColors.clear();
	texdata.clear();
	textures.resize(file->header.numSurfaces);
	diffuseColors.resize(file->header.numSurfaces);
	texdata.resize(file->header.numSurfaces);
	for(dword i = 0; i < file->header.numSurfaces; i++)
		texdata[i] = -1;
	CString VSize;
	CString TriVersion;
	CString VCount;
	CString BoxMax;
	CString BoxMin;
	CString SurCount;
	VSize.Format("%i", file->header.sizeVertex);
	TriVersion.Format("%i.%i", file->header.versionHi, file->header.versionLo);
	VCount.Format("%i", file->header.numVertices);
	BoxMax.Format("{%1.2f; %1.2f; %1.2f}", file->header.maxBox[0], file->header.maxBox[1], file->header.maxBox[2]);
	BoxMin.Format("{%1.2f; %1.2f; %1.2f}", file->header.minBox[0], file->header.minBox[1], file->header.minBox[2]);
	SurCount.Format("%i", file->header.numSurfaces);			
	m_VSize.SetWindowText(VSize);
	m_TriVersion.SetWindowText(TriVersion);
	m_VCount.SetWindowText(VCount);
	m_BoxMax.SetWindowText(BoxMax);
	m_BoxMin.SetWindowText(BoxMin);
	m_SurType.SetWindowText(SurType);
	m_SurCount.SetWindowText(SurCount);
	loaded = true;
}

void CMainDlg::Load(int out)
{

	CString SurType;
	if(!file->LoadFile(sf.files[out]))
	{
		MessageBox("Model not loaded", "Error", MB_ICONERROR | MB_OK);
		return;
	}
	SurType.Format("%i", file->surfaces[0].surfaceType);
	EnableAll();
	CString title = "TriExporter";
	title += " - ";
	title += sf.files[out].filename.c_str();
	SetWindowText(title);
	m_p3d.Open(*file);
	textures.clear();
	diffuseColors.clear();
	texdata.clear();
	textures.resize(file->header.numSurfaces);
	diffuseColors.resize(file->header.numSurfaces);
	texdata.resize(file->header.numSurfaces);
	for(dword i = 0; i < file->header.numSurfaces; i++)
		texdata[i] = -1;
	CString VSize;
	CString TriVersion;
	CString VCount;
	CString BoxMax;
	CString BoxMin;
	CString SurCount;
	VSize.Format("%i", file->header.sizeVertex);
	TriVersion.Format("%i.%i", file->header.versionHi, file->header.versionLo);
	VCount.Format("%i", file->header.numVertices);
	BoxMax.Format("{%1.2f; %1.2f; %1.2f}", file->header.maxBox[0], file->header.maxBox[1], file->header.maxBox[2]);
	BoxMin.Format("{%1.2f; %1.2f; %1.2f}", file->header.minBox[0], file->header.minBox[1], file->header.minBox[2]);
	SurCount.Format("%i", file->header.numSurfaces);			
	m_VSize.SetWindowText(VSize);
	m_TriVersion.SetWindowText(TriVersion);
	m_VCount.SetWindowText(VCount);
	m_BoxMax.SetWindowText(BoxMax);
	m_BoxMin.SetWindowText(BoxMin);
	m_SurType.SetWindowText(SurType);
	m_SurCount.SetWindowText(SurCount);
	loaded = true;
}

static int StoreREDLines(std::vector<RedLine> &redLines, const char *buf, dword bufSize)
{
	int nLines = 0;
	const char *endP = buf + bufSize;

	while(buf != endP)
	{
		// skip white spaces
		while((*buf == ' ') || (*buf == '\t'))
			++buf;

		RedLine l;
		const char *startP = buf;

		while(*buf != ':')
			++buf;
		l.id.append(startP, buf - startP);
		++buf; // skip ':'

		// skip white spaces
		while((*buf == ' ') || (*buf == '\t'))
			++buf;

		if ((*buf == '"') || (*buf == '['))
			++buf;

		startP = buf;

		// move to the line endings
		while((*buf != 0x0A) && (*buf != 0x0D))
			++buf;

		if ((*(buf-1) == '"') || (*(buf-1) == ']'))
			l.value.append(startP, buf - startP - 1);
		else
		{
			l.value.append(startP, buf - startP);
			if (l.id == "transform")
			{
				// check the next line till ']' is found
				// skip line endings
				while((*buf == 0x0A) || (*buf == 0x0D))
					++buf;
				// skip white spaces
				while((*buf == ' ') || (*buf == '\t'))
					++buf;
				startP = buf;
				// move to the line endings
				while((*buf != 0x0A) && (*buf != 0x0D))
					++buf;
				l.value.push_back(' ');
				l.value.append(startP, buf - startP - 1);
			}
		}
		redLines.push_back(l);

		// skip line endings
		while((*buf == 0x0A) || (*buf == 0x0D))
			++buf;

		++nLines;
	}

	return nLines;
}

static int SearchREDLine(std::vector<RedLine>& redLines, const char *id, int startLine = 0)
{
	size_t i = startLine;
	int sLen = strlen(id);
	for(; i<redLines.size(); i++)
	{
		if (redLines[i].id == std::string(id))
		{
			// match found
			return i;
		}
	}

	return -1;
}

static RedParamResType GetParamsType(const std::string& typeStr)
{
	for(int i=0; i<sizeof(RedParamsTypeStringMap)/sizeof(RedParamsTypeStringMap[0]); i++)
	{
		if (RedParamsTypeStringMap[i].typeString == typeStr)
			return RedParamsTypeStringMap[i].type;
	}
	return REDPARAM_UNKNOWN;
}

static RedParamResType GetResourcesType(const std::string& typeStr)
{
	for(int i=0; i<sizeof(RedResourcesTypeStringMap)/sizeof(RedResourcesTypeStringMap[0]); i++)
	{
		if (RedResourcesTypeStringMap[i].typeString == typeStr)
			return RedResourcesTypeStringMap[i].type;
	}
	return REDPARAM_UNKNOWN;
}

static int ParseMeshParameters(Tr2MeshArea* mesh, std::vector<RedLine>& redLines, int start)
{
	int i = SearchREDLine(redLines, "parameters", start) + 1;
	// parsing parameters section
	while(1)
	{
		RedParameter p;
		RedParamResType type = GetParamsType(redLines[i].value);
		if (type == REDPARAM_UNKNOWN)
			break;
		++i;
		switch(type)
		{
		case REDPARAM_VEC4:
			p.name = redLines[i++].value;
			if (redLines[i].id == "value")
			{
				p.value = redLines[i++].value;
			} else
			{
				p.value = "0, 0, 0, 0";
			}
			break;
		case REDPARAM_VEC3:
			p.name = redLines[i++].value;
			if (redLines[i].id == "value")
			{
				p.value = redLines[i++].value;
			} else
			{
				p.value = "0, 0, 0";
			}
			break;
		case REDPARAM_VEC2:
			p.name = redLines[i++].value;
			if (redLines[i].id == "value")
			{
				p.value = redLines[i++].value;
			} else
			{
				p.value = "0, 0";
			}
			break;
		case REDPARAM_FLOAT:
			p.name = redLines[i++].value;
			if (redLines[i].id == "value")
			{
				p.value = redLines[i++].value;
			} else
			{
				p.value = "0";
			}
			break;
		case REDPARAM_VARIABLE:
			p.name = redLines[i++].value;
			p.value = redLines[i++].value;
			break;
		}
		p.type = type;
		p.index_res = -1;
		mesh->parameters.push_back(p);
	}

	return i;
}

static int ParseMeshResources(Tr2MeshArea* mesh, std::vector<RedLine>& redLines, int start, const StuffFile &sf)
{
	int i = SearchREDLine(redLines, "resources", start) + 1;
	// parsing parameters section
	while(1)
	{
		RedParameter p;
		RedParamResType type = GetResourcesType(redLines[i].value);
		if (type == REDPARAM_UNKNOWN)
			break;
		++i;
		switch(type)
		{
		case REDPARAM_TEX2D:
			p.name = redLines[i++].value;
			if (redLines[i].id == "resourcePath")
			{
				p.index_res = SearchStuffByPath(sf, redLines[i].value);
			}
			p.value = redLines[i++].value;
			break;
		}
		p.type = type;
		p.index_res = -1;
		mesh->resources.push_back(p);
	}

	return i;
}

static int ParseOpaqueArea(RedEntity *redEntity, std::vector<RedLine>& redLines, int start, const StuffFile &sf)
{
	while(1)
	{
		if (redLines[start].value != "Tr2MeshArea")
			break;
		++start;

		Tr2MeshArea mesh;
		mesh.name = redLines[start++].value;
		if (redLines[start].id == "index")
		{
			mesh.index = atoi(redLines[start++].value.c_str());
		}
		else
			mesh.index = 0;
		start = ParseMeshParameters(&mesh, redLines, start);
		start = ParseMeshResources(&mesh, redLines, start, sf);
		redEntity->meshes.push_back(mesh);
	}

	return start;
}

static int ParseRED(RedEntity *redEntity, const char *buf, dword bufSize, const StuffFile &sf)
{
	std::vector<RedLine> redLines;
	int nLines = StoreREDLines(redLines, buf, bufSize);
	int line = SearchREDLine(redLines, "highDetailMesh");
	if (line < 0)
		return -1;
	line = SearchREDLine(redLines, "geometryResPath", line+1);
	if (line < 0)
		return -1;

	int index_gr2 = SearchStuffByPath(sf, redLines[line].value);
	if (index_gr2 < 0)
		return -1;

	redEntity->index_gr2 = index_gr2;

	line = SearchREDLine(redLines, "opaqueAreas", line+1);
	if (line < 0)
		return -1;
	
	ParseOpaqueArea(redEntity, redLines, line+1, sf);
	
	return 0;
}

void CMainDlg::LoadRED(int out)
{
	const StuffFileEntry &red_sfe = sf.files[out];
	ifstream *is = red_sfe.handle;
	is->seekg(red_sfe.fileOffset);

	char *red_buf = new char[red_sfe.fileSize];
	is->read(red_buf, red_sfe.fileSize);
/*
	FILE *fp = fopen("test.txt", "w");
	if (fp)
	{
		fwrite(red_buf, 1, red_sfe.fileSize, fp);
		fclose(fp);
	}
*/
	if (redEntity)
		delete redEntity;
	redEntity = new RedEntity;
	
	ParseRED(redEntity, red_buf, red_sfe.fileSize, sf);

	delete [] red_buf;
}

LRESULT CMainDlg::OnSetFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFolderDialog fd(m_hWnd, "Path to EVE's folder:");
	if(fd.DoModal(m_hWnd)!= IDCANCEL)
	{
		CRegKey rk;
		rk.Create(HKEY_CURRENT_USER, "Software\\TriExporter");
		rk.SetStringValue("EVE", fd.m_szFolderPath);
		rk.Close();
		sf.LoadDir(fd.m_szFolderPath);
		FillTree();
	}
	return TRUE;
}
LRESULT CMainDlg::OnScaleTrack(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if(((HWND)lParam) == ((HWND)m_Scale))
	{
		CString str;
		str.Format("%1.2f", (float)((m_Scale.GetPos()/20.0) + 0.05));
		m_eScale.SetWindowText(str);
	}
	if(((HWND)lParam) == ((HWND)m_Light))
	{
		m_p3d.alight = m_Light.GetPos()/(180/D3DX_PI);
	}
	if(((HWND)lParam) == ((HWND)m_UpDown))
	{
		int sel = m_Textures.GetCaretIndex();
		if(sel!=-1)
		{
			switch(m_UpDown.GetPos())
			{
				case 0:
					if(sel)
					{
						swap(textures[sel], textures[sel-1]);
						swap(diffuseColors[sel], diffuseColors[sel-1]);
						swap(texdata[sel], texdata[sel-1]);
						FillTextures();
						m_p3d.SwapTexture(sel, sel-1);
						m_Textures.SetCaretIndex(sel-1);
					}
					break;
				case 2:
					if(sel+1< m_Textures.GetCount())
					{
						swap(textures[sel], textures[sel+1]);
						swap(diffuseColors[sel], diffuseColors[sel+1]);
						swap(texdata[sel], texdata[sel+1]);
						FillTextures();
						m_p3d.SwapTexture(sel, sel+1);
						m_Textures.SetCaretIndex(sel+1);
					}
					break;
			}
		}
		m_UpDown.SetPos(1);
	}
	return TRUE;
}

LRESULT CMainDlg::OnSelect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//CFileDialog fd(TRUE, 0, 0, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "All files (*.*)\0*.*\0\0");
	//if(fd.DoModal(m_hWnd)!= IDCANCEL)
	//{
	//	m_Texture.SetWindowText(fd.m_szFileTitle);
	//}
	this->m_p3d.Reset();
	return 0;
}

LRESULT CMainDlg::OnOpenTri(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fd(TRUE, 0, 0, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "All files (*.tri)\0*.tri\0\0");
	if(fd.DoModal(m_hWnd)!= IDCANCEL)
	{
		string filen;
		filen = fd.m_szFileName;
		Load(filen);
	}
	return 0;
}

LRESULT CMainDlg::OnWireOnOff(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_p3d.SwapFillMode();
	wireMode = !wireMode;
	CheckMenuItem(GetMenu(), ID_CONFIG_WIREON, wireMode ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));
	return 0;
}
LRESULT CMainDlg::OnExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fd(FALSE, 0, 0, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "X files (*.x)\0*.x\0Obj files (*.obj)\0*.obj\0\x33\x64s files (*.3ds)\0*.3ds\0My files (*.my)\0*.my\0\0");
	if(fd.DoModal(this->m_hWnd)!= IDCANCEL)
	{
		void (TriFile::*Export[])(float, string, string)={NULL, &TriFile::ExportX,&TriFile::ExportObj,&TriFile::Export3ds,&TriFile::ExportMy};
		CString ext[] = {"",".x", ".obj", ".3ds", ".my"};
		int extl[] = {0,2,4,4};
		CString efile = fd.m_szFileTitle;
		CString efilepath = fd.m_szFileName;
		CString msg;
		msg.Format("Export to %s file complited!", ext[fd.m_ofn.nFilterIndex].Right(extl[fd.m_ofn.nFilterIndex]-1));
		int lf = efile.GetLength();
		efilepath.Delete(efilepath.GetLength() - lf, lf);
		if(!efile.Right(extl[fd.m_ofn.nFilterIndex]).Compare(ext[fd.m_ofn.nFilterIndex]))
		{
			efile.Delete(lf - extl[fd.m_ofn.nFilterIndex], extl[fd.m_ofn.nFilterIndex]);
		}
		file->textures = textures;
		file->diffuseColors = diffuseColors;
		(file->*Export[fd.m_ofn.nFilterIndex])((float)((m_Scale.GetPos()/20.0) + 0.05), (LPCSTR)efile, (LPCSTR)efilepath);
		ExportTextures(efilepath);
		MessageBox(msg, "Information", MB_ICONINFORMATION|MB_OK);
	}
	return 0;
}

LRESULT CMainDlg::OnUnstuff(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CTreeItem ti = m_Tree.GetSelectedItem();
	CFolderDialog fd(m_hWnd, "Select folder. The files will be unstuffed there:");
	if(fd.DoModal(m_hWnd)!= IDCANCEL)
	{
		CString path = fd.m_szFolderPath;
		if(ti)
		{
			int out = ti.GetData()-1;
			if(out >= 0)
			{
				sf.files[out].Unstuff(path);
			}
			else
			{
				CUnstuffDlg dlg;
				DWORD data[4];
				CString start;
				data[0] = (DWORD)&ti;
				data[1] = (DWORD)&sf;
				data[2] = (DWORD)&path;
				CTreeItem tmpti = ti;
				while(tmpti = tmpti.GetParent())
				{
					CString tmp;
					tmpti.GetText(tmp);
					start = tmp + "/" + start;
				}
				data[3] = (DWORD)&start;
				dlg.DoModal(m_hWnd, (LPARAM)data);
			}
		}	
	}
	MessageBox("Unstuffed done", "Information", MB_ICONINFORMATION|MB_OK);
	return 0;
}

void CMainDlg::ExportTextures(const CString &path)
{
	for(int i = 0; i < m_Textures.GetCount(); i++)
	{
		if(texdata[i] != -1)
		{
			char sfile[512];
			m_Textures.GetText(i, (LPTSTR)&sfile);
			CString file = sfile;
			vector<char> data;
			data.resize(sf.files[texdata[i]].fileSize);
			sf.files[texdata[i]].handle->seekg(sf.files[texdata[i]].fileOffset);
			sf.files[texdata[i]].handle->read(reinterpret_cast<char*>(&data[0]), sf.files[texdata[i]].fileSize);
			ofstream out;
			out.sync_with_stdio(false);
			out.open(path + file, ios::binary);
			out.write(reinterpret_cast<char*>(&data[0]), sf.files[texdata[i]].fileSize);
			out.close();
		}
	}
}

void CMainDlg::Add(const CString &texture, int data, float diffuseR, float diffuseG, float diffuseB)
{
	dword tc = (dword)m_Textures.GetCount();
	if(tc < file->header.numSurfaces)
	{
		textures[tc] = (LPCSTR)texture;
		diffuseColors[tc].r = diffuseR;
		diffuseColors[tc].g = diffuseG;
		diffuseColors[tc].b = diffuseB;
		texdata[tc] = data;
		FillTextures();
	}
	else
	{
		MessageBox("You have to remove one of textures to add another", "Error", MB_ICONERROR | MB_OK);
	}
}

void CMainDlg::FillTextures()
{
	dword tc = (dword)m_Textures.GetCount();
	for(int i = tc; i >= 0 ; --i)
		m_Textures.DeleteString(i);
	for(dword i = 0; i < textures.size() ; i++)
		if(textures[i].compare(""))
			m_Textures.AddString(textures[i].c_str());
}

LRESULT CMainDlg::OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString texture;
	m_Texture.GetWindowText(texture);
	Add(texture);
	m_p3d.TextureChange(sf, texdata);
	return 0;
}

LRESULT CMainDlg::OnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	vector<string> tmp;
	vector<int> tmp1;
	tmp = textures;
	dword tc = (dword)m_Textures.GetCount();
	int sel = m_Textures.GetCaretIndex();
	if(sel!=-1)
	{
		for(dword i = 0, c = 0; i < tc; i++, c++)
		{
			if(i == sel)
				c++;
			if(c < tc)
			{
				textures[i] = textures[c];
				diffuseColors[i] = diffuseColors[c];
				texdata[i] = texdata[c];
			}
			else
			{
				textures[i] = "";
				diffuseColors[i].r = 1;
				diffuseColors[i].g = 1;
				diffuseColors[i].b = 1;
				texdata[i] = -1;
			}
		}
		FillTextures();
		m_p3d.TextureChange(sf, texdata);
	}
	return 0;
}

void CMainDlg::EnableAll(BOOL bEnable)
{
	for(int i = m_Textures.GetCount(); i >= 0 ; i--)
		m_Textures.DeleteString(i);
	m_Light.EnableWindow(bEnable);
	m_Scale.EnableWindow(bEnable);
	m_eScale.EnableWindow(bEnable);
	m_Scale.SetPos(19);
	m_Texture.EnableWindow(bEnable);
	m_Textures.EnableWindow(bEnable);
	m_Add.EnableWindow(bEnable);
	m_Select.EnableWindow(bEnable);
	m_Remove.EnableWindow(bEnable);
	m_eScale.SetWindowText("1.0");
	m_Texture.SetWindowText("");
	m_VSize.SetWindowText("0");
	m_UpDown.SetPos(1);
	m_UpDown.EnableWindow(bEnable);
	m_TriVersion.SetWindowText("0.0");
	m_VCount.SetWindowText("0");
	m_BoxMax.SetWindowText("{0.0; 0.0; 0.0}");
	m_BoxMin.SetWindowText("{0.0; 0.0; 0.0}");
	m_SurType.SetWindowText("0");
	m_SurCount.SetWindowText("0");
	if(bEnable)
		menu.EnableMenuItem(ID_FILE_EXPORTMOD, MF_ENABLED);
	else
	{
		menu.EnableMenuItem(ID_FILE_EXPORTMOD, MF_GRAYED);
		SetWindowText("TriExporter");
	}
}

void CMainDlg::FillTree()
{
	m_Tree.DeleteAllItems();
	lvis.clear();
	string old = "";
	lvis[old] = TVI_ROOT;
	for(dword i = 0; i < sf.files.size(); i++)
	{
		CString f = sf.files[i].filename.c_str();
		//f.Replace('\\', '/');
		int u = 0;
		while((u = f.Find('/', u+1)) != -1)
		{
			CString n = f.Left(u);
			if(!lvis[(string)n])
			{
				CTreeItem a = m_Tree.InsertItem(n.Right(n.GetLength()- (old.length()==0?-1:old.length())-1),  1, 1, lvis[old] , TVI_SORT);
				a.SetData(0);
				lvis[(string)n] = a;
			}	
			old = n;
		}
	}
	for(dword i = 0; i < sf.files.size(); i++)
	{
		CString f = sf.files[i].filename.c_str();
		//f.Replace('\\', '/');
		int u = 0;
		while((u = f.Find('/', u+1)) != -1)
		{
			CString n = f.Left(u);
			if(!lvis[(string)n])
			{
				CTreeItem a = m_Tree.InsertItem(n.Right(n.GetLength()- (old.length()==0?-1:old.length())-1),  1, 1, lvis[old] , TVI_SORT);
				a.SetData(0);
				lvis[(string)n] = a;
			}	
			old = n;
		}
		CString fil;
		fil.Format("%s - %1.2fKB", f.Right(f.GetLength()-f.ReverseFind('/')-1), sf.files[i].fileSize/1024.0);
		CTreeItem fe;
		HTREEITEM order = TVI_LAST; //TVI_SORT
		if(!f.Right(4).Compare(".dds"))
			fe = m_Tree.InsertItem(fil, 2,2, lvis[old] , order);
		else if (!f.Right(4).Compare(".tri"))
			fe = m_Tree.InsertItem(fil, lvis[old] , order);
		else if (!f.Right(5).Compare(".blue"))
			fe = m_Tree.InsertItem(fil, 3,3, lvis[old] , order);
		else if (!f.Right(4).Compare(".ogg") || !f.Right(4).Compare(".wav") || !f.Right(4).Compare(".m3u") || !f.Right(4).Compare(".mp3"))
			fe = m_Tree.InsertItem(fil, 4,4, lvis[old] , order);
		else if (!f.Right(4).Compare(".jpg"))
			fe = m_Tree.InsertItem(fil, 5,5, lvis[old] , order);
		else if (!f.Right(4).Compare(".png"))
			fe = m_Tree.InsertItem(fil, 6,6, lvis[old] , order);
		else if (!f.Right(3).Compare(".py"))
			fe = m_Tree.InsertItem(fil, 7,7, lvis[old] , order);
		else if (!f.Right(4).Compare(".txt"))
			fe = m_Tree.InsertItem(fil, 8,8, lvis[old] , order);
		else if (!f.Right(4).Compare(".tga"))
			fe = m_Tree.InsertItem(fil, 9,9, lvis[old] , order);
		else if (!f.Right(3).Compare(".mb"))
			fe = m_Tree.InsertItem(fil, 9,9, lvis[old] , order);
		else if (!f.Right(4).Compare(".gr2"))
			fe = m_Tree.InsertItem(fil, 11,11, lvis[old] , order);
		else
			fe = m_Tree.InsertItem(fil, 10,10, lvis[old] , order);
		fe.SetData(i+1);
	}
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	//DlgResize_Init();
	CenterWindow();
	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_TRIEXPORTER), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SMALL), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);
	file = new TriFile;
	redEntity = 0;
	// register object for message filtering and idle updates
	loaded = false;
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	//UIAddChildWindowContainer(m_hWnd);
	menu.Attach(GetMenu());
	m_Tree = GetDlgItem(IDC_TREE);
	il.Create(16,16, ILC_COLOR32,0,0);
	HANDLE bmp = LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_ICONS),IMAGE_BITMAP,0,0,LR_LOADTRANSPARENT);
	il.Add((HBITMAP)bmp);
	DeleteObject(bmp);
	m_Tree.SetImageList(il, TVSIL_NORMAL);
	m_p3d.SubclassWindow(GetDlgItem(IDC_PREVIEW));
	m_Scale.Attach(GetDlgItem(IDC_SCALE));
	m_Light.Attach(GetDlgItem(IDC_LIGHT));
	m_Fps.Attach(GetDlgItem(IDC_FPS));
	m_eScale.Attach(GetDlgItem(IDC_ESCALE));
	m_Texture.Attach(GetDlgItem(IDC_TEXTURE));
	m_VSize.Attach(GetDlgItem(IDC_VSIZE));
	m_TriVersion.Attach(GetDlgItem(IDC_TRIVERSION));
	m_VCount.Attach(GetDlgItem(IDC_VCOUNT));
	m_BoxMax.Attach(GetDlgItem(IDC_BOXMAX));
	m_BoxMin.Attach(GetDlgItem(IDC_BOXMIN));
	m_SurType.Attach(GetDlgItem(IDC_SURTYPE));
	m_SurCount.Attach(GetDlgItem(IDC_SURCOUNT));
	m_Textures.Attach(GetDlgItem(IDC_TEXTURES));
	m_Add.Attach(GetDlgItem(IDC_ADD));
	m_Remove.Attach(GetDlgItem(IDC_REMOVE));
	m_Select.Attach(GetDlgItem(IDC_SELECT));
	m_Scale.SetRange(0, 19);
	m_Scale.SetPageSize(5);
	m_UpDown.Attach(GetDlgItem(IDC_UPDOWN));
	m_Light.SetRange(0, 360);
	m_Light.SetPageSize(1);
	m_Light.SetPos(100);
	m_p3d.alight = 100/(180/D3DX_PI);
	CRegKey rk;
	if(rk.Open(HKEY_CURRENT_USER, "Software\\TriExporter") == ERROR_SUCCESS)
	{
		ULONG g=256;
		TCHAR path[256];
		rk.QueryStringValue("EVE", path, &g);
		rk.Close();
		sf.LoadDir(path);
		FillTree();
	}
	m_p3d.Render();
	rotateOnIdle = true;
	CheckMenuItem(GetMenu(), ID_CONFIG_ROTATEMODEL, rotateOnIdle ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));
	wireMode = false;
	EnableAll(FALSE);
	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	CloseDialog(0);
	return 0;
}

LRESULT CMainDlg::OnConfigRotate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	rotateOnIdle = !rotateOnIdle;
	CheckMenuItem(GetMenu(), ID_CONFIG_ROTATEMODEL, rotateOnIdle ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	m_p3d.Detach();
	il.Destroy();
	delete file;
	DestroyWindow();
	::PostQuitMessage(nVal);
}


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

void CMainDlg::UpdateFPS()
{
	const int FRAMEINTERVAL = 1000;
	static DWORD nFrames = 0;
	static DWORD nLastTick = GetTickCount();
	DWORD nTick = GetTickCount();
	if(nTick-nLastTick>=FRAMEINTERVAL)
	{	
		float fFPS = 1000.0f*(float)nFrames/(float)(nTick-nLastTick);
		nLastTick = nTick;
		nFrames = 0;
		char szFPS[256];
		sprintf_s(szFPS,"%.2f", fFPS);
		//snprintf(szFPS, "%.2f", fFPS);
		m_Fps.SetWindowText(szFPS);
	}
	nFrames++;
}

BOOL CMainDlg::OnIdle()
{
	if (rotateOnIdle)
	{
		UpdateFPS();
		m_p3d.Render();
	}
	return FALSE;
}

LRESULT CMainDlg::OnTvnKeydownTree(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	if(pTVKeyDown->wVKey == VK_SPACE)
		OnTreeDblClick(idCtrl, pNMHDR, bHandled);
	return 0;
}
