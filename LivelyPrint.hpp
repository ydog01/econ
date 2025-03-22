#pragma once
#ifndef LivelyPrint_hpp
#define LivelyPrint_hpp
#include<vector>
#include<string>
#include<iostream>
#include<Windows.h>
namespace LivelyPT
{
	void init()
	{
		DWORD dwMode;
		GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode);
		dwMode |= 0x0004;
		SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode);
	}
	struct color_t
	{
		short r, g, b;
	};
	struct colora_t
	{
		short r, g, b,a;
	};
	inline void SetCLR(const color_t& color)
	{
		printf("\033[38;2;%d;%d;%dm", color.r, color.g, color.b);
	}
	inline void SetRGB(const short& r, const short& g, const short& b)
	{
		printf("\033[38;2;%d;%d;%dm", r, g, b);
	}
	inline void ReRGB()
	{
		printf("\033[0m");
	}
	struct texture
	{
		std::vector<color_t> color;
		std::vector<size_t> size;
		size_t cpos, spos, count;
		texture(const std::vector<color_t>& color_,
			const std::vector<size_t>& size_) :
			color(color_),
			size(size_) {}
		texture() = default;
		void print(const std::string& str)
		{
			cpos = spos = 0;
			count = size[0];
			for (const char& ch : str)
			{
				if ((!std::isspace(ch)) && count--)
				{
					SetCLR(color[cpos]);
					if (!count)
					{
						if (++cpos == color.size())
							cpos = 0;
						if (++spos == size.size())
							spos = 0;
						count = size[spos];
					}
				}
				std::cout.put(ch);
			}
			ReRGB();
		}
	};
}
#endif