#include "stdafx.h"
#include "bitmap.h"

#include "tga.h"

namespace softdraw
{
	struct rgb_t
	{
		color_t shift;
		color_t bits;
		color_t range;
		uint16_t lsb_mask;
	};
	static rgb_t __rgb{};

	static void __blit_key_color_not_black(const bitmap_t& SRC, bitmap_t& aDst, const uint16_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		const uint16_t* SHORTSRC;
		uint16_t* shortDst;
		const uint16_t* SHORTSCANSRC;
		uint16_t* shortScanDst;

		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//short copy (since key keying)
		SHORTSRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
		shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

		for (int32_t y = 0; y < aCopyHeight; ++y)
		{
			SHORTSCANSRC = SHORTSRC;
			shortScanDst = shortDst;

			for (int32_t x = 0; x < aCopyWidth; ++x)
			{
				if (*SHORTSRC != SRC.key)
				{
					if (*SHORTSRC != 0)
						*shortDst = aColor;
					else
						*shortDst = *SHORTSRC;
				}
				++SHORTSRC;
				++shortDst;
			}

			SHORTSRC = SHORTSCANSRC + SRC.width;
			shortDst = shortScanDst + aDst.width;
		}
	}

	static bool __has_extension(const char* anExtension, const char* aString)
	{
		const char* last_dot = ::strrchr(aString, '.');
		return nullptr != last_dot && 0 == ::strcmp(last_dot + 1, anExtension);
	}

	static void __min_max(int32_t& aMin, int32_t& aMax)
	{
		if (aMin > aMax)
		{
			const int32_t temp = aMin;
			aMin = aMax;
			aMax = temp;
		}
	}

	static bool __clip(const cliprect_t& aClipRect, int32_t& aSrcX, int32_t& aSrcY, int32_t& aDstX, int32_t& aDstY, int32_t& aWidth, int32_t& aHeight)
	{
		//REJECTION TEST
		if ((aDstX + aWidth) < aClipRect.x ||
			aDstX >= aClipRect.z ||
			(aDstY + aHeight) < aClipRect.y ||
			aDstY >= aClipRect.w)
		{
			return false;
		}

		int32_t clip;

		//CLIP
		//x
		if (aDstX < aClipRect.x)
		{
			clip = aDstX - aClipRect.x;
			aSrcX -= clip;
			aWidth += clip;
			aDstX = aClipRect.x;
		}

		//return if no width
		if (aWidth <= 0)
			return false;

		//z
		if ((aDstX + aWidth) >= aClipRect.z)
		{
			aWidth = aClipRect.z - aDstX;
		}

		//return if no width
		if (aWidth <= 0)
			return false;

		//y
		if (aDstY < aClipRect.y)
		{
			clip = aDstY - aClipRect.y;
			aSrcY -= clip;
			aHeight += clip;
			aDstY = aClipRect.y;
		}

		//return if no height
		if (aHeight <= 0)
			return false;

		//w
		if ((aDstY + aHeight) >= aClipRect.w)
		{
			aHeight = aClipRect.w - aDstY;
		}

		return aHeight > 0;
	}

	static void __h_mirror_modify_src_x(const int32_t anOriginalCopyWidth, const int32_t aCurrentCopyWidth, const int32_t anOriginalSrcX, int32_t& aSrcX)
	{
		//right clip / no clip
		if (anOriginalSrcX == aSrcX)
		{
			aSrcX += anOriginalCopyWidth - 1;
		}
		//left clip
		else
		{
			const int32_t DIFF = anOriginalCopyWidth - aCurrentCopyWidth;
			aSrcX += anOriginalCopyWidth - DIFF - DIFF - 1;
		}
	}

	static bool __allocate(const uint32_t aSize, bitmap_t& bm)
	{
		assert(!bm.pixels);
		bm.pixels = new uint16_t[aSize];
		if (!bm.pixels)
			return false;

		mem_load.current += aSize;
		if (mem_load.current > mem_load.high)
			mem_load.high = mem_load.current;

		return true;
	}

	void __deallocate(const uint32_t aSize, bitmap_t& bm)
	{
		if (aSize)
		{
			assert(bm.width && bm.height && bm.pixels && (uint32_t)(bm.width * bm.height) == aSize);
			mem_load.current -= aSize;
			delete[] bm.pixels;
			bm.pixels = nullptr;
		}
		else
		{
			assert(!bm.width && !bm.height && !bm.pixels);
		}
	}

	static cliprect_t __clip_rect(const cliprect_t* rect, const bitmap_t& bitmap)
	{
		if (rect)
			return *rect;

		return { 0, 0, bitmap.width, bitmap.height };
	}

	static void __blit_key(const bitmap_t& SRC, bitmap_t& aDst, const operation_t anOp, const int32_t aDstX, const int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, const int32_t aSrcX, const int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//short copy (since key keying)
		const uint16_t* SHORTSRC;
		uint16_t* shortDst;
		SHORTSRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
		shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

		const uint16_t* SHORTSCANSRC;
		uint16_t* shortScanDst;
		for (int32_t y = 0; y < aCopyHeight; ++y)
		{
			SHORTSCANSRC = SHORTSRC;
			shortScanDst = shortDst;

			for (int32_t x = 0; x < aCopyWidth; ++x)
			{
				if (*SHORTSRC != SRC.key)
					blend(anOp, *SHORTSRC, shortDst);
				++SHORTSRC;
				++shortDst;
			}

			SHORTSRC = SHORTSCANSRC + SRC.width;
			shortDst = shortScanDst + aDst.width;
		}
	}

	static void __blit_add_key(const bitmap_t& SRC, bitmap_t& aDst, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		const uint16_t* SHORT_SRC, *SHORT_SCAN_SRC;
		uint16_t* shortDst, *shortScanDst;

		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		SHORT_SRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
		shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

		for (int32_t y = 0; y < aCopyHeight; ++y)
		{
			SHORT_SCAN_SRC = SHORT_SRC;
			shortScanDst = shortDst;

			for (int32_t x = 0; x < aCopyWidth; ++x)
			{
				//key
				if (*SHORT_SRC != SRC.key)
				{
					//add blend
					add_blend(*SHORT_SRC, *shortDst, shortDst);
				}
				++SHORT_SRC;
				++shortDst;
			}

			SHORT_SRC = SHORT_SCAN_SRC + SRC.width;
			shortDst = shortScanDst + aDst.width;
		}
	}

	static void __blit_sub_key(const bitmap_t& SRC, bitmap_t& aDst, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		const uint16_t* SHORT_SRC, *SHORT_SCAN_SRC;
		uint16_t* shortDst, *shortScanDst;

		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		SHORT_SRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
		shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

		for (int32_t y = 0; y < aCopyHeight; ++y)
		{
			SHORT_SCAN_SRC = SHORT_SRC;
			shortScanDst = shortDst;

			for (int32_t x = 0; x < aCopyWidth; ++x)
			{
				//key
				if (*SHORT_SRC != SRC.key)
				{
					//add blend
					sub_blend(*SHORT_SRC, *shortDst, shortDst);
				}
				++SHORT_SRC;
				++shortDst;
			}

			SHORT_SRC = SHORT_SCAN_SRC + SRC.width;
			shortDst = shortScanDst + aDst.width;
		}
	}

	void __setup_rgb()
	{
		__rgb.lsb_mask = ((__rgb.range.x - 2) << __rgb.shift.x) | ((__rgb.range.y - 2) << __rgb.shift.y) | ((__rgb.range.z - 2) << __rgb.shift.z);

		//common colors
		white = rgb(255, 255, 255);
		light_gray = rgb(191, 191, 191);
		gray = rgb(127, 127, 127);
		dark_gray = rgb(63, 63, 63);
		black = rgb(0, 0, 0);
		red = rgb(255, 0, 0);
		dark_red = rgb(127, 0, 0);
		orange = rgb(255, 127, 0);
		green = rgb(0, 255, 0);
		dark_green = rgb(0, 127, 0);
		blue = rgb(0, 0, 255);
		dark_blue = rgb(0, 0, 127);
		light_blue = rgb(127, 127, 255);
		yellow = rgb(255, 255, 0);
		dark_yellow = rgb(127, 127, 0);
		that_pink = rgb(255, 0, 255);
		purple = rgb(127, 0, 127);
	}

	//public
	//public
	//public
	//public
	void set_555()
	{
		__rgb.shift = { 10, 5, 0 };
		__rgb.bits = { 5, 5, 5 };
		__rgb.range = { 32, 32, 32 };

		__setup_rgb();
	}

	void set_565()
	{
		__rgb.shift = { 11, 5, 0 };
		__rgb.bits = { 5, 6, 5 };
		__rgb.range = { 32, 64, 32 };

		__setup_rgb();
	}

	void blend(const operation_t anOperation, const uint16_t aColor, uint16_t* aDst)
	{
		switch (anOperation)
		{
		default:
			assert(0);

		case operation_t::add:
			return add_blend(*aDst, aColor, aDst);

		case operation_t::sub:
			return sub_blend(*aDst, aColor, aDst);

		case operation_t::mul:
			return multi_blend(*aDst, aColor, aDst);

		case operation_t::half:
			return half_blend(*aDst, aColor, aDst);
		}
	}

	//RGB conversion function (range 0 to 255)
	//from 24bit to 16bit
	uint16_t rgb(const uint8_t aR, const uint8_t aG, const uint8_t aB)
	{
		//use this if you want to reduce color resolution
		//const int32_t COLORSHIFT = 8 - 5;//8 - desired bits per channel
		//aR = ((aR >> COLORSHIFT) << COLORSHIFT);
		//aG = ((aG >> COLORSHIFT) << COLORSHIFT);
		//aB = ((aB >> COLORSHIFT) << COLORSHIFT);
		//use this if you want to reduce color resolution

		//real rgb
		return
			((aR >> (8 - __rgb.bits.x)) << __rgb.shift.x) |
			((aG >> (8 - __rgb.bits.y)) << __rgb.shift.y) |
			((aB >> (8 - __rgb.bits.z)) << __rgb.shift.z);
	}

	//RGB conversion function
	//from 16bit to 24bit
	void rgb(const uint16_t aSrc, uint8_t& aR, uint8_t& aG, uint8_t& aB)
	{
		aR = ((aSrc >> __rgb.shift.x) & (__rgb.range.x - 1)) << (8 - __rgb.bits.x);
		aG = ((aSrc >> __rgb.shift.y) & (__rgb.range.y - 1)) << (8 - __rgb.bits.y);
		aB = ((aSrc >> __rgb.shift.z) & (__rgb.range.z - 1)) << (8 - __rgb.bits.z);
	}

	//additive blending
	void add_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst)
	{
		//get components
		color_t one;
		rgb(aOne, one.x, one.y, one.z);

		color_t two;
		rgb(aTwo, two.x, two.y, two.z);

		//mix
		const uint16_t RED = __min(255, (uint16_t)one.x + (uint16_t)two.x);
		const uint16_t GRN = __min(255, (uint16_t)one.y + (uint16_t)two.y);
		const uint16_t BLU = __min(255, (uint16_t)one.z + (uint16_t)two.z);

		//generate final color
		*aDst = rgb((uint8_t)RED, (uint8_t)GRN, (uint8_t)BLU);
	}

	void half_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst)
	{
		//clear LSB of each channel, divide by two, and add together
		*aDst = ((aOne & __rgb.lsb_mask) >> 1) + ((aTwo & __rgb.lsb_mask) >> 1);
	}

	//subtractive blending
	void sub_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst)
	{
		//get components
		color_t one;
		rgb(aOne, one.x, one.y, one.z);

		color_t two;
		rgb(aTwo, two.x, two.y, two.z);

		//mix
		const int16_t RED = __max(0, (int16_t)one.x - (int16_t)two.x);
		const int16_t GRN = __max(0, (int16_t)one.y - (int16_t)two.y);
		const int16_t BLU = __max(0, (int16_t)one.z - (int16_t)two.z);

		//generate final color
		*aDst = rgb((uint8_t)RED, (uint8_t)GRN, (uint8_t)BLU);
	}

	//multiplicative blending
	void multi_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst)
	{
		const float ONEOVER255(1.f / 255.f);
		color_t one;
		color_t two;
		color_t mix;

		//get components
		rgb(aOne, one.x, one.y, one.z);
		rgb(aTwo, two.x, two.y, two.z);

		//mix and clamp to max value
		mix.x = (const uint8_t)(((const float)one.x * ONEOVER255) * ((const float)two.x * ONEOVER255) * 255.f);
		mix.y = (const uint8_t)(((const float)one.y * ONEOVER255) * ((const float)two.y * ONEOVER255) * 255.f);
		mix.z = (const uint8_t)(((const float)one.z * ONEOVER255) * ((const float)two.z * ONEOVER255) * 255.f);

		//generate final color
		*aDst = rgb(mix.x, mix.y, mix.z);
	}

	bool bitmap_init(const int32_t aWidth, const int32_t aHeight, const uint16_t aKeyColor, bitmap_t& bm)
	{
		//cleanup
		bitmap_relinquish(bm);

		//get new memory
		if (!__allocate(aWidth * aHeight, bm))
			return false;

		//clear with key color
		{
			const uint16_t* END = bm.pixels + aWidth * aHeight;
			uint16_t* ptr = bm.pixels;
			while (ptr < END)
				*ptr++ = aKeyColor;
		}

		//store new settings
		bm.width = aWidth;
		bm.height = aHeight;
		bm.key = aKeyColor;

		return true;
	}

	void bitmap_relinquish(bitmap_t& bm)
	{
		__deallocate(bm.width * bm.height, bm);
		bm.width = bm.height = bm.key = 0;
	}

	bitmap_t::bitmap_t()
	{
		width = 0;
		height = 0;
		key = 0;
		pixels = nullptr;
	}

	bitmap_t::~bitmap_t()
	{
		bitmap_relinquish(*this);
	}

	bool bitmap_clear_set(bitmap_t& bm, const cliprect_t* aClipRect, const uint16_t aColor, int32_t aDstX, int32_t aDstY, int32_t aClearWidth, int32_t aClearHeight)
	{
		//check defaults
		if (!aClearWidth || aClearWidth > bm.width)
			aClearWidth = bm.width;
		if (!aClearHeight || aClearHeight > bm.height)
			aClearHeight = bm.height;

		//clip
		int32_t srcX = 0;
		int32_t srcY = 0;
		if (clip(bm, aClipRect, srcX, srcY, aDstX, aDstY, aClearWidth, aClearHeight))
		{
			//short clear
			uint16_t* dst = bm.pixels + aDstX + aDstY * bm.width;
			uint16_t* scanDst;
			for (int32_t y = 0; y < aClearHeight; ++y)
			{
				scanDst = dst;

				for (int32_t x = 0; x < aClearWidth; ++x)
				{
					assert(dst >= bm.pixels && dst < (bm.pixels + bm.width * bm.height));
					*dst++ = aColor;
				}

				dst = scanDst + bm.width;
			}

			return true;
		}

		return false;
	}

	void bitmap_blit(const bitmap_t& SRC, bitmap_t& aDst, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//short (only when WIDTH is a 2-multiple, not the copy width)
		if (SRC.width % 2)
		{
			const uint16_t* SHORTSRC;
			const uint16_t* SHORTSCANSRC;
			uint16_t* shortDst;
			uint16_t* shortScanDst;

			SHORTSRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
			shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

			for (int32_t y = 0; y < aCopyHeight; ++y)
			{
				SHORTSCANSRC = SHORTSRC;
				shortScanDst = shortDst;

				for (int32_t x = 0; x < aCopyWidth; ++x)
				{
					assert(SHORTSRC >= SRC.pixels && SHORTSRC < (SRC.pixels + SRC.width * SRC.height));
					assert(shortDst >= aDst.pixels && shortDst < (aDst.pixels + aDst.width * aDst.height));
					*shortDst++ = *SHORTSRC++;
				}

				SHORTSRC = SHORTSCANSRC + SRC.width;
				shortDst = shortScanDst + aDst.width;
			}
		}
		//long
		else
		{
			const unsigned long* LONGSRC;
			const unsigned long* LONGSCANSRC;
			unsigned long* longDst;
			unsigned long* longScanDst;

			const int32_t LONG_SRCWIDTH = SRC.width >> 1;
			const int32_t LONG_DSTWIDTH = aDst.width >> 1;
			const int32_t LONG_COPYWIDTH = aCopyWidth >> 1;
			LONGSRC = (const unsigned long*)(SRC.pixels + aSrcX + aSrcY * SRC.width);
			longDst = (unsigned long*)(aDst.pixels + aDstX + aDstY * aDst.width);

			for (int32_t y = 0; y < aCopyHeight; ++y)
			{
				LONGSCANSRC = LONGSRC;
				longScanDst = longDst;

				for (int32_t x = 0; x < LONG_COPYWIDTH; ++x)
				{
					assert(LONGSRC >= (const unsigned long*)SRC.pixels && LONGSRC < (const unsigned long*)(SRC.pixels + SRC.width * SRC.height));
					assert(longDst >= (const unsigned long*)aDst.pixels && longDst < (const unsigned long*)(aDst.pixels + aDst.width * aDst.height));
					*longDst++ = *LONGSRC++;
				}

				LONGSRC = LONGSCANSRC + LONG_SRCWIDTH;
				longDst = longScanDst + LONG_DSTWIDTH;
			}
		}
	}

	void bitmap_blit_key(const bitmap_t& SRC, bitmap_t& aDst, const int32_t aDstX, const int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, const int32_t aSrcX, const int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//short copy (since key keying)
		const uint16_t* SHORTSRC;
		uint16_t* shortDst;
		SHORTSRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
		shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

		const uint16_t* SHORTSCANSRC;
		uint16_t* shortScanDst;
		for (int32_t y = 0; y < aCopyHeight; ++y)
		{
			SHORTSCANSRC = SHORTSRC;
			shortScanDst = shortDst;

			for (int32_t x = 0; x < aCopyWidth; ++x)
			{
				if (*SHORTSRC != SRC.key)
					*shortDst = *SHORTSRC;
				++SHORTSRC;
				++shortDst;
			}

			SHORTSRC = SHORTSCANSRC + SRC.width;
			shortDst = shortScanDst + aDst.width;
		}
	}

	//clipping blits
	bool clip(const bitmap_t& aDst, const cliprect_t* aClipRect, int32_t& aSrcX, int32_t& aSrcY, int32_t& aDstX, int32_t& aDstY, int32_t& aCopyWidth, int32_t& aCopyHeight)
	{
		return __clip(__clip_rect(aClipRect, aDst), aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight);
	}

	bool bitmap_blit_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//clip
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			//call normal blit
			bitmap_blit(SRC, aDst, aDstX, aDstY, aCopyWidth, aCopyHeight, aSrcX, aSrcY);
			return true;
		}

		return false;
	}

	//clipping key blit
	bool bitmap_blit_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//clip
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			//call normal blit
			bitmap_blit_key(SRC, aDst, aDstX, aDstY, aCopyWidth, aCopyHeight, aSrcX, aSrcY);
			return true;
		}

		return false;
	}

	bool bitmap_blit_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, const operation_t anOp, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//clip
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			//call normal blit
			__blit_key(SRC, aDst, anOp, aDstX, aDstY, aCopyWidth, aCopyHeight, aSrcX, aSrcY);
			return true;
		}

		return false;
	}

	bool bitmap_blit_clip_horizontal_mirror(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		assert(aSrcX >= 0 && aSrcX < SRC.width);
		assert(aSrcY >= 0 && aSrcY < SRC.height);

		//clip
		const int32_t ORIGINAL_SRCX(aSrcX), ORIGINAL_COPYWIDTH(aCopyWidth);
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			assert(aSrcX >= 0 && aSrcX < SRC.width);
			assert(aSrcY >= 0 && aSrcY < SRC.height);

			const uint16_t* SHORTSRC;
			const uint16_t* SHORTSCANSRC;
			uint16_t* shortDst;
			uint16_t* shortScanDst;

			//we are flipping
			__h_mirror_modify_src_x(ORIGINAL_COPYWIDTH, aCopyWidth, ORIGINAL_SRCX, aSrcX);

			//short copy (since key keying)
			SHORTSRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
			shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

			for (int32_t y = 0; y < aCopyHeight; ++y)
			{
				SHORTSCANSRC = SHORTSRC;
				shortScanDst = shortDst;

				for (int32_t x = 0; x < aCopyWidth; ++x)
				{
					assert(SHORTSRC >= SRC.pixels && SHORTSRC < (SRC.pixels + SRC.width + SRC.height * SRC.width));
					*shortDst = *SHORTSRC;
					--SHORTSRC;
					++shortDst;
				}

				SHORTSRC = SHORTSCANSRC + SRC.width;
				shortDst = shortScanDst + aDst.width;
			}

			return true;
		}

		return false;
	}

	void bitmap_blit_half_key(const bitmap_t& SRC, bitmap_t& aDst, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		const uint16_t* SHORTSRC;
		const uint16_t* SHORTSCANSRC;
		uint16_t* shortDst;
		uint16_t* shortScanDst;

		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		SHORTSRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
		shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

		for (int32_t y = 0; y < aCopyHeight; ++y)
		{
			SHORTSCANSRC = SHORTSRC;
			shortScanDst = shortDst;

			for (int32_t x = 0; x < aCopyWidth; ++x)
			{
				//key key
				if (*SHORTSRC != SRC.key)
				{
					//add blend
					half_blend(*SHORTSRC, *shortDst, shortDst);
				}
				++SHORTSRC;
				++shortDst;
			}

			SHORTSRC = SHORTSCANSRC + SRC.width;
			shortDst = shortScanDst + aDst.width;
		}
	}

	bool bitmap_blit_half_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//clip
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			bitmap_blit_half_key(SRC, aDst, aDstX, aDstY, aCopyWidth, aCopyHeight, aSrcX, aSrcY);
			return true;
		}

		return false;
	}

	//additive blit
	void bitmap_blit_add(const bitmap_t& SRC, bitmap_t& aDst, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		const uint16_t* SHORTSRC;
		const uint16_t* SHORTSCANSRC;
		uint16_t* shortDst;
		uint16_t* shortScanDst;

		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		SHORTSRC = SRC.pixels + aSrcX + aSrcY * SRC.width;
		shortDst = aDst.pixels + aDstX + aDstY * aDst.width;

		for (int32_t y = 0; y < aCopyHeight; ++y)
		{
			SHORTSCANSRC = SHORTSRC;
			shortScanDst = shortDst;

			for (int32_t x = 0; x < aCopyWidth; ++x)
			{
				assert(SHORTSRC >= SRC.pixels && SHORTSRC < SRC.pixels + SRC.width * SRC.height);
				assert(shortDst >= aDst.pixels && shortDst < aDst.pixels + aDst.width * aDst.height);

				//add blend
				add_blend(*SHORTSRC, *shortDst, shortDst);
				++SHORTSRC;
				++shortDst;
			}

			SHORTSRC = SHORTSCANSRC + SRC.width;
			shortDst = shortScanDst + aDst.width;
		}
	}

	bool bitmap_blit_add_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//clip
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			//call normal blit
			__blit_add_key(SRC, aDst, aDstX, aDstY, aCopyWidth, aCopyHeight, aSrcX, aSrcY);
			return true;
		}

		return false;
	}

	bool bitmap_blit_sub_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//clip
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			//call normal blit
			__blit_sub_key(SRC, aDst, aDstX, aDstY, aCopyWidth, aCopyHeight, aSrcX, aSrcY);
			return true;
		}

		return false;
	}

	//faster lines
	void bitmap_h_line(bitmap_t& bm, int32_t x1, int32_t x2, const int32_t y1, const uint16_t aColor)
	{
		//clip
		if (y1 < 0 || y1 >= bm.height)
			return;

		__min_max(x1, x2);
		x1 = __max(0, x1);
		x2 = __min(bm.width, x2);
		uint16_t* dst = bm.pixels + x1 + y1 * bm.width;

		for (int32_t x = x1; x < x2; ++x)
		{
			assert(dst >= bm.pixels && dst < bm.pixels + bm.width * bm.height);
			*dst++ = aColor;
		}
	}

	void bitmap_v_line(bitmap_t& bm, const int32_t x1, int32_t y1, int32_t y2, const uint16_t aColor)
	{
		//clip
		if (x1 < 0 || x1 >= bm.width)
			return;

		__min_max(y1, y2);
		y1 = __max(0, y1);
		y2 = __min(bm.height, y2);
		uint16_t* dst = bm.pixels + x1 + y1 * bm.width;

		for (int32_t y = y1; y < y2; ++y)
		{
			assert(dst >= bm.pixels && dst < bm.pixels + bm.width * bm.height);
			*dst = aColor;
			dst += bm.width;
		}
	}

	void bitmap_cross(bitmap_t& bm, const int32_t x, const int32_t y, const int32_t aSize, const uint16_t aColor)
	{
		bitmap_h_line(bm, x - aSize / 2, x + aSize / 2, y, aColor);
		bitmap_v_line(bm, x, y - aSize / 2, y + aSize / 2, aColor);
	}

	bool bitmap_load_24(const char* aFileName, bitmap_t& bm)
	{
		if (!aFileName || 0 == *aFileName)
		{
			//FS_ERROR("nullptr or empty filename");
			return false;
		}

		if (!__has_extension("tga", aFileName))
		{
			//FS_ERROR("unsupported format '%s'", aFileName);
			return false;
		}

		bool result = false;
		fs::tga::image_t image{};
		if (read_24(aFileName, image))
			result = bitmap_import_flip_vertical_swap_r_b(image.header->image_spec_width, image.header->image_spec_height, (const color_t*)image.pixels, bm);
		delete[] image.memory;
		return result;
	}

	bool bitmap_import_flip_vertical_swap_r_b(const int32_t aWidth, const int32_t aHeight, const color_t* somePixels, bitmap_t& bm)
	{
		if (!aWidth || !aHeight || !somePixels)
			return false;

		//init bitmap
		if (!bitmap_init(aWidth, aHeight, rgb(255, 0, 255), bm))
			return false;

		//convert to correct format
		const color_t* SRC = somePixels + (aHeight - 1) * aWidth;
		uint16_t* dst = bm.pixels;

		for (int32_t y = 0; y < aHeight; ++y)
		{
			for (int32_t x = 0; x < aWidth; ++x)
			{
				*dst = rgb(SRC->z, SRC->y, SRC->x);
				++dst;
				++SRC;
			}
			SRC -= aWidth * 2;
		}

		return true;
	}

	bool bitmap_blit_key_color_not_black_clip(const bitmap_t& SRC, bitmap_t& aDst, const uint16_t aColor, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth, int32_t aCopyHeight, int32_t aSrcX, int32_t aSrcY)
	{
		//check defaults
		if (0 == aCopyWidth || aCopyWidth > SRC.width)
			aCopyWidth = SRC.width;
		if (0 == aCopyHeight || aCopyHeight > SRC.height)
			aCopyHeight = SRC.height;

		//clip
		if (clip(aDst, aClipRect, aSrcX, aSrcY, aDstX, aDstY, aCopyWidth, aCopyHeight))
		{
			//call normal blit
			__blit_key_color_not_black(SRC, aDst, aColor, aDstX, aDstY, aCopyWidth, aCopyHeight, aSrcX, aSrcY);
			return true;
		}

		return false;
	}

	mem_load_t mem_load{};
	uint16_t white = 0;
	uint16_t light_gray = 0;
	uint16_t gray = 0;
	uint16_t dark_gray = 0;
	uint16_t black = 0;
	uint16_t red = 0;
	uint16_t dark_red = 0;
	uint16_t orange = 0;
	uint16_t green = 0;
	uint16_t dark_green = 0;
	uint16_t blue = 0;
	uint16_t dark_blue = 0;
	uint16_t light_blue = 0;
	uint16_t yellow = 0;
	uint16_t dark_yellow = 0;
	uint16_t purple = 0;
	uint16_t that_pink = 0;
}
