
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
	
	void Matrix4x4::Set(float matrix[4][4])
	{
		memcpy(this->matrix, matrix, sizeof(this->matrix));
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
	
	Vector3D Vector3D::operator *(const Matrix4x4& a) const
	{
		Vector3D newvec;
		newvec.x = x * a.matrix[0][0] + y * a.matrix[1][0] + z * a.matrix[2][0] + a.matrix[3][0];
		newvec.y = x * a.matrix[0][1] + y * a.matrix[1][1] + z * a.matrix[2][1] + a.matrix[3][1];
		newvec.z = x * a.matrix[0][2] + y * a.matrix[1][2] + z * a.matrix[2][2] + a.matrix[3][2];
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
		float c = cos(radians), s = sin(radians);
		m.matrix[1][1] = c;
		m.matrix[1][2] = -s;
		m.matrix[2][1] = s;
		m.matrix[2][2] = c;
		return m;
	}
	
	Matrix4x4 Matrix4x4::RotateYMatrix(float radians)
	{
		Matrix4x4 m;
		float c = cos(radians), s = sin(radians);
		m.matrix[0][0] = c;
		m.matrix[0][2] = s;
		m.matrix[2][0] = -s;
		m.matrix[2][2] = c;
		return m;
	}
	
	Matrix4x4 Matrix4x4::RotateZMatrix(float radians)
	{
		Matrix4x4 m;
		float c = cos(radians), s = sin(radians);
		m.matrix[0][0] = c;
		m.matrix[0][1] = -s;
		m.matrix[1][0] = s;
		m.matrix[1][1] = c;
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
	
	void Matrix4x4::Rotate(float x, float y, float z)
	{
		MulBy(RotateXMatrix(x) * RotateYMatrix(y) * RotateZMatrix(z));
	}
	
	void Matrix4x4::Rotate(Vector3D v)
	{
		MulBy(RotateXMatrix(v.x) * RotateYMatrix(v.y) * RotateZMatrix(v.z));
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
	
}
