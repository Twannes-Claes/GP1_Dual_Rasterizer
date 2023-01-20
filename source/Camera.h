#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <algorithm>

namespace dae
{
	struct Camera
	{
		Camera() = default;


		Vector3 origin{};
		float fovAngle{45.f};
		float fov{};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		float aspectRatio{};

		const float nearPlane{ 0.1f };
		const float farPlane{ 100.f };

		const float minPitch{ -89.99f * TO_RADIANS };
		const float maxPitch{ 89.99f * TO_RADIANS };

		const float speed{ 10 };
		const float speedRot{ 50 * TO_RADIANS };

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		const float sprintSpeedMultiplier{ 3 };

		void Initialize(const float _aspecRatio, const float _fovAngle = 90.f, const Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;

			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			aspectRatio = _aspecRatio;

			origin = _origin;

			CalculateProjectionMatrix();
		}

		void CalculateViewMatrix()
		{
			
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();

			up = Vector3::Cross(forward, right);

			invViewMatrix = Matrix
			{
				right,
				up,
				forward,
				origin
			};

			//viewMatrix = invViewMatrix.Inverse();
			viewMatrix = Matrix::Inverse(invViewMatrix);

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{

			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);

			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			float moveSpeed{ deltaTime * speed };
			const float rotSpeed{ deltaTime * speedRot };

			moveSpeed = (static_cast<float>(pKeyboardState[SDL_SCANCODE_LSHIFT]) * (sprintSpeedMultiplier) * moveSpeed) + moveSpeed;

			origin += (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP]) * forward * moveSpeed;
			origin -= (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN]) * forward * moveSpeed;

			origin -= (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT]) * right * moveSpeed;
			origin += (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT]) * right * moveSpeed;

			origin -= pKeyboardState[SDL_SCANCODE_Q] * up * moveSpeed;
			origin += pKeyboardState[SDL_SCANCODE_E] * up * moveSpeed;

			const float lmb { static_cast<float>(mouseState == SDL_BUTTON_LMASK) };
			const float rmb { static_cast<float>(mouseState == SDL_BUTTON_RMASK) };

			const float lrmb{ static_cast<float>(mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) };

			const float fMouseY { static_cast<float>(mouseY) };
			const float fMouseX { static_cast<float>(mouseX) };

			origin -= lmb * forward * moveSpeed * fMouseY;
			origin -= lrmb * up * (moveSpeed / 3) * fMouseY;

			totalPitch -= rmb * rotSpeed * fMouseY;
			totalPitch = std::clamp(totalPitch, minPitch, maxPitch);

			totalYaw += lmb * rotSpeed * fMouseX;
			totalYaw += rmb * rotSpeed * fMouseX;

			forward = (Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw)).TransformVector(Vector3::UnitZ);

			CalculateViewMatrix();
		}
	};
}