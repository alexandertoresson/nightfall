/*
 *
 * This is a highly optimized implementation of the A* algorithm.
 * A disadvantage with this implementation is that it may use quite
 * much memory.
 *
 * The algorithm ends when goal is reached, when the unit is close
 * enough to the goal (to execute given orders) or when the goal is
 * determined to be unreachable.
 *
 * As A* is quite slow (even though this implementation is very
 * optimized) some special techniques are used to prevent many
 * situations when it would not find any road to the goal. If A*
 * finds itself in that situation, it searches the full map.
 * Firstly, maps of so-called 'area codes' are used, one map per
 * movement type a unit can have. If two positions in the map have
 * the same area code, you can go from one of the squares to the
 * other, otherwise not. Also, if the target square is currently
 * blocked it is impossible to reach the other square (at least
 * when action is ACTION_GOTO).
 *
 * When it thinks that it is impossible to reach the target square, 
 * a special flood fill algorithm is used to find the nearest square
 * of the target square that you can actually go to from the
 * start square, and when that is determined, the path finding is
 * used normally to search from the start square to the nearest
 * reachable square from the target.
 *
 * Also, if it originally determines that it can probably reach the
 * target square, but it after 2000 iterations hasn't found the
 * target yet, it will try a flood fill to determine whether it can
 * reach the target and perhaps revise its target square, and then
 * run the path finding again. And the second time it will run the
 * path finding until it is done no matter if it finds a target square
 * within 2000 iterations or not.
 *
 */

#include "aipathfinding.h"

#include "game.h"
#include "dimension.h"
#include "ainode.h"
#include "unit.h"
#include "networking.h"
#define BINARY_HEAP_DATATYPE int
#include "binaryheap.h"

#define MAXIMUM_PATH_CALCULATIONS 2e16
#define RECALC_TRACE_LIMIT 1000
#define RECALC_FLOODFILL_LIMIT 4000

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
			
		int                lowestH;
		int                lowestDistance;
		int                nearestNode;
		binary_heap_t*     heap;
		int                firstScanlineIndex;
		int                lastScanlineIndex;
		int                numScanlines;
		int                nextFreeScanline;
		bool               calculateNearestReachable;
		bool               setAreaCodes;
		PreprocessState    preprocessState;

		unsigned short**   nodeTypes;
		int**              nodeNums;
		int***             areaMaps;
		int                areaMapIndex;
		int                areaCode;

		unsigned short NODE_TYPE_OPEN = 1, NODE_TYPE_CLOSED = 2, NODE_TYPE_BLOCKED = 3;

		struct node
		{
			int x, y;
			int f, g, h;
			int parent;
		};

		struct node *nodes;

		int *scores;
		int *positions;

		int *openList, openListSize, nextFreeNode;


		struct scanline
		{
			int start_x, end_x;
			int y;
		};

		struct scanline *scanlines;
		int scanlineArraySize;

		int *scanlineQueue;
		int scanlineQueueSize;

		int width, height;

#define FLOODFILL_FLAG_CALCULATE_NEAREST 1
#define FLOODFILL_FLAG_SET_AREA_CODE 2

		PathState InitFloodfill(Dimension::Unit* unit, int start_x, int start_y, int flags);
		PathState FloodfillStep(Dimension::Unit* unit, MovementData* md, int target_x, int target_y);

		bool IsWalkable_MType(int areaMapIndex, int x, int y)
		{
			return Game::Dimension::MovementTypeCanWalkOnSquare((Game::Dimension::MovementType) areaMapIndex, x, y);
		}

		void InitAreaCodes()
		{
			int x, y, i;
			for (i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
			{
				areaCode = 1;
				for (y = 0; y < height; y++)
				{
					for (x = 0; x < width; x++)
					{
						if (IsWalkable_MType(i, x, y) && areaMaps[i][y][x] == 0)
						{
							areaMapIndex = i;
							if (InitFloodfill(NULL, x, y, FLOODFILL_FLAG_SET_AREA_CODE) == PATHSTATE_OK)
							{
								while (FloodfillStep(NULL, NULL, 0, 0) == PATHSTATE_OK)
								{
									
								}
							}
							areaCode++;
						}
					}
				}
				printf("Area codes: %d\n", areaCode-1);
			}
		}

		void InitPathfindingThreading(void)
		{
			int y;
			width = Game::Dimension::pWorld->width;
			height = Game::Dimension::pWorld->width;
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

			nodeTypes = (unsigned short**) malloc(height * sizeof(unsigned short*));
			nodeNums = (int**) malloc(height * sizeof(int*));
			for (y = 0; y < height; y++)
			{
				nodeTypes[y] = (unsigned short*) calloc(width, sizeof(unsigned short));
				nodeNums[y] = (int*) calloc(width, sizeof(int));
			}
			areaMaps = (int***) malloc(Game::Dimension::MOVEMENT_TYPES_NUM * sizeof(int**));
			for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
			{
				areaMaps[i] = (int**) malloc(height * sizeof(int*));
				for (y = 0; y < height; y++)
				{
					areaMaps[i][y] = (int*) calloc(width, sizeof(int));
				}
			}
			nodes = (struct node*) malloc(width*height * sizeof(struct node));
			scores = (int*) malloc(width*height * sizeof(int));
			positions = (int*) malloc(width*height * sizeof(int));
			openList = (int*) malloc(width*height * sizeof(int));
			openListSize = width * height;

			scanlines = (struct scanline*) malloc(1024 * sizeof(struct scanline));
			scanlineArraySize = 1024;
			
			scanlineQueue = (int*) malloc((width + height) * sizeof(int));
			scanlineQueueSize = width + height;

			InitAreaCodes();

			heap = binary_heap_create(scores, openList, positions);
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
			
			p = new Node();

			p->pChild = NULL;
			p->pParent = NULL;
			
			p->x = x;
			p->y = y;
						
			return p;
		}
		
		inline void DeallocNode(Node*& p)
		{
#ifdef DEBUG_AI_PATHFINDING
			ALLOCATIONS--;
#endif
			delete p;
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
			
			md->action.startPos.x = 0;
			md->action.startPos.y = 0;
			md->action.goal.pos.x = 0;
			md->action.goal.pos.y = 0;
			md->action.goal.unit = NULL;
			md->action.goal.goal_id = 0xFFFF;
			md->action.changedGoalPos.x = 0;
			md->action.changedGoalPos.y = 0;
			md->action.arg = NULL;
			md->action.action = ACTION_GOTO;
			
			md->_currentState = INTTHRSTATE_NONE;
			md->_popFromQueue = false;
			md->_start = NULL;
			md->_goal = NULL;
			md->_action.startPos.x = 0;
			md->_action.startPos.y = 0;
			md->_action.goal.pos.x = 0;
			md->_action.goal.pos.y = 0;
			md->_action.goal.unit = NULL;
			md->_action.changedGoalPos.x = 0;
			md->_action.changedGoalPos.y = 0;
			md->_action.goal.unit = NULL;
			md->_action.goal.goal_id = 0xFFFF;
			md->_action.arg = NULL;
			md->_action.action = ACTION_GOTO;
			
			md->_newAction.startPos.x = 0;
			md->_newAction.startPos.y = 0;
			md->_newAction.goal.pos.x = 0;
			md->_newAction.goal.pos.y = 0;
			md->_newAction.goal.unit = NULL;
			md->_newAction.goal.goal_id = 0xFFFF;
			md->_newAction.changedGoalPos.x = 0;
			md->_newAction.changedGoalPos.y = 0;
			md->_newAction.goal.unit = NULL;
			md->_newAction.arg = NULL;
			md->_newAction.action = ACTION_GOTO;
			
#ifdef DEBUG_AI_PATHFINDING
			std::cout << "Movement data init: " << pUnit << std::endl;
			md->_cycles = 0;
#endif
		}
		
		IPResult CommandPathfinding(Dimension::Unit* pUnit, float start_x, float start_y, float goal_x, float goal_y, AI::UnitAction action, Dimension::Unit* target, void* args)
		{
			assert(pUnit != NULL);

			if (!pUnit->type->isMobile)
				return IPR_IS_IMMOBILE;

			if ((int)start_x == (int)goal_x && (int)start_y == (int)goal_y)
				return IPR_GOAL_IS_START;

			SDL_LockMutex(gpmxPathfinding);

			IntThrState curState = pUnit->pMovementData->_currentState;

#ifdef DEBUG_AI_PATHFINDING
			std::cout << "State: " << curState << " | Length: " << gCalcQueue.size() << std::endl;
#endif

			IPResult res;
			MovementData* md = pUnit->pMovementData;
			
			md->_newAction.startPos.x = start_x;
			md->_newAction.startPos.y = start_y;
			md->_newAction.goal.pos.x = goal_x;
			md->_newAction.goal.pos.y = goal_y;
			md->_newAction.goal.unit = target;
			md->_newAction.changedGoalPos.x = goal_x;
			md->_newAction.changedGoalPos.y = goal_y;
			md->_newAction.arg = args;
			md->_newAction.action = action;
				
			if (curState == INTTHRSTATE_PROCESSING)
			{
				md->_popFromQueue = true;
				md->_reason = POP_NEW_GOAL;
				
				res = IPR_SUCCESS_POPCMD_ISSUED;
			}
			else
			{
				if (md->_start != NULL)
					DeallocPathfindingNodes(pUnit, DPN_BACK);

				md->_action = md->_newAction;
			       
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

			md->action = md->_action;

			unit->action = md->_action.action;
			
			md->_start = NULL;
			md->_goal = NULL;
			
			md->_action.startPos.x = 0;
			md->_action.startPos.y = 0;
			md->_action.goal.pos.x = 0;
			md->_action.goal.pos.y = 0;
			md->_action.goal.unit  = NULL;
			md->_action.changedGoalPos.x = 0;
			md->_action.changedGoalPos.y = 0;
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
			
			if (data->_start != NULL)
			{
				DeallocPathfindingNodes(tdata->pUnit, DPN_BACK);
			}

			data->_start = NULL;
			data->_goal  = NULL;
			
			data->changedGoal = false;
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
		
		inline bool IsWalkable(Dimension::Unit* unit, int x, int y)
		{
			return Dimension::SquaresAreWalkable(unit, x, y, Dimension::SIW_IGNORE_MOVING);
		}

		inline int SameAreaAndWalkable(Dimension::Unit* unit, int start_x, int start_y, int cur_x, int cur_y)
		{
			if (cur_x > 0 && cur_y > 0 && cur_x < width && cur_y < height)
			{
				if (nodeTypes[cur_y][cur_x] == NODE_TYPE_OPEN)
				{
					return 1;
				}
				if (nodeTypes[cur_y][cur_x] == NODE_TYPE_CLOSED)
				{
					return 0;
				}
				if (areaMaps[unit->type->movementType][cur_y][cur_x] == areaMaps[unit->type->movementType][start_y][start_x])
				{
					if (IsWalkable(unit, cur_x, cur_y))
					{
						nodeTypes[cur_y][cur_x] = NODE_TYPE_OPEN;
						return 1;
					}
				}
				nodeTypes[cur_y][cur_x] = NODE_TYPE_CLOSED;
			}
			return 0;
		}

		int traceCurX, traceCurY;
		int traceStartX, traceStartY;
		int traceStage;
		int traceLastDir, traceFirstDir;

		int traceDirs[8][2] = {{-1,  0},
		                       {-1, -1},
		                       { 0, -1},
		                       { 1, -1},
		                       { 1,  0},
		                       { 1,  1},
		                       { 0,  1},
		                       {-1,  1}};

		bool InitTrace(Dimension::Unit* unit, int start_x, int start_y, int target_x, int target_y)
		{
			int y;
			traceCurX = target_x;
			traceCurY = target_y;

			NODE_TYPE_OPEN += 3;
			NODE_TYPE_CLOSED += 3;
			NODE_TYPE_BLOCKED += 3;
			if (NODE_TYPE_OPEN == 0 || NODE_TYPE_CLOSED == 0 || NODE_TYPE_BLOCKED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(nodeTypes[y], 0, width * sizeof(unsigned short));
					memset(nodeNums[y], 0, width * sizeof(int));
				}
				NODE_TYPE_OPEN = 1;
				NODE_TYPE_CLOSED = 2;
				NODE_TYPE_BLOCKED = 3;
			}

			if (SameAreaAndWalkable(unit, start_x, start_y, traceCurX, traceCurY))
			{
				return PATHSTATE_ERROR;
			}

			traceStage = 0;
			lowestDistance = 100000000;

			return PATHSTATE_OK;
		}

		int TraceStep(Dimension::Unit* unit, MovementData* md, int start_x, int start_y, int target_x, int target_y)
		{
			int new_distance;
			int i, j = 0;
			int old_x, old_y;

			if (traceStage == 0)
			{
				for ( ; j < 10; j++)
				{
					old_x = traceCurX;

					if (start_x < traceCurX)
						traceCurX--;
					
					if (start_x > traceCurX)
						traceCurX++;

					old_y = traceCurY;
					
					if (start_y < traceCurY)
						traceCurY--;
					
					if (start_y > traceCurY)
						traceCurY++;

					if (SameAreaAndWalkable(unit, start_x, start_y, traceCurX, traceCurY))
					{
						if (SameAreaAndWalkable(unit, start_x, start_y, traceCurX, old_y))
						{
							traceCurY = old_y;
						}

						for (i = 0; i < 8; i++)
						{
							if (traceDirs[i][0] == traceCurX - old_x && traceDirs[i][1] == traceCurY - old_y)
							{
								traceFirstDir = i;
								break;
							}
						}

						for (i = 0; i < 8; i++)
						{
							int dir = (traceFirstDir-i)&7;
							if (SameAreaAndWalkable(unit, start_x, start_y, traceCurX-traceDirs[dir][0], traceCurY-traceDirs[dir][1]))
							{
								traceFirstDir = traceLastDir = dir;

								break;
							}
						}

						traceStartX = traceCurX;
						traceStartY = traceCurY;

						traceStage = 1;

						break;

					}
					if (start_x == traceCurX && start_y == traceCurY)
					{
						return PATHSTATE_GOAL;
					}
				}

			}

			if (traceStage == 1)
			{

				for ( ; j < 10; j++)
				{
					int new_dir = -1;
					int first_dir_to_check = ((traceLastDir&6)-1)&7;
					int num_dirs_to_check = 6+(traceLastDir&1);
					new_distance = (target_y - traceCurY) * (target_y - traceCurY) + (target_x - traceCurX) * (target_x - traceCurX);
					if (new_distance < lowestDistance)
					{
						lowestDistance = new_distance;
						md->_action.changedGoalPos.x = traceCurX;
						md->_action.changedGoalPos.y = traceCurY;
						if (new_distance == 0)
						{
							return PATHSTATE_GOAL;
						}
					}
			//		printf("%d, %d\n", traceCurX, traceCurY);
					for (i = 0; i < num_dirs_to_check; i++)
					{
						int dir = (first_dir_to_check+i)&7;
						if (SameAreaAndWalkable(unit, start_x, start_y, traceCurX+traceDirs[dir][0], traceCurY+traceDirs[dir][1]))
						{
							traceCurX += traceDirs[dir][0];
							traceCurY += traceDirs[dir][1];
							new_dir = dir;
							break;
						}
					}
					if (new_dir == -1)
					{
						return PATHSTATE_GOAL;
					}
					traceLastDir = new_dir;
					if (traceStartX == traceCurX && traceStartY == traceCurY && traceLastDir == traceFirstDir)
					{
						return PATHSTATE_GOAL;
					}
				}
			}

			return PATHSTATE_OK;
		}

		PathState InitFloodfill(Dimension::Unit* unit, int start_x, int start_y, int flags)
		{
			int x, y;

			if (calculateNearestReachable)
			{
				if (!IsWalkable(unit, start_x, start_y))
				{
					return PATHSTATE_ERROR;
				}
			}
			else
			{
				if (!IsWalkable_MType(areaMapIndex, start_x, start_y))
				{
					return PATHSTATE_ERROR;
				}
			}

			calculateNearestReachable = flags & FLOODFILL_FLAG_CALCULATE_NEAREST;
			setAreaCodes = flags & FLOODFILL_FLAG_SET_AREA_CODE;

			firstScanlineIndex = 0;
			lastScanlineIndex = 0;
			numScanlines = 0;
			lowestDistance = 100000000;

			NODE_TYPE_OPEN += 3;
			NODE_TYPE_CLOSED += 3;
			NODE_TYPE_BLOCKED += 3;
			if (NODE_TYPE_OPEN == 0 || NODE_TYPE_CLOSED == 0 || NODE_TYPE_BLOCKED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(nodeTypes[y], 0, width * sizeof(unsigned short));
					memset(nodeNums[y], 0, width * sizeof(int));
				}
				NODE_TYPE_OPEN = 1;
				NODE_TYPE_CLOSED = 2;
				NODE_TYPE_BLOCKED = 3;
			}

			if (calculateNearestReachable)
			{
				scanlines[0].y = start_y;

				for (x = start_x-1; ; x--)
				{
					if (!IsWalkable(unit, x, start_y))
					{
						break;
					}
				}
				scanlines[0].start_x = x+1;

				if (x >= 0)
				{
					nodeTypes[start_y][x] = NODE_TYPE_BLOCKED;
				}

				nodeTypes[start_y][x+1] = NODE_TYPE_CLOSED;
				nodeNums[start_y][x+1] = 0;

				for (x = start_x+1; ; x++)
				{
					if (!IsWalkable(unit, x, start_y))
					{
						break;
					}
				}
			}
			else
			{
				scanlines[0].y = start_y;

				for (x = start_x-1; ; x--)
				{
					if (!IsWalkable_MType(areaMapIndex, x, start_y))
					{
						break;
					}
				}
				scanlines[0].start_x = x+1;

				if (x >= 0)
				{
					nodeTypes[start_y][x] = NODE_TYPE_BLOCKED;
				}

				nodeTypes[start_y][x+1] = NODE_TYPE_CLOSED;
				nodeNums[start_y][x+1] = 0;

				for (x = start_x+1; ; x++)
				{
					if (!IsWalkable_MType(areaMapIndex, x, start_y))
					{
						break;
					}
				}
			}
			scanlines[0].end_x = x-1;
				
			if (x < width)
			{
				nodeTypes[start_y][x] = NODE_TYPE_BLOCKED;
			}

			numScanlines = 1;

			nextFreeScanline = 1;

			scanlineQueue[0] = 0;

			return PATHSTATE_OK;
		}

		PathState FloodfillStep(Dimension::Unit* unit, MovementData* md, int target_x, int target_y)
		{
			int first_node;
			int start_x, end_x;
			int new_y;
			int new_start_x, new_end_x;
			int new_x;
			unsigned short *scanline_types;
			int *scanline_nums;
			int loop_start_y, loop_end_y;
			int loop_start_x, loop_end_x;
			int x, y;

			if (numScanlines == 0)
			{
				return PATHSTATE_GOAL;
			}

			first_node = scanlineQueue[firstScanlineIndex];

			start_x = scanlines[first_node].start_x;
			end_x = scanlines[first_node].end_x;
			y = scanlines[first_node].y;

			if (setAreaCodes)
			{
				for (x = start_x; x <= end_x; x++)
				{
					areaMaps[areaMapIndex][y][x] = areaCode;
				}
			}

			if (calculateNearestReachable)
			{
				int new_distance;
				int t_x;
				if (target_x >= start_x && target_x <= end_x)
				{
					new_distance = target_y - y;
					new_distance *= new_distance;
					t_x = target_x;
				}
				else if (target_x < start_x)
				{
					new_distance = (target_y - y) * (target_y - y) + (target_x - start_x) * (target_x - start_x);
					t_x = start_x;
				}
				else /* if (*target_x < start_x) */
				{
					new_distance = (target_y - y) * (target_y - y) + (target_x - end_x) * (target_x - end_x);
					t_x = end_x;
				}
				if (new_distance < lowestDistance)
				{
					lowestDistance = new_distance;
					md->_action.changedGoalPos.x = t_x;
					md->_action.changedGoalPos.y = y;
					if (new_distance == 0)
					{
						return PATHSTATE_GOAL;
					}
				}
			}

			firstScanlineIndex++;
			numScanlines--;
			
			if (firstScanlineIndex == scanlineQueueSize)
			{
				firstScanlineIndex = 0;
			}

			loop_start_y = y-1 >= 0 ? y-1 : 1;
			loop_end_y = y+1 < height ? y+1 : y-1;
			
			loop_start_x = start_x-1 >= 0 ? start_x-1 : 0;
			loop_end_x = end_x+1 < width ? end_x+1 : end_x;

			for (new_y = loop_start_y; new_y <= loop_end_y; new_y+=2)
			{
				scanline_types = nodeTypes[new_y];
				scanline_nums = nodeNums[new_y];
				for (x = loop_start_x; x <= loop_end_x; x++)
				{
					if (scanline_types[x] == NODE_TYPE_CLOSED)
					{
						x = scanlines[scanline_nums[x]].end_x+1;
					}
					else if (scanline_types[x] != NODE_TYPE_BLOCKED)
					{
						if ((calculateNearestReachable && IsWalkable(unit, x, new_y)) || (setAreaCodes && IsWalkable_MType(areaMapIndex, x, new_y)))
						{
							int new_scanline;
							if (setAreaCodes)
							{
								for (new_x = x-1; ; new_x--)
								{
									if (scanline_types[x] == NODE_TYPE_BLOCKED || !IsWalkable_MType(areaMapIndex, new_x, new_y))
									{
										break;
									}
								}
							}
							else
							{
								for (new_x = x-1; ; new_x--)
								{
									if (scanline_types[x] == NODE_TYPE_BLOCKED || !IsWalkable(unit, new_x, new_y))
									{
										break;
									}
								}
							}
							new_start_x = new_x+1;
						
							if (new_x >= 0)
							{
								scanline_types[new_x] = NODE_TYPE_BLOCKED;
							}

							if (scanline_types[new_start_x] != NODE_TYPE_CLOSED)
							{
								if (setAreaCodes)
								{
									for (new_x = x+1; ; new_x++)
									{
										if (scanline_types[x] == NODE_TYPE_BLOCKED || !IsWalkable_MType(areaMapIndex, new_x, new_y))
										{
											break;
										}
									}
								}
								else
								{
									for (new_x = x+1; ; new_x++)
									{
										if (scanline_types[x] == NODE_TYPE_BLOCKED || !IsWalkable(unit, new_x, new_y))
										{
											break;
										}
									}
								}
								new_end_x = new_x-1;
							
								if (new_x < width)
								{
									scanline_types[new_x] = NODE_TYPE_BLOCKED;
								}

								new_scanline = nextFreeScanline++;

								if (new_scanline >= scanlineArraySize)
								{
									scanlineArraySize = scanlineArraySize * 2;
									scanlines = (struct scanline*) realloc(scanlines, scanlineArraySize * sizeof(struct scanline));
								}

								scanlines[new_scanline].start_x = new_start_x;
								scanlines[new_scanline].end_x = new_end_x;
								scanlines[new_scanline].y = new_y;

								x = new_x;
								
								lastScanlineIndex++;
								numScanlines++;
								
								if (lastScanlineIndex == scanlineQueueSize)
								{
									lastScanlineIndex = 0;
								}
								else if (lastScanlineIndex == firstScanlineIndex && numScanlines != 1)
								{
									printf("FATAL - end of circular buffer reached start\n");
									md->_action.changedGoalPos.x = target_x;
									md->_action.changedGoalPos.y = target_y;
									return PATHSTATE_GOAL;
								}

								scanlineQueue[lastScanlineIndex] = new_scanline;
				
								if (scanline_types[new_start_x] == NODE_TYPE_CLOSED)
								{
									printf("FATAL - traversed same scanline twice!\n");
									md->_action.changedGoalPos.x = target_x;
									md->_action.changedGoalPos.y = target_y;
									return PATHSTATE_GOAL;
								}
				
								scanline_types[new_start_x] = NODE_TYPE_CLOSED;
								scanline_nums[new_start_x] = new_scanline;
								
								if (new_start_x > start_x)
								{
									nodeTypes[y][new_start_x-1] = NODE_TYPE_CLOSED;
									nodeNums[y][new_start_x-1] = first_node;
								}
							}
							else
							{
								new_x = scanlines[scanline_nums[new_start_x]].end_x+1;
								if (new_x <= x)
								{
									// what would have become an infinite loop has been detected,
									// cancel the calculation...
									md->_action.changedGoalPos.x = target_x;
									md->_action.changedGoalPos.y = target_y;
									return PATHSTATE_GOAL;
								}
								else
								{
									x = new_x;
								}
							}
						}
						else
						{
							scanline_types[x] = NODE_TYPE_BLOCKED;
						}
					}
				}
			}
			return PATHSTATE_OK;
		}

		int num_steps = 0;

		PathState InitPathfinding(ThreadData*& tdata)
		{
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;

			int start_x = (int) md->_action.startPos.x, start_y = (int) md->_action.startPos.y;
			int target_x = (int) md->_action.goal.pos.x, target_y = (int) md->_action.goal.pos.y;

			int y;

			num_steps = 0;

			lowestH = 1<<30;
			nearestNode = -1;

			if (!IsWalkable(unit, start_x, start_y))
			{
				return PATHSTATE_ERROR;
			}

			NODE_TYPE_OPEN += 3;
			NODE_TYPE_CLOSED += 3;
			NODE_TYPE_BLOCKED += 3;
			if (NODE_TYPE_OPEN == 0 || NODE_TYPE_CLOSED == 0 || NODE_TYPE_BLOCKED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(nodeTypes[y], 0, width * sizeof(unsigned short));
					memset(nodeNums[y], 0, width * sizeof(int));
				}
				NODE_TYPE_OPEN = 1;
				NODE_TYPE_CLOSED = 2;
				NODE_TYPE_BLOCKED = 3;
			}

			nodes[0].x = start_x;
			nodes[0].y = start_y;
			nodes[0].g = 0;
			nodes[0].h = (abs(start_y-target_y) + abs(start_x-target_x)) * 10;
			nodes[0].f = 0;
			nodes[0].parent = -1;
			nodeNums[start_y][start_x] = 0;
			nodeTypes[start_y][start_x] = NODE_TYPE_OPEN;
			nextFreeNode = 1;
			binary_heap_pop_all(heap); // To be sure the heap is empty...
			binary_heap_push_item(heap, 0, 0);
			return PATHSTATE_OK;
		}

		PathState PathfindingStep(ThreadData*& tdata)
		{
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;
			int target_x = (int) md->_action.changedGoalPos.x, target_y = (int) md->_action.changedGoalPos.y;
			int x, y;
			
			int first_node;
			int node_x, node_y;
			unsigned short *scanline_types;
			int *scanline_nums;
			first_node = binary_heap_pop_item(heap, -1);

			num_steps++;

			if (first_node == -1)
			{
	/*			printf("Did not reach target\n"); */
				md->_action.changedGoalPos.x = nodes[nearestNode].x;
				md->_action.changedGoalPos.y = nodes[nearestNode].y;
				return PATHSTATE_GOAL;
			}

			if (lowestH > nodes[first_node].h)
			{
				lowestH = nodes[first_node].h;
				nearestNode = first_node;
			}

			node_x = nodes[first_node].x;
			node_y = nodes[first_node].y;
			nodeTypes[node_y][node_x] = NODE_TYPE_CLOSED;

			if ((node_x == target_x && node_y == target_y) || SquareIsGoal(unit, node_x, node_y, true))
			{
	/*			printf("Reached target\n"); */
				binary_heap_pop_all(heap);
				return PATHSTATE_GOAL;
			}

			for (y = -1; y <= 1; y++)
			{
				int new_y = node_y+y;
				if (new_y >= 0 && new_y < height)
				{
					scanline_types = nodeTypes[new_y];
					scanline_nums = nodeNums[new_y];
					for (x = -1; x <= 1; x++)
					{
						int new_x = node_x+x;
						if ((y || x) && new_x >= 0 && new_x < width)
						{
							int node_type = scanline_types[new_x];
							if (node_type != NODE_TYPE_CLOSED)
							{
								int node_num;
								if (node_type == NODE_TYPE_OPEN)
								{
									int new_g = nodes[first_node].g;
									new_g += Dimension::GetTraversalTimeAdjusted(unit, node_x, node_y, x, y);

									node_num = scanline_nums[new_x];
									if (new_g < nodes[node_num].g)
									{
										if (node_num == first_node)
										{
											printf("Ouch\n");
										}
										nodes[node_num].g = new_g;
										nodes[node_num].f = nodes[node_num].h + new_g;
										nodes[node_num].parent = first_node;
										binary_heap_lower_item_score(heap, node_num, nodes[node_num].f);
									}
									else
									{
										continue;
									}
								}
								else
								{
									if (IsWalkable(unit, new_x, new_y))
									{
										int new_g = nodes[first_node].g;
										new_g += Dimension::GetTraversalTimeAdjusted(unit, node_x, node_y, x, y);

										node_num = scanline_nums[new_x] = nextFreeNode++;
										if (node_num == first_node)
										{
											printf("Ouch\n");
										}
										nodes[node_num].x = new_x;
										nodes[node_num].y = new_y;
										nodes[node_num].g = new_g;
										nodes[node_num].h = (abs(new_y-target_y) + abs(new_x-target_x)) * 10;
										nodes[node_num].f = nodes[node_num].h + new_g;
										nodes[node_num].parent = first_node;
										scanline_types[new_x] = NODE_TYPE_OPEN;
										binary_heap_push_item(heap, node_num, nodes[node_num].f);
									}
									else
									{
										scanline_types[new_x] = NODE_TYPE_CLOSED;
									}
								}
							}
						}
					}
				}
			}
			return PATHSTATE_OK;
		}

		void BuildNodeLinkedList(ThreadData*& tdata)
		{
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;
			Node *prev_node = NULL, *first_node = NULL, *new_node = NULL;
			int cur_node = nearestNode;
			while (cur_node != -1)
			{
				new_node = AllocNode(nodes[cur_node].x, nodes[cur_node].y);
				new_node->pChild = prev_node;
				if (prev_node)
				{
					prev_node->pParent = new_node;
				}
				else
				{
					new_node->pParent = NULL;
				}
				if (!first_node)
				{
					first_node = new_node;
				}
				cur_node = nodes[cur_node].parent;
				prev_node = new_node;
			}
			md->_goal = first_node;
			md->_start = new_node;
		}
		
		inline void ParsePopQueueReason(ThreadData*& tdata, MovementData* md)
		{
			switch (md->_reason)
			{
				case POP_NEW_GOAL:
				{
//					printf("new_goal\n");
					
					md->_popFromQueue = false;
					md->_currentState = INTTHRSTATE_WAITING;
					md->calcState     = CALCSTATE_WORKING;

					md->_action = md->_newAction;

					gCalcQueue.push(tdata->pUnit);

				} break;

				case POP_DELETED:
				{
//					printf("deleted\n");
					DeallocPathfinding(tdata);
					
					delete md;
					delete tdata->pUnit;

					md = NULL;
				} break;

				default:
				{
//					printf("default\n");
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
			static int fsteps = 0, psteps = 0;
			static int tsteps = 0;

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
					preprocessState = PREPROCESSSTATE_NONE;
				}
			}

			unit = tdata->pUnit;
			md   = tdata->pUnit->pMovementData;

			if (md->_popFromQueue)
			{
//				printf("begin\n");
				ParsePopQueueReason(tdata, md);
				tdata->pUnit = NULL;
				
				SDL_UnlockMutex(tdata->pMutex);
				return SUCCESS;
			}
			
			int steps = 0;
			bool quit = false; // <<< bad solution? See below...
			bool done = false;
			PathState state = PATHSTATE_OK;

			while (!done)
			{

				if (preprocessState < PREPROCESSSTATE_SKIPPED_TRACE)
				{
					if (preprocessState == PREPROCESSSTATE_NONE)
					{
						int m_type = unit->type->movementType;
						if (IsWalkable(unit, (int)md->_action.goal.pos.x, (int)md->_action.goal.pos.y) && areaMaps[m_type][(int)md->_action.startPos.y][(int)md->_action.startPos.x] == areaMaps[m_type][(int)md->_action.goal.pos.y][(int)md->_action.goal.pos.x])
						{
							preprocessState = PREPROCESSSTATE_SKIPPED_TRACE; // Skip it for now
							InitPathfinding(tdata);
						}
						else
						{
							preprocessState = PREPROCESSSTATE_PROCESSING_TRACE;
							if (InitTrace(unit, (int)md->_action.startPos.x, (int)md->_action.startPos.y, (int)md->_action.goal.pos.x, (int)md->_action.goal.pos.y) == PATHSTATE_ERROR)
							{
								preprocessState = PREPROCESSSTATE_PROCESSING_FLOOD;
								if (InitFloodfill(unit, (int)md->_action.startPos.x, (int)md->_action.startPos.y, FLOODFILL_FLAG_CALCULATE_NEAREST) == PATHSTATE_ERROR)
								{
									preprocessState = PREPROCESSSTATE_SKIPPED_FLOOD;
									InitPathfinding(tdata);
								}
							}
						}
					}
					if (preprocessState == PREPROCESSSTATE_PROCESSING_TRACE)
					{
						do
						{
							calcCount++;
							steps++;
							tsteps++;
						
							if (TraceStep(unit, md, (int)md->_action.startPos.x, (int)md->_action.startPos.x, (int)md->_action.goal.pos.x, (int)md->_action.goal.pos.y) == PATHSTATE_GOAL)
							{
								preprocessState = PREPROCESSSTATE_SKIPPED_FLOOD;
								InitPathfinding(tdata);
								break;
							}
						
							SDL_UnlockMutex(tdata->pMutex);

							if (steps > MAXIMUM_CALCULATIONS_PER_FRAME)
							{
								SDL_LockMutex(tdata->pMutex);
								
								md->calcState = CALCSTATE_AWAITING_NEXT_FRAME;
								
								SDL_UnlockMutex(tdata->pMutex);
								return SUCCESS;
							}
							else
							{
								SDL_LockMutex(tdata->pMutex);
							}
						} while (1);
					}
					if (preprocessState == PREPROCESSSTATE_PROCESSING_FLOOD)
					{
						do
						{
							calcCount++;
							steps++;
							fsteps++;
						
							if (FloodfillStep(unit, md, (int)md->_action.goal.pos.x, (int)md->_action.goal.pos.y) == PATHSTATE_GOAL)
							{
								preprocessState = PREPROCESSSTATE_DONE;
								InitPathfinding(tdata);
								break;
							}
						
							SDL_UnlockMutex(tdata->pMutex);

							if (steps > MAXIMUM_CALCULATIONS_PER_FRAME)
							{
								SDL_LockMutex(tdata->pMutex);
								
								md->calcState = CALCSTATE_AWAITING_NEXT_FRAME;
								
								SDL_UnlockMutex(tdata->pMutex);
								return SUCCESS;
							}
							else
							{
								SDL_LockMutex(tdata->pMutex);
							}
						} while (1);
					}
				}

				if (preprocessState >= PREPROCESSSTATE_SKIPPED_TRACE)
				{

					do
					{
						calcCount ++;
						steps ++;
						psteps++;
						
						if (calcCount > MAXIMUM_PATH_CALCULATIONS || 
							state == PATHSTATE_IMPOSSIBLE)
						{
							done = true;
							quit = true; // << is used here, in order to prevent... (below)
							break;
						}

						if (calcCount >= RECALC_TRACE_LIMIT && preprocessState == PREPROCESSSTATE_SKIPPED_TRACE)
						{
							if (InitTrace(unit, (int)md->_action.startPos.x, (int)md->_action.startPos.y, (int)md->_action.goal.pos.x, (int)md->_action.goal.pos.y) != PATHSTATE_ERROR)
							{
								preprocessState = PREPROCESSSTATE_PROCESSING_TRACE;
								break;
							}
							else
							{
								preprocessState = PREPROCESSSTATE_SKIPPED_FLOOD;
							}
						}

						if (calcCount >= RECALC_FLOODFILL_LIMIT && preprocessState == PREPROCESSSTATE_SKIPPED_FLOOD)
						{
							if (InitFloodfill(unit, (int)md->_action.startPos.x, (int)md->_action.startPos.y, FLOODFILL_FLAG_CALCULATE_NEAREST) != PATHSTATE_ERROR)
							{
								preprocessState = PREPROCESSSTATE_PROCESSING_FLOOD;
								break;
							}
							else
							{
								preprocessState = PREPROCESSSTATE_DONE;
							}
						}

						state = PathfindingStep(tdata);
						
						SDL_UnlockMutex(tdata->pMutex);

						if (md->_popFromQueue)
						{
//							printf("middle\n");
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
//							cout << calcCount << " total, " << tsteps << " trace, " << fsteps << " flood, " << psteps << " a*, " << unit << endl;
							done = true;
							quit = true;
							break;
						}
					
					} while (1);
				}
			}

			if (!quit)
				SDL_LockMutex(tdata->pMutex); // << ... mutex from locking when quitting the loop

			md->_currentState = INTTHRSTATE_NONE;
			md->changedGoal = false;
			calcCount = 0;
			psteps = 0;
			fsteps = 0;
			tsteps = 0;
			
			int ret = ERROR_GENERAL;
			if (md->_popFromQueue)
			{
//				printf("end\n");
				ParsePopQueueReason(tdata, md);
				ret = SUCCESS;
			}
			else
			{
				if (state == PATHSTATE_GOAL)
				{
					BuildNodeLinkedList(tdata);
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
