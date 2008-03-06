#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#ifdef DEBUG_DEP
#warning "utilities.h"
#endif

#include "utilities-pre.h"

#include "vector3d.h"

#include <fstream>
#include <vector>

namespace Utilities
{
	class ConfigurationFile
	{
		private:
			Hashtable mData;
			char* mFile;
	
		public:
			ConfigurationFile(void);
			ConfigurationFile(const std::string);
			
			~ConfigurationFile(void);
			
			void SetFile(const std::string);
			
			int Parse(void);
			int Update(std::string = "");
			void Clear(void);
			
			const char* GetValue(std::string);
			void        SetValue(std::string, std::string);
	};
	
	struct StructuredInstructionsItem
	{
		std::string instruction;
		std::string value;
	};	
	
	typedef std::vector<StructuredInstructionsItem*> StructuredInstructionsVector;
	
	class StructuredInstructionsFile
	{
		private:
			char* mFile;
			int   mIndex;
			int   mLength;
			StructuredInstructionsVector mItems;
			
		public:
			StructuredInstructionsFile(void);
			StructuredInstructionsFile(const std::string);
			
			~StructuredInstructionsFile(void);
			
			void SetFile(const std::string);
			
			int Parse(void);
			
			const StructuredInstructionsVector* GetInstructionVector(void) const;
			void GetInstructionVector(StructuredInstructionsVector*);
			
			void PrepareIterator(void);
			bool End(void) const;
			void GotoItem(int index);
			StructuredInstructionsItem* NextItem(void);
	};

	void ReadLineFromFile(std::ifstream& file, std::string& buffer);

	// Tar bort intendentering
	void StringRemoveIntendent(std::string& target);
	
	// Klipper en sträng från position 0 tills slutet av strängen - eller bokstaven/symbolen ending påträffas
	int StringCut(std::string target, std::string& result, char ending);
	
	//Trims ' ' and '\t' from string.
	void StringTrim(std::string target, std::string& result);
	
	// Returns end of token. Stores next token in result. ',' is considered a token.
	// Example: "0.1," Token[0] = "0.1" and Token[1] = ","
	// Handles token types: Normal (word), String type ("info mation")
	int StringGetToken(std::string target, std::string& result, int start);
	
	// Ensures that target consists of numbers. If letters were found, no is returned.
	int StringToInt(std::string target, int no = 0);

	// Validates Number
	bool IsCoordinate(std::string target);

	// Loads A Texture and converts it into OpenGL texture, it supports all formats that SDL_image can support with or without alpha.
	// Gives a 32 bit OpenGL texture
	GLuint LoadTexture(std::string path);
	SDL_Surface *LoadImage(std::string path);

	GLuint CreateGLTexture(SDL_Surface*);
		
	void WorldCoordToWindowCoord(Vector3D world_coord, Vector3D &windows_coord);
	void WindowCoordToVector(GLdouble clickx, GLdouble clicky, Vector3D &pos_vector_near, Vector3D &pos_vector_far);
	
	bool CheckLineIntersectTri(Vector3D tp1, Vector3D tp2, Vector3D tp3, Vector3D lp1, Vector3D lp2, Vector3D &hit_pos);
	bool CheckLineIntersect(Vector3D tp1, Vector3D tp2, Vector3D tp3, Vector3D lp1, Vector3D lp2, Vector3D &hit_pos);
	
	int SwitchTo2DViewport(float w, float h);
	int RevertViewport();
	
	Vector3D GetOGLPos(int x, int y);
	
	float InterpolateCatmullRom(float v0, float v1, float v2, float v3, float x);
	Vector3D InterpolateCatmullRom(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3, float x);
	float InterpolateCatmullRomBounded(float v0, float v1, float v2, float v3, float x);
	Vector3D InterpolateCatmullRomBounded(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3, float x);

}

#ifdef DEBUG_DEP
#warning "utilities.h-end"
#endif

#define __UTILITIES_H_END__

#endif
