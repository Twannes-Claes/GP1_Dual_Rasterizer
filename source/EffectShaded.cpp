#include "pch.h"
#include "EffectShaded.h"
#include "Texture.h"

namespace dae
{
	EffectShaded::EffectShaded(ID3D11Device* pDevice, const std::wstring& assetFile)
		:Effect{ pDevice, assetFile }
	{
		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();

		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable not valid!\n";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();

		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable not valid!\n";
		}

		m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();

		if (!m_pGlossinessMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVariable not valid!\n";
		}
	}

	EffectShaded::~EffectShaded()
	{
		m_pGlossinessMapVariable->Release();
		m_pNormalMapVariable->Release();
		m_pSpecularMapVariable->Release();
	}

	void EffectShaded::SetGlossinessMap(const Texture* pGlossMap) const
	{
		if (m_pGlossinessMapVariable)
			m_pGlossinessMapVariable->SetResource(pGlossMap->GetSRV());
	}

	void EffectShaded::SetNormalMap(const Texture* pNormalMap) const
	{
		if (m_pNormalMapVariable)
			m_pNormalMapVariable->SetResource(pNormalMap->GetSRV());
	}

	void EffectShaded::SetSpecularMap(const Texture* pSpecularMap) const
	{
		if (m_pSpecularMapVariable)
			m_pSpecularMapVariable->SetResource(pSpecularMap->GetSRV());
	}

}
