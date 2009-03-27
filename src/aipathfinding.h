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
#ifndef AIPATHFINDING_H
#define AIPATHFINDING_H

#ifdef DEBUG_DEP
#warning "aipathfinding.h"
#endif

#include "aipathfinding-pre.h"

#define USE_MULTIFRAMED_CALCULATIONS
//#define DEBUG_AI_PATHFINDING
#define USE_MULTITHREADED_CALCULATIONS

#ifdef USE_MULTIFRAMED_CALCULATIONS
	#define MAXIMUM_CALCULATIONS_PER_FRAME 1000
#endif

#include "sdlheader.h"
#include "ainode.h"

#include "dimension.h"
#include "unit.h"
#include "utilities.h"
#include "action.h"

#include <vector>

namespace Game
{
	namespace AI
	{	
		enum CalcStates
		{
			CALCSTATE_AWAITING_NEXT_FRAME = -1,
			CALCSTATE_WORKING             = 0,
			CALCSTATE_REACHED_GOAL        = 1,
			CALCSTATE_FAILURE             = 2
		};
		
		enum DPNArg
		{
			DPN_BACK = 0,
			DPN_FRONT
		};
		
		enum IPResult
		{
			IPR_SUCCESS,
			IPR_SUCCESS_POPCMD_ISSUED,
			IPR_IS_IMMOBILE,
			IPR_GOAL_IS_START
		};
		
		enum PathState
		{
			PATHSTATE_OK = 0,
			PATHSTATE_ERROR,
			PATHSTATE_GOAL,
			PATHSTATE_EMPTY_QUEUE,
			PATHSTATE_IMPOSSIBLE,
			PATHSTATE_DOES_NOT_EXIST
		};

		enum PopReason 
		{
			POP_DELETED = 0,
			POP_CANCELLED,
			POP_NEW_GOAL
		};

		enum IntThrState
		{
			INTTHRSTATE_NONE = 1,
			INTTHRSTATE_WAITING,
			INTTHRSTATE_PROCESSING,
			INTTHRSTATE_UNAPPLIED
		};

		enum PreprocessState
		{
			PREPROCESSSTATE_NONE = 0,
			PREPROCESSSTATE_PROCESSING_TRACE,
			PREPROCESSSTATE_PROCESSING_FLOOD,
			PREPROCESSSTATE_SKIPPED_TRACE,
			PREPROCESSSTATE_SKIPPED_FLOOD,
			PREPROCESSSTATE_DONE
		};

		typedef std::vector< Node* > AINodeList;
		typedef std::vector< Node* >::iterator AINodeListIter;
		
		struct ActionData : public Dimension::BaseActionData
		{
			Dimension::IntPosition startPos;
			Dimension::IntPosition changedGoalPos;

			ActionData()
			{
				Reset();
			}

			void Reset();
			void Set(int start_x, int start_y, int end_x, int end_y, const gc_ptr<Dimension::Unit>& goal, AI::UnitAction action, const Dimension::ActionArguments& args, float rotation);
		};

		struct MovementData
		{
			
			Utilities::Vector3D movementVector;
			float distanceLeft;
			float distancePerFrame;

			Node*        pStart;
			Node*        pGoal;
			
			ActionData   action;
			ActionData   secondaryAction;

			bool         changedGoal;
			int          calcState;

			Node*        pCurGoalNode;
			bool         switchedSquare;

			// Internal variables
			IntThrState  _currentState;
			bool         _popFromQueue;
			PopReason    _reason;
			bool         _newCommandWhileUnApplied;
			
			Node*        _start;
			Node*        _goal;

			ActionData   _action;
			
			ActionData   _newAction;

			int          _associatedThread;
			
			Uint32       _pathfindingStartingFrame;
			
#ifdef DEBUG_AI_PATHFINDING
			unsigned int _cycles;
#endif
			void shade()
			{
				action.shade();
				secondaryAction.shade();
				_action.shade();
				_newAction.shade();
			}

			~MovementData();
		};

		struct ThreadData;
		
		//
		// Prepare unit for pathfinding
		//
		void InitMovementData(const gc_ptr<Dimension::Unit>& unit);

		//
		// Push the given unit into the pathfinding waiting queue.
		// The pathfinding results are placed in an internal node tree,
		// namely _start -> _goal. DO NOT ATTEMPT TO ADDRESS THESE NODES
		// MANUALLY! Once the calcState has reached CALCSTATE_REACHED_GOAL, call
		// ApplyNewPath.
		//
		// Returns IPResult, IPR_*
		//
		// Note: IPR_SUCCESS and IPR_SUCCESS_POPCMD_ISSUED does both specify success.
		//       If IPR_SUCCESS_POPCMD_ISSUED is given, _goal/_start haven't been
		//       modified yet, because the unit is undergoing calculation. The changes
		//       will be applied during next calculation step.
		//
		// Note: you pass float values, but the moment they get "nodified",
		//       they will be casted to signed int:s.
		//
		// ADDITIONS: action - assigned unit action, ACTION_*. default ACTION_GOTO
		//            target - target unit. default NULL
		//            args   - unit action arguments. default NULL
		//
		IPResult CommandPathfinding(const gc_ptr<Dimension::Unit>& pUnit, int start_x, int start_y, int goal_x, int goal_y, AI::UnitAction action = AI::ACTION_GOTO, const gc_ptr<Dimension::Unit>& target = NULL, const Dimension::ActionArguments& args = Dimension::ActionArguments(), float rotation = 0.0f);
		
		//
		// Get the internal path state, PATHSTATE_*
		// *) PATHSTATE_DOES_NOT_EXIST - there is no internal path
		// *) PATHSTATE_GOAL           - the internal path is complete
		// *) PATHSTATE_ERROR          - the calculation failed
		// *) PATHSTATE_OK             - the path is being calculated
		//
		PathState GetInternalPathState(const gc_ptr<Dimension::Unit>&);
		
		//
		// Applies the internal node structure as the current path.
		// Returns true if success, false on failure.
		//
		bool ApplyNewPath(const gc_ptr<Dimension::Unit>&);
		
		//
		// Applies the internal node structure as the current path
		// for all units.
		//
		void ApplyAllNewPaths();

		//
		// Quit current path and deallocaet it
		// Returns true if success, false on failure
		//
		bool QuitCurrentPath(const gc_ptr<Dimension::Unit>&);
		
		void CancelUndergoingProc(const gc_ptr<Dimension::Unit>& unit);

		//
		// Internal function: do pathfinding work.
		//
		int PerformPathfinding(ThreadData*);

		//
		// Public pathfinding deallocation function.
		// May be provided with DPNArg: DPN_FRONT or DPN_BACK where
		// DPN_FRONT represents the pStart -> pGoal node tree
		// DPN_BACK represents the internal _start -> _goal node tree
		//
		void DeallocPathfindingNodes(const gc_ptr<Dimension::Unit>&, DPNArg = DPN_FRONT);

		//
		// Returns wheather the unit is busy (its path is being calculated).
		// Note: a unit in waiting queue is considered busy.
		//
		bool IsUndergoingPathCalc(const gc_ptr<Dimension::Unit>&);
		
		//
		// Sets _reason and _popFromQueue flags in order to ensure
		// immediate deletion of given unit.
		//
		void QuitUndergoingProc(const gc_ptr<Dimension::Unit>&);

		//
		//
		//
		//
		void DequeueNewPath(const gc_ptr<Dimension::Unit>& unit);

		//
		// Prepare pathfinding thread
		//
		void InitPathfindingThreading(void);
		
		//
		// Quit pathfinding thread
		//
		void QuitPathfindingThreading(void);
		
		void PausePathfinding();
		int PausePathfinding(const gc_ptr<Dimension::Unit>& unit);
		
		void ResumePathfinding();
		void ResumePathfinding(int thread);

		void DeleteUnitFromAreaMap(const gc_ptr<Dimension::Unit>& unit);
		void AddUnitToAreaMap(const gc_ptr<Dimension::Unit>& unit);

		const int STACK_ELEMENTS = 2048;
		
		extern volatile int cCount, fCount, tCount, pCount, numPaths, numFailed, notReachedPath, notReachedFlood, numGreatSuccess1, numGreatSuccess2, numTotalFrames;
		
		int GetQueueSize();
	}
}

#ifdef DEBUG_DEP
#warning "aipathfinding.h-end"
#endif

#define AIPATHFINDING_H_END

#endif
