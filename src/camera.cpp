#include "camera.h"

namespace Game
{
	namespace Dimension
	{
		float cameraRotationSpeed = 20.0f;
		float cameraFlySpeed      = 1.5f;
		float cameraZoomSpeed     = 40.0f;

		// Apply the camera settings to the opengl camera
		void Camera::PointCamera(void)
		{
			CheckPosition();
			gluLookAt(mPosition.x, mPosition.y, mPosition.z, 
			          mPosition.x + mView.x, mPosition.y + mView.y, mPosition.z + mView.z,
			          mUp.x, mUp.y, mUp.z);
		}
		
		// Set camera settings manually to a specific setting
		void Camera::SetCamera(Utilities::Vector3D focus, GLfloat zoom, GLfloat rotation)
		{
			mFocus = focus;
			mZoom = zoom;
			mRotation = rotation;
			CheckPosition();
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
			if (mFocus.x <= -terrainOffsetX)
				mFocus.x = -terrainOffsetX;
			else if (mFocus.x >= terrainOffsetX)
				mFocus.x = terrainOffsetX - 0.0001;
			
			if (mFocus.z <= -terrainOffsetY)
				mFocus.z = -terrainOffsetY;
			else if (mFocus.z >= terrainOffsetY)
				mFocus.z = terrainOffsetY - 0.0001;

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
	}
}
	
