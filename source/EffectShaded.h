#pragma once
#include "Effect.h"

namespace dae
{

	class EffectShaded final : public Effect
	{

		public:

			explicit EffectShaded(ID3D11Device* pDevice, const std::wstring& assetFile);

			~EffectShaded() override;

			virtual void SetGlossinessMap(const Texture* pGlossMap) const override;
			virtual void SetNormalMap(const Texture* pNormalMap) const override;
			virtual void SetSpecularMap(const Texture* pSpecularMap) const override;

		private:

			ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
			ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
			ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;

	};

}
