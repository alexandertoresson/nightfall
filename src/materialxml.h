
#ifndef __MATERIALXML_H__
#define __MATERIALXML_H__

#ifdef DEBUG_DEP
#warning "materialxml.h"
#endif

#include "materialxml-pre.h"

namespace Utilities
{
	ref_ptr<Material> LoadMaterialXML(std::string name);
}

#ifdef DEBUG_DEP
#warning "materialxml.h-end"
#endif

#endif
