#include "vector3d.h"
#include "terrain.h"
#include <cmath>

namespace Utilities
{
	Vector3D::Vector3D(void)
	{
		x = 0;
		y = 0;
		z = 0;
	}
			
	Vector3D::Vector3D(const Vector3D& a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
	}
	
	Vector3D::Vector3D(Game::Dimension::XYZCoord *a)
	{
		x = a->x;
		y = a->y;
		z = a->z;
	}
	
	Vector3D::Vector3D(Game::Dimension::UVWCoord *a)
	{
		x = a->u;
		y = a->v;
		z = a->w;
	}
	
	Vector3D::Vector3D(Game::Dimension::SphereNormal *a)
	{
		x = sin(a->phi) * cos(a->theta);
		y = sin(a->phi) * sin(a->theta);
		z = cos(a->phi);
	}
	
	Vector3D::Vector3D(float newX, float newY, float newZ)
	{
		x = newX;
		y = newY;
		z = newZ;
	}

	Vector3D& Vector3D::operator = (const Vector3D& a)
	{
		x = a.x; 
		y = a.y;
		z = a.z;
		
		return *this;
	}
	
	bool Vector3D::operator == (const Vector3D& a) const
	{
		return a.x == x && a.y == y && a.z == z;
	}
	
	bool Vector3D::operator != (const Vector3D& a) const
	{
		return a.x != x || a.y != y || a.z != z;
	}
	
	void Vector3D::zero(void)
	{
		x = y = z = 0;
	}
	
	Vector3D Vector3D::operator-(void) const
	{
		return Vector3D(-x, -y, -z);
	}
	
	Vector3D Vector3D::operator +(const Vector3D& a) const
	{
		return Vector3D(x + a.x, y + a.y, z + a.z);
	}
	
	Vector3D Vector3D::operator +(float a) const
	{
		return Vector3D(x + a, y + a, z + a);
	}
	
	Vector3D Vector3D::operator -(const Vector3D& a) const
	{
		return Vector3D(x - a.x, y - a.y, z - a.z);
	}
	
	Vector3D Vector3D::operator -(float a) const
	{
		return Vector3D(x - a, y - a, z - a);
	}
	
	Vector3D Vector3D::operator *(const Vector3D& a) const
	{
		return Vector3D(x * a.x, y * a.y, z * a.z);
	}
	
	Vector3D Vector3D::operator *(float a) const
	{
		return Vector3D(x * a, y * a, z * a);
	}

	Vector3D Vector3D::operator /(float a) const
	{
		float value = 1.0f / a;
		return Vector3D(
		                x * value,
		                y * value,
		                z * value
		               );
	}
	
	Vector3D& Vector3D::operator += (const Vector3D& a)
	{
		x += a.x;
		y += a.y;
		z += a.z;
		
		return *this;
	}
	
	Vector3D& Vector3D::operator -= (const Vector3D& a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
		
		return *this;
	}
	
	Vector3D& Vector3D::operator *= (float a)
	{
		x *= a;
		y *= a;
		z *= a;
		
		return *this;
	}
	
	Vector3D& Vector3D::operator /= (float a)
	{
		float value = 1.0f / a;
		x *= value;
		y *= value;
		z *= value;
		
		return *this;
	}
	
	void Vector3D::normalize(void)
	{
		float magnitude = sqrt(x*x + y*y + z*z);
		if (magnitude > 0.0f)
		{
			float value = 1.0f / magnitude;
			x *= value;
			y *= value;
			z *= value;
		}
	}
	
	float Vector3D::dot(const Vector3D& a) const
	{
		return x*a.x + y*a.y + z*a.z;
	}

	float Vector3D::angle(const Vector3D& a) const
	{
		Vector3D temp1, temp2;
		temp1 = *this;
		temp2 = a;
		temp1.normalize();
		temp2.normalize();
		return acos(temp1.dot(temp2));
	}

	void Vector3D::cross(const Vector3D& a)
	{
		Vector3D temp = *this;
		x = temp.y*a.z - temp.z*a.y;
		y = temp.z*a.x - temp.x*a.z;
		z = temp.x*a.y - temp.y*a.x;
	}

	void Vector3D::set(float vx, float vy, float vz)
	{
		x = vx;
		y = vy;
		z = vz;
	}

	void Vector3D::rotateX(float degrees)
	{
		Vector3D temp = *this;
		float rad = degrees * float(PI / 180);

		y = 	cos(rad)*temp.y-
			sin(rad)*temp.z;

		z = 	sin(rad)*temp.y+ 
			cos(rad)*temp.z;
	}

	void Vector3D::rotateY(float degrees)
	{
		Vector3D temp = *this;
		float rad = degrees * float(PI / 180);

		x = 	cos(rad)*temp.x-
			sin(rad)*temp.z;

		z = 	sin(rad)*temp.x+ 
			cos(rad)*temp.z;
	}

	void Vector3D::rotateZ(float degrees)
	{
		Vector3D temp = *this;
		float rad = degrees * float(PI / 180);

		x = 	cos(rad)*temp.x-
			sin(rad)*temp.y;

		y = 	sin(rad)*temp.x+ 
			cos(rad)*temp.y;
	}

	void Vector3D::rotate(float degrees, const Vector3D& axis)
	{
		this->rotateX(degrees * axis.x);
		this->rotateY(degrees * axis.y);
		this->rotateZ(degrees * axis.z);
	}

	float Vector3D::distance(const Vector3D& a) const
	{
		return sqrt((x - a.x) * (x - a.x) + (y - a.y) * (y - a.y) + (z - a.z) * (z - a.z));
	}
	
	std::ostream& operator<<(std::ostream& out, const Vector3D& v)
	{
		out << "{" << v.x << ", " << v.y << ", " << v.z << "}";
		return out;
	}
}

