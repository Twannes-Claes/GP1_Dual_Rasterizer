#pragma once
#include "Vector3.h"
//#include "ColorRGB.h"
#include "Effect.h"



namespace dae
{
	//class Effect;
	class Texture;
	class Mesh;

	struct Vertex
	{
		Vector3 position;
		Vector3 normal;
		Vector3 tangent;
		Vector2 uv;
	};

	class Mesh final
	{
	public:

		Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const EffectType typeEffect);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void Render(ID3D11DeviceContext* pDeviceContext) const;

		void SetProjectionMatrix(const Matrix& matrix) const
		{
			m_pEffect->SetProjectionMatrix(m_WorldMatrix * matrix);
		}

		void SetDiffuse(const Texture* pTexture) const
		{
			m_pEffect->SetDiffuseMap(pTexture);
		}

		void SetNormal(const Texture* pTexture) const
		{
			m_pEffect->SetNormalMap(pTexture);
		}

		void SetSpecular(const Texture* pTexture) const 
		{
			m_pEffect->SetSpecularMap(pTexture);
		}

		void SetGlossiness(const Texture* pTexture) const
		{
			m_pEffect->SetGlossinessMap(pTexture);
		}

		void SetWorldMatrix() const
		{
			m_pEffect->SetWorldMatrix(m_WorldMatrix);
		}

		void SetInvViewMatrix(const Matrix& invViewMatrix) const
		{
			m_pEffect->SetInvViewMatrix(invViewMatrix);
		}

		void ToggleSamplerState(ID3D11Device* pDevice) const
		{
			m_pEffect->ToggleSamplerState(pDevice);
		}

		void SetRotationY(const float angle)
		{
			m_WorldMatrix = m_WorldMatrix * Matrix::CreateRotationY(angle) ;
		}

	private:

		Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};

		uint32_t m_NumIndices{};

		Matrix m_WorldMatrix{};

	};
}
