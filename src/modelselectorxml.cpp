#include "modelselectorxml.h"

#include "ogrexmlmodel.h"
#include "materialxml.h"
#include "configuration.h"
#include "minixml.h"
#include "paths.h"
#include <cassert>

namespace Utilities
{
	static OgreMesh* mesh = NULL;
	static unsigned submesh_index = 0;

	static void ParseSubMesh(Utilities::XMLElement *elem)
	{
		if (submesh_index < mesh->submeshes.size())
		{
			if (mesh)
			{
				OgreSubMesh* submesh = mesh->submeshes[submesh_index];
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

	void ParseInclude(Utilities::XMLElement *elem)
	{
		std::string name = elem->GetAttribute("name");
		LoadModelSelector(name);
	}

	void ParseSelector(Utilities::XMLElement *elem)
	{
		TagFuncMap tfmap;
		tfmap["switch"] = ParseSwitch;
		tfmap["mesh"] = ParseMesh;
		tfmap["include"] = ParseInclude;
		elem->Iterate(tfmap, NULL);
	}

	void LoadModelSelector(std::string name)
	{
		Utilities::XMLReader xmlReader;
		
		std::string filename = Utilities::GetDataFile(name + ".sel.xml");

		if (filename.length() && xmlReader.Read(filename))
		{
			xmlReader.root->Iterate("selector", ParseSelector);
		}
		else
		{
			mesh = LoadSpecificOgreXMLModel(name);
		}
		
		xmlReader.Deallocate();

	}

	static std::map<std::string, OgreMesh*> filenameToMesh;

	OgreMesh* LoadOgreXMLModel(std::string name)
	{
		mesh = filenameToMesh[name];

		if (!mesh)
		{
			LoadModelSelector(name);
		}

		for (unsigned i = 0; i < mesh->submeshes.size(); i++)
			assert(mesh->submeshes[i]->material);

		filenameToMesh[name] = mesh;

		return mesh;

	}

}
