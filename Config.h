/*===================================================================================
*    Date : 2023/06/22(–Ø)
*        Author		: Gakuto.S
*        File		: Config.h
*        Detail		:
===================================================================================*/
#ifndef CONFIG_H_
#define CONFIG_H_

#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <string>
#include <thread>
#include <cassert>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

// ƒ}ƒNƒ’è‹`
#define SAFE_RELEASE(x) if(x) x->Release();

#endif // _WIN32

struct CAMERA
{
	glm::vec3 position;
	glm::vec3 rotation;
};

struct MATRICES
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 camPos;
};

#endif // !CONFIG_H_

