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
#ifndef SCENEGRAPH_H_PRE
#define SCENEGRAPH_H_PRE

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
	}
}

#endif
