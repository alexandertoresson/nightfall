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
#ifndef CONTAINERS_H
#define CONTAINERS_H 

#ifdef DEBUG_DEP
	#warning "containers.h"
#endif

#include "containers-pre.h"

#include "compositor.h"
#include "themeengine.h"

namespace GUI
{
	namespace Containers
	{
		class TablePanel : public Container
		{
			public:
				class RowColStyle
				{
					public:
						enum Style
						{
							PERCENT,
							FIXED,
							FILL,
							CONTENTS
						};

					private:
						Style style;
						float size;
					public:
						RowColStyle(Style style, float size = 0.0f) : style(style), size(size) {}
				};

			private:
				gc_array<Component, 2> subComponents;
				std::vector<RowColStyle> rowStyles;
				std::vector<RowColStyle> colStyles;

				struct Position
				{
					int x, y;
				};
				std::map<componentHandle, Position> positions;
			public:
				TablePanel(int cols, int rows);

				componentHandle add(Component* comp, int col, int row);

				void setRowStyle(int row, RowColStyle rowStyle);
				void setColStyle(int row, RowColStyle colStyle);
			
				virtual void remove(componentHandle handle);
				virtual void clear();

				virtual void layout();
			
				virtual void paintComponent();
		
				// No need for shade() as Container will shade the Components that were added
		};

		class DockPanel : public Container
		{
			public:
				enum DockPosition
				{
					DOCK_MIDDLE = 0,
					DOCK_ABOVE,
					DOCK_BELOW,
					DOCK_LEFT,
					DOCK_RIGHT,
					DOCK_NUM
				};

				DockPanel();

				componentHandle add(gc_ptr<Component> comp, DockPosition pos);

				virtual void remove(componentHandle handle);
				virtual void clear();
				virtual void layout();
				virtual void paintComponent();
			private:
				gc_ptr<Component> subComponents[DOCK_NUM];
				std::map<componentHandle, DockPosition> positions;
				
		};

		class FlowPanel : public Container
		{
			public:
				enum Direction
				{
					DIRECTION_VERTICAL,
					DIRECTION_HORIZONTAL,
					DIRECTION_VERTICAL_REVERSED,
					DIRECTION_HORIZONTAL_REVERSED,
					DIRECTION_NONE
				};
			private:
				Direction primaryDirection, secondaryDirection;
				VerticalAdjustment individualVAdjustment;
				HorizontalAdjustment individualHAdjustment;
			public:
				FlowPanel(Direction primaryDirection = DIRECTION_VERTICAL,
				          Direction secondaryDirection = DIRECTION_HORIZONTAL,
				          VerticalAdjustment individualVAdjustment = V_ADJUSTMENT_LEFT,
				          HorizontalAdjustment individualHAdjustment = H_ADJUSTMENT_TOP);

				componentHandle insert(gc_ptr<Component> comp, int position);

				virtual void layout();
		};
	}
}

#ifdef DEBUG_DEP
	#warning "containers.h-end"
#endif

#endif

