#ifndef __SCENEGRAPH_H_PRE__
#define __SCENEGRAPH_H_PRE__

#ifdef DEBUG_DEP
#warning "scenegraph.h-pre"
#endif

namespace Scene
{
	namespace Graph
	{
		class Node;

		class MatrixNode;

		class TranslationNode;
		class RotationNode;
		class ScaleNode;
		class MatrixAnimNode;

		class SwitchNode;
		class IndependentSwitchNode;

/*		class GUINode;*/
	}
}

#endif
