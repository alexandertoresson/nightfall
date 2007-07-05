/*
 *
 * This implementation is regarded as crude.
 *
 * We simply didn't have enough time to implement an
 * algorithm that divide the map in to separate walkable 
 * sections, which is a necessary complement to a working
 * implementation of the A-star algorithm.
 *
 * The following algorithm is derived from the original
 * A-star implementation, but does not quite work the same.
 *
 * All adjacent nodes are pushed into a heap-sorted open list
 * if walkable. Unlike the "original" A-algorithm previous instances
 * of the same nodes is persistent - they cannot be deleted. Instead,
 * the current node is deallocated. This makes the path generation some
 * what unstable, and does not, unlike the A-algorithm, ensure
 * success.
 *
 * The algorithm ends when goal is reached, when the unit is close
 * enough to the goal (to execute given orders) OR (important, important!)
 * when all adjacent nodes have been omitted!!! Meaning: when all adjacent
 * nodes are already present in the open and closed list.
 *
 */

#include "aipathfinding.h"

#include "game.h"
#include "dimension.h"
#include "ainode.h"
#include "unit.h"
#include "networking.h"

namespace Game
{
	namespace AI
	{
		static std::queue< Dimension::Unit* > gCalcQueue;
#ifdef DEBUG_AI_PATHFINDING
		static int ALLOCATIONS  = 0;
#endif

		static SDL_Thread* pPathfindingThread;
		static bool        threadRuntime;
		static SDL_mutex*  gpmxPathfinding;
		static Node*       gStack[STACK_ELEMENTS];
		static int         gStackSize = 0;
		
		void InitPathfindingThreading(void)
		{
			if (pPathfindingThread != NULL)
			{
				return;
			}
			
			threadRuntime = true;
			gpmxPathfinding = SDL_CreateMutex();

			ThreadData* tdata = new ThreadData;
			tdata->pMutex = gpmxPathfinding;
			tdata->pUnit = NULL;
			pPathfindingThread = SDL_CreateThread(Game::AI::_ThreadMethod, (void*)tdata);
		}
		
		void QuitPathfindingThreading(void)
		{
			if (pPathfindingThread == NULL)
			{
				return;
			}
		
			threadRuntime = false;
			
			SDL_WaitThread(pPathfindingThread, NULL);
			pPathfindingThread = NULL;
			
			if (gpmxPathfinding != NULL)
			{
				SDL_DestroyMutex(gpmxPathfinding);
				gpmxPathfinding = NULL;
			}
		}
		
		SDL_mutex* const GetMutex(void)
		{
			return gpmxPathfinding;
		}
		
		int _ThreadMethod(void* arg)
		{
			ThreadData* tdata = (ThreadData*)arg;
			while (threadRuntime == true)
			{
				if (PerformPathfinding(tdata) == PATHSTATE_EMPTY_QUEUE)
				{
					SDL_Delay(10);
					continue;
				}
				
				SDL_Delay(1);
			}
			delete tdata;
			tdata = NULL;

			return SUCCESS;
		}
		
		inline Node* AllocNode(int x, int y)
		{
#ifdef DEBUG_AI_PATHFINDING
			ALLOCATIONS++;
#endif
			Node* p = NULL;
			if (gStackSize > 0)
				p = gStack[--gStackSize];
			else
				p = new Node();

			p->pChild = NULL;
			p->pParent = NULL;
			
			p->g = 0;
			p->f = 0;
			p->h = 0;

			p->x = x;
			p->y = y;
						
			return p;
		}
		
		inline void DeallocNode(Node*& p)
		{
#ifdef DEBUG_AI_PATHFINDING
			ALLOCATIONS--;
#endif
			if (gStackSize == STACK_ELEMENTS)
			{
				delete p;
			}
			else
			{
				gStack[gStackSize++] = p;
			}
			p = NULL;
		}

		void InitMovementData(Dimension::Unit* unit)
		{
			assert (unit->pMovementData != NULL);

			MovementData* md = unit->pMovementData;

			md->pStart = NULL;
			md->pGoal = NULL;
			md->pCurGoalNode = NULL;
			md->calcState = CALCSTATE_REACHED_GOAL;
			
			md->action.goal.pos.x = 0;
			md->action.goal.pos.y = 0;
			md->action.goal.unit = NULL;
			md->action.arg = NULL;
			md->action.action = ACTION_GOTO;
			
			md->_currentState = INTTHRSTATE_NONE;
			md->_popFromQueue = false;
			md->_start = NULL;
			md->_goal = NULL;
			md->_action.goal.pos.x = 0;
			md->_action.goal.pos.y = 0;
			md->_action.goal.unit = NULL;
			md->_action.arg = NULL;
			md->_action.action = ACTION_GOTO;
			
#ifdef DEBUG_AI_PATHFINDING
			std::cout << "Movement data init: " << pUnit << std::endl;
			md->_cycles = 0;
#endif
		}
		
		IPResult InitPathfinding(Dimension::Unit* pUnit, float start_x, float start_y, float goal_x, float goal_y, AI::UnitAction action, Dimension::Unit* target, void* args)
		{
			assert(pUnit != NULL);

			if (!pUnit->type->isMobile)
				return IPR_IS_IMMOBILE;

			if ((int)start_x == (int)goal_x && (int)start_y == (int)goal_y)
				return IPR_GOAL_IS_START;
				
			if (!pUnit->pMovementData)
				InitMovementData(pUnit);

			SDL_LockMutex(gpmxPathfinding);

			IntThrState curState = pUnit->pMovementData->_currentState;

#ifdef DEBUG_AI_PATHFINDING
			std::cout << "State: " << curState << " | Length: " << gCalcQueue.size() << std::endl;
#endif

			IPResult res;
			MovementData* md = pUnit->pMovementData;
			
			md->_action.goal.pos.x = goal_x;
			md->_action.goal.pos.y = goal_y;
			md->_action.goal.unit = target;
			md->_action.arg = args;
			md->_action.action = action;
				
			if (curState == INTTHRSTATE_PROCESSING)
			{
				md->_popFromQueue = true;
				md->_reason = POP_NEW_GOAL;
				md->_newx = goal_x;
				md->_newy = goal_y;
				
				res = IPR_SUCCESS_POPCMD_ISSUED;
			}
			else
			{
				if (md->_start != NULL)
					DeallocPathfindingNodes(pUnit, DPN_BACK);
			       
				md->_start = AllocNode((int)start_x, (int)start_y);
				md->_goal  = AllocNode((int)goal_x, (int)goal_y);
				md->calcState = CALCSTATE_WORKING;
				md->changedGoal = false;
#ifdef DEBUG_AI_PATHFINDING
				md->_cycles = 0;
#endif

				if (curState != INTTHRSTATE_WAITING)
				{
					pUnit->pMovementData->_currentState = INTTHRSTATE_WAITING;
					gCalcQueue.push(pUnit);
				}
				
				res = IPR_SUCCESS;
			}

			SDL_UnlockMutex(gpmxPathfinding);

#ifdef DEBUG_AI_PATHFINDING
			std::cout << "[AI DEBUG] Unallocated AI memory: " << ALLOCATIONS << std::endl;
#endif

			return res;
		}
		
		PathState GetInternalPathState(Dimension::Unit* unit)
		{
			if (unit == NULL)
				return PATHSTATE_DOES_NOT_EXIST;
				
			if (unit->pMovementData == NULL)
				return PATHSTATE_DOES_NOT_EXIST;
				
			if (unit->pMovementData->_start == NULL)
				return PATHSTATE_DOES_NOT_EXIST;
				
			if (unit->pMovementData->calcState == CALCSTATE_REACHED_GOAL)
				return PATHSTATE_GOAL;
				
			if (unit->pMovementData->calcState == CALCSTATE_FAILURE)
				return PATHSTATE_ERROR;
				
			return PATHSTATE_OK;
		}
		
		bool ApplyNewPath(Dimension::Unit* unit)
		{
			if (unit == NULL)
				return false;
				
			if (unit->pMovementData == NULL)
				return false;
				
			if (GetInternalPathState(unit) != PATHSTATE_GOAL)
				return false;
				
			MovementData* md = unit->pMovementData;
			
			if (md->pStart != NULL)
				DeallocPathfindingNodes(unit, DPN_FRONT);
				
			md->pStart = md->_start;
			md->pGoal  = md->_goal;
			
			md->action.goal.pos.x = md->_action.goal.pos.x;
			md->action.goal.pos.y = md->_action.goal.pos.y;
			md->action.goal.unit = md->_action.goal.unit;
			md->action.arg = md->_action.arg;
			md->action.action = md->_action.action;
			unit->action = md->_action.action;
			
			md->_start = NULL;
			md->_goal = NULL;
			
			md->_action.goal.pos.x = 0;
			md->_action.goal.pos.y = 0;
			md->_action.goal.unit  = NULL;
			md->_action.arg        = NULL;
			
			return true;
		}
		
		bool QuitCurrentPath(Dimension::Unit* unit)
		{
			if (unit == NULL)
				return false;
			
			if (unit->pMovementData == NULL)
				return false;
				
			if (unit->pMovementData->pStart == NULL)
				return true;
				
			DeallocPathfindingNodes(unit, DPN_FRONT);
			
			return true;
		}

		bool IsUndergoingPathCalc(Dimension::Unit* unit)
		{
			if (unit == NULL)
				return false;

			if (unit->pMovementData == NULL)
				return false;

			if (unit->pMovementData->_currentState == INTTHRSTATE_NONE)
				return false;

			return true;
		}

		void QuitUndergoingProc(Dimension::Unit* unit)
		{
			if (unit == NULL)
				return;

			if (unit->pMovementData == NULL)
				return;

			SDL_LockMutex(gpmxPathfinding);

			unit->pMovementData->_popFromQueue = true;
			unit->pMovementData->_reason = POP_DELETED;

			SDL_UnlockMutex(gpmxPathfinding);
		}

		void DeallocPathfinding(ThreadData*& tdata)
		{
			assert(tdata->pUnit != NULL);
			assert(tdata->pUnit->pMovementData != NULL);
			
			MovementData* data = tdata->pUnit->pMovementData;
			AINodeListIter node;
			
			bool start_deallocated;
			if (data->_start->pChild != NULL)
			{
				node = tdata->openList.begin();
				while (node != tdata->openList.end())
				{
					if (*node == data->_start)
					{
						tdata->openList.erase(node);
						break;
					}
					node++;
				}

				make_heap(tdata->openList.begin(), tdata->openList.end(), PathfindingHeapCmp());

				node = tdata->closedList.begin();
				while (node != tdata->closedList.end())
				{
					if (*node == data->_start)
					{
						tdata->closedList.erase(node);
						break;
					}
					node++;
				}

				DeallocPathfindingNodes(tdata->pUnit, DPN_BACK);
				start_deallocated = true;
			}
			else
			{
				DeallocNode(data->_goal);

				start_deallocated = false;
			}

			node = tdata->openList.begin();
			while (node != tdata->openList.end())
			{
				if (!start_deallocated)
				{
					if (*node == data->_start)
						start_deallocated = true;
				}

				DeallocNode(*node);
				node++;
			}
			
			node = tdata->closedList.begin();
			while(node != tdata->closedList.end())
			{
				if (!start_deallocated)
				{
					if (*node == data->_start)
						start_deallocated = true;
				}

				DeallocNode(*node);
				node++;
			}
			
			tdata->openList.clear();
			tdata->closedList.clear();

			if (!start_deallocated)
				DeallocNode(data->_start);

			data->_start = NULL;
			data->_goal  = NULL;
			
			data->changedGoal = false;
		}

		void DeallocUnusedNodes(ThreadData*& tdata)
		{
			assert(tdata != NULL);

			AINodeListIter node;
			for (node = tdata->openList.begin(); node != tdata->openList.end(); node++)
			{
				if ((*node)->pChild == NULL)
					DeallocNode(*node);
			}

			for (node = tdata->closedList.begin(); node != tdata->closedList.end(); node++)
			{
				if ((*node)->pChild == NULL)
					DeallocNode(*node);
			}
			
			tdata->openList.clear();
			tdata->closedList.clear();
		}

		void DeallocPathfindingNodes(Dimension::Unit*& unit, DPNArg target)
		{
			assert (unit != NULL);
			assert (unit->pMovementData != NULL);

			MovementData* md = unit->pMovementData;
			
			Node** goal = NULL;
			Node** start = NULL;
			
			switch (target)
			{
				case DPN_FRONT:
					goal = &md->pGoal;
					start = &md->pStart;
					break;
				case DPN_BACK:
					goal = &md->_goal;
					start = &md->_start;
					break;
				default:
					assert(false && "Wrong DPN arguments!");
					return;
			}

			if (*goal)
			{
				if ((*goal)->pParent)
				{
					Node* pNode = *goal;
					Node* pRem  = NULL;

					do
					{
						pRem = pNode;
						pNode = pNode->pParent;

						DeallocNode(pRem);
						pRem = NULL;
						
					} while (pNode != *start);

					// Delete start
					DeallocNode(*start);
				}
				else
				{
					if (*start && *start != *goal)
						DeallocNode(*start); 

					if (*goal)
						DeallocNode(*goal);
				}
			}

			*start = NULL;
			*goal  = NULL;	
		}
		
		inline void CalculateCost(Node* p, Dimension::Unit* pUnit = NULL)
		{
			assert (p->pParent != NULL);
			
			p->g = Dimension::GetTraversalTime(pUnit, p->pParent->x, p->pParent->y, p->x - p->pParent->x, p->y - p->pParent->y);
		}

		inline void CalculateDistance(Node* p, Node* goal)
		{
			assert(goal != NULL);

			p->h = 10 * (abs(p->x - goal->x) + abs(p->y  - goal->y));
		}

		inline void CalculateScore(Node* p, Node* goal, Dimension::Unit* pUnit = NULL)
		{
			CalculateCost(p, pUnit);
			CalculateDistance(p, goal);
			p->f = p->g + p->h;
		}

		void AddAdjecentNodes(Dimension::Unit* unit, Node* node, AINodeList* list)
		{
			Node* adj_node = NULL;

			for (int y = -1; y <= 1; y++)
			{
				for (int x = -1; x <= 1; x++)
				{
					if (!x && !y)
						continue;
					
					if (!Dimension::SquaresAreWalkable(unit, node->x + x, node->y + y, Dimension::SIW_IGNORE_MOVING))
						continue;

					adj_node = AllocNode(node->x + x, node->y + y);
					adj_node->pParent = node;
					list->push_back(adj_node);
				}
			}
		}

		PathState PathfindingStep(ThreadData*& tdata)
		{
			// For simplicity...
			AINodeList* openList   = &tdata->openList;
			AINodeList* closedList = &tdata->closedList;
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;

			// Make sure the open-list is -not- empty
			if (openList->empty())
				return PATHSTATE_ERROR;

			// Pop the best (= lowest f) node
//			std::cout << "HEAP POP BEGIN" << std::endl;
			Node* pCurNode = openList->front();
			pop_heap(openList->begin(), openList->end(), PathfindingHeapCmp());
			openList->pop_back();
//			std::cout << "END" << std::endl;

			bool reachedGoal = false;
			
			if (md->changedGoal)
			{
				if (pCurNode == md->_goal) // << if changed goal, pointers are the same
					reachedGoal = true; 
			}
			else
			{
				if (SquareIsGoal(unit, pCurNode->x, pCurNode->y, true))
					reachedGoal = true;
				else
				{
					if (md->_action.goal.unit == NULL && md->_action.action == AI::ACTION_ATTACK)
					{
						md->action.action = AI::ACTION_GOTO;
					}
				}
			}
			
			if (reachedGoal)
			{
				if (!md->changedGoal && md->_start != md->_goal)
				{
					// Note: if original goal wasn't walkable (or unreachable for some 
					// reason) the nearest calculated node is used.		
					DeallocNode(md->_goal);
				}

				md->_goal = pCurNode;
				
				if (*pCurNode != *md->_start)
				{
					// Link each node to each other.
					Node* pChildNode = md->_goal;
					Node* pParentNode = md->_goal->pParent;

					int count = 0;
					while (*pChildNode != *md->_start)
					{
						assert(pChildNode != NULL);
						
						if (pParentNode == NULL) // << warning: this check is -not- nice! It's evil.
						{
							return PATHSTATE_ERROR;
						}

						pParentNode->pChild = pChildNode;

						pChildNode = pParentNode;
						pParentNode = pParentNode->pParent;

						count++;
						AI::pathnodes++;
					}
				}

				// Done! Let's deallocate the resources
				// we don't need to trace the path
				DeallocUnusedNodes(tdata);
				
				AI::paths++;
				return PATHSTATE_GOAL;
			}

			// Obviously not goal... let us continue our search!
			else
			{
				AINodeList adjNodes;
				AddAdjecentNodes(unit, pCurNode, &adjNodes);
				
				unsigned int length = (unsigned int)adjNodes.size();
				unsigned int omittedNodes = 0;

				for (AINodeListIter node = adjNodes.begin(); node != adjNodes.end(); node++)
				{	
					AINodeListIter nodeCmpOpen;
					AINodeListIter nodeCmpClosed;
					for (nodeCmpOpen = openList->begin(); nodeCmpOpen != openList->end(); nodeCmpOpen++)
					{
						if (*(*nodeCmpOpen) == *(*node))
							break;
					}
					if (nodeCmpOpen != openList->end())
					{
						//if ((*nodeCmpOpen)->g <= (*node)->g)
						//{
							DeallocNode((*node));
							omittedNodes++;
							continue;
						//}
					}

					for (nodeCmpClosed = closedList->begin(); nodeCmpClosed != closedList->end(); nodeCmpClosed++)
					{
						if (*(*nodeCmpClosed) == *(*node))
							break;
					}
					if (nodeCmpClosed != closedList->end())
					{
						//if ((*nodeCmpClosed)->g <= (*node)->g)
						//{
							DeallocNode((*node));
							omittedNodes++;
							continue;
						//}
					}

					CalculateScore(*node, md->_goal, unit);

//					std::cout << "HEAP PUSH BEGIN" << std::endl;
					openList->push_back((*node));
					push_heap(openList->begin(), openList->end(), PathfindingHeapCmp());
//					std::cout << "END" << std::endl;
				}
				
				if (omittedNodes == length)
					return PATHSTATE_IMPOSSIBLE;
					
				closedList->push_back(pCurNode);
			}

			return PATHSTATE_OK;
		}
		
		
		int GetNearestNode(ThreadData*& tdata, PathState& state, int& calcCount)
		{
			MovementData* md = tdata->pUnit->pMovementData;
		
			if (tdata->openList.empty() ||
			    tdata->openList.front() == md->_start)
			{
				state = PATHSTATE_ERROR;
				return ERROR_GENERAL;
			}
			
			Node* pCurNode = tdata->openList.front();
			
			int lowestIndex = 0;
			int index = 0;		
			{
				float lowestH = pCurNode->h;
				while (true) 
				{
					if (lowestH >= pCurNode->h && pCurNode != md->_start)
					{
						lowestH = pCurNode->h;
						lowestIndex = index;
					}
				
					// Skip start position (because its H-value is zero)
					if (pCurNode->pParent != NULL)
					{
						if (*pCurNode->pParent != *md->_start)
						{
							index++;
							pCurNode = pCurNode->pParent;
							continue;
						}
					}
				
					break;
				}
			}
			
			pCurNode = tdata->openList.front();
			index = 0;
			while (true)
			{
				for (unsigned int i = 0; i < tdata->openList.size(); i++)
				{
					if (tdata->openList.at(i) == pCurNode)
					{
						tdata->openList.erase(tdata->openList.begin() + i);
						i--;
					}
				}
				
				for (unsigned int i = 0; i < tdata->closedList.size(); i++)
				{
					if (tdata->closedList.at(i) == pCurNode)
					{
						tdata->closedList.erase(tdata->closedList.begin() + i);
						i--;
					}
				}

				if (index == lowestIndex)
				{
					DeallocNode(md->_goal);
					md->_goal = pCurNode;
					md->changedGoal = true;
					tdata->openList.insert(tdata->openList.begin(), pCurNode);

					do
					{
//						std::cout << "MAKING HEAP IN GetNearestNode!" << std::endl;
						make_heap(tdata->openList.begin(), tdata->openList.end(), PathfindingHeapCmp());
//						std::cout << "END" << std::endl;
						if (*tdata->openList.front() != *pCurNode)
						{
							pCurNode->f -= 0.25f;
							continue;
						}

						break;
					} while (true);
					
					calcCount = 0;
					break;
				}
				
				Node* pTemp = pCurNode;
				pCurNode = pCurNode->pParent;							
				DeallocNode(pTemp);
					
				if (pCurNode->pParent == NULL)
				{
					state = PATHSTATE_ERROR;
					return ERROR_GENERAL;
				}
				
				index ++;
			}
			return SUCCESS;
		}

		inline void ParsePopQueueReason(ThreadData*& tdata, MovementData* md)
		{
			switch (md->_reason)
			{
				case POP_NEW_GOAL:
				{
					int start_x = (int)md->_start->x, 
					    start_y = (int)md->_start->y;

					DeallocPathfinding(tdata);
					
					md->_popFromQueue = false;
					md->_currentState = INTTHRSTATE_WAITING;
					md->calcState     = CALCSTATE_WORKING;

					float* x = &md->_newx;
					float* y = &md->_newy;
					
					md->_start = AllocNode(start_x, start_y);
					md->_goal  = AllocNode((int)*x, (int)*y);
					
					gCalcQueue.push(tdata->pUnit);

					*x = 0; *y = 0;
				} break;

				case POP_DELETED:
				{
					DeallocPathfinding(tdata);
					delete md;
					delete tdata->pUnit;

					md = NULL;
				} break;

				default:
				{
					DeallocPathfinding(tdata);
					md->_currentState = INTTHRSTATE_NONE;
					md->calcState = CALCSTATE_FAILURE;
				}
			}
			
			tdata->pUnit = NULL;
		}

		int PerformPathfinding(ThreadData* tdata)
		{
			static int calcCount = 0;

			SDL_LockMutex(tdata->pMutex);

			Dimension::Unit* unit = NULL;
			MovementData* md = NULL;

			if (tdata->pUnit == NULL)
			{
				if (gCalcQueue.empty())
				{
					SDL_UnlockMutex(tdata->pMutex);	
					return PATHSTATE_EMPTY_QUEUE;
				}
				else
				{
					tdata->pUnit = gCalcQueue.front();
					gCalcQueue.pop();

					tdata->pUnit->pMovementData->_currentState = INTTHRSTATE_PROCESSING;
				}
			}

			unit = tdata->pUnit;
			md   = tdata->pUnit->pMovementData;

//			SDL_UnlockMutex(tdata->pMutex);

			if (md->_popFromQueue)
			{
//				SDL_LockMutex(tdata->pMutex);
				
				ParsePopQueueReason(tdata, md);
				tdata->pUnit = NULL;
				
				SDL_UnlockMutex(tdata->pMutex);
				return SUCCESS;
			}
			
			if (md->_goal == NULL)
			{
//				SDL_LockMutex(tdata->pMutex);

				md->_currentState = INTTHRSTATE_NONE;
				tdata->pUnit = NULL;
				
				SDL_UnlockMutex(tdata->pMutex);
				return SUCCESS;
			}

			if (md->calcState != CALCSTATE_AWAITING_NEXT_FRAME)
			{
				if (tdata->openList.size() > 0)
				{
					for (size_t i = 0; i < tdata->openList.size(); i++)
						DeallocNode(tdata->openList.at(i));
					tdata->openList.clear();
				}
				
				if (tdata->closedList.size() > 0)
				{
					for (size_t i = 0; i < tdata->closedList.size(); i++)
						DeallocNode(tdata->closedList.at(i));
					tdata->closedList.clear();
				}

//				std::cout << "MAKE HEAP IN INITIAL" << std::endl;
				make_heap(tdata->openList.begin(), tdata->openList.end(), PathfindingHeapCmp());
//				std::cout << "END" << std::endl;

				tdata->openList.push_back( md->_start );
				
//				std::cout << "PUSH HEAP IN OPENLIST" << std::endl;
				push_heap(tdata->openList.begin(), tdata->openList.end(), PathfindingHeapCmp());
//				std::cout << "END" << std::endl;
			}

			int steps = 0;
			bool quit = false; // <<< bad solution? See below...
			PathState state = PATHSTATE_OK;

			do
			{
				calcCount ++;
				steps ++;
				
//				SDL_LockMutex(tdata->pMutex);

				if (calcCount > MAXIMUM_PATH_CALCULATIONS || 
					state == PATHSTATE_IMPOSSIBLE)
				{
					if (GetNearestNode(tdata, state, calcCount) == ERROR_GENERAL)
					{
						quit = true; // << is used here, in order to prevent... (below)
						break;
					}
					
					state = PATHSTATE_OK;
				}			
				
				state = PathfindingStep(tdata);
				
				SDL_UnlockMutex(tdata->pMutex);

				if (md->_popFromQueue)
				{
					SDL_LockMutex(tdata->pMutex);
					
					ParsePopQueueReason(tdata, md);
					tdata->pUnit = NULL;
					
					SDL_UnlockMutex(tdata->pMutex);
					return SUCCESS;
				}

				if (steps > MAXIMUM_CALCULATIONS_PER_FRAME && state != PATHSTATE_GOAL)
				{
					SDL_LockMutex(tdata->pMutex);
					
					md->calcState = CALCSTATE_AWAITING_NEXT_FRAME;
					
					SDL_UnlockMutex(tdata->pMutex);
					return SUCCESS;
				}
				else  if (state == PATHSTATE_OK || state == PATHSTATE_IMPOSSIBLE)
				{
					SDL_LockMutex(tdata->pMutex);
					continue;
				}
				else
				{
					break;
				}
			
			} while (1);


			if (!quit)
				SDL_LockMutex(tdata->pMutex); // << ... mutex from locking when quitting the loop

			md->_currentState = INTTHRSTATE_NONE;
			md->changedGoal = false;
			calcCount = 0;
			
			int ret = ERROR_GENERAL;
			if (md->_popFromQueue)
			{
				ParsePopQueueReason(tdata, md);
				ret = SUCCESS;
			}
			else
			{
				if (state == PATHSTATE_GOAL)
				{
					md->calcState = CALCSTATE_REACHED_GOAL;
					ret = SUCCESS;
				}
				else
				{
					DeallocPathfinding(tdata);

					md->calcState = CALCSTATE_FAILURE;
				}
			}
			
			tdata->pUnit = NULL;
			
			SDL_UnlockMutex(tdata->pMutex);
			
			return ret;
		}
	}
}
