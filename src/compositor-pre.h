
#ifndef __COMPOSITOR_H_PRE__
#define __COMPOSITOR_H_PRE__ 

#ifdef DEBUG_DEP
	#warning "compositor.h-pre"
#endif

namespace GUI
{
	namespace Core
	{
		struct MouseEvent;
		struct KeyboardEvent;
		struct WindowEvent;
		
		struct Listener;
		
		typedef void(*mouse)(MouseEvent, void*);
		typedef void(*keyboard)(KeyboardEvent, void*);
		typedef void(*window)(WindowEvent, void*);
		typedef void(*render)(float diff, void*);
	}
	
	typedef void(*universalCallback)(void*);
	
	class Event;
	class Window;
	class Compositor;
	class Metrics;
	
}

#endif

