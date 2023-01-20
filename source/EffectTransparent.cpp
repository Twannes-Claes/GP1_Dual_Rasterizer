#include "pch.h"
#include "EffectTransparent.h"

namespace dae
{

	EffectTransparent::EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile)
	:Effect{ pDevice, assetFile }
	{
	}

}
