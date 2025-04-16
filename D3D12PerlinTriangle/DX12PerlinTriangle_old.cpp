#include <windows.h>
#include <wrl.h>
#include <directx/d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <vector>
#include <iostream>
#include "stb_perlin.h"  // Include STB Perlin Noise header

using namespace Microsoft::WRL;

// **Window Dimensions**
const int WIDTH = 800;
const int HEIGHT = 600;
const int FRAME_COUNT = 2;
const int TEXTURE_SIZE = 256;

// **Global DX12 Variables**
ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12DescriptorHeap> rtvHeap;
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12PipelineState> pipelineState;
ComPtr<ID3D12RootSignature> rootSignature;
ComPtr<ID3D12Resource> renderTargets[ FRAME_COUNT ];
ComPtr<ID3D12Resource> texture;
ComPtr<ID3D12DescriptorHeap> srvHeap;
ComPtr<ID3D12Resource> vertexBuffer;

UINT rtvDescriptorSize;
UINT frameIndex;
HWND hwnd;

// **Vertex Structure**
struct Vertex
{
    float position[ 3 ];
    float uv[ 2 ];
};

// **Triangle Vertex Data**
Vertex triangleVertices[] = {
    { { 0.0f,  0.5f, 0.0f }, { 0.5f, 0.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },
    { {-0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } }
};

// **Generate Perlin Noise Texture**
std::vector<uint8_t> GeneratePerlinNoiseTexture( int size )
{
    std::vector<uint8_t> textureData( size * size * 4 );
    for( int y = 0; y < size; y++ )
    {
        for( int x = 0; x < size; x++ )
        {
            float noise = stb_perlin_noise3( (float)x / size, (float)y / size, 0, size, size, size );
            uint8_t value = static_cast<uint8_t>( ( noise * 0.5f + 0.5f ) * 255 );
            int index = ( y * size + x ) * 4;
            textureData[ index ] = value;
            textureData[ index + 1 ] = value;
            textureData[ index + 2 ] = value;
            textureData[ index + 3 ] = 255;
        }
    }
    return textureData;
}

// **Vertex Shader**
const char* vertexShaderSource = R"(
struct VSInput {
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = float4(input.position, 1.0);
    output.uv = input.uv;
    return output;
}
)";

// **Pixel Shader**
const char* pixelShaderSource = R"(
Texture2D myTexture : register(t0);
SamplerState mySampler : register(s0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET {
    return myTexture.Sample(mySampler, input.uv);
}
)";

void GetHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter )
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if( SUCCEEDED( pFactory->QueryInterface( IID_PPV_ARGS( &factory6 ) ) ) )
    {
        for(
            UINT adapterIndex = 0;
            SUCCEEDED( factory6->EnumAdapterByGpuPreference(
            adapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS( &adapter ) ) );
            ++adapterIndex )
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1( &desc );

            if( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if( SUCCEEDED( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof( ID3D12Device ), nullptr ) ) )
            {
                break;
            }
        }
    }

    if( adapter.Get() == nullptr )
    {
        for( UINT adapterIndex = 0; SUCCEEDED( pFactory->EnumAdapters1( adapterIndex, &adapter ) ); ++adapterIndex )
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1( &desc );

            if( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if( SUCCEEDED( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof( ID3D12Device ), nullptr ) ) )
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

// Load the rendering pipeline dependencies.
void InitPipeline()
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
    CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &factory ) );

    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter( factory.Get(), &hardwareAdapter );

    D3D12CreateDevice(
        hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS( &device )
    );

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &commandQueue ) );

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = WIDTH;
    swapChainDesc.Height = HEIGHT;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swap;
    factory->CreateSwapChainForHwnd(
        commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swap
    );

    // This sample does not support fullscreen transitions.
    factory->MakeWindowAssociation( hwnd, DXGI_MWA_NO_ALT_ENTER );

    swap.As( &swapChain );
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FRAME_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &rtvHeap ) );

        rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( rtvHeap->GetCPUDescriptorHandleForHeapStart() );

        // Create a RTV for each frame.
        for( UINT n = 0; n < FRAME_COUNT; n++ )
        {
            swapChain->GetBuffer( n, IID_PPV_ARGS( &renderTargets[ n ] ) );
            device->CreateRenderTargetView( renderTargets[ n ].Get(), nullptr, rtvHandle );
            rtvHandle.Offset( 1, rtvDescriptorSize );
        }
    }

    device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &commandAllocator ) );
}

// **Create Root Signature**
void CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.NumParameters = 0;
    rootSignatureDesc.pParameters = nullptr;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error );
    device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) );
}

// **Create Vertex Buffer**
void CreateVertexBuffer()
{
    const UINT vertexBufferSize = sizeof( triangleVertices );

    // Create the vertex buffer resource
    const D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    const D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer( vertexBufferSize );

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( &vertexBuffer ) );

    // Copy the vertex data to the buffer
    void* vertexData;
    vertexBuffer->Map( 0, nullptr, &vertexData );
    memcpy( vertexData, triangleVertices, vertexBufferSize );
    vertexBuffer->Unmap( 0, nullptr );
}

// **Create Texture in GPU**
void CreateTexture()
{
    auto textureData = GeneratePerlinNoiseTexture( TEXTURE_SIZE );

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = TEXTURE_SIZE;
    desc.Height = TEXTURE_SIZE;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS( &texture ) );

    // Upload Texture Data
    ComPtr<ID3D12Resource> uploadBuffer;
    UINT64 uploadBufferSize;
    device->GetCopyableFootprints( &desc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize );

    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    desc = CD3DX12_RESOURCE_DESC::Buffer( uploadBufferSize );
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( &uploadBuffer ) );

    // Copy Data
    void* mappedData;
    uploadBuffer->Map( 0, nullptr, &mappedData );
    memcpy( mappedData, textureData.data(), textureData.size() );
    uploadBuffer->Unmap( 0, nullptr );

    // Transition to Shader Resource
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    // Create the command list.
    device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS( &commandList ) );

    commandList->CopyResource( texture.Get(), uploadBuffer.Get() );

    commandList->ResourceBarrier( 1, &barrier );

    commandList->Close();
}

// **Create Shaders and Pipeline**
void CreateShaders()
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    D3DCompile( vertexShaderSource, strlen( vertexShaderSource ), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShader, nullptr );
    D3DCompile( pixelShaderSource, strlen( pixelShaderSource ), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShader, nullptr );

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pipelineState ) );
}

// **Window Callback Function**
LRESULT CALLBACK WindowProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;
    }
    return DefWindowProc( hwnd, msg, wParam, lParam );
}

// **Initialize Window**
void InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DX12WindowClass";
    RegisterClass( &wc );

    hwnd = CreateWindowEx( 0, L"DX12WindowClass", L"DX12 Perlin Triangle", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT, nullptr, nullptr, hInstance, nullptr );
    ShowWindow( hwnd, nCmdShow );
}

// **Render Function**
void Render()
{
    // **Prepare Command List**
    commandAllocator->Reset();
    commandList->Reset( commandAllocator.Get(), pipelineState.Get() );

    // **Set the render target**
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[ frameIndex ].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
    commandList->ResourceBarrier( 1, &barrier );

    // **Set the render target view**
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize );
    commandList->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );

    // **Set the viewport and scissor rect**
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>( WIDTH ), static_cast<float>( HEIGHT ), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, WIDTH, HEIGHT };
    commandList->RSSetViewports( 1, &viewport );
    commandList->RSSetScissorRects( 1, &scissorRect );

    // **Render the Triangle**
    D3D12_VERTEX_BUFFER_VIEW vbView = {
        vertexBuffer->GetGPUVirtualAddress(),
        sizeof( triangleVertices ),
        sizeof( Vertex )
    };
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    commandList->IASetVertexBuffers( 0, 1, &vbView );

    commandList->DrawInstanced( 3, 1, 0, 0 );

    // **Transition the back buffer to present**
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[ frameIndex ].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
    commandList->ResourceBarrier( 1, &barrier );

    // **Execute command list**
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    // **Present the frame**
    swapChain->Present( 1, 0 );
    frameIndex = swapChain->GetCurrentBackBufferIndex();
}

// **Main Function**
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
    InitWindow( hInstance, nCmdShow );

    // **Initialize DX12 (Device, SwapChain, Pipeline, etc.)**
    InitPipeline();
    CreateRootSignature();
    CreateShaders();
    CreateTexture();
    CreateVertexBuffer();

    // **Main Loop**
    MSG msg = {};
    while( msg.message != WM_QUIT )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    return 0;
}
