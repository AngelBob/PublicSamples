//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "D3D12PerlinTriangle.h"

D3D12PerlinTriangle::D3D12PerlinTriangle( UINT width, UINT height, std::wstring name ) :
    DXSample( width, height, name ),
    m_frameIndex( 0 ),
    m_viewport( 0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ) ),
    m_scissorRect( 0, 0, static_cast<LONG>( width ), static_cast<LONG>( height ) ),
    m_rtvDescriptorSize( 0 ),
    m_vertexBufferSize( m_triangleVertices.size() * sizeof( Vertex ) )
{
}

void D3D12PerlinTriangle::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12PerlinTriangle::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugController ) ) ) )
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed( CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &factory ) ) );

    if( m_useWarpDevice )
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed( factory->EnumWarpAdapter( IID_PPV_ARGS( &warpAdapter ) ) );
        ThrowIfFailed( D3D12CreateDevice( warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_device ) ) );
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter( factory.Get(), &hardwareAdapter );
        ThrowIfFailed( D3D12CreateDevice( hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_device ) ) );
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed( m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &m_commandQueue ) ) );

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed( factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ) );

    // This sample does not support full screen transitions.
    ThrowIfFailed( factory->MakeWindowAssociation( Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER ) );

    ThrowIfFailed( swapChain.As( &m_swapChain ) );
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed( m_device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &m_rtvHeap ) ) );

        // Describe and create a depth stencil view (DSV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed( m_device->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS( &m_dsvHeap ) ) );

        // Describe and create a shader resource view (SRV) heap for the texture.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed( m_device->CreateDescriptorHeap( &srvHeapDesc, IID_PPV_ARGS( &m_srvHeap ) ) );

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_rtvHeap->GetCPUDescriptorHandleForHeapStart() );

        // Create a RTV for each frame.
        for( UINT n = 0; n < FrameCount; n++ )
        {
            ThrowIfFailed( m_swapChain->GetBuffer( n, IID_PPV_ARGS( &m_renderTargets[ n ] ) ) );
            m_device->CreateRenderTargetView( m_renderTargets[ n ].Get(), nullptr, rtvHandle );
            rtvHandle.Offset( 1, m_rtvDescriptorSize );
        }
    }

    ThrowIfFailed( m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS( &m_commandAllocator ) ) );
}

// Create the pipeline state objects
void D3D12PerlinTriangle::CreateObjectPSO()
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed( D3DCompileFromFile(
        GetAssetFullPath( L"shaders.hlsl" ).c_str(),
        nullptr,
        nullptr,
        "VSMainObject",
        "vs_5_0",
        compileFlags,
        0,
        &vertexShader,
        nullptr ) );

    ThrowIfFailed( D3DCompileFromFile(
        GetAssetFullPath( L"shaders.hlsl" ).c_str(),
        nullptr,
        nullptr,
        "PSMainObject",
        "ps_5_0",
        compileFlags,
        0,
        &pixelShader,
        nullptr ) );

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof( inputElementDescs ) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE( vertexShader.Get() );
    psoDesc.PS = CD3DX12_SHADER_BYTECODE( pixelShader.Get() );
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT );
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed( m_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &m_psoSquare ) ) );
}

void D3D12PerlinTriangle::CreateBackgroundPSO()
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed( D3DCompileFromFile(
        GetAssetFullPath( L"shaders.hlsl" ).c_str(),
        nullptr,
        nullptr,
        "VSMainBackground",
        "vs_5_0",
        compileFlags,
        0,
        &vertexShader,
        nullptr ) );

    ThrowIfFailed( D3DCompileFromFile(
        GetAssetFullPath( L"shaders.hlsl" ).c_str(),
        nullptr,
        nullptr,
        "PSMainBackground",
        "ps_5_0",
        compileFlags,
        0,
        &pixelShader,
        nullptr ) );

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE( vertexShader.Get() );
    psoDesc.PS = CD3DX12_SHADER_BYTECODE( pixelShader.Get() );
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT );
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed( m_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &m_psoBackground ) ) );
}

// Load the sample assets.
void D3D12PerlinTriangle::LoadAssets()
{
    // Create the depth stencil buffer
    {
        D3D12_RESOURCE_DESC depthResourceDesc = {};
        depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthResourceDesc.Width = m_width;
        depthResourceDesc.Height = m_height;
        depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        depthResourceDesc.DepthOrArraySize = 1;
        depthResourceDesc.MipLevels = 1;
        depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthResourceDesc.SampleDesc.Count = 1;
        depthResourceDesc.SampleDesc.Quality = 0;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = depthResourceDesc.Format;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
            D3D12_HEAP_FLAG_NONE,
            &depthResourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS( &m_depthStencilBuffer )
        );

        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
        depthStencilViewDesc.Texture2D.MipSlice = 0;

        m_device->CreateDepthStencilView(
            m_depthStencilBuffer.Get(),
            &depthStencilViewDesc,
            m_dsvHeap->GetCPUDescriptorHandleForHeapStart() );
    }

    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports.
        // If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if( FAILED( m_device->CheckFeatureSupport( D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof( featureData ) ) ) )
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[ 1 ] = {};
        ranges[ 0 ].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC );

        CD3DX12_ROOT_PARAMETER1 rootParameters[ 1 ] = {};
        rootParameters[ 0 ].InitAsDescriptorTable( 1, &ranges[ 0 ], D3D12_SHADER_VISIBILITY_PIXEL );

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(
            _countof( rootParameters ),
            rootParameters,
            1,
            &sampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed( D3DX12SerializeVersionedRootSignature(
            &rootSignatureDesc,
            featureData.HighestVersion,
            &signature,
            &error ) );

        ThrowIfFailed( m_device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS( &m_rootSignature ) ) );
    }

    // Create the pipeline states, which includes compiling and loading shaders.
    CreateObjectPSO();
    CreateBackgroundPSO();

    // Create the command list.
    ThrowIfFailed( m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator.Get(),
        m_psoSquare.Get(),
        IID_PPV_ARGS( &m_commandList ) ) );

    // Create the vertex buffer.
    {
        // Define the geometry for a pyramid.
        // Must use a tri list because each side of the pyramid will be rotated
        // separately.
        m_triangleVertices =
        {{
            // Front face
            {{  0.0f,   0.25f * m_aspectRatio,  0.0f  }, { 1.0f, 0.0f, 0.0f, 1.0f }},
            {{ -0.25f, -0.25f * m_aspectRatio,  0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f }},
            {{  0.25f, -0.25f * m_aspectRatio,  0.25f }, { 0.0f, 1.0f, 1.0f, 1.0f }},

            // Right face
            {{  0.0f,   0.25f * m_aspectRatio,  0.0f  }, { 1.0f, 0.0f, 0.0f, 1.0f }},
            {{  0.25f, -0.25f * m_aspectRatio,  0.25f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
            {{  0.25f, -0.25f * m_aspectRatio, -0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f }},

            // Back face
            {{  0.0f,   0.25f * m_aspectRatio,  0.0f  }, { 1.0f, 0.0f, 0.0f, 1.0f }},
            {{  0.25f, -0.25f * m_aspectRatio, -0.25f }, { 0.0f, 1.0f, 0.0f, 1.0f }},
            {{ -0.25f, -0.25f * m_aspectRatio, -0.25f }, { 0.0f, 1.0f, 1.0f, 1.0f }},

            // Left face
            {{  0.0f,   0.25f * m_aspectRatio,  0.0f  }, { 1.0f, 0.0f, 0.0f, 1.0f }},
            {{ -0.25f, -0.25f * m_aspectRatio, -0.25f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
            {{ -0.25f, -0.25f * m_aspectRatio,  0.25f }, { 0.0f, 0.0f, 1.0f, 1.0f }},
        }};

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        ThrowIfFailed( m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( m_vertexBufferSize ),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS( &m_vertexBuffer ) ) );

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange( 0, 0 );        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed( m_vertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) ) );
        memcpy( pVertexDataBegin, m_triangleVertices.data(), m_vertexBufferSize );
        m_vertexBuffer->Unmap( 0, nullptr );

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof( Vertex );
        m_vertexBufferView.SizeInBytes = static_cast<UINT>( m_vertexBufferSize );
    }

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    ComPtr<ID3D12Resource> textureUploadHeap;

    // Create the texture.
    {
        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = TextureWidth;
        textureDesc.Height = TextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed( m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS( &m_texture ) ) );

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize( m_texture.Get(), 0, 1 );

        // Create the GPU upload buffer.
        ThrowIfFailed( m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( uploadBufferSize ),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS( &textureUploadHeap ) ) );

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        /*
        std::vector<UINT8> texture = GenerateTextureData();
        /*/
        std::vector<UINT8> texture = GeneratePerlinTextureData();
        //*/

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[ 0 ];
        textureData.RowPitch = TextureWidth * TexturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        UpdateSubresources( m_commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData );
        m_commandList->ResourceBarrier(
            1,
            &CD3DX12_RESOURCE_BARRIER::Transition(
                m_texture.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) );

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView( m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart() );
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed( m_commandList->Close() );
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed( m_device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) ) );
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
        if( m_fenceEvent == nullptr )
        {
            ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> D3D12PerlinTriangle::GenerateTextureData()
{
    const UINT rowPitch = TextureWidth * TexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;      // The width of a cell in the checkerboard texture.
    const UINT cellHeight = TextureWidth >> 3; // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * TextureHeight;

    std::vector<UINT8> data( textureSize );
    UINT8* pData = &data[ 0 ];

    for( UINT n = 0; n < textureSize; n += TexturePixelSize )
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if( i % 2 == j % 2 )
        {
            pData[ n ] = 0x00;        // R
            pData[ n + 1 ] = 0x00;    // G
            pData[ n + 2 ] = 0x00;    // B
            pData[ n + 3 ] = 0xff;    // A
        }
        else
        {
            pData[ n ] = 0xff;        // R
            pData[ n + 1 ] = 0xff;    // G
            pData[ n + 2 ] = 0xff;    // B
            pData[ n + 3 ] = 0xff;    // A
        }
    }

    return data;
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> D3D12PerlinTriangle::GeneratePerlinTextureData()
{
    const UINT rowPitch = TextureWidth * TexturePixelSize;
    const UINT textureSize = rowPitch * TextureHeight;
    std::vector<UINT8> data( textureSize );

    for( size_t n = 0; n < textureSize; n += TexturePixelSize )
    {
        // Calculate a Perlin noise value based on texture coordinates.
        // Texture coordinates are normalized to the range [0, 1].
        float x = static_cast<float>( n % rowPitch ) / static_cast<float>( rowPitch );
        float y = static_cast<float>( n / rowPitch ) / static_cast<float>( TextureHeight );
        float noise = stb_perlin_noise3( x, y, ( x + y ) / 2.0f, 0, 0, 0 );

        // Convert the noise value to an offset from a fixed color.
        // The noise value is in the range [-1, 1], so scale it to [0, 255].
        UINT8 full_color = static_cast<UINT8>( ( noise + 1 ) * 128.0f );

        data.at( n )     = full_color; // R
        data.at( n + 1 ) = full_color; // G
        data.at( n + 2 ) = full_color; // B
        data.at( n + 3 ) = 0xff;       // A
    }

    return data;
}

// Update frame-based values.
void D3D12PerlinTriangle::OnUpdate()
{
    // Update the vertex buffer to rotate the object.
    {
        // Get a pointer to the vertex buffer.
        Vertex* pVertexDataBegin;
        CD3DX12_RANGE readRange( 0, 0 );
        ThrowIfFailed( m_vertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) ) );

        // Update the vertex buffer with the new rotation angle.
        static const float msPerSecond = 1000.0f; // milliseconds per second
        static const float degreesPerSecond = 60.0f; // degrees per second
        static const float radiansPerSecond = XM_PI / 180.0f * degreesPerSecond;

        float theta = ( GetElapsedTimeMs() / msPerSecond ) * radiansPerSecond;
        XMMATRIX rotationMatrixY = XMMatrixRotationY( theta );
        XMMATRIX translationMatrix = XMMatrixTranslation( 0.0f, 0.0f, 0.5f );

        for( size_t i = 0; i < m_triangleVertices.size(); i++ )
        {
            // Rotate the vertex around the Y-axis and translate along the
            // Z-axis to keep it in the view frustum.
            XMVECTOR vertexVector = XMLoadFloat3( &m_triangleVertices[ i ].position );
            XMVECTOR transformedVertex = XMVector3Transform( vertexVector, rotationMatrixY * translationMatrix );
            XMStoreFloat3( &pVertexDataBegin[ i ].position, transformedVertex );
        }

        // Release the local copy of the vertex data.
        m_vertexBuffer->Unmap( 0, nullptr );
    }
}

// Render the scene.
void D3D12PerlinTriangle::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    // Present the frame.
    ThrowIfFailed( m_swapChain->Present( 1, 0 ) );

    WaitForPreviousFrame();
}

void D3D12PerlinTriangle::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle( m_fenceEvent );
}

void D3D12PerlinTriangle::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed( m_commandAllocator->Reset() );

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed( m_commandList->Reset( m_commandAllocator.Get(), m_psoSquare.Get() ) );

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature( m_rootSignature.Get() );

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

    m_commandList->SetGraphicsRootDescriptorTable( 0, m_srvHeap->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->RSSetViewports( 1, &m_viewport );
    m_commandList->RSSetScissorRects( 1, &m_scissorRect );

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[ m_frameIndex ].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_frameIndex,
        m_rtvDescriptorSize );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle( m_dsvHeap->GetCPUDescriptorHandleForHeapStart() );
    m_commandList->OMSetRenderTargets( 1, &rtvHandle, FALSE, &dsvHandle );

    // Record commands.
    // Not specifically required to clear the RTV, but it is good practice.
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_commandList->ClearRenderTargetView( rtvHandle, clearColor, 0, nullptr );
    m_commandList->ClearDepthStencilView( dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );

    // Set necessary state and draw the object.
    m_commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_commandList->IASetVertexBuffers( 0, 1, &m_vertexBufferView );
    m_commandList->SetPipelineState( m_psoSquare.Get() );
    m_commandList->DrawInstanced( static_cast<UINT>( m_triangleVertices.size() ), 1, 0, 0);

    // Set necessary state and draw the background.
    m_commandList->IASetVertexBuffers( 0, 1, nullptr );
    m_commandList->SetPipelineState( m_psoBackground.Get() );
    m_commandList->DrawInstanced( 3, 1, 0, 0 );

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[ m_frameIndex ].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT ) );

    ThrowIfFailed( m_commandList->Close() );
}

void D3D12PerlinTriangle::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed( m_commandQueue->Signal( m_fence.Get(), fence ) );
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if( m_fence->GetCompletedValue() < fence )
    {
        ThrowIfFailed( m_fence->SetEventOnCompletion( fence, m_fenceEvent ) );
        WaitForSingleObject( m_fenceEvent, INFINITE );
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
