#pragma once
#include "Effect.h"

namespace dae
{

	class EffectTransparent final : public Effect
	{

		public:

			EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile);

			virtual  ~EffectTransparent() override = default;

		private:

		virtual void SetNormalMap(const Texture* pTexture) const override{};;
		virtual void SetSpecularMap(const Texture* pTexture) const override{};
		virtual void SetGlossinessMap(const Texture* pTexture) const override{};

	};

}

