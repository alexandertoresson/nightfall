#include "unittype.h"

#include "unit.h"

namespace Game
{
	namespace Dimension
	{
		RangeScanlines::~RangeScanlines()
		{
			if (scanlines != NULL)
			{
				delete[] scanlines;
			}
		}
		
		RangeArray::~RangeArray()
		{
			if (array != NULL)
			{
				for (int i = 0; i < size; i++)
				{
					delete[] array[i];
				}
				delete[] array;
			}
		}
		
		SingleAnimData::SingleAnimData()
		{
			animPos = 0;
			for (int i = 0; i < 4; i++)
				curFrames[i].model = NULL;
			nextFrameIndex = 0;
			curFrameLength = 0;
		}

		UnitAnimData::UnitAnimData()
		{
			transitionPos = 0;
			transitionLength = 0;
			anim[0] = NULL;
			anim[1] = NULL;
			isTransition = false;
		}

		UnitType::~UnitType()
		{
			if (attackRangeArray)
				delete attackRangeArray;
			
			if (sightRangeArray)
				delete sightRangeArray;
			
			if (lightRangeArray)
				delete lightRangeArray;
			
			if (lightRangeScanlines)
				delete lightRangeScanlines;
			
			if (sightRangeScanlines)
				delete sightRangeScanlines;
		}

		RangeScanlines *GenerateRangeScanlines(float maxrange)
		{
			RangeScanlines *ret = new RangeScanlines;
			int height = ((int)floor(maxrange) << 1) + 1;
			int y_offset = (int)floor(maxrange);
			int x_min = -y_offset;
			int x_max = y_offset;
			ret->height = height;
			ret->yOffset = y_offset;
			ret->scanlines = new Scanline[height];
			for (int y = 0; y < height; y++)
			{
				int x;
				for (x = x_min; x <= x_max; x++)
				{
					if (Distance2D(x, y - y_offset) <= maxrange)
					{
						ret->scanlines[y].startX = x;
						break;
					}
				}
				for (x++; x <= x_max+1; x++)
				{
					if (Distance2D(x, y - y_offset) > maxrange)
					{
						ret->scanlines[y].endX = x-1;
						break;
					}
				}
			}
			return ret;
		}

		RangeArray *GenerateRangeArray(float maxrange, float minrange)
		{
			RangeArray *ret = new RangeArray;
			int size = ((int)floor(maxrange) << 1) + 1;
			int offset = (int)floor(maxrange);
			ret->size = size;
			ret->offset = offset;
			ret->array = new char*[size];
			for (int y = 0; y < size; y++)
			{
				ret->array[y] = new char[size];
				for (int x = 0; x < size; x++)
				{
					float distance = Distance2D(float(x - offset), float(y - offset));
					ret->array[y][x] = distance <= maxrange && distance >= minrange;
				}
			}
			return ret;
		}

		void UnitType::GenerateRanges()
		{
			this->attackRangeArray = GenerateRangeArray(this->attackMaxRange, this->attackMinRange);
			this->sightRangeArray = GenerateRangeArray(this->sightRange, 0);
			this->lightRangeArray = GenerateRangeArray(this->lightRange, 0);
			this->sightRangeScanlines = GenerateRangeScanlines(this->sightRange);
			this->lightRangeScanlines = GenerateRangeScanlines(this->lightRange);
		}

		ProjectileType::ProjectileType(char* model, float aoe, Utilities::Vector3D start_pos, float speed, float size)
		{
			this->model = LoadModel(model);
			this->areaOfEffect = aoe;
			this->startPos = start_pos;
			this->speed = speed;
			this->size = size;
			this->isHoming = false;
		}

	}
}
