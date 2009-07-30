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

#include "matrix4x4.h"
#include "vector3d.h"
#include <cmath>

namespace Utilities
{

	Matrix4x4::Matrix4x4()
	{
		Identity();
	}
	
	Matrix4x4::Matrix4x4(float matrix[4][4])
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				this->matrix[i][j] = matrix[i][j];
			}
		}
	}
	
	Matrix4x4::~Matrix4x4()
	{
		
	}
	
	Matrix4x4& Matrix4x4::operator =(const Matrix4x4& a)
	{
		Set(a.matrix);
		return *this;
	}

	void Matrix4x4::Identity()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				matrix[i][j] = i == j ? 1.0f : 0.0f;
			}
		}
	}

	void Matrix4x4::Zero()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				matrix[i][j] = 0.0f;
			}
		}
	}
	
	void Matrix4x4::Set(const float a[4][4])
	{
		memcpy(this->matrix, a, sizeof(this->matrix));
	}

	void Matrix4x4::MulBy(const Matrix4x4& a)
	{
		float newmat[4][4];
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				newmat[j][i] = 0.0f;
				for (int q = 0; q < 4; q++)
				{
					newmat[j][i] += matrix[q][i] * a.matrix[j][q];
				}
			}
		}
		Set(newmat);
	}

	Matrix4x4 Matrix4x4::operator *(const Matrix4x4& a) const
	{
		Matrix4x4 newmat = *this;
		newmat.MulBy(a);
		return newmat;
	}
	
	Vector3D Matrix4x4::operator *(const Vector3D& a) const
	{
		Vector3D newvec;
		newvec.x = a.x * matrix[0][0] + a.y * matrix[1][0] + a.z * matrix[2][0] + matrix[3][0];
		newvec.y = a.x * matrix[0][1] + a.y * matrix[1][1] + a.z * matrix[2][1] + matrix[3][1];
		newvec.z = a.x * matrix[0][2] + a.y * matrix[1][2] + a.z * matrix[2][2] + matrix[3][2];
		return newvec;
	}

	Matrix4x4 Matrix4x4::TranslationMatrix(float x, float y, float z)
	{
		Matrix4x4 m;
		m.matrix[3][0] = x;
		m.matrix[3][1] = y;
		m.matrix[3][2] = z;
		return m;
	}
	
	Matrix4x4 Matrix4x4::ScaleMatrix(float x, float y, float z)
	{
		Matrix4x4 m;
		m.matrix[0][0] = x;
		m.matrix[1][1] = y;
		m.matrix[2][2] = z;
		return m;
	}
	
	Matrix4x4 Matrix4x4::RotateXMatrix(float radians)
	{
		Matrix4x4 m;
		float c = cos(-radians), s = sin(-radians);
		m.matrix[1][1] = c;
		m.matrix[1][2] = -s;
		m.matrix[2][1] = s;
		m.matrix[2][2] = c;
		return m;
	}
	
	Matrix4x4 Matrix4x4::RotateYMatrix(float radians)
	{
		Matrix4x4 m;
		float c = cos(-radians), s = sin(-radians);
		m.matrix[0][0] = c;
		m.matrix[0][2] = s;
		m.matrix[2][0] = -s;
		m.matrix[2][2] = c;
		return m;
	}
	
	Matrix4x4 Matrix4x4::RotateZMatrix(float radians)
	{
		Matrix4x4 m;
		float c = cos(-radians), s = sin(-radians);
		m.matrix[0][0] = c;
		m.matrix[0][1] = -s;
		m.matrix[1][0] = s;
		m.matrix[1][1] = c;
		return m;
	}

	Matrix4x4 Matrix4x4::RotateMatrix(float radians, float x, float y, float z)
	{
		Matrix4x4 m;
		float c = cos(-radians), s = sin(-radians), t = 1 - c;
		float tx = t * x, ty = t * y, tz = t * z;
		float txy = tx * y, txz = tx * z, tyz = ty * z;
		float sx = s * x, sy = s * y, sz = s * z;
		m.matrix[0][0] = tx * x + c;
		m.matrix[0][1] = txy - sz;
		m.matrix[0][2] = txz + sy;
		m.matrix[1][0] = txy + sz;
		m.matrix[1][1] = ty * y + c;
		m.matrix[1][2] = tyz - sx;
		m.matrix[2][0] = txz - sy;
		m.matrix[2][1] = tyz + sx;
		m.matrix[2][2] = tz * z + c;
		return m;
	}

	void Matrix4x4::Translate(float x, float y, float z)
	{
		MulBy(TranslationMatrix(x, y, z));
	}
	
	void Matrix4x4::Translate(Vector3D v)
	{
		MulBy(TranslationMatrix(v.x, v.y, v.z));
	}
	
	void Matrix4x4::RotateX(float radians)
	{
		MulBy(RotateXMatrix(radians));
	}
	
	void Matrix4x4::RotateY(float radians)
	{
		MulBy(RotateYMatrix(radians));
	}
	
	void Matrix4x4::RotateZ(float radians)
	{
		MulBy(RotateZMatrix(radians));
	}
	
	void Matrix4x4::Rotate(float radians, float x, float y, float z)
	{
		MulBy(RotateMatrix(radians, x, y, z));
	}
	
	void Matrix4x4::Rotate(float radians, Vector3D v)
	{
		MulBy(RotateMatrix(radians, v.x, v.y, v.z));
	}

	void Matrix4x4::Scale(float x, float y, float z)
	{
		MulBy(ScaleMatrix(x, y, z));
	}
	
	void Matrix4x4::Scale(float amount)
	{
		MulBy(ScaleMatrix(amount, amount, amount));
	}
	
	void Matrix4x4::Scale(Vector3D v)
	{
		MulBy(ScaleMatrix(v.x, v.y, v.z));
	}
	
	void Matrix4x4::Apply()
	{
		glLoadMatrixf((GLfloat*)matrix);
	}
	
	std::ostream& operator << (std::ostream& os, const Matrix4x4& a)
	{
		for (int i = 0; i < 4; i++)
		{
			os << (i ? " {" : "{{");
			for (int j = 0; j < 4; j++)
			{
				std::cout << a.matrix[i][j];
				os << (j == 3 ? "}" : ", ");
			}
			os << (i == 3 ? "}" : ",") << std::endl;
		}
		return os;
	}

}
