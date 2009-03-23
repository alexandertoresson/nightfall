/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "selectorxml.h"

#include "ogrexmlmodel.h"
#include "materialxml.h"
#include "configuration.h"
#include "minixml.h"
#include "vfs.h"
#include "game.h"
#include <cassert>

namespace Utilities
{
	static gc_ptr<OgreMesh> mesh = NULL;
	static gc_ptr<Material> material = NULL;
	static unsigned submesh_index = 0;
	static enum
	{
		FILETYPE_MESH,
		FILETYPE_MATERIAL
	} filetype;

	static void ParseSubMesh(Utilities::XMLElement *elem)
	{
		if (submesh_index < mesh->submeshes.size())
		{
			if (mesh)
			{
				gc_ptr<OgreSubMesh> submesh = mesh->submeshes[submesh_index];
				submesh->material = LoadMaterialXML(elem->GetAttribute("material"));
			}
			else
			{
				std::cout << "Warning: no mesh loaded, material " << elem->GetAttribute("material") << " can not be loaded" << std::endl;
			}
			submesh_index++;
		}
	}

	static void ParseMesh(Utilities::XMLElement *elem)
	{
		if (elem->HasAttribute("mesh"))
		{
			if (!mesh)
			{
				mesh = LoadSpecificOgreXMLModel(elem->GetAttribute("mesh"));
			}
			else
			{
				std::cout << "Warning: Mesh already loaded; can't load mesh " << elem->GetAttribute("mesh") << "!" << std::endl;
			}
		}
		if (elem->HasAttribute("material"))
		{
			if (mesh)
			{
				for (unsigned i = 0; i < mesh->submeshes.size(); i++)
					mesh->submeshes[i]->material = LoadMaterialXML(elem->GetAttribute("material"));
			}
			else
			{
				std::cout << "Warning: no mesh loaded, material " << elem->GetAttribute("material") << " can not be loaded" << std::endl;
			}
		}

		elem->Iterate("submesh", ParseSubMesh);
	}
	
	static void ParseMaterial(Utilities::XMLElement *elem)
	{
		material = LoadSpecificMaterialXML(elem->GetAttribute("name"));
	}
	
	static std::stack<bool> case_found;
	static std::stack<std::string> case_value;
	static std::stack<bool> did_fallthrough;

	void ParseSelector(Utilities::XMLElement *elem);

	void ParseCase(Utilities::XMLElement *elem)
	{
		if ((!case_found.top() && elem->GetAttribute("value") == case_value.top()) || did_fallthrough.top())
		{
			bool fallthrough = elem->GetAttribute("fallthrough") == "true";

			case_found.pop();
			case_found.push(true);

			ParseSelector(elem);

			did_fallthrough.pop();
			did_fallthrough.push(fallthrough);
		}
	}

	void ParseDefault(Utilities::XMLElement *elem)
	{
		if (!case_found.top() || did_fallthrough.top())
		{
			bool fallthrough = elem->GetAttribute("fallthrough") == "true";

			case_found.pop();
			case_found.push(true);

			ParseSelector(elem);

			did_fallthrough.pop();
			did_fallthrough.push(fallthrough);
		}
	}

	void ParseSwitch(Utilities::XMLElement *elem)
	{
		case_found.push(false);
		case_value.push(Utilities::mainConfig.GetValue(elem->GetAttribute("option")));
		did_fallthrough.push(false);

		TagFuncMap tfmap;
		tfmap["case"] = ParseCase;
		tfmap["default"] = ParseDefault;
		elem->Iterate(tfmap, NULL);
		
		case_found.pop();
		case_value.pop();
		did_fallthrough.pop();
	}
	
	void LoadModelSelector(std::string name);
	void LoadMaterialSelector(std::string name);

	void ParseInclude(Utilities::XMLElement *elem)
	{
		std::string name = elem->GetAttribute("name");
		if (filetype == FILETYPE_MESH)
		{
			LoadModelSelector(name);
		}
		else
		{
			LoadMaterialSelector(name);
		}
	}

	void ParseSelector(Utilities::XMLElement *elem)
	{
		TagFuncMap tfmap;
		if (filetype == FILETYPE_MESH)
		{
			tfmap["mesh"] = ParseMesh;
		}
		else
		{
			tfmap["material"] = ParseMaterial;
		}
		tfmap["switch"] = ParseSwitch;
		tfmap["include"] = ParseInclude;
		elem->Iterate(tfmap, NULL);
	}

	void LoadModelSelector(std::string name)
	{
		Utilities::XMLReader xmlReader;
		
		std::string filename = VFS::ResolveReadable("/data/models/" + name + ".sel.xml");

		if (filename.length() && xmlReader.Read(filename))
		{
			filetype = FILETYPE_MESH;
			xmlReader.root->Iterate("modelselector", ParseSelector);
		}
		else
		{
			mesh = LoadSpecificOgreXMLModel(name);
		}
		
		xmlReader.Deallocate();

	}

	static std::map<std::string, gc_ptr<OgreMesh> > filenameToMesh;

	gc_ptr<OgreMesh> LoadOgreXMLModel(std::string name)
	{
		mesh = filenameToMesh[name];

		if (!mesh)
		{
			LoadModelSelector(name);
		}

		if (!Game::Rules::noGraphics)
			for (unsigned i = 0; i < mesh->submeshes.size(); i++)
				assert(mesh->submeshes[i]->material);

		filenameToMesh[name] = mesh;

		return mesh;

	}

	void LoadMaterialSelector(std::string name)
	{
		Utilities::XMLReader xmlReader;
		
		std::string filename = VFS::ResolveReadable("/data/materials/" + name + ".sel.xml");

		if (filename.length() && xmlReader.Read(filename))
		{
			filetype = FILETYPE_MATERIAL;
			xmlReader.root->Iterate("materialselector", ParseSelector);
		}
		else
		{
			material = LoadSpecificMaterialXML(name);
		}
		
		xmlReader.Deallocate();

	}

	static std::map<std::string, gc_ptr<Material> > filenameToMaterial;

	gc_ptr<Material> LoadMaterialXML(std::string name)
	{
		if (Game::Rules::noGraphics)
			return NULL;

		material = filenameToMaterial[name];

		if (!material)
		{
			LoadMaterialSelector(name);
		}

		filenameToMaterial[name] = material;

		return material;

	}

}
