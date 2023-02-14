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
		void Render();

		void PrintInstructions() const
		{
			// Show keybinds
			std::cout << "\033[33m"; // TEXT COLOR
			std::cout << "[Key Bindings - SHARED]\n\n";
			std::cout << "\t[F1]  Toggle Rasterizer Mode (HARDWARE / SOFTWARE)\n";
			std::cout << "\t[F2]  Toggle Vehicle Rotation (ON / OFF)\n";
			std::cout << "\t[F9]  Cycle CullMode (BACK / FRONT / NONE)\n";
			std::cout << "\t[F10] Toggle Uniform ClearColor (ON / OFF)\n";
			std::cout << "\t[F11] Toggle Print FPS (ON / OFF)\n";
			std::cout << "\n\t[ESC] Toggle Mouse lock (ON / OFF)\n";
			std::cout << "\t[ENTER] Refresh console\n";
			std::cout << "\n";
			std::cout << "\033[32m"; // TEXT COLOR
			std::cout << "[Key Bindings - HARDWARE]\n\n";
			std::cout << "\t[F3] Toggle FireFX (ON / OFF)\n";
			std::cout << "\t[F4] Cycle Sampler State (POINT / LINEAR / ANISOTROPIC)\n";
			std::cout << "\n";
			std::cout << "\033[35m"; // TEXT COLOR
			std::cout << "[Key Bindings - SOFTWARE]\n\n";
			std::cout << "\t[F5] Cycle Shading Mode (COMBINED / OBSERVED_AREA / DIFFUSE / SPECULAR)\n";
			std::cout << "\t[F6] Toggle NormalMap (ON / OFF)\n";
			std::cout << "\t[F7] Toggle DepthBuffer Visualization (ON / OFF)\n";
			std::cout << "\t[F8] Toggle BoundingBox Visualization (ON / OFF)\n";
			std::cout << "\n\n";
		}

		void ToggleDepth()
		{
			m_ShowDepthBuffer = !m_ShowDepthBuffer;

			std::cout << "\033[35m"; // TEXT COLOR
			std::cout << "**(SOFTWARE) Depth ";
			if (m_ShowDepthBuffer)
			{
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void ToggleRotate()
		{
			m_IsRotating = !m_IsRotating;

			std::cout << "\033[33m"; // TEXT COLOR
			std::cout << "**(SHARED) Rotate ";
			if (m_IsRotating)
			{
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void ToggleNormal()
		{
			m_ShowNormal = !m_ShowNormal;

			std::cout << "\033[35m"; // TEXT COLOR
			std::cout << "**(SOFTWARE) Normal ";
			if (m_ShowNormal)
			{
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void ToggleBounding()
		{
			m_ShowBoundingBoxes = !m_ShowBoundingBoxes;

			std::cout << "\033[35m"; // TEXT COLOR
			std::cout << "**(SOFTWARE) Bounding box ";
			if (m_ShowBoundingBoxes)
			{
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void ToggleSoftwareState()
		{
			if (m_CurrentRasterizerState != RasterizerState::software) return;

			m_CurrentSoftwareMode = static_cast<SoftwareModes>((static_cast<int>(m_CurrentSoftwareMode) + 1) % m_SoftwareModeSize);

			std::cout << "\033[35m"; // TEXT COLOR

			switch (m_CurrentSoftwareMode)
			{
				case SoftwareModes::Combined:
					std::cout << "**(SOFTWARE) Combined\n";
					break;
				case SoftwareModes::Specular:
					std::cout << "**(SOFTWARE) Specular\n";
					break;
				case SoftwareModes::Diffuse:
					std::cout << "**(SOFTWARE) Diffuse\n";
					break;
				case SoftwareModes::ObservedArea:
					std::cout << "**(SOFTWARE) ObservedArea\n";
					break;
			}
		}

		void ToggleCameraLock()
		{
			m_IsCamLocked = !m_IsCamLocked;

			SDL_SetRelativeMouseMode(static_cast<SDL_bool>(m_IsCamLocked));

			std::cout << "\033[33m"; // TEXT COLOR
			std::cout << "**(SHARED) Camera lock ";
			if (m_IsCamLocked)
			{
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void ToggleSampleState() const
		{
			if (m_CurrentRasterizerState != RasterizerState::hardware) return;

			m_pMesh->ToggleSamplerState(m_pDevice);
		}

		void ToggleCullMode()
		{
			m_CurrentCullMode = static_cast<CullMode>((static_cast<int>(m_CurrentCullMode) + 1) % 3);

			m_pMesh->ToggleCullMode(m_pDevice);
		}

		void ToggleRotation()
		{
			m_IsRotating = !m_IsRotating;

			std::cout << "\033[33m"; // TEXT COLOR
			std::cout << "**(SHARED) Vehicle Rotation ";
			if (m_IsRotating)
			{
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void ToggleUniform()
		{
			m_IsUniform = !m_IsUniform;

			std::cout << "\033[33m"; // TEXT COLOR
			std::cout << "**(SHARED) Uniform ClearColor ";
			if (m_IsUniform)
			{ 
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void ToggleBackgroundState()
		{
			m_CurrentRasterizerState = static_cast<RasterizerState>((static_cast<int>(m_CurrentRasterizerState) + 1) % (m_RasterizerStateSize - 1));

			std::cout << "\033[33m"; // TEXT COLOR
			std::cout << "**(SHARED) Rasterizer Mode = ";

			switch (m_CurrentRasterizerState)
			{
			case RasterizerState::software:
				std::cout << "SOFTWARE\n";
				break;
			case RasterizerState::hardware:
				std::cout << "HARDWARE\n";
				break;
			case RasterizerState::uniform:
				break;
			}
		}

		void ToggleFireMesh()
		{
			if (m_CurrentRasterizerState != RasterizerState::hardware) return;

			m_CanShowFire = !m_CanShowFire;

			std::cout << "\033[32m"; // TEXT COLOR

			std::cout << "**(HARDWARE)FireFX ";
			if (m_CanShowFire)
			{
				std::cout << "ON\n";
			}
			else
			{
				std::cout << "OFF\n";
			}
		}

		void TogglePrintingFPS()
		{
			m_CanPrint = !m_CanPrint;
		}

		bool CanPrintFPS() const { return m_CanPrint; }

	private:

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		bool m_IsCamLocked{ true };

		bool m_IsRotating{ false };

		float m_RotationSpeed{ 45 * TO_RADIANS };

		bool m_CanShowFire{ true };

		bool m_CanPrint{ false };

		bool m_IsUniform{ false };

		void InitializeMesh();

		//DIRECTX
		HRESULT InitializeDirectX();

		std::unique_ptr<Mesh> m_pMesh{};
		std::unique_ptr<Mesh> m_pFireMesh{};

		std::unique_ptr<Camera> m_pCamera{};

		//resources
		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};

		enum class RasterizerState
		{
			hardware,
			software,
			uniform
		};

		enum class CullMode
		{
			front,
			back,
			none
		};

		CullMode m_CurrentCullMode{ CullMode::back };

		static constexpr int m_CullmodeSize{ static_cast<int>(CullMode::none) + 1 };


		RasterizerState m_CurrentRasterizerState{ RasterizerState::hardware };

		static constexpr int m_RasterizerStateSize{ static_cast<int>(RasterizerState::uniform) + 1 };


		const ColorRGB m_ColorsState[m_RasterizerStateSize]{  { 0.39f, 0.59f, .93f } , { 0.39f, 0.39f, .39f }, { 0.1f, 0.1f, .1f } };

#pragma region software_code

		//buffers for software
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		int m_NrOfPixels;

		//textures
		Texture* m_pDiffuseTexture{};
		Texture* m_pNormalTexture{};
		Texture* m_pSpecularTexture{};
		Texture* m_pGlossTexture{};

		//light data
		const Vector3 m_LightDir{ 0.577f, -0.577f , 0.577f };
		const float m_LightIntensity{ 7.f };

		const float m_KD{ 1.f };
		const float m_Shinyness{ 25 };

		const ColorRGB m_AmbientColor{ 0.025f, 0.025f, 0.025f };

		float m_AspectRatio;


		bool m_ShowBoundingBoxes{ false };

		bool m_ShowDepthBuffer{ false };

		bool m_ShowNormal{ true };


		const float m_BoundingMargin{ 1.f };

		std::vector<Vector2> m_Vertices_ScreenSpace{};

		std::vector<Vertex_Out> m_Vertices_Out{};

		enum class SoftwareModes
		{
			Combined,
			ObservedArea,
			Diffuse,
			Specular
		};

		SoftwareModes m_CurrentSoftwareMode{ SoftwareModes::Combined };

		static constexpr int m_SoftwareModeSize{ static_cast<int>(SoftwareModes::Specular) + 1 };


		bool CheckValidCullCrosses(const float edge01, const float edge02, const float edge03) const;

		void VertexTransformationFunction();

		Vector2 CalcUVComponent(const float weight, const float invDepth, const size_t& index) const;

		ColorRGB CalculateSpecular(const Vector3& sampledNormal, const Vertex_Out& v) const;

		void InterpolatePixelInfo(Vertex_Out& pixelInfo, const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float depth) const;

		float CalculateInterpolateDepth(const float w0, const float w1, const float w2, const float d0, const float d1, const float d2) const;

		float CalculateDepth(const Vertex_Out& v, const bool usingAxisW) const;

		void ConvertColorToPixel(ColorRGB& finalColor, const int pixelIndex) const;

		void RenderTriangle(const size_t& index, const bool swapVertices) const;

		void PixelShading(const Vertex_Out& vOut, ColorRGB& finalColor) const;

		void ClearBackGround() const
		{
			if(m_IsUniform)
			{
				SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, static_cast<Uint8>(m_ColorsState[static_cast<int>(RasterizerState::uniform)].r * 255), static_cast<Uint8>(m_ColorsState[static_cast<int>(RasterizerState::uniform)].g * 255), static_cast<Uint8>(m_ColorsState[static_cast<int>(RasterizerState::uniform)].b * 255)));
				return;
			}

			SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, static_cast<Uint8>(m_ColorsState[static_cast<int>(RasterizerState::software)].r * 255), static_cast<Uint8>(m_ColorsState[static_cast<int>(RasterizerState::software)].g * 255), static_cast<Uint8>(m_ColorsState[static_cast<int>(RasterizerState::software)].b * 255)));
		}

		void constexpr ClearDepthBuffer() const
		{
			std::fill_n(m_pDepthBufferPixels, m_NrOfPixels, FLT_MAX);
		}

		bool IsOutOfFrustrum(const Vertex_Out& vOut) const;


#pragma endregion

	};
}
