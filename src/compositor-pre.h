
#ifndef __COMPOSITOR_H_PRE__
#define __COMPOSITOR_H_PRE__ 

#ifdef DEBUG_DEP
	#warning "compositor.h-pre"
#endif

namespace GUI
{	
	typedef void(*universalCallback)(void*);
	
	class Event;
	class Window;
	class Compositor;
	class Metrics;
	
}

#endif

