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
#ifndef __CONTAINERS_H__
#define __CONTAINERS_H__ 

#ifdef DEBUG_DEP
	#warning "containers.h"
#endif

#include "containers-pre.h"

#include "compositor.h"

namespace GUI
{
	namespace Containers
	{
		class TablePanel : public Component
		{
			TablePanel(int cols, int rows);

			void add(Component* comp, int col, int row);
		};

		class DockPanel : public Component
		{
			enum DockPosition
			{
				DOCK_MIDDLE,
				DOCK_ABOVE,
				DOCK_BELOW,
				DOCK_LEFT,
				DOCK_RIGHT
			};

			DockPanel();

			void add(Component* comp, DockPosition pos);
		};

		class FlowPanel : public Component
		{
			void add(Component* comp, int position=-1);
			void add(Component* comp, std::string key);
		};
	}
}

#ifdef DEBUG_DEP
	#warning "containers.h-end"
#endif

#endif

