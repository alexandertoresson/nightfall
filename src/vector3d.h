/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef VECTOR3D_H
#define VECTOR3D_H

#ifdef DEBUG_DEP
#warning "vector3d.h"
#endif

#include "vector3d-pre.h"

#include "terrain-pre.h"
#include "matrix4x4-pre.h"
#include <ostream>

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
			Vector3D(const Game::Dimension::XYZCoord& a);//                  : x(a->x) , y(a->y) , z(a->z)  {}
			
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
			
			// Multiplikation och division
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
	
	std::ostream& operator<<(std::ostream& out, const Vector3D& v);
	
}

#ifdef DEBUG_DEP
#warning "vector3d.h-end"
#endif

#endif
