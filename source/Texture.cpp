#include "pch.h"
#include "Texture.h"
#include <SDL_image.h>



namespace dae
{
	Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice)
	{

		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};

		desc.Width = pSurface->w;
		desc.Height = pSurface->h;

		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

		if (FAILED(hr)) return;

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};

		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);

		m_pSurface = pSurface;

		m_pSurfacePixels = static_cast<uint32_t*>(pSurface->pixels);

		//SDL_FreeSurface(pSurface);

	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		// The rgb values in [0, 255] range
		Uint8 r{};
		Uint8 g{};
		Uint8 b{};

		// Calculate the UV coordinates using clamp adressing mode
		const int x{ static_cast<int>(std::clamp(uv.x, 0.0f, 1.0f) * static_cast<float>(m_pSurface->w)) };
		const int y{ static_cast<int>(std::clamp(uv.y, 0.0f, 1.0f) * static_cast<float>(m_pSurface->h)) };

		// Calculate the current pixelIdx on the texture
		const Uint32 pixelIdx{ m_pSurfacePixels[x + y * m_pSurface->w] };

		// Get the r g b values from the current pixel on the texture
		SDL_GetRGB(pixelIdx, m_pSurface->format, &r, &g, &b);

		// The max value of a color attribute
		constexpr float maxColorValue{ 255.0f };

		// Return the color in range [0, 1]
		return ColorRGB{ static_cast<float>(r) / maxColorValue,  static_cast<float>(g) / maxColorValue,  static_cast<float>(b) / maxColorValue };

	}

	Vector3 Texture::SampleVector3(const Vector2& uv) const
	{

		Uint8 r{}, g{}, b{};

		const size_t sampleX{ static_cast<size_t>(uv.x * static_cast<float>(m_pSurface->w)) };
		const size_t sampleY{ static_cast<size_t>(uv.y * static_cast<float>(m_pSurface->h)) };

		const Uint32 pixelIndex{ m_pSurfacePixels[sampleX + sampleY * m_pSurface->w] };

		SDL_GetRGB(pixelIndex, m_pSurface->format, &r, &g, &b);

		const constexpr float invMax{ 1 / 255.f };

		return Vector3{ static_cast<float>(r) * invMax, static_cast<float>(g) * invMax, static_cast<float>(b) * invMax };

	}

	Texture::~Texture()
	{

		if (m_pSRV) m_pSRV->Release();
		if (m_pResource) m_pResource->Release();

		SDL_FreeSurface(m_pSurface);
	}

	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		return new Texture{ IMG_Load(path.c_str()), pDevice };
	}

}