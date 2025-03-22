#ifndef PCH_H
#define PCH_H

#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<math.h>
#include<thread>
#include<mutex>
#include<atomic>
#include<SDL.h>
#include"eval.hpp"
#include"LivelyPrint.hpp"

#define winapi extern "C" __declspec(dllexport)
#endif 