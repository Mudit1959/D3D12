#include "Game.h"
#include "Graphics.h"
#include "BufferStructs.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

std::vector<std::shared_ptr<Entity>> entities;
std::shared_ptr<Material> wood, diamond, metal46, metal49;
std::shared_ptr<Camera> camera;
// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	CreateRootSigAndPipelineState();
	CreateGeometry();
	

	camera = std::make_shared<Camera>(0, 0, -10.0f, Window::AspectRatio(), true);
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for GPU before shut down
	Graphics::WaitForGPU();
}


// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr <ID3DBlob > vertexShaderByteCode;
	Microsoft::WRL::ComPtr <ID3DBlob > pixelShaderByteCode;
	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(
			FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(
			FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}
	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION"; // Name must match semantic in shader
		inputElements[0].SemanticIndex = 0; // This is the first POSITION semantic

		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT; // R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0; // This is the first TEXCOORD semantic

		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0; // This is the first NORMAL semantic

		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0; // This is the first TANGENT semantic
	}
	// Root Signature
	{
		// Define a range of CBV's (constant buffer views) for VERTEX SHADER
		D3D12_DESCRIPTOR_RANGE cbvRangeVS = {};
		cbvRangeVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangeVS.NumDescriptors = 1;
		cbvRangeVS.BaseShaderRegister = 0;
		cbvRangeVS.RegisterSpace = 0;
		cbvRangeVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Define a range of CBV's (constant buffer views) for PIXEL SHADER
		D3D12_DESCRIPTOR_RANGE cbvRangePS = {};
		cbvRangePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangePS.NumDescriptors = 1;
		cbvRangePS.BaseShaderRegister = 0;
		cbvRangePS.RegisterSpace = 0;
		cbvRangePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Define root parameter to access an unbounded space for TEXTURES in PIXEL SHADER
		D3D12_DESCRIPTOR_RANGE bindlessRange{};
		bindlessRange.BaseShaderRegister = 0; // Matches t0 in shader
		bindlessRange.RegisterSpace = 0; // Matches space0 in shader
		bindlessRange.NumDescriptors = -1; // All (or Graphics::MaxTextureDescriptors)
		bindlessRange.OffsetInDescriptorsFromTableStart = 0;
		bindlessRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;


		D3D12_ROOT_PARAMETER rootParams[3] = {};


		// Define the pointer to a descriptor table for the VERTEX SHADER
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvRangeVS;

		// Define the pointer to a descriptor table for the PIXEL SHADER
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvRangePS;

		// Define the pointer to a descriptor table for TEXTURES in the PIXEL SHADER
		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &bindlessRange;

		// Create a single static sampler (available to all pixel shaders)
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // Means register(s0) in the shader
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe the full root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;


		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;
		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);
		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors -> GetBufferPointer());
		}
		// Actually create the root sig
		Graphics::Device -> CreateRootSignature(
			0,
			serializedRootSig -> GetBufferPointer(),
			serializedRootSig -> GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}
	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options
		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();
		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode -> GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode -> GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode -> GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode -> GetBufferSize();
		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;
		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask =
			D3D12_COLOR_WRITE_ENABLE_ALL;
		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;
		// Create the pipe state object
		Graphics::Device -> CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));
	}
	// Set up the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	CreateMaterials();

	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>("Cube", FixPath("../../Assets/Meshes/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinder = std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/Meshes/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>("Helix", FixPath("../../Assets/Meshes/helix.obj").c_str());
	std::shared_ptr<Mesh> plane = std::make_shared<Mesh>("Plane", FixPath("../../Assets/Meshes/quad.obj").c_str());
	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>("Quad", FixPath("../../Assets/Meshes/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>("Sphere", FixPath("../../Assets/Meshes/sphere.obj").c_str());
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>("Torus", FixPath("../../Assets/Meshes/torus.obj").c_str());

	entities.push_back(std::make_shared<Entity>(helix, wood));
	entities.push_back(std::make_shared<Entity>(sphere, metal46));
	entities.push_back(std::make_shared<Entity>(torus, diamond));

	entities[0]->GetTransform()->SetPosition(0, 0, 0);

	entities[1]->GetTransform()->SetPosition(-3, 0, 0);

	entities[2]->GetTransform()->SetPosition(3, 0, 0);

}

void Game::CreateMaterials() 
{
	unsigned int woodAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/wood_albedo.png").c_str());
	unsigned int woodNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/wood_normal.png").c_str());
	unsigned int woodRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/wood_roughness.png").c_str());
	

	unsigned int diamondAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/diamond_albedo.png").c_str());
	unsigned int diamondNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/diamond_normal.png").c_str());
	unsigned int diamondRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/diamond_roughness.png").c_str());
	unsigned int diamondMetalness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/diamond_metalness.png").c_str());

	unsigned int metal46_Albedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal46_albedo.png").c_str());
	unsigned int metal46_Normal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal46_normal.png").c_str());
	unsigned int metal46_Roughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal46_roughness.png").c_str());
	unsigned int metal46_Metalness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal46_metalness.png").c_str());

	unsigned int metal49_Albedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal49_albedo.png").c_str());
	unsigned int metal49_Normal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal49_normal.png").c_str());
	unsigned int metal49_Roughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal49_roughness.png").c_str());
	unsigned int metal49_Metalness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/metal49_metalness.png").c_str());

	wood = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT3(1,1,1));
	wood->SetAlbedoIndex(woodAlbedo);
	wood->SetNormalMapIndex(woodNormal);
	wood->SetRoughnessIndex(woodRoughness);

	diamond = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT3(1, 1, 1));
	diamond->SetAlbedoIndex(diamondAlbedo);
	diamond->SetNormalMapIndex(diamondNormal);
	diamond->SetRoughnessIndex(diamondRoughness);
	diamond->SetMetalnessIndex(diamondMetalness);

	metal46 = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT3(1, 1, 1));
	metal46->SetAlbedoIndex(metal46_Albedo);
	metal46->SetNormalMapIndex(metal46_Normal);
	metal46->SetRoughnessIndex(metal46_Roughness);
	metal46->SetMetalnessIndex(metal46_Metalness);

	metal49 = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT3(1, 1, 1));
	metal49->SetAlbedoIndex(metal49_Albedo);
	metal49->SetNormalMapIndex(metal49_Normal);
	metal49->SetRoughnessIndex(metal49_Roughness);
	metal49->SetMetalnessIndex(metal49_Metalness);

}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Resize the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}

	if (camera != NULL) { camera->UpdateProjMatrix(Window::AspectRatio()); }
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	camera->Update(deltaTime);
	
	//"auto& to meaningfully modify items in a sequence", such as a vector -> https://stackoverflow.com/questions/29859796/c-auto-vs-auto
	for (auto& e : entities) 
	{
		e->GetTransform()->Rotate(0, deltaTime, 0);
	}

	
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr <ID3D12Resource > currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];
	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList -> ResourceBarrier(1, &rb);
		// Background color (Cornflower Blue in this case) for clearing
		float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };
		// Clear the RTV
		Graphics::CommandList -> ClearRenderTargetView(
			Graphics::RTVHandles[Graphics::SwapChainIndex()],
			color,
			0, 0); // No scissor rectangles
		// Clear the depth buffer, too
		Graphics::CommandList -> ClearDepthStencilView(
			Graphics::DSVHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f, // Max depth = 1.0f
			0, // Not clearing stencil, but need a value
			0, 0); // No scissor rects
	}
	
	// Rendering here!
	{
		// Set overall pipeline state -> prone to change depending on object
		Graphics::CommandList -> SetPipelineState(pipelineState.Get());

		// Set the CBV/SRV Descriptor Heap -> must happen before root signature
		Graphics::CommandList->SetDescriptorHeaps(1, Graphics::CBVSRVDescriptorHeap.GetAddressOf());

		// Now bind the beginning of all the SRVs - partial binding 
		// Navigate to start of the CBV/SRV buffer -> skip past reserved space for constants to get to textures
		D3D12_GPU_DESCRIPTOR_HANDLE startInGPU = Graphics::CBVSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		startInGPU.ptr += (Graphics::MaxConstantBuffers * Graphics::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		Graphics::CommandList->SetGraphicsRootDescriptorTable(2, startInGPU);

		// Root sig (must happen before root descriptor table)
		Graphics::CommandList -> SetGraphicsRootSignature(rootSignature.Get());
		
		// Set up other commands for rendering
		Graphics::CommandList -> OMSetRenderTargets(
		1, &Graphics::RTVHandles[Graphics::SwapChainIndex()], true , &Graphics::DSVHandle);

		
		Graphics::CommandList -> RSSetViewports(1, &viewport);
		Graphics::CommandList -> RSSetScissorRects(1, &scissorRect);
		Graphics::CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto& e : entities) 
		{
			VSConstants vsData = {};
			vsData.world = e->GetTransform()->GetWorldMatrix();
			vsData.worldInv = e->GetTransform()->GetWorldInverseTransposeMatrix();
			vsData.view = camera->GetView();
			vsData.proj = camera->GetProj();

			D3D12_GPU_DESCRIPTOR_HANDLE vsDataInCBHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
				(void*)&vsData, sizeof(VSConstants)
			);

			Graphics::CommandList->SetGraphicsRootDescriptorTable(0, vsDataInCBHandle);

			PSConstants psData = {};
			psData.albedoIndex = e->GetMaterial()->GetAlbedoIndex();
			psData.normalIndex = e->GetMaterial()->GetNormalMapIndex();
			psData.roughnessIndex = e->GetMaterial()->GetRoughnessIndex();
			psData.metalnessIndex = e->GetMaterial()->GetMetalnessIndex();
			psData.UVOffset = e->GetMaterial()->GetOffset();
			psData.UVScale = e->GetMaterial()->GetScale();
			psData.cameraWorldPos = camera->GetPos();

			D3D12_GPU_DESCRIPTOR_HANDLE psDataInCBHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
				(void*)&psData, sizeof(PSConstants)
			);

			Graphics::CommandList->SetGraphicsRootDescriptorTable(1, psDataInCBHandle);

			std::shared_ptr<Mesh> mesh = e->GetMesh();
			D3D12_VERTEX_BUFFER_VIEW vbView = mesh->GetVBView();
			D3D12_INDEX_BUFFER_VIEW ibView = mesh->GetIBView();

			Graphics::CommandList->SetPipelineState(e->GetMaterial()->GetPipelineState().Get());
			Graphics::CommandList->IASetVertexBuffers(0, 1, &vbView);
			Graphics::CommandList->IASetIndexBuffer(&ibView);

			Graphics::CommandList->DrawIndexedInstanced((UINT)mesh->GetIndexCount(), 1, 0, 0, 0);
		}
		
		
		// Draw
		Graphics::CommandList -> DrawIndexedInstanced(3, 1, 0, 0, 0);
	}

	// Present
	{
		// Transition back to present
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList -> ResourceBarrier(1, &rb);
		// Must occur BEFORE present
		Graphics::CloseAndExecuteCommandList();
		// Present the current back buffer and move to the next one
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain -> Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
		Graphics::AdvanceSwapChainIndex();
		// Waits for the GPU to be done -> Handled by multi-frame sync in AdvanceSwapChainIndex()!
		// Resets the command list & allocator
		// Graphics::WaitForGPU();
		Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
	}
}



