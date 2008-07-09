#ifndef __MODELSELECTORXML_H__
#define __MODELSELECTORXML_H__

#ifdef DEBUG_DEP
#warning "modelselectorxml.h"
#endif

#include "modelselectorxml-pre.h"

#include "ogrexmlmodel-pre.h"
#include <string>

namespace Utilities
{
	gc_ptr<OgreMesh> LoadOgreXMLModel(std::string name);
}

#ifdef DEBUG_DEP
#warning "modelselectorxml.h-end"
#endif

#endif
