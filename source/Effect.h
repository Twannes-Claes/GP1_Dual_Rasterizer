#pragma once

namespace dae
{

	class Texture;

	class Effect
	{
		public:

			enum class SamplerStates
			{
				point,
				linear,
				anisotropic
			};

			enum class CullModes
			{
				front,
				back,
				none
			};

			explicit Effect(ID3D11Device* pDevice, const std::wstring& assetFile);

			virtual ~Effect();

			Effect(const Effect&) = delete;
			Effect(Effect&&) noexcept = delete;
			Effect& operator=(const Effect&) = delete;
			Effect& operator=(Effect&&) noexcept = delete;

			static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

			ID3DX11Effect* GetEffect() const
			{
				return m_pEffect;
			}

			ID3DX11EffectTechnique* GetTechnique() const
			{
				return m_pTechnique;
			}

			void SetProjectionMatrix(const Matrix& matrix) const;

			void SetInvViewMatrix(const Matrix& invViewMatrix) const;

			void SetWorldMatrix(const Matrix& worldMatrix) const;


			void SetDiffuseMap(const Texture* pTexture) const;

			 
			virtual void SetNormalMap(const Texture* pTexture) const = 0;
			
			virtual void SetSpecularMap(const Texture* pTexture) const = 0;
			
			virtual void SetGlossinessMap(const Texture* pTexture) const = 0;

			
			void ToggleSamplerState(ID3D11Device* pDevice, const bool changeState = true)
		{

			if (changeState)
			{
				m_currentSampleState = static_cast<SamplerStates>((static_cast<int>(m_currentSampleState) + 1) % 3);
			}

			std::cout << "\033[32m"; // TEXT COLOR

			switch (m_currentSampleState)
			{
				case SamplerStates::point:
					{
						//std::cout << "POINT\n";
						m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

						std::cout << "**(HARDWARE) POINT \n";
					}
					break;
				case SamplerStates::linear:
					{
						//std::cout << "LINEAR\n";
						m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

						std::cout << "**(HARDWARE) ANISOTROPIC \n";
					}
					break;
				case SamplerStates::anisotropic:
					{
						//std::cout << "ANISOTROPIC\n";
						m_SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;

						std::cout << "**(HARDWARE) LINEAR \n";
					}
					break;
				//default:
				//	break;
			}

			//release sampler state and initialize it with the new one
			if (m_pSamplerState) m_pSamplerState->Release();

			if (const HRESULT result{ pDevice->CreateSamplerState(&m_SamplerDesc, &m_pSamplerState) }; FAILED(result)) return;

			m_pEffectSamplerVariable->SetSampler(0, m_pSamplerState);
		}

			void ToggleCullMode(ID3D11Device* pDevice, const bool changeState = true)
			{

				if (changeState)
				{
					m_CurrentCullmode = static_cast<CullModes>((static_cast<int>(m_CurrentCullmode) + 1) % 3);
				}

				std::cout << "\033[33m"; // TEXT COLOR

				switch (m_CurrentCullmode)
				{
					case CullModes::front:
					{
						m_RasterizerDesc.CullMode = D3D11_CULL_FRONT;

						std::cout << "**(SHARED) Cullmode FRONT \n";
					}
					break;
					case CullModes::back:
					{
						m_RasterizerDesc.CullMode = D3D11_CULL_BACK;
						std::cout << "**(SHARED) Cullmode BACK \n";
					}
					break;
					case CullModes::none:
					{
						m_RasterizerDesc.CullMode = D3D11_CULL_NONE;
						std::cout << "**(SHARED) Cullmode NONE \n";
					}
					break;
				}

				//release rasterizer state and initialize it with the new one
				if (m_pRasterizerState) m_pRasterizerState->Release();

				if (const HRESULT result{ pDevice->CreateRasterizerState(&m_RasterizerDesc, &m_pRasterizerState) }; FAILED(result)) return;

				m_pEffectRasterizerVariable->SetRasterizerState(0, m_pRasterizerState);

			}

		protected:

			SamplerStates m_currentSampleState{ SamplerStates::point };
			CullModes m_CurrentCullmode{ CullModes::back };

			ID3DX11Effect* m_pEffect{};

			ID3DX11EffectTechnique* m_pTechnique{};

			ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
			ID3DX11EffectMatrixVariable* m_pMatWorldMatrixVariable{};
			ID3DX11EffectMatrixVariable* m_pMatInverseViewMatrixVariable{};

			ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};

			ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable{};
			ID3DX11EffectRasterizerVariable* m_pEffectRasterizerVariable{};

			ID3D11SamplerState* m_pSamplerState{};
			D3D11_SAMPLER_DESC m_SamplerDesc{};

			ID3D11RasterizerState* m_pRasterizerState{};
			D3D11_RASTERIZER_DESC m_RasterizerDesc{};

	};

}
