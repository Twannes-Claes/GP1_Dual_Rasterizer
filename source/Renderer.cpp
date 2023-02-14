#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();

		if (result == S_OK)
		{
			m_IsInitialized = true;

			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		//create buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);

		m_NrOfPixels = m_Width * m_Height;

		m_pDepthBufferPixels = new float[static_cast<unsigned long long>(m_NrOfPixels)];

		m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);


		m_pCamera = std::make_unique<Camera>();

		m_pCamera->Initialize(m_AspectRatio, 45, Vector3{ 0, 0, -50 });


		InitializeMesh();

		std::cout << "\x1B[2J\x1B[H";//clear console

		PrintInstructions();

		SDL_SetRelativeMouseMode(static_cast<SDL_bool>(m_IsCamLocked));
	}

	Renderer::~Renderer()
	{
		//release resources
		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();
		if (m_pSwapChain) m_pSwapChain->Release();
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();

		//delette buffer and textures
		delete[] m_pDepthBufferPixels;

		delete m_pDiffuseTexture;
		delete m_pNormalTexture;
		delete m_pSpecularTexture;
		delete m_pGlossTexture;
	}

	void Renderer::Update(const Timer* pTimer) const
	{
		//update camera
		m_pCamera->Update(pTimer);

		//update mesh matrices
		m_pMesh->SetProjectionMatrix(m_pCamera->viewMatrix * m_pCamera->projectionMatrix);
		m_pMesh->SetWorldMatrix();
		m_pMesh->SetInvViewMatrix(m_pCamera->invViewMatrix);

		//update fire effect matrices
		m_pFireMesh->SetProjectionMatrix(m_pCamera->viewMatrix * m_pCamera->projectionMatrix);
		m_pFireMesh->SetWorldMatrix();
		m_pFireMesh->SetInvViewMatrix(m_pCamera->invViewMatrix);

		//if is able to rotate rotate matrices of both meshes
		if(m_IsRotating)
		{
			m_pMesh->SetRotationY(m_RotationSpeed * pTimer->GetElapsed());
			m_pFireMesh->SetRotationY(m_RotationSpeed * pTimer->GetElapsed());
		}

	}


	void Renderer::Render()
	{

		if (!m_IsInitialized) return;


		

		if(m_CurrentRasterizerState == RasterizerState::hardware)
		{

			//1. Clear RTV & DSV
			if (m_IsUniform)
			{
				m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &m_ColorsState[static_cast<int>(RasterizerState::uniform)].r);
			}
			else
			{
				m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &m_ColorsState[static_cast<int>(m_CurrentRasterizerState)].r);
			}

			m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

			//2. Set Pipeline + Invoke DrawCalls (==Render)
			m_pMesh->RenderHardware(m_pDeviceContext);

			if (m_CanShowFire)
			{
				m_pFireMesh->RenderHardware(m_pDeviceContext);
			}

			//3. Present Backbuffer (Swap)
			m_pSwapChain->Present(0, 0);
		}

		if(m_CurrentRasterizerState == RasterizerState::software)
		{

			//reset the buffer and background
			ClearDepthBuffer();
			ClearBackGround();

			//Lock BackBuffer
			SDL_LockSurface(m_pBackBuffer);

			//convert vertices from mesh into ndc space and then convert to screenspace
			VertexTransformationFunction();

			switch (m_pMesh->GetPrimitiveTopology())
			{
			case PrimitiveTopology::TriangleList:
			{
				//for each triangle in the mesh
				for (size_t vertexIndex{}; vertexIndex < m_pMesh->GetIndices().size(); vertexIndex += 3)
				{
					RenderTriangle(vertexIndex, false);
				}

			}
			break;

			case PrimitiveTopology::TriangleStrip:
			{
				for (size_t vertexIndex{}; vertexIndex < m_pMesh->GetIndices().size() - 2; ++vertexIndex)
				{
					RenderTriangle(vertexIndex, vertexIndex % 2);
				}
			}
			break;

			}

			//@END 
			//Update SDL Surface
			SDL_UnlockSurface(m_pBackBuffer);
			SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
			SDL_UpdateWindowSurface(m_pWindow);

		}

	}

	void Renderer::RenderTriangle(const size_t& index, const bool swapVertices) const
	{
		//calculate the indexes of the vertices of the triangle
		const size_t index0{ m_pMesh->GetIndices()[index] };
		const size_t index1{ m_pMesh->GetIndices()[index + 1 + swapVertices] };
		const size_t index2{ m_pMesh->GetIndices()[index + 1 + !swapVertices] };

		//has same index twice return
		if (index0 == index1 || index1 == index2 || index0 == index2) return;

		//get the vertex of the indexes
		const Vertex_Out vertex_OutV0{ m_Vertices_Out[index0] };
		const Vertex_Out vertex_OutV1{ m_Vertices_Out[index1] };
		const Vertex_Out vertex_OutV2{ m_Vertices_Out[index2] };

		//if out of frustrum return
		if (IsOutOfFrustrum(vertex_OutV0) || IsOutOfFrustrum(vertex_OutV1) || IsOutOfFrustrum(vertex_OutV2)) return;

		//calc vertices
		const Vector2 v0{ m_Vertices_ScreenSpace[index0] };
		const Vector2 v1{ m_Vertices_ScreenSpace[index1] };
		const Vector2 v2{ m_Vertices_ScreenSpace[index2] };

		//calculate the edges of the triangle
		const Vector2 edgeV0V1{ v1 - v0 };
		const Vector2 edgeV1V2{ v2 - v1 };
		const Vector2 edgeV2V0{ v0 - v2 };

		//calc the inverse area of the triangle
		const float invTriangleArea{ 1 / Vector2::Cross(edgeV0V1, edgeV1V2) };

		//calc bounding box
		AABB boundingBox
		{
			Vector2::Min(v0, Vector2::Min(v1, v2)),
			Vector2::Max(v0, Vector2::Max(v1, v2))
		};

		boundingBox.minAABB.Clamp(static_cast<float>(m_Width), static_cast<float>(m_Height));
		boundingBox.maxAABB.Clamp(static_cast<float>(m_Width), static_cast<float>(m_Height));

		// calc the start and end of of the pixels of the triangle
		const int minX{ std::clamp(static_cast<int>(boundingBox.minAABB.x - m_BoundingMargin),0, m_Width) };
		const int minY{ std::clamp(static_cast<int>(boundingBox.minAABB.y - m_BoundingMargin),0, m_Height) };

		const int maxX{ std::clamp(static_cast<int>(boundingBox.maxAABB.x + m_BoundingMargin),0, m_Width) };
		const int maxY{ std::clamp(static_cast<int>(boundingBox.maxAABB.y + m_BoundingMargin),0, m_Height) };

		for (int px{ minX }; px < maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				//calc index of the current pixel
				const int pixelIndex{ px + py * m_Width };

				//only render the pixels of the bounding box
				if (m_ShowBoundingBoxes)
				{
					m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255));
					continue;
				}
				//calc current pixel
				const Vector2 point{ static_cast<float>(px), static_cast<float>(py) };

				//calculate vector between point and the edges
				const Vector2 pointToEdgeSide1{ point - v0 };
				float edge0{ Vector2::Cross(edgeV0V1, pointToEdgeSide1) };

				const Vector2 pointToEdgeSide2{ point - v1 };
				float edge1{ Vector2::Cross(edgeV1V2, pointToEdgeSide2) };

				const Vector2 pointToEdgeSide3{ point - v2 };
				float edge2{ Vector2::Cross(edgeV2V0, pointToEdgeSide3) };

				//cullmode check
				if (!CheckValidCullCrosses(edge0, edge1, edge2)) continue;

				//calc barycentric weights
				const float weightV0{ edge1 * invTriangleArea };
				const float weightV1{ edge2 * invTriangleArea };
				const float weightV2{ edge0 * invTriangleArea };

				//calc barycentric depths
				float invDepthV0{ CalculateDepth(vertex_OutV0, false) };
				float invDepthV1{ CalculateDepth(vertex_OutV1, false) };
				float invDepthV2{ CalculateDepth(vertex_OutV2, false) };

				//calc z depth
				const float interpolateDepthZ{ CalculateInterpolateDepth(weightV0, weightV1, weightV2, invDepthV0, invDepthV1, invDepthV2) };

				//if current buffer is less than the z depth continue
				if (m_pDepthBufferPixels[pixelIndex] < interpolateDepthZ) continue;

				//save the new depth
				m_pDepthBufferPixels[pixelIndex] = interpolateDepthZ;

				ColorRGB finalColor{};

				//remap z depth when showing depth and output the depth as color
				if (m_ShowDepthBuffer)
				{
					const float colorDepth{ Remap(interpolateDepthZ, 0.997f, 1.0f) };
					finalColor = { colorDepth, colorDepth, colorDepth };
				}
				else
				{
					Vertex_Out pixelInformation{};

					//calculate w depth
					invDepthV0 = CalculateDepth(vertex_OutV0, true);
					invDepthV1 = CalculateDepth(vertex_OutV1, true);
					invDepthV2 = CalculateDepth(vertex_OutV2, true);

					const float interpolateDepthW{ CalculateInterpolateDepth(weightV0, weightV1, weightV2, invDepthV0, invDepthV1, invDepthV2) };

					//calculate the uv of the current pixel
					const Vector2 uvPixel
					{
							(CalcUVComponent(weightV0, invDepthV0, index0)
						+ CalcUVComponent(weightV1, invDepthV1, index1)
						+ CalcUVComponent(weightV2, invDepthV2, index2))
						* interpolateDepthW
					};

					//save it to the uv
					pixelInformation.uv = uvPixel;

					//calculate the rest of the pixelInformation

					InterpolatePixelInfo(pixelInformation, vertex_OutV0, vertex_OutV1, vertex_OutV2, weightV0, weightV1, weightV2, interpolateDepthW);

					//calculate shading of currennt pixel
					PixelShading(pixelInformation, finalColor);

				}

				//show pixel to screen with given color
				ConvertColorToPixel(finalColor, pixelIndex);

			}
		}
	}

	void Renderer::PixelShading(const Vertex_Out& vOut, ColorRGB& finalColor) const
	{
		//store normal
		Vector3 sampledNormal{ vOut.normal };

		if (m_ShowNormal)
		{
			//calc binormal
			const Vector3 binormal{ Vector3::Cross(vOut.normal, vOut.tangent) };
			//create matrix out of tangent binormal and normal
			const Matrix tangentSpaceAxis{ vOut.tangent, binormal.Normalized(), vOut.normal, Vector3::Zero };

			//sample color of the uv of the texture and clamp it between -1 and 1
			sampledNormal = m_pNormalTexture->SampleVector3(vOut.uv);
			sampledNormal = 2 * sampledNormal - Vector3::Identity;

			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);

			sampledNormal.Normalize();
		}

		//calc observedArea
		const float observedArea{ Vector3::ClampDot(sampledNormal, -m_LightDir) };

		switch (m_CurrentSoftwareMode)
		{
			case SoftwareModes::ObservedArea:
			{
				finalColor = colors::White * observedArea;
			}
			break;
			case SoftwareModes::Diffuse:
			{
					//calc lamber shader with  the observer area and lightintensity
				finalColor = (m_pDiffuseTexture->Sample(vOut.uv) * m_KD / PI) * m_LightIntensity * observedArea;
			}
			break;
			case SoftwareModes::Specular:
			{
				//calc calc color of the specular
				const ColorRGB specularColor{ CalculateSpecular(sampledNormal, vOut) };

				finalColor = specularColor * observedArea;
			}
			break;
			case SoftwareModes::Combined:
			{
				//sum them all up to combine them
				const ColorRGB specularColor{ CalculateSpecular(sampledNormal, vOut) };

				const ColorRGB diffuseColor{ (m_pDiffuseTexture->Sample(vOut.uv) * m_KD / PI) * m_LightIntensity };

				finalColor = diffuseColor * observedArea + specularColor;
			}
			break;
		}

		finalColor += m_AmbientColor;
	}

	bool Renderer::CheckValidCullCrosses(const float edge01, const float edge02, const float edge03) const
	{

		switch (m_CurrentCullMode)
		{
			case CullMode::back:
			{
				return (edge01 > 0 && edge02 > 0 && edge03 > 0);
			}
			break;
			case CullMode::front:
			{
				return (edge01 < 0 && edge02 < 0 && edge03 < 0);
			}
			break;
			case CullMode::none:
			{
				return (edge01 > 0 && edge02 > 0 && edge03 > 0) || (edge01 < 0 && edge02 < 0 && edge03 < 0);
			}
			break;
		}

		return false;
	}

	void Renderer::VertexTransformationFunction()
	{
		//clear the vertices
		m_Vertices_ScreenSpace.clear();

		m_Vertices_Out.clear();

		//calc transform matrix of the mesh
		const Matrix worldViewProjectionMatrix{ m_pMesh->GetWorldMatrix() * m_pCamera->viewMatrix * m_pCamera->projectionMatrix };

		for (const Vertex& vertex : m_pMesh->GetVertices())
		{
			//calc viewDirection
			Vector3 viewDirection{ m_pMesh->GetWorldMatrix().TransformPoint(vertex.position) - m_pCamera->origin };
			viewDirection.Normalize();

			//fill in vertex information
			Vertex_Out temp
			{
				//transform vertex with the matrix
				worldViewProjectionMatrix.TransformPoint({vertex.position, 1.f}),
				//transform normal and tangent of the vertex
				m_pMesh->GetWorldMatrix().TransformVector(vertex.normal).Normalized(),
				m_pMesh->GetWorldMatrix().TransformVector(vertex.tangent).Normalized(),
				vertex.uv,
				vertex.color,
				viewDirection
			};
			//perspective divide
			//divide position by w
			temp.position.x /= temp.position.w;
			temp.position.y /= temp.position.w;
			temp.position.z /= temp.position.w;

			//add to the vertices_out
			m_Vertices_Out.emplace_back(temp);
		}

		//calc ndc to raster space
		for (const Vertex_Out& vertice : m_Vertices_Out)
		{
			Vector2 v{
			((vertice.position.x + 1) / 2) * static_cast<float>(m_Width),
			((1 - vertice.position.y) / 2) * static_cast<float>(m_Height) };

			m_Vertices_ScreenSpace.emplace_back(v);
		}

	}

	Vector2 Renderer::CalcUVComponent(const float weight, const float invDepth, const size_t& index) const
	{
		return (weight * m_pMesh->GetVertices()[index].uv) * invDepth;
	}

	ColorRGB Renderer::CalculateSpecular(const Vector3& sampledNormal, const Vertex_Out& v) const
	{
		//get direction of reflection
		const Vector3 reflectDirection{ Vector3::Reflect(m_LightDir, sampledNormal) };
		//get angle of reflection
		const float reflectionAngle{ Vector3::ClampDot(reflectDirection, -v.viewDirection) };

		//calc phong exponent
		const float glossExponent{ m_pGlossTexture->Sample(v.uv).r * m_Shinyness };
		//calc phong value
		const float phong{ powf(reflectionAngle, glossExponent) };
	
		return m_pSpecularTexture->Sample(v.uv) * phong;
	}

	void Renderer::InterpolatePixelInfo(Vertex_Out& pixelInfo, const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const float w0, const float w1, const float w2, const float depth) const
	{

		pixelInfo.normal =
		{

			((((v0.normal / v0.position.w) * w0) +
			((v1.normal / v1.position.w) * w1) +
			((v2.normal / v2.position.w) * w2)) * depth).Normalized()

		};
		pixelInfo.tangent =
		{

			((((v0.tangent / v0.position.w) * w0) +
			((v1.tangent / v1.position.w) * w1) +
			((v2.tangent / v2.position.w) * w2)) * depth).Normalized()

		};
		pixelInfo.viewDirection =
		{

			((((v0.viewDirection / v0.position.w) * w0) +
			((v1.viewDirection / v1.position.w) * w1) +
			((v2.viewDirection / v2.position.w) * w2)) * depth).Normalized()

		};

	}

	float Renderer::CalculateInterpolateDepth(const float w0, const float w1, const float w2, const float d0, const float d1, const float d2) const
	{
		return 1 / (w0 * d0 + w1 * d1 + w2 * d2);
	}

	float Renderer::CalculateDepth(const Vertex_Out& v, const bool usingAxisW) const
	{
		return 1 / v.position[2 + usingAxisW];
	}

	void Renderer::ConvertColorToPixel(ColorRGB& finalColor, const int pixelIndex) const
	{
		finalColor.MaxToOne();

		m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
			static_cast<uint8_t>(finalColor.r * 255),
			static_cast<uint8_t>(finalColor.g * 255),
			static_cast<uint8_t>(finalColor.b * 255));
	}

	bool Renderer::IsOutOfFrustrum(const Vertex_Out& vOut) const
	{
		return (vOut.position.x < -1 || vOut.position.x > 1) || (vOut.position.y < -1 || vOut.position.y > 1) || (vOut.position.z < 0 || vOut.position.z > 1);
	}

	void Renderer::InitializeMesh()
	{
		//initialize mesh data & mesh

		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		m_pMesh = std::make_unique<Mesh>(m_pDevice, vertices, indices, EffectType::shaded);

		Texture* pTexture{ Texture::LoadFromFile("Resources/vehicle_diffuse.png", m_pDevice) };

		m_pMesh->SetDiffuse(pTexture);

		m_pDiffuseTexture = pTexture;

		//delete pTexture;

		pTexture = Texture::LoadFromFile("Resources/vehicle_normal.png", m_pDevice);

		m_pMesh->SetNormal(pTexture);
		m_pNormalTexture = pTexture;

		//delete pTexture;

		pTexture = Texture::LoadFromFile("Resources/vehicle_specular.png", m_pDevice);

		m_pMesh->SetSpecular(pTexture);
		m_pSpecularTexture = pTexture;

		//delete pTexture;

		pTexture = Texture::LoadFromFile("Resources/vehicle_gloss.png", m_pDevice);

		m_pMesh->SetGlossiness(pTexture);
		m_pGlossTexture = pTexture;

		//delete pTexture;


		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		m_pFireMesh = std::make_unique<Mesh>(m_pDevice, vertices, indices, EffectType::transparent);

		pTexture = Texture::LoadFromFile("Resources/fireFX_diffuse.png", m_pDevice);

		m_pFireMesh->SetDiffuse(pTexture);

		delete pTexture;
	}

	HRESULT Renderer::InitializeDirectX()
	{

		//1. Create Device & DeviceContext
		D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)

		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

#endif

		HRESULT result{
			D3D11CreateDevice(
				nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, &featureLevel,
				1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext
			)
		};
		if (FAILED(result)) return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};

		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result)) return result;

		//2. Create SwapChain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version)
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result)) return result;

		//Release DXGI Factory
		if (pDxgiFactory) pDxgiFactory->Release();

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result)) return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))return result;

		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result)) return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) return result;

		//5. Bind RTV & DSV to Output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set Viewport
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;

	}

}
