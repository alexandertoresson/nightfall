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
#include <map>
#include <set>

#define MAXIMUM_PATH_CALCULATIONS 10000
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
		set<Dimension::Unit*>         doneUnits;
			
		int                lowestH, highestH;
		int                circumTracking;
		int                highestDistance, circumTracking_Flood;
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
		bool               hasBegunPathfinding;
		Dimension::Position           oldGoal;

		unsigned char**    squareTypes;
		int**              squareNums;
		unsigned char**    nodeTypes;
		int**              nodeNums;
		Uint16****         areaMaps;
		int                areaMapIndex;
		int                areaCode;
		int                unitSize;
		bool**             regenerateAreaCodes;
		bool**             changedSinceLastRegen;
		int**              numNotReached;
		Uint16*****        hConsts;
		int                hConstHeight, hConstWidth;
		unsigned char      xShift, yShift;
		int                cCount = 0, fCount = 0, tCount = 0, pCount = 0, numPaths = 0, numFailed = 0, notReachedFlood = 0, notReachedPath = 0, numGreatSuccess1 = 0, numGreatSuccess2 = 0;

		unsigned char NODE_TYPE_OPEN = 1, NODE_TYPE_CLOSED = 2;
		
		unsigned char SQUARE_TYPE_OPEN = 1, SQUARE_TYPE_CLOSED = 2, SQUARE_TYPE_BLOCKED = 3;

		struct node
		{
			int x, y;
			int f, g, h;
			int parent;
		};

		struct node *nodes;

		Uint32 *scores;
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

		bool IsWalkable_MType(int x, int y)
		{
			return Game::Dimension::MovementTypeCanWalkOnSquare_Pathfinding((Game::Dimension::MovementType) areaMapIndex, unitSize, x, y);
		}

		void InitAreaMap(int size, int mt)
		{
			int x, y;
			int time = SDL_GetTicks();

			unitSize = size;
			areaMapIndex = mt;

			if (areaMaps[size][mt] == NULL)
			{
				areaMaps[size][mt] = (Uint16**) malloc(height * sizeof(Uint16*));
				for (y = 0; y < height; y++)
				{
					areaMaps[size][mt][y] = (Uint16*) calloc(width, sizeof(Uint16));
				}
			}

			for (y = 0; y < height; y++)
			{
				memset(areaMaps[size][mt][y], 0, width * sizeof(Uint16));
			}
			areaCode = 1;
			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					if (IsWalkable_MType(x, y) && areaMaps[size][mt][y][x] == 0)
					{
						if (InitFloodfill(NULL, x, y, FLOODFILL_FLAG_SET_AREA_CODE) == PATHSTATE_OK)
						{
							while (FloodfillStep(NULL, NULL, 0, 0) == PATHSTATE_OK)
							{
								
							}
						}
						areaCode++;
						if (areaCode == 0)
						{
							areaCode = 1;
						}
					}
				}
			}
			regenerateAreaCodes[size][mt] = false;
			changedSinceLastRegen[size][mt] = false;
			printf("Area code regen %d ms\n", SDL_GetTicks() - time);
		}

		void InitAreaMaps()
		{
			for (int j = 0; j < 4; j++)
			{
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					if (regenerateAreaCodes[j][i] && changedSinceLastRegen[j][i] && Dimension::numUnitsPerAreaMap[j][i])
					{
						InitAreaMap(j, i);
					}
				}
			}
		}

		void DeleteUnitFromAreaMap(Dimension::Unit* pUnit)
		{
			int start_x, start_y, end_x, end_y;
			GetUnitUpperLeftCorner(pUnit, start_x, start_y);
			end_x = start_x + pUnit->type->widthOnMap - 1;
			end_y = start_y + pUnit->type->heightOnMap - 1;
			for (int j = 0; j < 4; j++)
			{
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					if (areaMaps[j][i])
					{
						int start_x_path = start_x - ((i-1) << 1) - 1;
						int start_y_path = start_y - ((i-1) << 1) - 1;
						int end_x_path = end_x + (i << 1) + 1;
						int end_y_path = end_y + (i << 1) + 1;
						int last_found_area_code = 0;
						int x, y;
						for (int k = 0; k < 2; k++)
						{
							y = k ? end_y_path : start_y_path;
							for (x = start_x_path; x <= end_x_path; x++)
							{
								if (x >= 0 && y >= 0 && x < width && y < height)
								{
									if (areaMaps[j][i][y][x] && areaMaps[j][i][y][x] != last_found_area_code)
									{
										if (last_found_area_code == 0)
										{
											last_found_area_code = areaMaps[j][i][y][x];
										}
										else
										{
											goto fail;
										}
									}
								}
							}
						}
						for (int k = 0; k < 2; k++)
						{
							x = k ? end_x_path : start_x_path;
							for (y = start_y_path+1; y < end_y_path; y++)
							{
								if (x >= 0 && y >= 0 && x < width && y < height)
								{
									if (areaMaps[j][i][y][x] && areaMaps[j][i][y][x] != last_found_area_code)
									{
										if (last_found_area_code == 0)
										{
											last_found_area_code = areaMaps[j][i][y][x];
										}
										else
										{
											goto fail;
										}
									}
								}
							}
						}

						if (!last_found_area_code)
						{
							goto fail;
						}

						for (y = start_y; y <= end_y; y++)
						{
							for (x = start_x; x <= end_x; x++)
							{
								areaMaps[j][i][y][x] = last_found_area_code;
							}
						}
						continue;

						fail:
						changedSinceLastRegen[j][i] = true;
						regenerateAreaCodes[j][i] = true;
					}
				}
			}
		}

		void AddUnitToAreaMap(Dimension::Unit* unit)
		{
			for (int j = 0; j < 4; j++)
			{
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					changedSinceLastRegen[j][i] = true;
				}
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

			squareTypes = (unsigned char**) malloc(height * sizeof(unsigned char*));
			squareNums = (int**) malloc(height * sizeof(int*));
			nodeTypes = (unsigned char**) malloc(height * sizeof(unsigned char*));
			nodeNums = (int**) malloc(height * sizeof(int*));
			for (y = 0; y < height; y++)
			{
				squareTypes[y] = (unsigned char*) calloc(width, sizeof(unsigned char));
				squareNums[y] = (int*) calloc(width, sizeof(int));
				nodeTypes[y] = (unsigned char*) calloc(width, sizeof(unsigned char));
				nodeNums[y] = (int*) calloc(width, sizeof(int));
			}
			numNotReached = (int**) malloc(4 * sizeof(int*));
			changedSinceLastRegen = (bool**) malloc(4 * sizeof(bool*));
			regenerateAreaCodes = (bool**) malloc(4 * sizeof(bool*));
			areaMaps = (Uint16****) malloc(4 * sizeof(Uint16***));
			for (int j = 0; j < 4; j++)
			{
				numNotReached[j] = (int*) malloc(Game::Dimension::MOVEMENT_TYPES_NUM * sizeof(int));
				changedSinceLastRegen[j] = (bool*) malloc(Game::Dimension::MOVEMENT_TYPES_NUM * sizeof(bool));
				regenerateAreaCodes[j] = (bool*) malloc(Game::Dimension::MOVEMENT_TYPES_NUM * sizeof(bool));
				areaMaps[j] = (Uint16***) malloc(Game::Dimension::MOVEMENT_TYPES_NUM * sizeof(Uint16**));
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					numNotReached[j][i] = 0;
					changedSinceLastRegen[j][i] = true;
					regenerateAreaCodes[j][i] = true;
					areaMaps[j][i] = NULL;
				}
			}
			nodes = (struct node*) malloc(width*height * sizeof(struct node));
			scores = (Uint32*) malloc(width*height * sizeof(Uint32));
			positions = (int*) malloc(width*height * sizeof(int));
			openList = (int*) malloc(width*height * sizeof(int));
			openListSize = width * height;

			scanlines = (struct scanline*) malloc(1024 * sizeof(struct scanline));
			scanlineArraySize = 1024;
			
			scanlineQueue = (int*) malloc((width + height) * sizeof(int));
			scanlineQueueSize = width + height;

			heap = binary_heap_create(scores, openList, positions);

			hConstWidth = width;
			hConstHeight = height;

			xShift = 0;
			yShift = 0;

			while (hConstWidth > 33)
			{
				xShift++;
				hConstWidth = (width >> xShift) + 1;
			}
			
			while (hConstHeight > 33)
			{
				yShift++;
				hConstHeight = (height >> yShift) + 1;
			}

			hConsts = (Uint16*****) malloc(4 * sizeof(Uint16****));
			for (int i = 0; i < 4; i++)
			{
				hConsts[i] = NULL;
			}
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

			if (goal_x < 0)
				goal_x = 0;

			if (goal_y < 0)
				goal_y = 0;

			if (goal_x >= width)
				goal_x = width-1;

			if (goal_y >= height)
				goal_y = height-1;

			SDL_LockMutex(gpmxPathfinding);

			IntThrState curState = pUnit->pMovementData->_currentState;

#ifdef DEBUG_AI_PATHFINDING
			std::cout << "State: " << curState << " | Length: " << gCalcQueue.size() << std::endl;
#endif

			IPResult res;
			MovementData* md = pUnit->pMovementData;

//			cout << "Command " << pUnit << " " << start_x << ", " << start_y << " " << goal_x << ", " << goal_y << " " << action <<  " " << target << " " << args << " " << currentFrame << endl;
			
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
				delete[] *goal;
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
				if (squareTypes[cur_y][cur_x] == SQUARE_TYPE_OPEN)
				{
					return 1;
				}
				if (squareTypes[cur_y][cur_x] == SQUARE_TYPE_CLOSED)
				{
					return 0;
				}
				if (areaMaps[unitSize][areaMapIndex][cur_y][cur_x] == areaMaps[unitSize][areaMapIndex][start_y][start_x])
				{
					if (IsWalkable(unit, cur_x, cur_y))
					{
						squareTypes[cur_y][cur_x] = SQUARE_TYPE_OPEN;
						return 1;
					}
				}
				squareTypes[cur_y][cur_x] = SQUARE_TYPE_CLOSED;
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

			SQUARE_TYPE_OPEN += 3;
			SQUARE_TYPE_CLOSED += 3;
			SQUARE_TYPE_BLOCKED += 3;
			if (SQUARE_TYPE_OPEN == 0 || SQUARE_TYPE_CLOSED == 0 || SQUARE_TYPE_BLOCKED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(squareTypes[y], 0, width * sizeof(unsigned char));
					memset(squareNums[y], 0, width * sizeof(int));
				}
				SQUARE_TYPE_OPEN = 1;
				SQUARE_TYPE_CLOSED = 2;
				SQUARE_TYPE_BLOCKED = 3;
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

			calculateNearestReachable = flags & FLOODFILL_FLAG_CALCULATE_NEAREST;
			setAreaCodes = flags & FLOODFILL_FLAG_SET_AREA_CODE;

			if (calculateNearestReachable)
			{
				if (!IsWalkable(unit, start_x, start_y))
				{
					return PATHSTATE_ERROR;
				}
			}
			else
			{
				if (!IsWalkable_MType(start_x, start_y))
				{
					return PATHSTATE_ERROR;
				}
			}

			firstScanlineIndex = 0;
			lastScanlineIndex = 0;
			numScanlines = 0;
			lowestDistance = 100000000;
			highestDistance = 0;
			circumTracking_Flood = 0;

			SQUARE_TYPE_OPEN += 3;
			SQUARE_TYPE_CLOSED += 3;
			SQUARE_TYPE_BLOCKED += 3;
			if (SQUARE_TYPE_OPEN == 0 || SQUARE_TYPE_CLOSED == 0 || SQUARE_TYPE_BLOCKED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(squareTypes[y], 0, width * sizeof(unsigned char));
					memset(squareNums[y], 0, width * sizeof(int));
				}
				SQUARE_TYPE_OPEN = 1;
				SQUARE_TYPE_CLOSED = 2;
				SQUARE_TYPE_BLOCKED = 3;
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
					squareTypes[start_y][x] = SQUARE_TYPE_BLOCKED;
				}

				squareTypes[start_y][x+1] = SQUARE_TYPE_CLOSED;
				squareNums[start_y][x+1] = 0;

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
					if (!IsWalkable_MType(x, start_y))
					{
						break;
					}
				}
				scanlines[0].start_x = x+1;

				if (x >= 0)
				{
					squareTypes[start_y][x] = SQUARE_TYPE_BLOCKED;
				}

				squareTypes[start_y][x+1] = SQUARE_TYPE_CLOSED;
				squareNums[start_y][x+1] = 0;

				for (x = start_x+1; ; x++)
				{
					if (!IsWalkable_MType(x, start_y))
					{
						break;
					}
				}
			}
			scanlines[0].end_x = x-1;
				
			if (x < width)
			{
				squareTypes[start_y][x] = SQUARE_TYPE_BLOCKED;
			}

			numScanlines = 1;

			nextFreeScanline = 1;

			scanlineQueue[0] = 0;

			return PATHSTATE_OK;
		}

		PathState FloodfillStep(Dimension::Unit* unit, MovementData* md, int target_x, int target_y)
		{
			int first_square;
			int start_x, end_x;
			int new_y;
			int new_start_x, new_end_x;
			int new_x;
			unsigned char *scanline_types;
			int *scanline_nums;
			int loop_start_y, loop_end_y;
			int loop_start_x, loop_end_x;
			int x, y;

			while (1)
			{

				if (numScanlines == 0)
				{
					// Target not found...
					if (calculateNearestReachable)
					{
						notReachedFlood++;
						if (changedSinceLastRegen[unitSize][areaMapIndex])
						{
							numNotReached[unitSize][areaMapIndex]++;
						}
					}
					return PATHSTATE_GOAL;
				}

				first_square = scanlineQueue[firstScanlineIndex];

				start_x = scanlines[first_square].start_x;
				end_x = scanlines[first_square].end_x;
				y = scanlines[first_square].y;

				firstScanlineIndex++;
				numScanlines--;
				
				if (firstScanlineIndex == scanlineQueueSize)
				{
					firstScanlineIndex = 0;
				}

				if (setAreaCodes)
				{
					for (x = start_x; x <= end_x; x++)
					{
						areaMaps[unitSize][areaMapIndex][y][x] = areaCode;
					}
				}

				if (calculateNearestReachable)
				{
					int new_distance;
					int t_x;
					if (target_x >= start_x && target_x <= end_x)
					{
						if (y < target_y)
						{
							circumTracking_Flood |= 1;
						}
						else
						{
							circumTracking_Flood |= 2;
						}
						new_distance = target_y - y;
						new_distance *= new_distance;
						t_x = target_x;
					}
					else if (target_x < start_x)
					{
						new_distance = (target_y - y) * (target_y - y) + (target_x - start_x) * (target_x - start_x);
						t_x = start_x;
						if (y == target_y)
						{
							circumTracking_Flood |= 4;
						}
					}
					else /* if (*target_x < start_x) */
					{
						new_distance = (target_y - y) * (target_y - y) + (target_x - end_x) * (target_x - end_x);
						t_x = end_x;
						if (y == target_y)
						{
							circumTracking_Flood |= 8;
						}
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
					if (circumTracking_Flood == 15)
					{
						if (new_distance > lowestDistance)
						{
//							cout << "Great success 2!" << endl;
							numGreatSuccess2++;
							continue;
						}
					}
					else
					{
						if (highestDistance < new_distance)
						{
							highestDistance = new_distance;
						}
					}
				}

				loop_start_y = y-1 >= 0 ? y-1 : 1;
				loop_end_y = y+1 < height ? y+1 : y-1;
				
				loop_start_x = start_x-1 >= 0 ? start_x-1 : 0;
				loop_end_x = end_x+1 < width ? end_x+1 : end_x;

				for (new_y = loop_start_y; new_y <= loop_end_y; new_y+=2)
				{
					scanline_types = squareTypes[new_y];
					scanline_nums = squareNums[new_y];
					for (x = loop_start_x; x <= loop_end_x; x++)
					{
						if (scanline_types[x] == SQUARE_TYPE_CLOSED)
						{
							x = scanlines[scanline_nums[x]].end_x+1;
						}
						else if (scanline_types[x] != SQUARE_TYPE_BLOCKED)
						{
							if ((calculateNearestReachable && IsWalkable(unit, x, new_y)) || (setAreaCodes && IsWalkable_MType(x, new_y)))
							{
								int new_scanline;
								if (setAreaCodes)
								{
									for (new_x = x-1; ; new_x--)
									{
										if (scanline_types[x] == SQUARE_TYPE_BLOCKED || !IsWalkable_MType(new_x, new_y))
										{
											break;
										}
									}
								}
								else
								{
									for (new_x = x-1; ; new_x--)
									{
										if (scanline_types[x] == SQUARE_TYPE_BLOCKED || !IsWalkable(unit, new_x, new_y))
										{
											break;
										}
									}
								}
								new_start_x = new_x+1;
							
								if (new_x >= 0)
								{
									scanline_types[new_x] = SQUARE_TYPE_BLOCKED;
								}

								if (scanline_types[new_start_x] != SQUARE_TYPE_CLOSED)
								{
									if (setAreaCodes)
									{
										for (new_x = x+1; ; new_x++)
										{
											if (scanline_types[x] == SQUARE_TYPE_BLOCKED || !IsWalkable_MType(new_x, new_y))
											{
												break;
											}
										}
									}
									else
									{
										for (new_x = x+1; ; new_x++)
										{
											if (scanline_types[x] == SQUARE_TYPE_BLOCKED || !IsWalkable(unit, new_x, new_y))
											{
												break;
											}
										}
									}
									new_end_x = new_x-1;
								
									if (new_x < width)
									{
										scanline_types[new_x] = SQUARE_TYPE_BLOCKED;
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
					
									if (scanline_types[new_start_x] == SQUARE_TYPE_CLOSED)
									{
										printf("FATAL - traversed same scanline twice!\n");
										md->_action.changedGoalPos.x = target_x;
										md->_action.changedGoalPos.y = target_y;
										return PATHSTATE_GOAL;
									}
					
									scanline_types[new_start_x] = SQUARE_TYPE_CLOSED;
									scanline_nums[new_start_x] = new_scanline;
									
									if (new_start_x > start_x)
									{
										squareTypes[y][new_start_x-1] = SQUARE_TYPE_CLOSED;
										squareNums[y][new_start_x-1] = first_square;
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
								scanline_types[x] = SQUARE_TYPE_BLOCKED;
							}
						}
					}
				}
				break;
			}
			return PATHSTATE_OK;
		}

		int CalcH(int new_x, int new_y, int target_x, int target_y)
		{
			int h_diagonal = min(abs(new_x-target_x), abs(new_y-target_y));
			int h_straight = (abs(new_x-target_x) + abs(new_y-target_y));
			Uint16 hConst = hConsts[unitSize][target_y>>yShift][target_x>>xShift][new_y>>yShift][new_x>>xShift];
			return ((30 * h_diagonal + 20 * (h_straight - 2*h_diagonal)) * hConst) >> 8;
/*			int diff_x = abs(new_x - target_x);
			int diff_y = abs(new_y - target_y);
			return (diff_x + diff_y) * 20;*/
		}

		int num_steps = 0;

		PathState InitPathfinding(ThreadData*& tdata)
		{
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;

			int start_x = (int) md->_action.startPos.x, start_y = (int) md->_action.startPos.y;
			int target_x = (int) md->_action.changedGoalPos.x, target_y = (int) md->_action.changedGoalPos.y;

			int y;

			if (hasBegunPathfinding)
			{
				if ((int) oldGoal.x == (int) md->_action.changedGoalPos.x && (int) oldGoal.y == (int) md->_action.changedGoalPos.y)
				{
					return PATHSTATE_OK;
				}
			}

			oldGoal = md->_action.changedGoalPos;

			hasBegunPathfinding = true;

			num_steps = 0;

			lowestH = 1<<30;
			highestH = 0;
			circumTracking = 0;
			nearestNode = -1;

			if (!IsWalkable(unit, start_x, start_y))
			{
				return PATHSTATE_ERROR;
			}

			NODE_TYPE_OPEN += 2;
			NODE_TYPE_CLOSED += 2;
			if (NODE_TYPE_OPEN == 0 || NODE_TYPE_CLOSED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(nodeTypes[y], 0, width * sizeof(unsigned char));
					memset(nodeNums[y], 0, width * sizeof(int));
				}
				NODE_TYPE_OPEN = 1;
				NODE_TYPE_CLOSED = 2;
			}

			nodes[0].x = start_x;
			nodes[0].y = start_y;
			nodes[0].g = 0;
			nodes[0].h = CalcH(start_x, start_y, target_x, target_y);
			nodes[0].f = 0;
			nodes[0].parent = -1;
			nodeNums[start_y][start_x] = 0;
			nodeTypes[start_y][start_x] = NODE_TYPE_OPEN;
			nextFreeNode = 1;
			binary_heap_pop_all(heap); // To be sure the heap is empty...
			binary_heap_push_item(heap, 0, 0);
			return PATHSTATE_OK;
		}

		int calcCount = 0;
		int fsteps = 0, psteps = 0;
		int tsteps = 0;

		PathState PathfindingStep(ThreadData*& tdata)
		{
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;
			int target_x = (int) md->_action.changedGoalPos.x, target_y = (int) md->_action.changedGoalPos.y;
			int x, y;
			
			int first_node;
			int node_x, node_y;
			unsigned char *scanline_types;
			int *scanline_nums;
			first_node = binary_heap_pop_item(heap, -1);

			num_steps++;

			if (first_node == -1)
			{
	/*			printf("Did not reach target\n"); */
				notReachedPath += psteps;
				if (changedSinceLastRegen[unitSize][areaMapIndex])
				{
					numNotReached[unitSize][areaMapIndex]++;
				}
				if (nearestNode != -1)
				{
					md->_action.changedGoalPos.x = nodes[nearestNode].x;
					md->_action.changedGoalPos.y = nodes[nearestNode].y;
					return PATHSTATE_GOAL;
				}
				else
				{
					// Happens when InitPathfinding() failed and this routine was called nonetheless
					return PATHSTATE_ERROR;
				}
			}

			if (lowestH > nodes[first_node].h || nearestNode == -1)
			{
				lowestH = nodes[first_node].h;
				nearestNode = first_node;
			}
			if (circumTracking != 15 && highestH < nodes[first_node].h)
			{
				highestH = nodes[first_node].h;
			}

			node_x = nodes[first_node].x;
			node_y = nodes[first_node].y;
			nodeTypes[node_y][node_x] = NODE_TYPE_CLOSED;

			if (node_x == target_x)
			{
				if (node_y < target_y)
				{
					circumTracking |= 1;
				}
				else
				{
					circumTracking |= 2;
				}
			}
			
			if (node_y == target_y)
			{
				if (node_x < target_x)
				{
					circumTracking |= 4;
				}
				else
				{
					circumTracking |= 8;
				}
			}

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

									node_num = scanline_nums[new_x];
									if (new_g < nodes[node_num].g)
									{
										continue;
									}

									new_g += Dimension::GetTraversalTimeAdjusted(unit, node_x, node_y, x, y);

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
										nodes[node_num].h = CalcH(new_x, new_y, target_x, target_y);
										if (circumTracking != 15 || nodes[node_num].h <= lowestH)
										{
											nodes[node_num].x = new_x;
											nodes[node_num].y = new_y;
											nodes[node_num].g = new_g;
											nodes[node_num].f = nodes[node_num].h + new_g;
											nodes[node_num].parent = first_node;
											scanline_types[new_x] = NODE_TYPE_OPEN;
											binary_heap_push_item(heap, node_num, (nodes[node_num].f<<16)+nodes[node_num].h);
										}
										else
										{
											numGreatSuccess1++;
//											cout << "Great success!" << endl;
											nextFreeNode--;
										}
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
			int hConstXTarget = nodes[nearestNode].x>>xShift;
			int hConstYTarget = nodes[nearestNode].y>>yShift;
			Node *prev_node = NULL, *first_node = NULL, *new_node = NULL;
			int num_nodes = 0, i;
			int cur_node = nearestNode;

			while (cur_node != -1)
			{
				num_nodes++;
				cur_node = nodes[cur_node].parent;
			}

			Node *new_nodes = new Node[num_nodes];

			i = 0;

			cur_node = nearestNode;
			while (cur_node != -1)
			{
				int hConstXNode = nodes[cur_node].x>>xShift;
				int hConstYNode = nodes[cur_node].y>>yShift;
				Uint16 old_hconst = hConsts[unitSize][hConstYTarget][hConstXTarget][hConstYNode][hConstXNode];
				Uint16 new_hconst = 0;
				Uint16 mixed_hconst;
				if (nodes[cur_node].h && nodes[nearestNode].g - nodes[cur_node].g)
				{
					new_hconst = ((nodes[nearestNode].g - nodes[cur_node].g << 8) / nodes[cur_node].h);
					mixed_hconst = (old_hconst * 15 + new_hconst)>>4;
					if (old_hconst == mixed_hconst && new_hconst != mixed_hconst)
					{
						mixed_hconst += new_hconst < old_hconst ? -1 : 1;
					}
					hConsts[unitSize][hConstYTarget][hConstXTarget][hConstYNode][hConstXNode] = mixed_hconst;
				}

				new_node = &new_nodes[i++];
				new_node->x = nodes[cur_node].x;
				new_node->y = nodes[cur_node].y;
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
			nearestNode = -1;
		}
		
		inline void ParsePopQueueReason(ThreadData*& tdata, MovementData* md)
		{
			cCount += calcCount;
			tCount += tsteps;
			fCount += fsteps;
			pCount += psteps;
			numPaths++;

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
					unitSize = tdata->pUnit->type->heightOnMap-1;
					areaMapIndex = tdata->pUnit->type->movementType;

					if (numNotReached[unitSize][areaMapIndex] > 5)
					{
						regenerateAreaCodes[unitSize][areaMapIndex] = true;
						numNotReached[unitSize][areaMapIndex] = 0;
					}
					
					InitAreaMaps();

					unitSize = tdata->pUnit->type->heightOnMap-1;
					areaMapIndex = tdata->pUnit->type->movementType;
					calcCount = 0;
					fsteps = 0;
					psteps = 0;
					tsteps = 0;

					hasBegunPathfinding = false;
					
					if (!hConsts[unitSize])
					{
						hConsts[unitSize] = (Uint16****) malloc(hConstHeight * sizeof(Uint16***));
						for (int y1 = 0; y1 < hConstHeight; y1++)
						{
							hConsts[unitSize][y1] = (Uint16***) malloc(hConstWidth * sizeof(Uint16**));
							for (int x1 = 0; x1 < hConstWidth; x1++)
							{
								hConsts[unitSize][y1][x1] = (Uint16**) malloc(hConstHeight * sizeof(Uint16*));
								for (int y2 = 0; y2 < hConstHeight; y2++)
								{
									hConsts[unitSize][y1][x1][y2] = (Uint16*) malloc(hConstHeight * sizeof(Uint16));
									for (int x2 = 0; x2 < hConstWidth; x2++)
									{
										hConsts[unitSize][y1][x1][y2][x2] = 0x100;
									}
								}
							}
						}
					}

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
						if (IsWalkable(unit, (int)md->_action.goal.pos.x, (int)md->_action.goal.pos.y) && areaMaps[unitSize][areaMapIndex][(int)md->_action.startPos.y][(int)md->_action.startPos.x] == areaMaps[unitSize][areaMapIndex][(int)md->_action.goal.pos.y][(int)md->_action.goal.pos.x])
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
							notReachedPath += psteps;
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
			
			int ret = ERROR_GENERAL;
			if (md->_popFromQueue)
			{
//				printf("end\n");
				ParsePopQueueReason(tdata, md);
				ret = SUCCESS;
			}
			else
			{
				cCount += calcCount;
				tCount += tsteps;
				fCount += fsteps;
				pCount += psteps;
				numPaths++;

				if (state == PATHSTATE_GOAL || nearestNode != -1)
				{
					BuildNodeLinkedList(tdata);
					md->calcState = CALCSTATE_REACHED_GOAL;
					ret = SUCCESS;
				}
				else
				{
					numFailed++;
					DeallocPathfinding(tdata);

					md->calcState = CALCSTATE_FAILURE;
				}
				doneUnits.insert(unit);
			}

			
			tdata->pUnit = NULL;
			
			SDL_UnlockMutex(tdata->pMutex);
			
			return ret;
		}

		int GetQueueSize()
		{
			int ret;
			SDL_LockMutex(gpmxPathfinding);
			ret = gCalcQueue.size();
			SDL_UnlockMutex(gpmxPathfinding);
			return ret;
		}
	}
}
