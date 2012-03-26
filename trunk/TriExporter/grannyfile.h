#pragma once

#include "stufffile.h"
#include "trifile.h"
#include "half.h"



class GrannyTriFile : public TriFile
{
	private:
		//typedef float (__thiscall GrannyTriFile::* convertFunc)(unsigned char* value);
	
		//struct VertexComponent
		//{
		//	const char* name;		// name of the component as used in granny files
		//	int index;				// index of -1 means it doesn't exist
		//	bool exists;			// does this component exit
		//	convertFunc convert;	// convertion function to convert from this components type to 32bit float
		//	unsigned int offset;	// offset relative to the start of the vertex at which this component starts
		//	unsigned int typeSize;	// size in bytes of the type of this component
		//};
		//static VertexComponent vertexComponents_[];

	public:
		GrannyTriFile();
		virtual bool LoadFile(StuffFileEntry &sfe);
		virtual bool LoadFile(string filename);
		virtual bool LoadFile(ifstream &is);
	private:
		static bool loadedstuff;
		static bool dllloaded;
		//inline int getTypeSize(int type) 
		//{
		//	switch(type) 
		//	{
		//		case 10: return 4;  // 32bit float
		//		case 11:            // char
		//		case 12:            // unsigned char
		//		case 14: return 1;  // unsigned char
		//		case 21: return 2;  // 16bit float
		//		default: return 0;
		//	}
		//}

		//float convertDummy(unsigned char* value) 
		//{
		//	return 0.0f;
		//}

		//float convert10(unsigned char* value) 
		//{
		//	return *(float*)value;
		//}

		//float convert11(unsigned char* value) 
		//{
		//	return *(char*)value;
		//}
		//
		//float convert12or14(unsigned char* value) 
		//{
		//	return *(unsigned char*)value;
		//}

		//float convert21(unsigned char* value) 
		//{
		//	unsigned int result = half_to_float(*(unsigned short*)value);
		//	return *(float*)&result;
		//}

		//inline convertFunc getConvertFunc(int type) 
		//{
		//	switch (type) 
		//	{
		//		case 10: return &GrannyTriFile::convert10;
		//		case 11: return &GrannyTriFile::convert11;
		//		case 12:
		//		case 14: return &GrannyTriFile::convert12or14;
		//		case 21: return &GrannyTriFile::convert21;
		//		default: return &GrannyTriFile::convertDummy;
		//	}
		//}
};