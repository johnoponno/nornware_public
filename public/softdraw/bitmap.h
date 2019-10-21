#pragma once

namespace softdraw
{
	struct cliprect_t
	{
		int32_t x;
		int32_t y;
		int32_t z;
		int32_t w;
	};

	struct color_t
	{
		uint8_t x;
		uint8_t y;
		uint8_t z;
	};

	struct mem_load_t
	{
		uint32_t current;
		uint32_t high;
	};

	enum struct operation_t
	{
		set,
		add,
		sub,
		mul,
		half,
	};

	struct bitmap_t
	{
		explicit bitmap_t();
		~bitmap_t();

		int32_t width;
		int32_t height;
		uint16_t key;
		uint16_t* pixels;

	private:

		explicit bitmap_t(const bitmap_t& other);
		void operator = (const bitmap_t& other);
	};

	void set_555();
	void set_565();
	void blend(const operation_t anOperation, const uint16_t aColor, uint16_t* aDst);
	uint16_t rgb(const uint8_t aR, const uint8_t aG, const uint8_t aB);
	void add_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);
	void half_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);
	void sub_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);
	void multi_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);

	bool clip(const bitmap_t& aDst, const cliprect_t* aClipRect, int32_t& aSrcX, int32_t& aSrcY, int32_t& aDstX, int32_t& aDstY, int32_t& aCopyWidth, int32_t& aCopyHeight);

	bool bitmap_init(const int32_t aWidth, const int32_t aHeight, const uint16_t aKey, bitmap_t& bm);
	void bitmap_relinquish(bitmap_t& bm);
	bool bitmap_load_24(const char* aFileName, bitmap_t& bm);
	bool bitmap_import_flip_vertical_swap_r_b(const int32_t aWidth, const int32_t aHeight, const color_t* somePixels, bitmap_t& bm);
	bool bitmap_clear_set(bitmap_t& bm, const cliprect_t* aClipRect, const uint16_t aColor, int32_t aDstX = 0, int32_t aDstY = 0, int32_t aClearWidth = 0, int32_t aClearHeight = 0);
	void bitmap_cross(bitmap_t& bm, const int32_t x, const int32_t y, const int32_t aSize, const uint16_t aColor);

	void bitmap_blit(const bitmap_t& SRC, bitmap_t& aDst, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	bool bitmap_blit_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	void bitmap_blit_key(const bitmap_t& SRC, bitmap_t& aDst, const int32_t aDstX, const int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, const int32_t aSrcX = 0, const int32_t aSrcY = 0);
	bool bitmap_blit_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	bool bitmap_blit_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, const operation_t anOp, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	bool bitmap_blit_key_color_not_black_clip(const bitmap_t& SRC, bitmap_t& aDst, const uint16_t aColor, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	bool bitmap_blit_half_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	void bitmap_blit_add(const bitmap_t& SRC, bitmap_t& aDst, const int32_t aDstX, const int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	bool bitmap_blit_add_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
	bool bitmap_blit_sub_key_clip(const bitmap_t& SRC, bitmap_t& aDst, const cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);

	extern mem_load_t mem_load;
	extern uint16_t white;
	extern uint16_t light_gray;
	extern uint16_t gray;
	extern uint16_t dark_gray;
	extern uint16_t black;
	extern uint16_t red;
	extern uint16_t dark_red;
	extern uint16_t orange;
	extern uint16_t green;
	extern uint16_t dark_green;
	extern uint16_t blue;
	extern uint16_t dark_blue;
	extern uint16_t light_blue;
	extern uint16_t yellow;
	extern uint16_t dark_yellow;
	extern uint16_t purple;
	extern uint16_t that_pink;
}
