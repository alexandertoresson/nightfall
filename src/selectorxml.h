#ifndef __SELECTORXML_H__
#define __SELECTORXML_H__

#ifdef DEBUG_DEP
#warning "selectorxml.h"
#endif

#include "selectorxml-pre.h"

#include "ogrexmlmodel-pre.h"
#include "materialxml-pre.h"
#include <string>

namespace Utilities
{
	gc_ptr<OgreMesh> LoadOgreXMLModel(std::string name);
	gc_ptr<Material> LoadMaterialXML(std::string name);
}

#ifdef DEBUG_DEP
#warning "selectorxml.h-end"
#endif

#endif
