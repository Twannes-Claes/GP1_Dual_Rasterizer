#pragma once

struct SDL_Window;
struct SDL_Surface;

#include <memory>
#include "Camera.h"
#include "Mesh.h"

namespace dae
{
	//class Mesh;
	struct Camera;

	class Renderer final
	{
	public:
		explicit Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer) const;
		void Render() const;

		void ToggleCameraLock()
		{
			m_IsCamLocked = !m_IsCamLocked;

			SDL_SetRelativeMouseMode(static_cast<SDL_bool>(m_IsCamLocked));
		}

		void ToggleSampleState() const
		{
			m_pMesh->ToggleSamplerState(m_pDevice);
		}

	private:

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		bool m_IsCamLocked{ true };

		//DIRECTX
		HRESULT InitializeDirectX();

		std::unique_ptr<Mesh> m_pMesh{};
		std::unique_ptr<Camera> m_pCamera{};

		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};

		std::unique_ptr<Mesh> m_pFireMesh{};

		float m_RotationSpeed{ 90 * TO_RADIANS };

	};
}
