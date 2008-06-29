#include "camera.h"

#include "unit.h"
#include "vector3d.h"
#include "matrix4x4.h"

namespace Game
{
	namespace Dimension
	{
		float cameraRotationSpeed = 20.0f;
		float cameraFlySpeed      = 1.5f;
		float cameraZoomSpeed     = 40.0f;

		void Camera::ApplyMatrix()
		{
			PushMatrix(MATRIXTYPE_MODELVIEW);

			CheckPosition();

			// Emulate gluLookAt

			Utilities::Vector3D f, upn, s, u;
			Utilities::Matrix4x4 m;

			f = mView;
			f.normalize();

			upn = mUp;
			upn.normalize();

			s = f;
			s.cross(upn);

			u = s;
			u.cross(f);

			m.matrix[0][0] = s.x;
			m.matrix[1][0] = s.y;
			m.matrix[2][0] = s.z;

			m.matrix[0][1] = u.x;
			m.matrix[1][1] = u.y;
			m.matrix[2][1] = u.z;

			m.matrix[0][2] = -f.x;
			m.matrix[1][2] = -f.y;
			m.matrix[2][2] = -f.z;

			matrices[MATRIXTYPE_MODELVIEW].MulBy(m);
			matrices[MATRIXTYPE_MODELVIEW].Translate(-(mPosition + mView));

		}
		
		void Camera::PostRender()
		{
			PopMatrix(MATRIXTYPE_MODELVIEW);
		}

		// Set camera settings manually to a specific setting
		void Camera::SetCamera(Utilities::Vector3D focus, GLfloat zoom, GLfloat rotation)
		{
			mFocus = focus;
			mZoom = zoom;
			mRotation = rotation;
			CheckPosition();
		}

		Utilities::Vector3D Camera::GetFocus()
		{
			return mFocus;
		}

		GLfloat Camera::GetZoom()
		{
			return mZoom;
		}

		GLfloat Camera::GetRotation()
		{
			return mRotation;
		}

		void Camera::SetCamera(Unit* unit, GLfloat zoom, GLfloat rotation)
		{
			mFocus = GetTerrainCoord(unit->pos.x, unit->pos.y);
			mZoom = zoom;
			mRotation = rotation;
			CheckPosition();
		}

		void Camera::SetFocus(float terrain_x, float terrain_y)
		{
			mFocus.x = (terrain_x / 8) - terrainOffsetX;
			mFocus.z = (terrain_y / 8) - terrainOffsetY;
			CheckPosition();
		}

		// Go through the camera settings, check that they are correct, and set mPosition and mView accordingly after mFocus, mZoom and mRotation
		void Camera::CheckPosition()
		{
			if (mZoom < 5.0f) 
				mZoom = 5.0f;
			if (mZoom > 67.5f) 
				mZoom = 67.5f;
			
			// Check so x and z of mFocus isn't outside the map
			if (mFocus.x <= -terrainOffsetX + 1)
				mFocus.x = -terrainOffsetX + 1;
			else if (mFocus.x >= terrainOffsetX - 1)
				mFocus.x = terrainOffsetX - 1;
			
			if (mFocus.z <= -terrainOffsetY + 1)
				mFocus.z = -terrainOffsetY + 1;
			else if (mFocus.z >= terrainOffsetY - 1)
				mFocus.z = terrainOffsetY - 1;

			// Set Y of focus to right above the ground at the focus point
			mFocus.y = GetTerrainHeightHighestLevel((mFocus.x + terrainOffsetX) * 8, (mFocus.z + terrainOffsetY) * 8) + 0.5;

			// begin by setting mView to point right into -z
			mView.set(0.0f, 0.0f, -1.0f);

			// rotate around X upwards by mZoom, so the more you zoom out, the more you see stuff from 'upwards'
			mView.rotateX(-mZoom);

			// rotate around Y by mRotation
			mView.rotateY(mRotation);
			
			// Set mPosition to be a position in the absolute direction of mView, where the distance from mFocus is (mZoom * 2) * 0.2
			mPosition = mFocus - mView * (mZoom + 2) * 0.2;

			// Set up vector to point right into +y
			mUp.set(0.0f, 1.0f, 0.0f);

			// Check so that mPosition is within the map
			if (mPosition.x <= -terrainOffsetX)
				mPosition.x = -terrainOffsetX;
			else if (mPosition.x >= terrainOffsetX)
				mPosition.x = terrainOffsetX - 0.0001;
			
			if (mPosition.z <= -terrainOffsetY)
				mPosition.z = -terrainOffsetY;
			else if (mPosition.z >= terrainOffsetY)
				mPosition.z = terrainOffsetY - 0.0001;

			// Check that mPosition.y is above mYMin units above ground level, and that it's below mYMax
			if (mPosition.y - mYMin < GetTerrainHeightHighestLevel((mPosition.x + terrainOffsetX) * 8, (mPosition.z + terrainOffsetY) * 8))
				mPosition.y = GetTerrainHeightHighestLevel((mPosition.x + terrainOffsetX) * 8, (mPosition.z + terrainOffsetY) * 8) + mYMin;
			else if (mPosition.y > mYMax)
				mPosition.y = mYMax;

			// In case mPosition has changed by any of the statements above, mView won't point from mPosition to mFocus anymore.
			// Make sure you still look directly from mPosition to mFocus.
			mView = mFocus - mPosition;

			mView.normalize();
			
		}

		// Fly in the 'forwards' direction
		void Camera::Fly(GLfloat speed)
		{
			Utilities::Vector3D forward;

			// Calculate what is the forward direction by taking a vector directly into -z and rotating it by mRotation degrees
			forward.set(0.0f, 0.0f, -1.0f);
			
			forward.rotateY(mRotation);

			// Increase speed when stuff is more zoomed out
			speed *= 1.0f + mZoom / 33.75;

			// Do the actual moving of mFocus
			mFocus.z -= speed * forward.z;
			mFocus.x -= speed * forward.x;

			// Update mPosition and mView and make sure new values are within limits
			CheckPosition();
		}

		// like Camera::Fly, but fly to the right and left in the current view
		void Camera::FlyHorizontally(GLfloat speed)
		{
			Utilities::Vector3D forward;
			
			forward.set(0.0f, 0.0f, -1.0f);
			
			forward.rotateY(mRotation);

			speed *= 1.0f + mZoom / 33.75;
			
			mFocus.z += speed * forward.x;
			mFocus.x -= speed * forward.z;

			CheckPosition();
		}

		// Change how zoomed in the view is
		void Camera::Zoom(GLfloat speed)
		{
			mZoom += speed;
			CheckPosition();
		}
	
		// Changed rotation
		void Camera::Rotate(GLfloat speed)
		{
			mRotation += speed;
			CheckPosition();
		}
		
		Camera Camera::instance;
	}
}
	
