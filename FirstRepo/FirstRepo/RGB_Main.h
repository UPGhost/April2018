#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <conio.h>
#include <Keyboard.h>
#include <cstdlib> 
#include <string>
#include "resource.h"
#include "iostream"

using namespace DirectX;
using namespace std;



//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT3 Pos;
	//XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	XMFLOAT4 Color;

};



