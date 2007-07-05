#ifndef __VECTOR3D_H_PRE__
#define __VECTOR3D_H_PRE__

#ifdef DEBUG_DEP
#warning "vector3d.h-pre"
#endif

#include <iostream>

namespace Utilities
{
	class Vector3D;
}

#define PI 3.14159265

#define __VECTOR3D_H_PRE_END__

#include "terrain.h"

#endif

#ifdef __TERRAIN_H_PRE_END__

#ifndef __VECTOR3D_H__
#define __VECTOR3D_H__

#ifdef DEBUG_DEP
#warning "vector3d.h"
#endif

using namespace std;

namespace Utilities
{
	
	// ATT GÖRA:
	// > beräkna vektor magnituden. Extern funktion.
	class Vector3D
	{
		public:
			float x, y, z;
		
		// Konstruering
			
			// Tom konstruktör
			Vector3D(void);//				     : x(0),     y(0),    z(0)	 {}
			
			// Kopieringskonstruktör
			Vector3D(const Vector3D& a);//                  : x(a.x) , y(a.y) , z(a.z)  {}
			
			// Konstruktör från XYZCoord
			Vector3D(Game::Dimension::XYZCoord *a);//                  : x(a->x) , y(a->y) , z(a->z)  {}
			
			// Konstruktör från UVWCoord
			Vector3D(Game::Dimension::UVWCoord *a);//                  : x(a->u) , y(a->v) , z(a->w)  {}
			
			// Konstruktör från SphereNormal
			Vector3D(Game::Dimension::SphereNormal *a);//                  : x(a->u) , y(a->v) , z(a->w)  {}
			
			// Konstruktör med 3 initiella värden
			Vector3D(float newX, float newY, float newZ);// : x(newX), y(newY), z(newZ) {}
			
		// Objekthantering
		
			// defininering
			Vector3D& operator = (const Vector3D& a);
			
			// Kontrollera likvärdighet
			bool operator == (const Vector3D& a) const;
			bool operator != (const Vector3D& a) const;
			
		// Vektorhantering
		
			// Nollställning
			void zero(void);
			
			// Returnera negerad version
			Vector3D operator-(void) const;
			
			// Addition och subtraktion
			Vector3D operator +(const Vector3D& a) const;
			Vector3D operator +(float a) const;
			Vector3D operator -(const Vector3D& a) const;
			Vector3D operator -(float a) const;
			
			// Multiplikation och heltalsdivision
			Vector3D operator *(const Vector3D& a) const;
			Vector3D operator *(float a) const;
			Vector3D operator /(float a) const;
			
			// Hantering av C "förkortningar"
			Vector3D& operator += (const Vector3D& a);
			Vector3D& operator -= (const Vector3D& a);
			Vector3D& operator *= (float a);
			Vector3D& operator /= (float a);
			
			// Normalisering
			void normalize(void);
			
			// Dot-product
			// Dvs. vinkeln mellan två vektorer
			//
			// \       /
			//  \     /
			//   \_v_/
			//    \ /
			// v = dot product°
			float dot(const Vector3D& a) const;
			
			// Cross-product
			// Dvs normalen för ytan som de två vektorerna this och a ligger på
			void cross(const Vector3D& a);
	
			// kalkylerar vinkeln i radianer mellan två vektorer
			float angle(const Vector3D& a) const;
	
			// kalkylera distansen mellan två punkter
			float distance(const Vector3D& a) const;
	
			// sätter en vektor till ett visst värde
			void set(float vx, float vy, float vz);
			
			// rotera runt X-axeln
			void rotateX(float degrees);
			
			// rotera runt Y-axeln
			void rotateY(float degrees);
			
			// rotera runt Z-axeln
			void rotateZ(float degrees);
	
			// rotera runt vilken axel som helst
			void rotate(float degrees, const Vector3D& axis);
	};
	
	ostream& operator<<(ostream& out, const Vector3D& v);
	
}

#define __VECTOR3D_H_END__

#ifdef DEBUG_DEP
#warning "vector3d.h-end"
#endif

#endif
#endif

