#ifndef __AIPATHFINDING_H__
#define __AIPATHFINDING_H__

#ifdef DEBUG_DEP
#warning "aipathfinding.h"
#endif

#define USE_MULTIFRAMED_CALCULATIONS
//#define DEBUG_AI_PATHFINDING
#define USE_MULTITHREADED_CALCULATIONS

#ifdef USE_MULTIFRAMED_CALCULATIONS
	#define MAXIMUM_CALCULATIONS_PER_FRAME 1000
#endif

#include "sdlheader.h"
#include "ainode.h"

#include "dimension.h"
#include "unit-pre.h"
#include "utilities.h"

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
		
		struct UnitGoal
		{
			Dimension::Unit* unit;
			Dimension::IntPosition pos;
			Uint16 goal_id;
		};
		
		struct ActionData
		{
			Dimension::IntPosition startPos;
			UnitGoal goal;
			Dimension::IntPosition changedGoalPos;
			void*    arg;
			UnitAction action;
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
		};

		struct ThreadData;
		
		//
		// Prepare unit for pathfinding
		//
		void InitMovementData(Dimension::Unit* unit);

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
		IPResult CommandPathfinding(Dimension::Unit* pUnit, int start_x, int start_y, int goal_x, int goal_y, AI::UnitAction action = AI::ACTION_GOTO, Dimension::Unit* target = NULL, void* args = NULL);
		
		//
		// Get the internal path state, PATHSTATE_*
		// *) PATHSTATE_DOES_NOT_EXIST - there is no internal path
		// *) PATHSTATE_GOAL           - the internal path is complete
		// *) PATHSTATE_ERROR          - the calculation failed
		// *) PATHSTATE_OK             - the path is being calculated
		//
		PathState GetInternalPathState(Dimension::Unit*);
		
		//
		// Applies the internal node structure as the current path.
		// Returns true if success, false on failure.
		//
		bool ApplyNewPath(Dimension::Unit*);
		
		//
		// Applies the internal node structure as the current path
		// for all units.
		//
		void ApplyAllNewPaths();

		//
		// Quit current path and deallocaet it
		// Returns true if success, false on failure
		//
		bool QuitCurrentPath(Dimension::Unit*);
		
		void CancelUndergoingProc(Dimension::Unit* unit);

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
		void DeallocPathfindingNodes(Dimension::Unit*&, DPNArg = DPN_FRONT);

		//
		// Returns wheather the unit is busy (its path is being calculated).
		// Note: a unit in waiting queue is considered busy.
		//
		bool IsUndergoingPathCalc(Dimension::Unit*);
		
		//
		// Sets _reason and _popFromQueue flags in order to ensure
		// immediate deletion of given unit.
		//
		void QuitUndergoingProc(Dimension::Unit*);

		//
		//
		//
		//
		void DequeueNewPath(Dimension::Unit* unit);

		//
		// Prepare pathfinding thread
		//
		void InitPathfindingThreading(void);
		
		//
		// Quit pathfinding thread
		//
		void QuitPathfindingThreading(void);
		
		void PausePathfinding();
		int PausePathfinding(Dimension::Unit* unit);
		
		void ResumePathfinding();
		void ResumePathfinding(int thread);

		void DeleteUnitFromAreaMap(Dimension::Unit* unit);
		void AddUnitToAreaMap(Dimension::Unit* unit);

		const int STACK_ELEMENTS = 2048;
		
		extern volatile int cCount, fCount, tCount, pCount, numPaths, numFailed, notReachedPath, notReachedFlood, numGreatSuccess1, numGreatSuccess2, numTotalFrames;
		
		int GetQueueSize();
	}
}

#ifdef DEBUG_DEP
#warning "aipathfinding.h-end"
#endif

#define __AIPATHFINDING_H_END__

#endif
