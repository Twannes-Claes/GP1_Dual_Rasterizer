#pragma once
#include <SDL_surface.h>
#include <string>
//#include "ColorRGB.h"
//#include "Vector3.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);

		ID3D11ShaderResourceView* GetSRV() const { return m_pSRV; }

		// Software Rasterizer
		ColorRGB Sample(const Vector2& uv) const;

		Vector3 SampleVector3(const Vector2& uv) const;

	private:

		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		// Software Rasterizer
		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };

		//hardware Rasterizer
		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pSRV{};
	};
}