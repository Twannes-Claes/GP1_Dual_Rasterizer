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

			switch (m_currentSampleState)
			{
				case SamplerStates::point:
					{
						std::cout << "POINT\n";
						m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
					}
					break;
				case SamplerStates::linear:
					{
						std::cout << "LINEAR\n";
						m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					}
					break;
				case SamplerStates::anisotropic:
					{
						std::cout << "ANISOTROPIC\n";
						m_SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
					}
					break;
				//default:
				//	break;
			}

			if (m_pSamplerState) m_pSamplerState->Release();

			if (const HRESULT result{ pDevice->CreateSamplerState(&m_SamplerDesc, &m_pSamplerState) }; FAILED(result)) return;

			m_pEffectSamplerVariable->SetSampler(0, m_pSamplerState);
		}

		protected:

			SamplerStates m_currentSampleState{ SamplerStates::point };

			ID3DX11Effect* m_pEffect{};
			ID3DX11EffectTechnique* m_pTechnique{};

			ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
			ID3DX11EffectMatrixVariable* m_pMatWorldMatrixVariable{};
			ID3DX11EffectMatrixVariable* m_pMatInverseViewMatrixVariable{};

			ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};

			ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable{};
			ID3D11SamplerState* m_pSamplerState{};
			D3D11_SAMPLER_DESC m_SamplerDesc{};

	};

}
