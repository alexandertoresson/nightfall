/*
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
 * situations where it would not find any road to the goal. If A*
 * finds itself in that situation, it searches the full map.
 * Firstly, maps of so-called 'area codes' are used, where every
 * area code map is specific to a movement type and a size of the
 * unit, and these are calculated on the fly when needed. 
 * If two positions in the map have the same area code, you can
 * go from one of the squares to the other, otherwise not. Also,
 * if the target square is currently blocked it is impossible to
 * reach the other square (at least when action is ACTION_GOTO).
 *
 * When it thinks that it is impossible to reach the target square,
 * it uses a special 'tracing' algorithm that traces all squares around
 * the area that can not be reached, and the new goal is set to the
 * nearest square of those traced that is nearest to the goal.
 * 
 * If it originally determined that it could reach the target square,
 * but it after RECALC_TRACE_LIMIT steps hasn't, it will try a new trace
 * if possible.
 *
 * If it hasn't reached the target square after RECALC_FLOODFILL_LIMIT
 * steps, it will run a special floodfill algorithm starting from the
 * starting square, and setting the goal to the square nearest to the
 * old goal that was reached.
 */

#include "aipathfinding.h"

#include "game.h"
#include "dimension.h"
#include "ainode.h"
#include "unit.h"
#include "unitsquares.h"
#include "networking.h"
#define BINARY_HEAP_DATATYPE int
#include "binaryheap.h"
#include <map>
#include <set>
#include <cassert>
#include <cmath>
#include <iostream>

#define MAXIMUM_PATH_CALCULATIONS 10000
#define RECALC_TRACE_LIMIT 1000
#define RECALC_FLOODFILL_LIMIT 4000

using namespace std;

namespace Game
{
	namespace AI
	{
		std::queue< Dimension::Unit* > gCalcQueue;
		SDL_mutex* gpmxQueue;
		SDL_mutex* gpmxCommand;
		SDL_mutex* gpmxDone;
		SDL_mutex* gpmxThreadState;
		SDL_mutex* gpmxAreaMap;
		SDL_mutex* gpmxHConst;
		set<Dimension::Unit*> doneUnits;

		int numPathfindingThreads = 2;

		ThreadData**         pThreadDatas;
		volatile Uint16****  areaMaps;
		volatile bool**      regenerateAreaCodes;
		volatile bool**      changedSinceLastRegen;
		volatile int**       numNotReached;
		volatile Uint16***** hConsts;
		int                hConstHeight, hConstWidth;
		unsigned char      xShift, yShift;
		volatile int       cCount = 0, fCount = 0, tCount = 0, pCount = 0, numPaths = 0, numFailed = 0, notReachedFlood = 0, notReachedPath = 0, numGreatSuccess1 = 0, numGreatSuccess2 = 0, numTotalFrames = 0;

		struct node
		{
			int x, y;
			int f, g, h;
			int parent;
		};


		struct scanline
		{
			int start_x, end_x;
			int y;
		};

		int width, height;

		int  _ThreadMethod(void* arg);

		struct ThreadData
		{
			int         threadIndex;
			SDL_Thread* pThread;
			SDL_mutex*  pMutex;
			bool        threadRuntime;
			
			Dimension::Unit*  pUnit;

			Dimension::IntPosition oldGoal;

			int                nearestNode;
			binary_heap_t*     heap;
			bool               calculateNearestReachable;
			bool               setAreaCodes;
			PreprocessState    preprocessState;
			bool               hasBegunPathfinding;

			// All

			int areaMapIndex;
			int unitSize;

			// Trace
			
			int traceCurX, traceCurY;
			int traceStartX, traceStartY;
			int traceStage;
			int traceLastDir, traceFirstDir;

			int lowestDistance;
			
			// Trace & Floodfill
			
			unsigned char SQUARE_TYPE_OPEN, SQUARE_TYPE_CLOSED, SQUARE_TYPE_BLOCKED;

			unsigned char**    squareTypes;
			int**              squareNums;
			
			// Floodfill
			
			int highestDistance, circumTracking_Flood;

			int numScanlines;
			int nextFreeScanline;
			int firstScanlineIndex;
			int lastScanlineIndex;

			struct scanline *scanlines;
			int scanlineArraySize;

			int *scanlineQueue;
			int scanlineQueueSize;

			Uint16 areaCode;

			// Pathfinding

			unsigned char NODE_TYPE_OPEN, NODE_TYPE_CLOSED;
			
			unsigned char**    nodeTypes;
			int**              nodeNums;

			int lowestH;
			int circumTracking;

			struct node *nodes;

			Uint32 *scores;
			int *positions;

			int *openList, openListSize, nextFreeNode;

			ThreadData(int index)
			{
				this->threadIndex = index;
				this->threadRuntime = true;
				this->pMutex = SDL_CreateMutex();
				this->pUnit = NULL;
				this->pThread = SDL_CreateThread(Game::AI::_ThreadMethod, (void*)this);

				this->squareTypes = (unsigned char**) malloc(height * sizeof(unsigned char*));
				this->squareNums = (int**) malloc(height * sizeof(int*));
				this->nodeTypes = (unsigned char**) malloc(height * sizeof(unsigned char*));
				this->nodeNums = (int**) malloc(height * sizeof(int*));
				for (int y = 0; y < height; y++)
				{
					this->squareTypes[y] = (unsigned char*) calloc(width, sizeof(unsigned char));
					this->squareNums[y] = (int*) calloc(width, sizeof(int));
					this->nodeTypes[y] = (unsigned char*) calloc(width, sizeof(unsigned char));
					this->nodeNums[y] = (int*) calloc(width, sizeof(int));
				}
				if (MAXIMUM_PATH_CALCULATIONS < width*height)
				{
					this->openListSize = MAXIMUM_PATH_CALCULATIONS;
				}
				else
				{
					this->openListSize = width * height;
				}
				this->nodes = new node[this->openListSize];
				this->scores = new Uint32[this->openListSize];
				this->positions = new int[this->openListSize];
				this->openList = new int[this->openListSize];

				this->scanlines = (scanline*) malloc(sizeof(scanline) * 1024);
				this->scanlineArraySize = 1024;
				
				this->scanlineQueue = new int[width + height];
				this->scanlineQueueSize = width + height;

				this->heap = binary_heap_create(this->scores, this->openList, this->positions);

				this->SQUARE_TYPE_OPEN = 1;
				this->SQUARE_TYPE_CLOSED = 2;
				this->SQUARE_TYPE_BLOCKED = 3;
				
				this->NODE_TYPE_OPEN = 1;
				this->NODE_TYPE_CLOSED = 2;
			}

			~ThreadData()
			{
				SDL_DestroyMutex(this->pMutex);

				for (int y = 0; y < height; y++)
				{
					free(this->squareTypes[y]);
					free(this->squareNums[y]);
					free(this->nodeTypes[y]);
					free(this->nodeNums[y]);
				}
				free(this->squareTypes);
				free(this->squareNums);
				free(this->nodeTypes);
				free(this->nodeNums);

				delete[] this->nodes;
				delete[] this->scores;
				delete[] this->positions;
				delete[] this->openList;

				free(this->scanlines);
				
				delete[] this->scanlineQueue;

				binary_heap_destroy(this->heap);

			}
		};

#define FLOODFILL_FLAG_CALCULATE_NEAREST 1
#define FLOODFILL_FLAG_SET_AREA_CODE 2

		PathState InitFloodfill(ThreadData* tdata, int start_x, int start_y, int flags);
		PathState FloodfillStep(ThreadData* tdata, int target_x, int target_y);

		bool IsWalkable_MType(int x, int y, int areaMapIndex, int unitSize)
		{
			return Game::Dimension::MovementTypeCanWalkOnSquare_Pathfinding((Game::Dimension::MovementType) areaMapIndex, unitSize, x, y);
		}

		void InitAreaMap(ThreadData *tdata, int size, int mt)
		{
			int x, y;
			int time = SDL_GetTicks();

			SDL_LockMutex(gpmxAreaMap);

			tdata->unitSize = size;
			tdata->areaMapIndex = mt;

			if (areaMaps[size][mt] == NULL)
			{
				areaMaps[size][mt] = new volatile Uint16*[height];
				for (y = 0; y < height; y++)
				{
					areaMaps[size][mt][y] = new Uint16[width];
				}
			}

			for (y = 0; y < height; y++)
			{
				memset((void*)areaMaps[size][mt][y], 0, width * sizeof(Uint16));
			}
			tdata->areaCode = 1;
			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					if (areaMaps[size][mt][y][x] == 0 && IsWalkable_MType(x, y, mt, size))
					{
						if (InitFloodfill(tdata, x, y, FLOODFILL_FLAG_SET_AREA_CODE) == PATHSTATE_OK)
						{
							while (FloodfillStep(tdata, 0, 0) == PATHSTATE_OK)
							{
								
							}
						}
						tdata->areaCode++;
						if (tdata->areaCode == 0)
						{
							tdata->areaCode = 1;
						}
					}
				}
			}
			regenerateAreaCodes[size][mt] = false;
			changedSinceLastRegen[size][mt] = false;

			SDL_UnlockMutex(gpmxAreaMap);

			printf("Area code regen %d ms\n", SDL_GetTicks() - time);
		}

		void InitAreaMaps(ThreadData* tdata)
		{
			for (int j = 0; j < 4; j++)
			{
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					if (regenerateAreaCodes[j][i] && changedSinceLastRegen[j][i] && Dimension::numUnitsPerAreaMap[j][i])
					{
						InitAreaMap(tdata, j, i);
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
			SDL_LockMutex(gpmxAreaMap);
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
						Uint16 last_found_area_code = 0;
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
			SDL_UnlockMutex(gpmxAreaMap);
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
			width = Game::Dimension::pWorld->width;
			height = Game::Dimension::pWorld->width;
			if (pThreadDatas != NULL)
			{
				return;
			}
			
			pThreadDatas = new ThreadData*[numPathfindingThreads];
			
			gpmxQueue = SDL_CreateMutex();
			gpmxCommand = SDL_CreateMutex();
			gpmxDone = SDL_CreateMutex();
			gpmxThreadState = SDL_CreateMutex();
			gpmxAreaMap = SDL_CreateMutex();
			gpmxHConst = SDL_CreateMutex();

			numNotReached = new volatile int*[4];
			changedSinceLastRegen = new volatile bool*[4];
			regenerateAreaCodes = new volatile bool*[4];
			areaMaps = new volatile Uint16***[4];
			for (int j = 0; j < 4; j++)
			{
				numNotReached[j] = new volatile int[Game::Dimension::MOVEMENT_TYPES_NUM];;
				changedSinceLastRegen[j] = new volatile bool[Game::Dimension::MOVEMENT_TYPES_NUM];
				regenerateAreaCodes[j] = new volatile bool[Game::Dimension::MOVEMENT_TYPES_NUM];
				areaMaps[j] = new volatile Uint16**[Game::Dimension::MOVEMENT_TYPES_NUM];
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					numNotReached[j][i] = 0;
					changedSinceLastRegen[j][i] = true;
					regenerateAreaCodes[j][i] = true;
					areaMaps[j][i] = NULL;
				}
			}

			for (int i = 0; i < numPathfindingThreads; i++)
			{
				pThreadDatas[i] = new ThreadData(i);
			}

			hConstWidth = width;
			hConstHeight = height;

			xShift = 0;
			yShift = 0;

			while (hConstWidth > 25)
			{
				xShift++;
				hConstWidth = (width >> xShift) + 1;
			}
			
			while (hConstHeight > 25)
			{
				yShift++;
				hConstHeight = (height >> yShift) + 1;
			}

			hConsts = new volatile Uint16****[4];
			for (int i = 0; i < 4; i++)
			{
				hConsts[i] = NULL;
			}

		}
		
		void QuitPathfindingThreading(void)
		{
			if (pThreadDatas == NULL)
			{
				return;
			}
		
			for (int i = 0; i < numPathfindingThreads; i++)
			{
				ThreadData *tdata = pThreadDatas[i];
				tdata->threadRuntime = false;
				
				SDL_WaitThread(tdata->pThread, NULL);
				
			}
			
			if (gpmxQueue != NULL)
			{
				SDL_DestroyMutex(gpmxQueue);
				gpmxQueue = NULL;
			}
			
			for (int j = 0; j < 4; j++)
			{
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					if (areaMaps[j][i])
					{
						for (int y = 0; y < height; y++)
						{
							delete[] areaMaps[j][i][y];
						}
						delete areaMaps[j][i];
					}
				}

				delete[] numNotReached[j];
				delete[] changedSinceLastRegen[j];
				delete[] regenerateAreaCodes[j];
				delete[] areaMaps[j];
			}
			delete[] numNotReached;
			delete[] changedSinceLastRegen;
			delete[] regenerateAreaCodes;
			delete[] areaMaps;

			delete[] pThreadDatas;
			pThreadDatas = NULL;
		}
		
		void PausePathfinding()
		{
			for (int i = 0; i < numPathfindingThreads; i++)
			{
				SDL_LockMutex(pThreadDatas[i]->pMutex);
			}
		}
		
		void ResumePathfinding()
		{
			for (int i = 0; i < numPathfindingThreads; i++)
			{
				SDL_UnlockMutex(pThreadDatas[i]->pMutex);
			}
		}
		
		int PausePathfinding(Dimension::Unit* unit)
		{
			SDL_LockMutex(gpmxCommand);
			SDL_LockMutex(gpmxQueue);
			int thread = unit->pMovementData->_associatedThread;
			SDL_UnlockMutex(gpmxCommand);
			SDL_UnlockMutex(gpmxQueue);
			if (thread != -1)
				SDL_LockMutex(pThreadDatas[thread]->pMutex);
			return thread;
		}
		
		void ResumePathfinding(int thread)
		{
			if (thread != -1)
				SDL_UnlockMutex(pThreadDatas[thread]->pMutex);
		}
		
		int _ThreadMethod(void* arg)
		{
			ThreadData* tdata = (ThreadData*)arg;
			while (tdata->threadRuntime == true)
			{
				for (int i = 0; i < 200; i++)
				{
					if (PerformPathfinding(tdata) == PATHSTATE_EMPTY_QUEUE)
					{
						SDL_Delay(10);
						continue;
					}
				}
				
				SDL_Delay(10);
			}

			delete tdata;
			tdata = NULL;

			return SUCCESS;
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
			md->action.action = ACTION_NONE;
			
			md->_currentState = INTTHRSTATE_NONE;
			md->_popFromQueue = false;
			md->_associatedThread = -1;
			md->_newCommandWhileUnApplied = false;

			md->_start = NULL;
			md->_goal = NULL;
			md->_action.startPos.x = 0;
			md->_action.startPos.y = 0;
			md->_action.goal.pos.x = 0;
			md->_action.goal.pos.y = 0;
			md->_action.goal.unit = NULL;
			md->_action.changedGoalPos.x = 0;
			md->_action.changedGoalPos.y = 0;
			md->_action.goal.goal_id = 0xFFFF;
			md->_action.arg = NULL;
			md->_action.action = ACTION_NONE;
			
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
			md->_newAction.action = ACTION_NONE;
			
#ifdef DEBUG_AI_PATHFINDING
			std::cout << "Movement data init: " << pUnit << std::endl;
			md->_cycles = 0;
#endif
		}

		IPResult CommandPathfinding(Dimension::Unit* pUnit, int start_x, int start_y, int goal_x, int goal_y, AI::UnitAction action, Dimension::Unit* target, void* args)
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

			SDL_LockMutex(gpmxCommand);

			if (action == ACTION_ATTACK || action == ACTION_FOLLOW || action == ACTION_MOVE_ATTACK_UNIT)
			{
				if (!Dimension::IsValidUnitPointer(target))
				{
					cout << "Invalid unit target, fixing up action" << std::endl;
					action = ACTION_GOTO;
				}
			}

			if (action == ACTION_BUILD)
			{
				if (!target && !args)
				{
					cout << "Invalid unit target and arg, fixing up action" << std::endl;
					action = ACTION_GOTO;
				}
			}
			
			if (action == ACTION_RESEARCH)
			{
				if (!args)
				{
					cout << "Invalid action arg, fixing up action" << std::endl;
					action = ACTION_GOTO;
				}
			}

			if (action == ACTION_NONE)
			{
				std::cout << "Invalid action; has action == none." << std::endl;
				return IPR_SUCCESS;
			}

#ifdef DEBUG_AI_PATHFINDING
			SDL_LockMutex(gpmxQueue);
			std::cout << "State: " << curState << " | Length: " << gCalcQueue.size() << std::endl;
			SDL_UnlockMutex(gpmxQueue);
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

			IntThrState curState = pUnit->pMovementData->_currentState;

			if (curState == INTTHRSTATE_NONE)
			{
				if (md->_start != NULL)
					DeallocPathfindingNodes(pUnit, DPN_BACK);

				md->_action = md->_newAction;
			       
				md->calcState = CALCSTATE_WORKING;
				md->changedGoal = false;
#ifdef DEBUG_AI_PATHFINDING
				md->_cycles = 0;
#endif

//				cout << "NoLock " << pUnit->id << endl;

				md->_pathfindingStartingFrame = currentFrame;

				pUnit->pMovementData->_currentState = INTTHRSTATE_WAITING;
				
				SDL_LockMutex(gpmxQueue);
				gCalcQueue.push(pUnit);
				SDL_UnlockMutex(gpmxQueue);
				
				res = IPR_SUCCESS;
			}
			else
			{
				SDL_LockMutex(gpmxThreadState);

				curState = pUnit->pMovementData->_currentState;

				if (curState == INTTHRSTATE_PROCESSING)
				{
					md->_popFromQueue = true;
					md->_reason = POP_NEW_GOAL;
//					cout << "PopNewGoal " << pUnit->id << endl;
					
					res = IPR_SUCCESS_POPCMD_ISSUED;
				}
				else if (curState == INTTHRSTATE_UNAPPLIED)
				{
					md->_newCommandWhileUnApplied = true;
//					cout << "CommandWhileUnapplied " << pUnit->id << endl;

					res = IPR_SUCCESS;
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
						md->_pathfindingStartingFrame = currentFrame;

						pUnit->pMovementData->_currentState = INTTHRSTATE_WAITING;
						
						SDL_LockMutex(gpmxQueue);
						gCalcQueue.push(pUnit);
						SDL_UnlockMutex(gpmxQueue);

//						cout << "Lock " << pUnit->id << endl;
					}
					else
					{
//						cout << "NewGoal " << pUnit->id << endl;
					}
					
					res = IPR_SUCCESS;
				}
				SDL_UnlockMutex(gpmxThreadState);
			}

			SDL_UnlockMutex(gpmxCommand);

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
				
//			cout << "Apply " << unit->id << endl;

			SDL_LockMutex(gpmxCommand);
			
			if (md->_popFromQueue)
			{
				if (md->_reason == POP_CANCELLED || md->_reason == POP_DELETED)
				{
					md->_popFromQueue = false;
					SDL_UnlockMutex(gpmxCommand);
					return false;
				}
			}

			if (md->_action.action == ACTION_ATTACK || md->_action.action == ACTION_FOLLOW || md->_action.action == ACTION_MOVE_ATTACK_UNIT)
			{
				if (!Dimension::IsValidUnitPointer(md->_action.goal.unit))
				{
					std::cout << "Invalid action, needs target. Fixing up action." << std::endl;
					md->_action.action = ACTION_GOTO;
//					*(int*) 0 = 0;
				}
			}

			if (md->_action.action == ACTION_BUILD)
			{
				if (!md->_action.goal.unit && !md->_action.arg)
				{
					std::cout << "Invalid action, needs target or arg. Fixing up action." << std::endl;
					md->_action.action = ACTION_GOTO;
//					*(int*) 0 = 0;
				}
			}
			
			if (md->_action.action == ACTION_RESEARCH)
			{
				if (!md->_action.arg)
				{
					std::cout << "Invalid action, needs arg. Fixing up action." << std::endl;
					md->_action.action = ACTION_GOTO;
//					*(int*) 0 = 0;
				}
			}
			
			if (md->_action.action == ACTION_NONE)
			{
				std::cout << "Invalid action; path with no action calculated." << std::endl;
//				*(int*) 0 = 0;
			}

			md->pStart = md->_start;
			md->pGoal  = md->_goal;

			md->action = md->_action;

			md->_start = NULL;
			md->_goal = NULL;
			
			md->_action.startPos.x = 0;
			md->_action.startPos.y = 0;
			md->_action.action = ACTION_NONE;
			md->_action.goal.pos.x = 0;
			md->_action.goal.pos.y = 0;
			md->_action.goal.unit  = NULL;
			md->_action.changedGoalPos.x = 0;
			md->_action.changedGoalPos.y = 0;
			md->_action.arg        = NULL;

			SDL_UnlockMutex(gpmxCommand);

			return true;
		}

		void ApplyUnappliedCommandIfAny(Dimension::Unit* unit)
		{
			MovementData* md = unit->pMovementData;
			
			SDL_LockMutex(gpmxCommand);
			
			SDL_LockMutex(gpmxThreadState);

			if (md->_newCommandWhileUnApplied)
			{
//				cout << "Unapplied " << unit->id << endl;
				md->_action = md->_newAction;

				md->calcState = CALCSTATE_WORKING;
				md->changedGoal = false;
				md->_newCommandWhileUnApplied = false;

				md->_pathfindingStartingFrame = currentFrame;

				unit->pMovementData->_currentState = INTTHRSTATE_WAITING;
				
				SDL_LockMutex(gpmxQueue);
				gCalcQueue.push(unit);
				SDL_UnlockMutex(gpmxQueue);

			}
			else
			{
//				cout << "none2 " << unit->id << endl;
				md->_associatedThread = -1;
				md->_currentState = INTTHRSTATE_NONE;
			}
			SDL_UnlockMutex(gpmxThreadState);

			SDL_UnlockMutex(gpmxCommand);
		}

		void ApplyAllNewPaths()
		{
			SDL_LockMutex(gpmxDone);

			for (set<Dimension::Unit*>::iterator it = doneUnits.begin(); it != doneUnits.end(); it++)
			{
				Dimension::Unit* pUnit = *it;
				if (!Dimension::IsValidUnitPointer(pUnit) || pUnit->pMovementData->action.action == ACTION_DIE)
				{
					continue;
				}
				PathState state = GetInternalPathState(pUnit);
				
				if (!Networking::isNetworked)
				{
					if (state == PATHSTATE_GOAL)
					{
						ApplyNewPath(pUnit);
						ApplyUnappliedCommandIfAny(pUnit);
						pUnit->pMovementData->pCurGoalNode = NULL;
					}
					else if (state == PATHSTATE_ERROR)
					{
						CancelAction(pUnit);
						pUnit->pMovementData->pCurGoalNode = NULL;
					}
				}
				else
				{
					if (state == PATHSTATE_GOAL)
					{
						Networking::PreparePath(pUnit, pUnit->pMovementData->_start, pUnit->pMovementData->_goal);
						ApplyUnappliedCommandIfAny(pUnit);
					}
					else if (state == PATHSTATE_ERROR)
					{
						IssueNextAction(pUnit);
					}
				}
			}

			doneUnits.clear();

			SDL_UnlockMutex(gpmxDone);

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

			unit->pMovementData->_popFromQueue = true;
			unit->pMovementData->_reason = POP_DELETED;
		}

		void CancelUndergoingProc(Dimension::Unit* unit)
		{
			if (unit == NULL)
				return;

			if (unit->pMovementData == NULL)
				return;

			SDL_LockMutex(gpmxCommand);
			unit->pMovementData->_popFromQueue = true;
			unit->pMovementData->_reason = POP_CANCELLED;
			SDL_UnlockMutex(gpmxCommand);

		}

		void DequeueNewPath(Dimension::Unit* unit)
		{
			if (unit == NULL)
				return;

			if (unit->pMovementData == NULL)
				return;

			SDL_LockMutex(gpmxCommand);

			if (unit->pMovementData->_popFromQueue && unit->pMovementData->_reason == POP_NEW_GOAL)
			{
				unit->pMovementData->_popFromQueue = false;
			}
			
			if (unit->pMovementData->_newCommandWhileUnApplied)
			{
				unit->pMovementData->_newCommandWhileUnApplied = false;
			}
			SDL_UnlockMutex(gpmxCommand);
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
					md->pCurGoalNode = NULL;
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
				delete[] *goal; // The nodes in a path are allocated as an array
			}

			*start = NULL;
			*goal  = NULL;	
		}
		
		inline bool IsWalkable(Dimension::Unit* unit, int x, int y)
		{
			return Dimension::SquaresAreWalkable(unit, x, y, Dimension::SIW_IGNORE_MOVING);
		}

		inline int SameAreaAndWalkable(ThreadData* tdata, int start_x, int start_y, int cur_x, int cur_y)
		{
			if (cur_x > 0 && cur_y > 0 && cur_x < width && cur_y < height)
			{
				if (tdata->squareTypes[cur_y][cur_x] == tdata->SQUARE_TYPE_OPEN)
				{
					return 1;
				}
				if (tdata->squareTypes[cur_y][cur_x] == tdata->SQUARE_TYPE_CLOSED)
				{
					return 0;
				}
				if (areaMaps[tdata->unitSize][tdata->areaMapIndex][cur_y][cur_x] == areaMaps[tdata->unitSize][tdata->areaMapIndex][start_y][start_x])
				{
					if (IsWalkable(tdata->pUnit, cur_x, cur_y))
					{
						tdata->squareTypes[cur_y][cur_x] = tdata->SQUARE_TYPE_OPEN;
						return 1;
					}
				}
				tdata->squareTypes[cur_y][cur_x] = tdata->SQUARE_TYPE_CLOSED;
			}
			return 0;
		}

		int traceDirs[8][2] = {{-1,  0},
		                       {-1, -1},
		                       { 0, -1},
		                       { 1, -1},
		                       { 1,  0},
		                       { 1,  1},
		                       { 0,  1},
		                       {-1,  1}};

		bool InitTrace(ThreadData* tdata, int start_x, int start_y, int target_x, int target_y)
		{
			int y;
			tdata->traceCurX = target_x;
			tdata->traceCurY = target_y;

			tdata->SQUARE_TYPE_OPEN += 3;
			tdata->SQUARE_TYPE_CLOSED += 3;
			tdata->SQUARE_TYPE_BLOCKED += 3;
			if (tdata->SQUARE_TYPE_OPEN == 0 || tdata->SQUARE_TYPE_CLOSED == 0 || tdata->SQUARE_TYPE_BLOCKED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(tdata->squareTypes[y], 0, width * sizeof(unsigned char));
					memset(tdata->squareNums[y], 0, width * sizeof(int));
				}
				tdata->SQUARE_TYPE_OPEN = 1;
				tdata->SQUARE_TYPE_CLOSED = 2;
				tdata->SQUARE_TYPE_BLOCKED = 3;
			}

			if (SameAreaAndWalkable(tdata, start_x, start_y, tdata->traceCurX, tdata->traceCurY))
			{
				return PATHSTATE_ERROR;
			}

			tdata->traceStage = 0;
			tdata->lowestDistance = 100000000;

			return PATHSTATE_OK;
		}

		int TraceStep(ThreadData* tdata, int start_x, int start_y, int target_x, int target_y)
		{
			int new_distance;
			int i, j = 0;
			int old_x, old_y;
			int traceCurX = tdata->traceCurX, traceCurY = tdata->traceCurY;

			if (tdata->traceStage == 0)
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

					if (SameAreaAndWalkable(tdata, start_x, start_y, traceCurX, traceCurY))
					{
						if (SameAreaAndWalkable(tdata, start_x, start_y, traceCurX, old_y))
						{
							traceCurY = old_y;
						}

						for (i = 0; i < 8; i++)
						{
							if (traceDirs[i][0] == traceCurX - old_x && traceDirs[i][1] == traceCurY - old_y)
							{
								tdata->traceFirstDir = i;
								break;
							}
						}

						for (i = 0; i < 8; i++)
						{
							int dir = (tdata->traceFirstDir-i)&7;
							if (SameAreaAndWalkable(tdata, start_x, start_y, traceCurX-traceDirs[dir][0], traceCurY-traceDirs[dir][1]))
							{
								tdata->traceFirstDir = tdata->traceLastDir = dir;

								break;
							}
						}

						tdata->traceStartX = traceCurX;
						tdata->traceStartY = traceCurY;

						tdata->traceStage = 1;

						break;

					}
					if (start_x == traceCurX && start_y == traceCurY)
					{
						return PATHSTATE_GOAL;
					}
				}

			}

			if (tdata->traceStage == 1)
			{
				int traceStartX = tdata->traceStartX, traceStartY = tdata->traceStartY;
				int traceFirstDir = tdata->traceFirstDir, traceLastDir = tdata->traceLastDir;

				for ( ; j < 10; j++)
				{
					int new_dir = -1;
					int first_dir_to_check = ((traceLastDir&6)-1)&7;
					int num_dirs_to_check = 6+(traceLastDir&1);
					new_distance = (target_y - traceCurY) * (target_y - traceCurY) + (target_x - traceCurX) * (target_x - traceCurX);
					if (new_distance < tdata->lowestDistance)
					{
						tdata->lowestDistance = new_distance;
						tdata->pUnit->pMovementData->_action.changedGoalPos.x = traceCurX;
						tdata->pUnit->pMovementData->_action.changedGoalPos.y = traceCurY;
						if (new_distance == 0)
						{
							return PATHSTATE_GOAL;
						}
					}
			//		printf("%d, %d\n", traceCurX, traceCurY);
					for (i = 0; i < num_dirs_to_check; i++)
					{
						int dir = (first_dir_to_check+i)&7;
						if (SameAreaAndWalkable(tdata, start_x, start_y, traceCurX+traceDirs[dir][0], traceCurY+traceDirs[dir][1]))
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
				tdata->traceLastDir = traceLastDir;
			}

			tdata->traceCurX = traceCurX;
			tdata->traceCurY = traceCurY;

			return PATHSTATE_OK;
		}

		PathState InitFloodfill(ThreadData* tdata, int start_x, int start_y, int flags)
		{
			int x, y;
			int unitSize = tdata->unitSize, areaMapIndex = tdata->areaMapIndex;

			tdata->calculateNearestReachable = flags & FLOODFILL_FLAG_CALCULATE_NEAREST;
			tdata->setAreaCodes = flags & FLOODFILL_FLAG_SET_AREA_CODE;

			if (tdata->calculateNearestReachable)
			{
				if (!IsWalkable(tdata->pUnit, start_x, start_y))
				{
					return PATHSTATE_ERROR;
				}
			}
			else
			{
				if (!IsWalkable_MType(start_x, start_y, tdata->areaMapIndex, tdata->unitSize))
				{
					return PATHSTATE_ERROR;
				}
			}

			tdata->firstScanlineIndex = 0;
			tdata->lastScanlineIndex = 0;
			tdata->numScanlines = 0;
			tdata->lowestDistance = 100000000;
			tdata->highestDistance = 0;
			tdata->circumTracking_Flood = 0;

			tdata->SQUARE_TYPE_OPEN += 3;
			tdata->SQUARE_TYPE_CLOSED += 3;
			tdata->SQUARE_TYPE_BLOCKED += 3;
			if (tdata->SQUARE_TYPE_OPEN == 0 || tdata->SQUARE_TYPE_CLOSED == 0 || tdata->SQUARE_TYPE_BLOCKED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(tdata->squareTypes[y], 0, width * sizeof(unsigned char));
					memset(tdata->squareNums[y], 0, width * sizeof(int));
				}
				tdata->SQUARE_TYPE_OPEN = 1;
				tdata->SQUARE_TYPE_CLOSED = 2;
				tdata->SQUARE_TYPE_BLOCKED = 3;
			}

			if (tdata->calculateNearestReachable)
			{
				tdata->scanlines[0].y = start_y;

				for (x = start_x-1; ; x--)
				{
					if (!IsWalkable(tdata->pUnit, x, start_y))
					{
						break;
					}
				}
				tdata->scanlines[0].start_x = x+1;

				if (x >= 0)
				{
					tdata->squareTypes[start_y][x] = tdata->SQUARE_TYPE_BLOCKED;
				}

				tdata->squareTypes[start_y][x+1] = tdata->SQUARE_TYPE_CLOSED;
				tdata->squareNums[start_y][x+1] = 0;

				for (x = start_x+1; ; x++)
				{
					if (!IsWalkable(tdata->pUnit, x, start_y))
					{
						break;
					}
				}
			}
			else
			{
				tdata->scanlines[0].y = start_y;

				for (x = start_x-1; ; x--)
				{
					if (!IsWalkable_MType(x, start_y, areaMapIndex, unitSize))
					{
						break;
					}
				}
				tdata->scanlines[0].start_x = x+1;

				if (x >= 0)
				{
					tdata->squareTypes[start_y][x] = tdata->SQUARE_TYPE_BLOCKED;
				}

				tdata->squareTypes[start_y][x+1] = tdata->SQUARE_TYPE_CLOSED;
				tdata->squareNums[start_y][x+1] = 0;

				for (x = start_x+1; ; x++)
				{
					if (!IsWalkable_MType(x, start_y, areaMapIndex, unitSize))
					{
						break;
					}
				}
			}
			tdata->scanlines[0].end_x = x-1;
				
			if (x < width)
			{
				tdata->squareTypes[start_y][x] = tdata->SQUARE_TYPE_BLOCKED;
			}

			tdata->numScanlines = 1;

			tdata->nextFreeScanline = 1;

			tdata->scanlineQueue[0] = 0;

			return PATHSTATE_OK;
		}

		PathState FloodfillStep(ThreadData* tdata, int target_x, int target_y)
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
			int unitSize = tdata->unitSize, areaMapIndex = tdata->areaMapIndex;
			struct scanline *scanlines = tdata->scanlines;
			int *scanlineQueue = tdata->scanlineQueue;
			bool setAreaCodes = tdata->setAreaCodes;
			bool calculateNearestReachable = tdata->calculateNearestReachable;
			unsigned char SQUARE_TYPE_CLOSED = tdata->SQUARE_TYPE_CLOSED,
			              SQUARE_TYPE_BLOCKED = tdata->SQUARE_TYPE_BLOCKED;
			Dimension::Unit *unit = tdata->pUnit;
			MovementData *md = unit->pMovementData;
			Uint16 areaCode = tdata->areaCode; 

			while (1)
			{

				if (tdata->numScanlines == 0)
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

				first_square = scanlineQueue[tdata->firstScanlineIndex];

				start_x = scanlines[first_square].start_x;
				end_x = scanlines[first_square].end_x;
				y = scanlines[first_square].y;

				tdata->firstScanlineIndex++;
				tdata->numScanlines--;
				
				if (tdata->firstScanlineIndex == tdata->scanlineQueueSize)
				{
					tdata->firstScanlineIndex = 0;
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
							tdata->circumTracking_Flood |= 1;
						}
						else
						{
							tdata->circumTracking_Flood |= 2;
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
							tdata->circumTracking_Flood |= 4;
						}
					}
					else /* if (*target_x < start_x) */
					{
						new_distance = (target_y - y) * (target_y - y) + (target_x - end_x) * (target_x - end_x);
						t_x = end_x;
						if (y == target_y)
						{
							tdata->circumTracking_Flood |= 8;
						}
					}
					if (new_distance < tdata->lowestDistance)
					{
						tdata->lowestDistance = new_distance;
						md->_action.changedGoalPos.x = t_x;
						md->_action.changedGoalPos.y = y;
						if (new_distance == 0)
						{
							return PATHSTATE_GOAL;
						}
					}
					if (tdata->circumTracking_Flood == 15)
					{
						if (new_distance > tdata->lowestDistance)
						{
//							cout << "Great success 2!" << endl;
							numGreatSuccess2++;
							continue;
						}
					}
					else
					{
						if (tdata->highestDistance < new_distance)
						{
							tdata->highestDistance = new_distance;
						}
					}
				}

				loop_start_y = y-1 >= 0 ? y-1 : 1;
				loop_end_y = y+1 < height ? y+1 : y-1;
				
				loop_start_x = start_x-1 >= 0 ? start_x-1 : 0;
				loop_end_x = end_x+1 < width ? end_x+1 : end_x;

				for (new_y = loop_start_y; new_y <= loop_end_y; new_y+=2)
				{
					scanline_types = tdata->squareTypes[new_y];
					scanline_nums = tdata->squareNums[new_y];
					for (x = loop_start_x; x <= loop_end_x; x++)
					{
						if (scanline_types[x] == SQUARE_TYPE_CLOSED)
						{
							x = scanlines[scanline_nums[x]].end_x+1;
						}
						else if (scanline_types[x] != SQUARE_TYPE_BLOCKED)
						{
							if ((calculateNearestReachable && IsWalkable(unit, x, new_y)) || (setAreaCodes && IsWalkable_MType(x, new_y, areaMapIndex, unitSize)))
							{
								int new_scanline;
								if (setAreaCodes)
								{
									for (new_x = x-1; ; new_x--)
									{
										if (scanline_types[x] == SQUARE_TYPE_BLOCKED || !IsWalkable_MType(new_x, new_y, areaMapIndex, unitSize))
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
											if (scanline_types[x] == SQUARE_TYPE_BLOCKED || !IsWalkable_MType(new_x, new_y, areaMapIndex, unitSize))
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

									new_scanline = tdata->nextFreeScanline++;

									if (new_scanline >= tdata->scanlineArraySize)
									{
										tdata->scanlineArraySize = tdata->scanlineArraySize * 2;
										scanlines = tdata->scanlines = (struct scanline*) realloc(scanlines, tdata->scanlineArraySize * sizeof(struct scanline));
									}

									scanlines[new_scanline].start_x = new_start_x;
									scanlines[new_scanline].end_x = new_end_x;
									scanlines[new_scanline].y = new_y;

									x = new_x;
									
									tdata->lastScanlineIndex++;
									tdata->numScanlines++;
									
									if (tdata->lastScanlineIndex == tdata->scanlineQueueSize)
									{
										tdata->lastScanlineIndex = 0;
									}
									else if (tdata->lastScanlineIndex == tdata->firstScanlineIndex && tdata->numScanlines != 1)
									{
										printf("FATAL - end of circular buffer reached start\n");
										md->_action.changedGoalPos.x = target_x;
										md->_action.changedGoalPos.y = target_y;
										return PATHSTATE_GOAL;
									}

									scanlineQueue[tdata->lastScanlineIndex] = new_scanline;
					
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
										tdata->squareTypes[y][new_start_x-1] = SQUARE_TYPE_CLOSED;
										tdata->squareNums[y][new_start_x-1] = first_square;
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

		int CalcH(int new_x, int new_y, int target_x, int target_y, int unitSize)
		{
			int h_diagonal = min(abs(new_x-target_x), abs(new_y-target_y));
			int h_straight = (abs(new_x-target_x) + abs(new_y-target_y));
			Uint16 hConst = hConsts[unitSize][target_y>>yShift][target_x>>xShift][new_y>>yShift][new_x>>xShift];
			return ((30 * h_diagonal + 20 * (h_straight - 2*h_diagonal)) * hConst) >> 8;
/*			int diff_x = fabs(new_x - target_x);
			int diff_y = fabs(new_y - target_y);
			return (diff_x + diff_y) * 20;*/
		}

		int num_steps = 0;

		PathState InitPathfinding(ThreadData*& tdata)
		{
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;

			int start_x = md->_action.startPos.x, start_y = md->_action.startPos.y;
			int target_x = md->_action.changedGoalPos.x, target_y = md->_action.changedGoalPos.y;

			int y;

			if (tdata->hasBegunPathfinding)
			{
				if (tdata->oldGoal.x == md->_action.changedGoalPos.x && tdata->oldGoal.y == md->_action.changedGoalPos.y)
				{
					return PATHSTATE_OK;
				}
			}

			tdata->oldGoal = md->_action.changedGoalPos;

			tdata->hasBegunPathfinding = true;

			num_steps = 0;

			tdata->lowestH = 1<<30;
			tdata->circumTracking = 0;
			tdata->nearestNode = -1;

			if (!IsWalkable(unit, start_x, start_y))
			{
				return PATHSTATE_ERROR;
			}

			tdata->NODE_TYPE_OPEN += 2;
			tdata->NODE_TYPE_CLOSED += 2;
			if (tdata->NODE_TYPE_OPEN == 0 || tdata->NODE_TYPE_CLOSED == 0)
			{
				for (y = 0; y < height; y++)
				{
					memset(tdata->nodeTypes[y], 0, width * sizeof(unsigned char));
					memset(tdata->nodeNums[y], 0, width * sizeof(int));
				}
				tdata->NODE_TYPE_OPEN = 1;
				tdata->NODE_TYPE_CLOSED = 2;
			}

			tdata->nodes[0].x = start_x;
			tdata->nodes[0].y = start_y;
			tdata->nodes[0].g = 0;
			tdata->nodes[0].h = CalcH(start_x, start_y, target_x, target_y, tdata->unitSize);
			tdata->nodes[0].f = 0;
			tdata->nodes[0].parent = -1;
			tdata->nodeNums[start_y][start_x] = 0;
			tdata->nodeTypes[start_y][start_x] = tdata->NODE_TYPE_OPEN;
			tdata->nextFreeNode = 1;
			binary_heap_pop_all(tdata->heap); // To be sure the heap is empty...
			binary_heap_push_item(tdata->heap, 0, 0);
			return PATHSTATE_OK;
		}

		int calcCount = 0;
		int fsteps = 0, psteps = 0;
		int tsteps = 0;

		PathState PathfindingStep(ThreadData*& tdata)
		{
			Dimension::Unit* unit  = tdata->pUnit;
			MovementData* md       = unit->pMovementData;
			int target_x = md->_action.changedGoalPos.x, target_y = md->_action.changedGoalPos.y;
			int unitSize = tdata->unitSize, areaMapIndex = tdata->areaMapIndex;
			struct node *nodes = tdata->nodes;
			unsigned char NODE_TYPE_OPEN = tdata->NODE_TYPE_OPEN,
			              NODE_TYPE_CLOSED = tdata->NODE_TYPE_CLOSED;
			
			int x, y;
			
			int first_node;
			int node_x, node_y;
			unsigned char *scanline_types;
			int *scanline_nums;
			first_node = binary_heap_pop_item(tdata->heap, -1);

			num_steps++;

			if (first_node == -1 /* || (circumTracking == 15 && nodes[first_node].h > highestH) */)
			{
	/*			printf("Did not reach target\n"); */
				notReachedPath += psteps;
				if (changedSinceLastRegen[unitSize][areaMapIndex])
				{
					numNotReached[unitSize][areaMapIndex]++;
				}
				if (tdata->nearestNode != -1)
				{
					md->_action.changedGoalPos.x = nodes[tdata->nearestNode].x;
					md->_action.changedGoalPos.y = nodes[tdata->nearestNode].y;
					return PATHSTATE_GOAL;
				}
				else
				{
					// Happens when InitPathfinding() failed and this routine was called nonetheless
					return PATHSTATE_ERROR;
				}
			}

			if (tdata->lowestH > nodes[first_node].h || tdata->nearestNode == -1)
			{
				tdata->lowestH = nodes[first_node].h;
				tdata->nearestNode = first_node;
			}
/*			if (circumTracking != 15 && highestH < nodes[first_node].h)
			{
				highestH = nodes[first_node].h;
			}*/

			node_x = nodes[first_node].x;
			node_y = nodes[first_node].y;
			tdata->nodeTypes[node_y][node_x] = NODE_TYPE_CLOSED;

			if (node_x == target_x)
			{
				if (node_y < target_y)
				{
					tdata->circumTracking |= 1;
				}
				else
				{
					tdata->circumTracking |= 2;
				}
			}
			
			if (node_y == target_y)
			{
				if (node_x < target_x)
				{
					tdata->circumTracking |= 4;
				}
				else
				{
					tdata->circumTracking |= 8;
				}
			}

			if ((node_x == target_x && node_y == target_y) || SquareIsGoal(unit, node_x, node_y, true))
			{
	/*			printf("Reached target\n"); */
				binary_heap_pop_all(tdata->heap);
				return PATHSTATE_GOAL;
			}

			for (y = -1; y <= 1; y++)
			{
				int new_y = node_y+y;
				if (new_y >= 0 && new_y < height)
				{
					scanline_types = tdata->nodeTypes[new_y];
					scanline_nums = tdata->nodeNums[new_y];
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
										nodes[node_num].g = new_g;
										nodes[node_num].f = nodes[node_num].h + new_g;
										nodes[node_num].parent = first_node;
										binary_heap_lower_item_score(tdata->heap, node_num, nodes[node_num].f);
									}
									else
									{
										continue;
									}
								}
								else
								{
									if (IsWalkable(unit, new_x, new_y) && tdata->nextFreeNode < tdata->openListSize)
									{
										int new_g = nodes[first_node].g;
										new_g += Dimension::GetTraversalTimeAdjusted(unit, node_x, node_y, x, y);

										node_num = scanline_nums[new_x] = tdata->nextFreeNode++;
										nodes[node_num].h = CalcH(new_x, new_y, target_x, target_y, tdata->unitSize);
										if (tdata->circumTracking != 15 || nodes[node_num].h <= tdata->lowestH)
										{
											nodes[node_num].x = new_x;
											nodes[node_num].y = new_y;
											nodes[node_num].g = new_g;
											nodes[node_num].f = nodes[node_num].h + new_g;
											nodes[node_num].parent = first_node;
											scanline_types[new_x] = NODE_TYPE_OPEN;
											binary_heap_push_item(tdata->heap, node_num, (nodes[node_num].f<<16)+nodes[node_num].h);
										}
										else
										{
											numGreatSuccess1++;
//											cout << "Great success!" << endl;
											tdata->nextFreeNode--;
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
			int hConstXTarget = tdata->nodes[tdata->nearestNode].x>>xShift;
			int hConstYTarget = tdata->nodes[tdata->nearestNode].y>>yShift;
			Node *prev_node = NULL, *first_node = NULL, *new_node = NULL;
			int num_nodes = 0, i;
			int cur_node = tdata->nearestNode;

			while (cur_node != -1)
			{
				num_nodes++;
				cur_node = tdata->nodes[cur_node].parent;
			}

			Node *new_nodes = new Node[num_nodes];

			i = 0;

			cur_node = tdata->nearestNode;
			
			SDL_LockMutex(gpmxHConst);
			while (cur_node != -1)
			{
				int hConstXNode = tdata->nodes[cur_node].x>>xShift;
				int hConstYNode = tdata->nodes[cur_node].y>>yShift;
				Uint16 old_hconst = hConsts[tdata->unitSize][hConstYTarget][hConstXTarget][hConstYNode][hConstXNode];
				Uint16 new_hconst = 0;
				Uint16 mixed_hconst;
				if (tdata->nodes[cur_node].h && tdata->nodes[tdata->nearestNode].g - tdata->nodes[cur_node].g)
				{
					new_hconst = Uint16(((tdata->nodes[tdata->nearestNode].g - tdata->nodes[cur_node].g) << 8) / tdata->nodes[cur_node].h);
					mixed_hconst = Uint16(((int)old_hconst * 15 + (int) new_hconst)>>4);
					if (old_hconst == mixed_hconst && new_hconst != mixed_hconst)
					{
						mixed_hconst += new_hconst < old_hconst ? (Uint16) -1 : (Uint16) 1;
					}
					hConsts[tdata->unitSize][hConstYTarget][hConstXTarget][hConstYNode][hConstXNode] = mixed_hconst;
				}

				new_node = &new_nodes[i++];
				new_node->x = tdata->nodes[cur_node].x;
				new_node->y = tdata->nodes[cur_node].y;
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
				cur_node = tdata->nodes[cur_node].parent;
				prev_node = new_node;
			}
			SDL_UnlockMutex(gpmxHConst);

			md->_goal = first_node;
			md->_start = new_node;
			tdata->nearestNode = -1;
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
//					printf("new_goal\n");
					
					md->_popFromQueue = false;

					SDL_LockMutex(gpmxThreadState);
					md->_currentState = INTTHRSTATE_WAITING;
					md->calcState     = CALCSTATE_WORKING;
					SDL_UnlockMutex(gpmxThreadState);

					md->_action = md->_newAction;

					SDL_LockMutex(gpmxQueue);
					gCalcQueue.push(tdata->pUnit);
					SDL_UnlockMutex(gpmxQueue);

					break;

				case POP_CANCELLED:

					md->_popFromQueue = false;

					SDL_LockMutex(gpmxThreadState);
					md->_currentState = INTTHRSTATE_NONE;
					md->calcState = CALCSTATE_FAILURE;
					SDL_UnlockMutex(gpmxThreadState);

					DeallocPathfinding(tdata);
//					std::cout << "handlecancelled " << tdata->pUnit->id << std::endl;
					
					break;

				case POP_DELETED:
//					printf("deleted\n");
					DeallocPathfinding(tdata);
					
					delete md;
					delete tdata->pUnit;

					md = NULL;
					break;

				default:
//					printf("default\n");
					DeallocPathfinding(tdata);
					
					SDL_LockMutex(gpmxThreadState);
//					cout << "none1 " << tdata->pUnit->id << endl;
					md->_currentState = INTTHRSTATE_NONE;
					md->calcState = CALCSTATE_FAILURE;
					SDL_UnlockMutex(gpmxThreadState);
			}
			
			tdata->pUnit = NULL;
		}

		bool CheckPop(ThreadData* tdata, MovementData* md)
		{
			SDL_LockMutex(gpmxCommand);
			if (md->_popFromQueue)
			{
				ParsePopQueueReason(tdata, md);
				tdata->pUnit = NULL;
				
				SDL_UnlockMutex(tdata->pMutex);
				SDL_UnlockMutex(gpmxCommand);
				return true;
			}
			SDL_UnlockMutex(gpmxCommand);
			return false;
		}
			
		int PerformPathfinding(ThreadData* tdata)
		{
			SDL_LockMutex(tdata->pMutex);

			Dimension::Unit* unit = NULL;
			MovementData* md = NULL;

			if (tdata->pUnit == NULL)
			{
				SDL_UnlockMutex(tdata->pMutex);
				SDL_LockMutex(gpmxQueue);
				if (gCalcQueue.empty())
				{
					SDL_UnlockMutex(gpmxQueue);
					return PATHSTATE_EMPTY_QUEUE;
				}
				else
				{
					tdata->pUnit = gCalcQueue.front();
					gCalcQueue.pop();
					tdata->pUnit->pMovementData->_associatedThread = tdata->threadIndex;
					SDL_UnlockMutex(gpmxQueue);
					SDL_LockMutex(tdata->pMutex);

					if (CheckPop(tdata, tdata->pUnit->pMovementData))
					{
						return SUCCESS;
					}

					SDL_LockMutex(gpmxThreadState);
					tdata->pUnit->pMovementData->_currentState = INTTHRSTATE_PROCESSING;
					SDL_UnlockMutex(gpmxThreadState);

					tdata->preprocessState = PREPROCESSSTATE_NONE;
					tdata->unitSize = tdata->pUnit->type->heightOnMap-1;
					tdata->areaMapIndex = tdata->pUnit->type->movementType;

					if (numNotReached[tdata->unitSize][tdata->areaMapIndex] > 10)
					{
						regenerateAreaCodes[tdata->unitSize][tdata->areaMapIndex] = true;
						numNotReached[tdata->unitSize][tdata->areaMapIndex] = 0;
					}
					
					InitAreaMaps(tdata);

					tdata->unitSize = tdata->pUnit->type->heightOnMap-1;
					tdata->areaMapIndex = tdata->pUnit->type->movementType;
					calcCount = 0;
					fsteps = 0;
					psteps = 0;
					tsteps = 0;

					tdata->hasBegunPathfinding = false;
					
					SDL_LockMutex(gpmxHConst);
					if (!hConsts[tdata->unitSize])
					{
						hConsts[tdata->unitSize] = new volatile Uint16***[hConstHeight];
						for (int y1 = 0; y1 < hConstHeight; y1++)
						{
							hConsts[tdata->unitSize][y1] = new volatile Uint16**[hConstWidth];
							for (int x1 = 0; x1 < hConstWidth; x1++)
							{
								hConsts[tdata->unitSize][y1][x1] = new volatile Uint16*[hConstHeight];
								for (int y2 = 0; y2 < hConstHeight; y2++)
								{
									hConsts[tdata->unitSize][y1][x1][y2] = new volatile Uint16[hConstHeight];
									for (int x2 = 0; x2 < hConstWidth; x2++)
									{
										hConsts[tdata->unitSize][y1][x1][y2][x2] = 0x100;
									}
								}
							}
						}
					}
					SDL_UnlockMutex(gpmxHConst);

				}
			}

			unit = tdata->pUnit;
			md   = unit->pMovementData;

			if (CheckPop(tdata, md))
			{
				return SUCCESS;
			}
			
			int steps = 0;
			bool quit = false; // <<< bad solution? See below...
			bool done = false;
			PathState state = PATHSTATE_OK;

			while (!done)
			{

				if (tdata->preprocessState < PREPROCESSSTATE_SKIPPED_TRACE)
				{
					if (tdata->preprocessState == PREPROCESSSTATE_NONE)
					{
						SDL_LockMutex(gpmxAreaMap);
						if (IsWalkable(unit, md->_action.goal.pos.x, md->_action.goal.pos.y) &&
						    areaMaps[tdata->unitSize][tdata->areaMapIndex][md->_action.startPos.y][md->_action.startPos.x] ==
						    areaMaps[tdata->unitSize][tdata->areaMapIndex][md->_action.goal.pos.y][md->_action.goal.pos.x])
						{
							SDL_UnlockMutex(gpmxAreaMap);
							tdata->preprocessState = PREPROCESSSTATE_SKIPPED_TRACE; // Skip it for now
							InitPathfinding(tdata);
						}
						else
						{
							SDL_UnlockMutex(gpmxAreaMap);
							tdata->preprocessState = PREPROCESSSTATE_PROCESSING_TRACE;
							if (InitTrace(tdata, md->_action.startPos.x, md->_action.startPos.y, md->_action.goal.pos.x, md->_action.goal.pos.y) == PATHSTATE_ERROR)
							{
								tdata->preprocessState = PREPROCESSSTATE_PROCESSING_FLOOD;
								if (InitFloodfill(tdata, md->_action.startPos.x, md->_action.startPos.y, FLOODFILL_FLAG_CALCULATE_NEAREST) == PATHSTATE_ERROR)
								{
									tdata->preprocessState = PREPROCESSSTATE_SKIPPED_FLOOD;
									InitPathfinding(tdata);
								}
							}
						}
					}
					if (tdata->preprocessState == PREPROCESSSTATE_PROCESSING_TRACE)
					{
						do
						{
							calcCount++;
							steps++;
							tsteps++;
						
							if (TraceStep(tdata, md->_action.startPos.x, md->_action.startPos.x, md->_action.goal.pos.x, md->_action.goal.pos.y) == PATHSTATE_GOAL)
							{
								tdata->preprocessState = PREPROCESSSTATE_SKIPPED_FLOOD;
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
								if (CheckPop(tdata, md))
								{
									return SUCCESS;
								}
							}
						} while (1);
					}
					if (tdata->preprocessState == PREPROCESSSTATE_PROCESSING_FLOOD)
					{
						do
						{
							calcCount++;
							steps++;
							fsteps++;
						
							if (FloodfillStep(tdata, md->_action.goal.pos.x, md->_action.goal.pos.y) == PATHSTATE_GOAL)
							{
								tdata->preprocessState = PREPROCESSSTATE_DONE;
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
								if (CheckPop(tdata, md))
								{
									return SUCCESS;
								}
							}
						} while (1);
					}
				}

				if (tdata->preprocessState >= PREPROCESSSTATE_SKIPPED_TRACE)
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

						if (calcCount >= RECALC_TRACE_LIMIT && tdata->preprocessState == PREPROCESSSTATE_SKIPPED_TRACE)
						{
							if (InitTrace(tdata, md->_action.startPos.x, md->_action.startPos.y, md->_action.goal.pos.x, md->_action.goal.pos.y) != PATHSTATE_ERROR)
							{
								tdata->preprocessState = PREPROCESSSTATE_PROCESSING_TRACE;
								break;
							}
							else
							{
								tdata->preprocessState = PREPROCESSSTATE_SKIPPED_FLOOD;
							}
						}

						if (calcCount >= RECALC_FLOODFILL_LIMIT && tdata->preprocessState == PREPROCESSSTATE_SKIPPED_FLOOD)
						{
							if (InitFloodfill(tdata, md->_action.startPos.x, md->_action.startPos.y, FLOODFILL_FLAG_CALCULATE_NEAREST) != PATHSTATE_ERROR)
							{
								tdata->preprocessState = PREPROCESSSTATE_PROCESSING_FLOOD;
								break;
							}
							else
							{
								tdata->preprocessState = PREPROCESSSTATE_DONE;
							}
						}

						state = PathfindingStep(tdata);
						
						if (CheckPop(tdata, md))
						{
							return SUCCESS;
						}

						SDL_UnlockMutex(tdata->pMutex);

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
							if (CheckPop(tdata, md))
							{
								return SUCCESS;
							}
							continue;
						}
						else
						{
//							cout << calcCount << " total, " << tsteps << " trace, " << fsteps << " flood, " << psteps << " a*, " << unit << endl;
							done = true;
							quit = false;
							break;
						}
					
					} while (1);
				}
			}

			if (!quit)
			{
				SDL_LockMutex(tdata->pMutex); // << ... mutex from locking when quitting the loop
			}

			SDL_LockMutex(gpmxThreadState);
			md->_currentState = INTTHRSTATE_UNAPPLIED;
			SDL_UnlockMutex(gpmxThreadState);
			md->changedGoal = false;
			
			int ret = ERROR_GENERAL;
			if (CheckPop(tdata, md))
			{
				return SUCCESS;
			}
			else
			{
				cCount += calcCount;
				tCount += tsteps;
				fCount += fsteps;
				pCount += psteps;
				numPaths++;

				if (state == PATHSTATE_GOAL || tdata->nearestNode != -1)
				{
					BuildNodeLinkedList(tdata);
					md->calcState = CALCSTATE_REACHED_GOAL;
					ret = SUCCESS;
					numTotalFrames += currentFrame - md->_pathfindingStartingFrame;
				}
				else
				{
					numFailed++;
					DeallocPathfinding(tdata);

					md->calcState = CALCSTATE_FAILURE;
				}
				SDL_LockMutex(gpmxDone);
				doneUnits.insert(unit);
				SDL_UnlockMutex(gpmxDone);
//				cout << "Done " << unit->id << endl;
			}

			
			tdata->pUnit = NULL;
			
			SDL_UnlockMutex(tdata->pMutex);
			
			return ret;
		}

		int GetQueueSize()
		{
			int ret;
			SDL_LockMutex(gpmxQueue);
			ret = gCalcQueue.size();
			SDL_UnlockMutex(gpmxQueue);
			return ret;
		}
	}
}
