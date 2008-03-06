#include "unit-pre.h"

#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifdef DEBUG_DEP
#warning "camera.h"
#endif

#include "vector3d.h"
#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		class Camera
		{
			private:
				Utilities::Vector3D mPosition;
				Utilities::Vector3D mView;
				Utilities::Vector3D mUp;
				Utilities::Vector3D mFocus;
				GLfloat mYMax, mYMin, mZoom, mRotation;
				
			public:
				Camera(void)     : mPosition(Utilities::Vector3D(0.0f, 0.0f, 0.0f)),
				                   mView(Utilities::Vector3D(0.0f, 0.0f, 0.0f)),
				                   mUp(Utilities::Vector3D(0.0f, 1.0f, 0.0f)),
						   mFocus(Utilities::Vector3D(0.0, 0.0, 0.0)),
								   mYMax(10.0f),
								   mYMin(0),
								   mZoom(2.0f),
								   mRotation(0.0f)
				{}
				Camera(Utilities::Vector3D position,
				       Utilities::Vector3D view) : mPosition(position),
												   mView(view),
												   mYMax(10.0f),
												   mYMin(0),
												   mZoom(2.0f),
												   mRotation(0.0f)
				{}
								
				void PointCamera(void);
				void SetCamera(Utilities::Vector3D, GLfloat, GLfloat);
				void SetCamera(Unit* unit, GLfloat zoom, GLfloat rotation);
				void SetFocus(float terrain_x, float terrain_y);
				void CheckPosition();
				void Fly(GLfloat);
				void FlyHorizontally(GLfloat);
				void Zoom(GLfloat);
				void Rotate(GLfloat);
		
				Utilities::Vector3D GetFocus();
				GLfloat GetZoom();
				GLfloat GetRotation();
				
				void SetYMaximum(GLfloat value) { mYMax = value; }
				void SetYMinimum(GLfloat value) { mYMin = value; }
				
				GLfloat GetYMaximum(void) const { return mYMax; }
				GLfloat GetYMinimum(void) const { return mYMin; }

				GLfloat GetZoom(void) const { return mZoom; }
				GLfloat GetRotation(void) const { return mRotation; }

				const Utilities::Vector3D* GetPosVector(void) { return &mPosition; };
		};
		
		extern float cameraRotationSpeed;
		extern float cameraFlySpeed;
		extern float cameraZoomSpeed;
	}
}

#ifdef DEBUG_DEP
#warning "camera.h-end"
#endif

#endif
