#pragma warning( disable : 4251 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4996 )
#pragma warning( disable : 4355 )

#pragma once
#pragma unmanaged
#define _WIN32_WINNT 0x0501
#define DIRECTINPUT_VERSION 0x0800

#include <fstream>

#include <windows.h>
#include <Winuser.h>
#include <d3d9.h>
#include <dinput.h>

#include <detours.h>

#include <memory>
#include <xmemory>
#include <stack>
#include <iostream>
#include <string>

#include "Hook/Function.hpp"
#include "Dinput/Input.hpp"
#include "Directx/myIDirect3D9.h"
#include "Directx/myIDirect3DDevice9.h"



extern std::ofstream file;