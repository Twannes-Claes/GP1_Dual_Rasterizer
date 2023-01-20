#include "pch.h"
#include "Effect.h"
#include "Texture.h"

namespace dae
{

	dae::Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		: m_pEffect{ LoadEffect(pDevice, assetFile) }
	{
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");

		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid!\n";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();

		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
		}

		m_pMatWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();

		if (!m_pMatWorldMatrixVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldMatrixVariable not valid!\n";
		}

		m_pMatInverseViewMatrixVariable = m_pEffect->GetVariableByName("gInverseViewMatrix")->AsMatrix();

		if (!m_pMatInverseViewMatrixVariable->IsValid())
		{
			std::wcout << L"m_pMatInverseViewMatrixVariable not valid!\n";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();

		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid!\n";
		}

		//m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		//
		//if (!m_pNormalMapVariable->IsValid())
		//{
		//	std::wcout << L"m_pNormalMapVariable not valid!\n";
		//}
		//
		//m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		//
		//if (!m_pSpecularMapVariable->IsValid())
		//{
		//	std::wcout << L"m_pSpecularMapVariable not valid!\n";
		//}
		//
		//m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		//
		//if (!m_pGlossinessMapVariable->IsValid())
		//{
		//	std::wcout << L"m_pGlossinessMapVariable not valid!\n";
		//}

		m_pEffectSamplerVariable = m_pEffect->GetVariableByName("gSampler")->AsSampler();

		if (!m_pEffectSamplerVariable->IsValid())
		{
			std::wcout << L"m_pEffectSamplerVariable not valid!\n";
		}

		m_SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		m_SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		m_SamplerDesc.MipLODBias = 0;
		m_SamplerDesc.MinLOD = 0;

		m_SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		m_SamplerDesc.MaxAnisotropy = 16;

		ToggleSamplerState(pDevice, false);

	}

	Effect::~Effect()
	{
		if (m_pSamplerState) m_pSamplerState->Release();

		if (m_pEffectSamplerVariable) m_pEffectSamplerVariable->Release();

		//if (m_pGlossinessMapVariable) m_pGlossinessMapVariable->Release();
		//
		//if (m_pSpecularMapVariable) m_pSpecularMapVariable->Release();
		//
		//if (m_pNormalMapVariable) m_pNormalMapVariable->Release();

		if (m_pDiffuseMapVariable) m_pDiffuseMapVariable->Release();

		if (m_pMatInverseViewMatrixVariable) m_pMatInverseViewMatrixVariable->Release();

		if (m_pMatWorldMatrixVariable) m_pMatWorldMatrixVariable->Release();

		if (m_pMatWorldViewProjVariable) m_pMatWorldViewProjVariable->Release();

		if (m_pTechnique) m_pTechnique->Release();

		if (m_pEffect) m_pEffect->Release();
	}

	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(
			assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob
		);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << "\n";
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << "\n";
				return nullptr;
			}
		}

		return pEffect;
	}

	void Effect::SetProjectionMatrix(const Matrix& matrix) const
	{
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
	}

	void Effect::SetWorldMatrix(const Matrix& worldMatrix) const
	{
		m_pMatWorldMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
	}

	void Effect::SetInvViewMatrix(const Matrix& invViewMatrix) const
	{
		m_pMatInverseViewMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&invViewMatrix));
	}

	void Effect::SetDiffuseMap(const Texture* pTexture) const
	{

		if (m_pDiffuseMapVariable == nullptr) return;

		m_pDiffuseMapVariable->SetResource(pTexture->GetSRV());

	}
	
}
