#ifndef __OGREXMLMODEL_H__
#define __OGREXMLMODEL_H__

#ifdef DEBUG_DEP
#warning "ogrexmlmodel.h"
#endif

#include "unit-pre.h"
#include "ogrexmlmodel-pre.h"

namespace Utilities
{
	OgreMesh* LoadOgreXMLModel(std::string filename);
}

#ifdef DEBUG_DEP
#warning "ogrexmlmodel.h-end"
#endif

#endif
