#pragma once
#include "Vector3.h"
//#include "ColorRGB.h"
#include "Effect.h"
#include "DataTypes.h"



namespace dae
{
	class Texture;

	class Mesh final
	{
	public:

		Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const EffectType typeEffect);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void RenderHardware(ID3D11DeviceContext* pDeviceContext) const;

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

		Matrix& GetWorldMatrix() { return m_WorldMatrix; }

		void SetInvViewMatrix(const Matrix& invViewMatrix) const
		{
			m_pEffect->SetInvViewMatrix(invViewMatrix);
		}

		void ToggleSamplerState(ID3D11Device* pDevice) const
		{
			m_pEffect->ToggleSamplerState(pDevice);
		}

		void ToggleCullMode(ID3D11Device* pDevice) const
		{
			m_pEffect->ToggleCullMode(pDevice);
		}

		void SetRotationY(const float angle)
		{
			m_WorldMatrix = m_WorldMatrix * Matrix::CreateRotationY(angle) ;
		}

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<uint32_t>& GetIndices() { return m_Indices; }

		PrimitiveTopology GetPrimitiveTopology() const { return m_PrimitiveTopology; }

	private:

		Effect* m_pEffect{};

		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};

		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};

		uint32_t m_NumIndices{};

		Matrix m_WorldMatrix{};

		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };

	};
}
