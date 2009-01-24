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

#include "containers.h"

namespace GUI
{
	namespace Containers
	{
		////////////////////////////////////////////
		// FlowPanel

		FlowPanel::FlowPanel(Direction primaryDirection,
		                     Direction secondaryDirection,
		                     VerticalAdjustment individualVAdjustment,
		                     HorizontalAdjustment individualHAdjustment) :
		                     primaryDirection(primaryDirection),
		                     secondaryDirection(secondaryDirection),
		                     individualVAdjustment(individualVAdjustment),
		                     individualHAdjustment(individualHAdjustment)
		{
			
		}

		componentHandle FlowPanel::insert(Component* comp, int position)
		{
			return Container::insert(comp, position);
		}

		void FlowPanel::layout()
		{
			Container::layoutAll();
			
			{
				float posX = 0.0f;
				float posY = 0.0f;
				switch (primaryDirection)
				{
					case DIRECTION_VERTICAL_REVERSED:
						posY = dimensions.h;
					case DIRECTION_VERTICAL:
						if (secondaryDirection == DIRECTION_HORIZONTAL_REVERSED)
						{
							posX = dimensions.w;
						}
						break;
					
					case DIRECTION_HORIZONTAL_REVERSED:
						posX = dimensions.w;
					case DIRECTION_HORIZONTAL:
						if (secondaryDirection == DIRECTION_VERTICAL_REVERSED)
						{
							posY = dimensions.h;
						}
						break;
					default:
						break;
				}
			}

			Container::postLayout();
		}
	}
}
