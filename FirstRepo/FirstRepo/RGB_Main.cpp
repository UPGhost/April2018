
#include "RGB_Main.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11Texture2D*        depthStencil = nullptr;
ID3D11DepthStencilView* depthStencilView = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;
ID3D11PixelShader*      g_pPixelShader = nullptr;

ID3D11InputLayout*      g_pVertexLayout = nullptr;
ID3D11Buffer*           vertexBuffer = nullptr;
ID3D11Buffer*			indexBuffer = nullptr;
ID3D11Buffer*			constantBuffer = nullptr;

XMMATRIX				theWorldPlayer; //Translations, Scales, Spins, Etc.
XMMATRIX				theWorldBlue;
XMMATRIX				theWorldGreen;
XMMATRIX				theWorldRed;

XMMATRIX				theWorldPlayerLives1; //Player Lives
XMMATRIX				theWorldPlayerLives2;
XMMATRIX				theWorldPlayerLives3;

XMMATRIX				theWorldThreshold; //Fall Fail Threshold

XMMATRIX				theWorldBorderBottom; //Borders
XMMATRIX				theWorldBorderTop;
XMMATRIX				theWorldBorderColumnLeft;
XMMATRIX				theWorldBorderColumnRight;
XMMATRIX				theWorldBorderColumnLeftMiddle;
XMMATRIX				theWorldBorderColumnRightMiddle;

//Camera
XMMATRIX				theView;
XMMATRIX				theProjection;

XMFLOAT4				theColor(1.0f, 1.0f, 1.0f, 1.0f); //Default Color

char					ch = 0; //Get Keyboard Input

//Pause Checks
bool					pause = false;
float					pauseTime;
float					currentTime;

//Mathematics
float					colourChangeTime;
float					gameOverTime;

float					lastDigitOfT;
float					randomColumn;

float					column0TooManyTimes;
float					column1TooManyTimes;
float					column2TooManyTimes;


//Instructions for Game Start Up

bool					gameLoaded = true;

//COLUMNS

float column[3] = { -2.0f, 1.0f, 4.0f };

//PLAYER

//Player Parameters

float xPlayerPos = column[1];
float yPlayerPos = -2.5f;


float playerRed = 1.0f;
float playerGreen = 1.0f;
float playerBlue = 1.0f;

int playerPoints = 0;
float playerFailCount = 0.0f;

float playerLife1 = 1.0f;
float playerLife2 = 1.0f;
float playerLife3 = 1.0f;

//Player Check

bool tooFarLeft = false;
bool tooFarRight = false;
bool buttonPressed = false;

//RGB Shape Parameters

float xRedPos = column[0];
float xGreenPos = column[1];
float xBluePos = column[2];

float yRedPos = 15.0f;
float yGreenPos = 4.0f;
float yBluePos = 4.0f;

float yCurrentRedPos;
float yCurrentGreenPos;
float yCurrentBluePos;


float redSuccess = 1.0f;
float redFallTime;

float redSpeedMultiplier = 2.5f;
float greenSpeedMultiplier = 2.0f;
float blueSpeedMultiplier = 1.5f;

//Game Parameters

bool gameOver = false;
bool gameFinished = false;

//Keyboard Check
std::unique_ptr<Keyboard> keyboard = std::make_unique<Keyboard>();;

//Tracker for if key was JUST pressed or JUST released
Keyboard::KeyboardStateTracker tracker;

//Score Count
std::string playerScore = std::to_string(playerPoints);

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
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

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 800, 600 }; //Window Parameters

    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"RGB - The Game",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	//Random Numbers based on Computer Time
	void srand(int seed);

	//Initialise Window and Such
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );

        if ( hr == E_INVALIDARG )
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                                    D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        }

        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface( __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice) );
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory) );
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface( __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2) );
    if ( dxgiFactory2 )
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface( __uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1) );
        if (SUCCEEDED(hr))
        {
            (void) g_pImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1) );
        }

        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd( g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1 );
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface( __uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain) );
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation( g_hWnd, DXGI_MWA_NO_ALT_ENTER );

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &pBackBuffer ) );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;



	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &depthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(depthStencil, &descDSV, &depthStencilView);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile( L"RGB_PSandVS.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX File is Probably Named Wrong, or doesn't exist. Check again.", L"Error", MB_OK );
        return hr;
    }

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile( L"RGB_PSandVS.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX File is Probably Named Wrong, or doesn't exist. Check again.", L"Error", MB_OK );
        return hr;
    }

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
	{
		//Player Square + Colour Shapes 

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f) }, //Pos (X, Y, Z), Color (R, G, B, A)
		{ XMFLOAT3( -0.5f, 1.0f, 1.0f ) },
		{ XMFLOAT3( -0.5f, 0.5f,  1.0f) },
		{ XMFLOAT3(-1.0f, 0.5f,  1.0f), },

		//Thresh Hold
		
		{ XMFLOAT3(-6.0f, 0.1f, 1.0f) }, 
		{ XMFLOAT3(6.0f, 0.1f, 1.0f) },
		{ XMFLOAT3(6.0f, 0.0f,  1.0f) },
		{ XMFLOAT3(-6.0f, 0.0f,  1.0f), },

		//Border Width

		{ XMFLOAT3(-6.0f, 0.5f, 1.0f) }, 
		{ XMFLOAT3(6.0f, 0.5f, 1.0f) },
		{ XMFLOAT3(6.0f, 0.0f,  1.0f) },
		{ XMFLOAT3(-6.0f, 0.0f,  1.0f), },

		//Border Column / Length

		{ XMFLOAT3(-0.125f, 6.0f, 1.0f) },
		{ XMFLOAT3(0.125f, 6.0f, 1.0f) },
		{ XMFLOAT3(0.125f, -6.0f,  1.0f) },
		{ XMFLOAT3(-0.125f, -6.0f,  1.0f), },

		// EXTRA

		// { XMFLOAT3(-1.25f, 0.75f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },

		//{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }, //Pos (X, Y, Z), Color (R, G, B, A)
		//{ XMFLOAT3(-0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		//{ XMFLOAT3(-0.5f, 0.5f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		//{ XMFLOAT3(-1.0f, 0.5f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		
    };
    D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 16; //Set Multiplier to how many VERTICES
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &vertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &vertexBuffer, &stride, &offset );


	//Create Index Buffer
	WORD indi [] =
	{
		3, 0, 1,
		2, 3, 1,

		7, 4, 5,
		6, 7, 5,

		11, 8, 9,
		10, 11, 9,

		15, 12, 13,
		14, 15, 13,
	};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 24;  //Set to Number of Indicies
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indi;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &indexBuffer);
	if (FAILED(hr))
		return hr;

	//Set Index Buffer
	g_pImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);


    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ); //TriangleList for Triangles, TriangleStrip for Squares. 3 and 4.

	//Create Constant Buffer

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer); 
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &constantBuffer);
	if (FAILED(hr))
		return hr;

	//Initialise World Matrix

	/*
	theWorldPlayer = XMMatrixIdentity();
	theWorldRed = XMMatrixIdentity();
	theWorldGreen = XMMatrixIdentity();
	theWorldBlue = XMMatrixIdentity();

	theWorldPlayerLives1 = XMMatrixIdentity(), theWorldPlayerLives2 = XMMatrixIdentity(), theWorldPlayerLives3 = XMMatrixIdentity();
	*/

	//View
	XMVECTOR TiltAndDistance =	 XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f); //0.0f, Tilting Up and Down, Distance, 0.0f
	XMVECTOR At =				 XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR UpOrDown =			 XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  //0.0f, Camera Up and Down, 0.0f, 0.0f

	theView = XMMatrixLookAtLH(TiltAndDistance, At, UpOrDown);

	//Projection

	theProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f); //Width and Height of Window

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( vertexBuffer ) vertexBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
	if (constantBuffer) constantBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
	if (depthStencil) depthStencil->Release();
	if (depthStencilView) depthStencilView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice1 ) g_pd3dDevice1->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;


	if (gameOver == true)
	{
		MessageBoxA(hWnd, playerScore.c_str(), "RGB - Your Score!", MB_OKCANCEL) == IDOK;

		if (MessageBox(hWnd, L"Game Over! Thanks for Playing!" , L"RGB - The Game", MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(hWnd);
		}

		gameFinished = false;
	}

    switch( message )
    {

	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	case WM_KEYDOWN:

	case WM_SYSKEYDOWN:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	case WM_KEYUP:

	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;

    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

	case WM_CLOSE:

		if (gameOver == true)
		{
			if (MessageBox(hWnd, L"Game Over! Quit RGB?", L"RGB - The Game", MB_OKCANCEL) == IDOK)
			{
				DestroyWindow(hWnd);
			}
		}
		else
		{
			DestroyWindow(hWnd);
		}
	
		return 0;

		
    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }



    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{

	//Game Loaded, Explain Rules
	if (gameLoaded == true)
	{
		MessageBox(NULL,
			L"How to Play: Press the Left and Right Arrow Keys to Move. When the Coloured Shapes falls on the White Player Square, press the corresponding button to Colour to rack up score: Z for Red, X for Green, C for Blue.",
			L"RGB - The Game", MB_OKCANCEL) == IDOK;

		MessageBox(NULL,
			L"How to Play Continued: Make sure not to let the shapes fall below the White line, miss-time your button press or press the wrong button for the colour or you'll lose a Life! You can also pause with ESC. You've got 3 Lives, Good Luck!",
			L"RGB - The Game", MB_OKCANCEL) == IDOK;
	}
	gameLoaded = false;



	//Keyboard Get
	auto kb = keyboard->GetState();

	// kb.W = W . means W is down

	//Keyboard Tracker
	auto state = keyboard->GetState();
	tracker.Update(state);

	// tracker.pressed.Space . means Space was JUST pressed

	// tracker.IskeyReleased( VK_F1) . means F1 key was JUST released 

	// Update our time
	static float t = 0.0f;


		//Time Stuff

	/*if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			t += (float)XM_PI * 0.0125f - currentTime;
		}

		else
		{*/

	//CONTROLS TIME, more than the first lines 

	static ULONGLONG timeStart = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
	{
		timeStart = timeCur;
	}
	t = ((timeCur - timeStart) / 1000.0f);
	

	//Random Column for Falling Shapes

	lastDigitOfT = rand() % 10 + 1;

	if (lastDigitOfT <= 2)
	{
		if (column0TooManyTimes < 1) //Column[0]
		{
			randomColumn = column[0];
			column0TooManyTimes += 1;
		}
		else //Check Random Number Doesn't Choose the same Column over and Over.
		{
			randomColumn = column[2];
			column0TooManyTimes = 0;
		}
	}
	else if (lastDigitOfT > 2 || lastDigitOfT < 6)
	{
		if (column1TooManyTimes < 1) //Column[1]
		{
			randomColumn = column[1];
			column1TooManyTimes += 1;
		} 
		else //Check Random Number Doesn't Choose the same Column over and Over.
		{
			randomColumn = column[2];
			column1TooManyTimes = 0;
		}
	}
	else if (lastDigitOfT >= 6)
	{
		if (column1TooManyTimes < 1) //Column[2]
		{
			randomColumn = column[2];
			column2TooManyTimes += 1;
		}
		else //Check Random Number Doesn't Choose the same Column over and Over.
		{
			randomColumn = column[1];
			column2TooManyTimes = 0;
		}
	}
	//for (int x; x++; x = 9)
	//{

	//}

	if (pause == true)
	{
		
		currentTime = t - pauseTime;

		//THIS LINE ACTUALLY PAUSES
		t = pauseTime;
	}
	else
	{
		//
		t -= currentTime;
	}
	
		// PAUSING

	if (tracker.pressed.Escape)
	{
		if (pause == false)
		{
			pause = true;
			pauseTime = t;
		}
		else
		{
			pause = false;
		}
	}


	
	//Way Pausing Works

	/*
	When the Game is Paused, take the Current Time and save it under: 'pauseTime'

	'pauseTime = t;'

	Use this to get the value between the ongoing time, 't', and the amount of time the game has been 
	paused for.

	To do this, do: 't - pauseTime = currentTime;'

	Then when the game is unpaused, take the final value for the amount of time the game has been paused
	'currentTime' and minus it from the ongoing time 't' to get the Game time.

	This looks like: 't -= currentTime;'
	*/

	//INPUTS

	//Player Movement

	if (pause == false)
	{
		if (tracker.pressed.Left && tooFarLeft == false) //LEFT
		{
			xPlayerPos -= 3.0f;
		}

		if (tracker.pressed.Right && tooFarRight == false) //RIGHT
		{
			xPlayerPos += 3.0f;
		}

		if (tracker.pressed.Z && buttonPressed == false) //RED BUTTON
		{
			//If Pressed at the Right Time

			if (xPlayerPos == xRedPos && yCurrentRedPos <= yPlayerPos + 0.5f && yCurrentRedPos >= yPlayerPos - 0.5f) //If Player lines up with the Red Shape, and the Red Shape is within the Player Shape...
			{
				//redSpeedMultiplier = redSpeedMultiplier*1.2f;		

				yRedPos = yRedPos + 6.5f; //Multiplier is to Adjust/Fix Where it Fall Starts.

				xRedPos = randomColumn; //Set Random Column for Falling

				playerPoints += 1; //Add to Points		
			}
			else
			{
				playerFailCount += 1; //Add to Fail Count
			}

			playerRed = 1.0f, playerGreen = 0.0f, playerBlue = 0.0f;
			colourChangeTime = t;

			buttonPressed == true;
		}


		if (tracker.pressed.X && buttonPressed == false) //GREEN BUTTON
		{
			//If Pressed at the Right Time

			if (xPlayerPos == xGreenPos && yCurrentGreenPos <= yPlayerPos + 0.5f && yCurrentGreenPos >= yPlayerPos - 0.5f) //If Player lines up with the Red Shape, and the Red Shape is within the Player Shape...
			{
				yGreenPos = yGreenPos + 6.5f;

				//greenSpeedMultiplier = greenSpeedMultiplier*1.2f; //Multiplier is to Adjust/Fix Where it Fall Starts.	

				xGreenPos = randomColumn; //Set Random Column for Falling

				playerPoints += 1; //Add to Points
			}
			//Wrong Time
			else
			{
				playerFailCount += 1; //Add to Fail Count
			}

			//Colour Change
			playerRed = 0.0f, playerGreen = 1.0f, playerBlue = 0.0f;
			colourChangeTime = t;

			buttonPressed == true;
		}

		if (tracker.pressed.C && buttonPressed == false) //BLUE BUTTON
		{
			//If Pressed at the Right Time

			if (xPlayerPos == xBluePos && yCurrentBluePos <= yPlayerPos + 0.5f && yCurrentBluePos >= yPlayerPos - 0.5f) //If Player lines up with the Red Shape, and the Red Shape is within the Player Shape...
			{
				//blueSpeedMultiplier = blueSpeedMultiplier*1.2f; //Multiplier is to Adjust/Fix Where it Fall Starts.

				yBluePos = yBluePos + 6.5f;

				xBluePos = randomColumn; //Set Random Column for Falling

				playerPoints += 1; //Add to Points
			}
			else
			{
				playerFailCount += 1; //Add to Fail Count
			}

			playerRed = 0.0f, playerGreen = 0.0f, playerBlue = 1.0f;
			colourChangeTime = t;

			buttonPressed == true;
		}
	}

	//Button Check Released

	buttonPressed = false;

	//Check Player Location

	if (xPlayerPos <= column[0])
	{
		tooFarLeft = true;
	}
	else
	{
		tooFarLeft = false;
	}

	if (xPlayerPos >= column[2])
	{
		tooFarRight = true;
	}
	else
	{
		tooFarRight = false;
	}

	//Lock in Positions

	//RESET PLAYER COLOUR

	if (t - colourChangeTime >= 0.2)
	{
		playerRed = 1.0f, playerGreen = 1.0f, playerBlue = 1.0f;
	}

	//MISSED SHAPED
	 
	//Red Shape

	if (yCurrentRedPos <= -3.5f)
	{
		playerFailCount += 1;
		yRedPos += 7.0f;
		xRedPos = randomColumn;
	}

	//Green Shape

	if (yCurrentGreenPos <= -3.5f)
	{
		playerFailCount += 1;
		yGreenPos += 7.0f;
		xGreenPos = randomColumn;
	}

	//Blue Shape

	if (yCurrentBluePos <= -3.5f)
	{
		playerFailCount += 1;
		yBluePos += 7.0f;
		xBluePos = randomColumn;
	}

	//PLAYER LIFE LOST

	if (playerFailCount > 0.0f)
	{
		playerLife1 = 0.0f;
	}

	if (playerFailCount > 1.0f)
	{
		playerLife2 = 0.0f;
	}

	if (playerFailCount > 2.0f)
	{
		playerLife3 = 0.0f;
		gameOverTime = t;
		gameOver = true;
	}

	//GAME OVER


	if (t - gameOverTime >= 0.2 && gameOverTime != 0) //After x Number of Seconds, Close Game
	{
		gameFinished = true;
	}

	//Set Player Parameters

	XMMATRIX translatePlayer = XMMatrixTranslation(xPlayerPos, yPlayerPos, -1.99f);
	theWorldPlayer = translatePlayer;

	//Setting Blue Square Parameters

	yCurrentBluePos = yBluePos - t * blueSpeedMultiplier;

	XMMATRIX translateBlue = XMMatrixTranslation(xBluePos, yCurrentBluePos, -2.0f); //MOVEMENT	Use T to Move based on Time.
	theWorldBlue = translateBlue;

	//Setting Green Triangle Parameters

	yCurrentGreenPos = yGreenPos - t * greenSpeedMultiplier;

	XMMATRIX translateGreen = XMMatrixTranslation(xGreenPos, yCurrentGreenPos, -2.0f); //MOVEMENT	Use T to Move based on Time.
	theWorldGreen = translateGreen;

	//Setting Red Triangle Parameters

	yCurrentRedPos = yRedPos - t * redSpeedMultiplier;

	XMMATRIX translateRed = XMMatrixTranslation(xRedPos, yCurrentRedPos, -2.0f); //MOVEMENT	Use T to Move based on Time.
	theWorldRed = translateRed;


	//Set Player Lives Parameters
	
	XMMATRIX translateLives1 = XMMatrixTranslation(5.5f, 3.4f, -2.0f);
	XMMATRIX translateLives2 = XMMatrixTranslation(5.5f, 2.7f, -2.0f);
	XMMATRIX translateLives3 = XMMatrixTranslation(5.5f, 2.0f, -2.0f);

	theWorldPlayerLives1 = translateLives1;
	theWorldPlayerLives2 = translateLives2;
	theWorldPlayerLives3 = translateLives3;
	
	//Set Fall Threshold Parameters

	XMMATRIX translateThreshold = XMMatrixTranslation(0.0f, -2.3f, -2.0f);
	theWorldThreshold = translateThreshold;

	//Set Border Parameters

	XMMATRIX translateBorderTop = XMMatrixTranslation(0.0f, 4.6f , -1.9f);
	XMMATRIX translateBorderBottom = XMMatrixTranslation(0.0f, -3.1f, -1.9f);
	XMMATRIX translateBorderColumnLeft = XMMatrixTranslation(-5.5f + 0.125, 0.0f, -1.9f); //0.125 is Object Width
	XMMATRIX translateBorderColumnRight = XMMatrixTranslation(5.5f - 0.125, 0.0f, -1.9f);
	XMMATRIX translateBorderColumnLeftMiddle = XMMatrixTranslation(-1.5f, 0.0f, -1.9f);
	XMMATRIX translateBorderColumnRightMiddle = XMMatrixTranslation(2.0f, 0.0f, -1.9f);

	theWorldBorderTop = translateBorderTop;
	theWorldBorderBottom = translateBorderBottom;
	theWorldBorderColumnLeft = translateBorderColumnLeft;
	theWorldBorderColumnRight = translateBorderColumnRight;
	theWorldBorderColumnLeftMiddle = translateBorderColumnLeftMiddle;
	theWorldBorderColumnRightMiddle = translateBorderColumnRightMiddle;

	//Set Border Parameters
	

    // Clear the back buffer 

    //g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, Colors::MidnightBlue );

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::Black);

	//
	// Clear the depth buffer to 1.0 (max depth)
	//

	g_pImmediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Update Player Variables

	ConstantBuffer cb1;
	

	cb1.world = XMMatrixTranspose(theWorldPlayer);
	cb1.view = XMMatrixTranspose(theView);
	cb1.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = playerRed; theColor.y = playerGreen; theColor.z = playerBlue; theColor.w = 1.0f;
	cb1.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb1, 0, 0);


    // Render a triangle
	g_pImmediateContext->VSSetShader( g_pVertexShader, nullptr, 0 );
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader( g_pPixelShader, nullptr, 0 );
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

    g_pImmediateContext->DrawIndexed( 6, 0, 0 ); //Set (x, 0, 0) to number of Draw Vertices for First Object. 

	// BLUE OBJECT

	ConstantBuffer cb2;

	cb2.world = XMMatrixTranspose(theWorldBlue);
	cb2.view = XMMatrixTranspose(theView);
	cb2.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = 0.0f; theColor.y = 0.0f; theColor.z = 1.0f; theColor.w = 1.0f;

	cb2.Color = theColor;
	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb2, 0, 0);

	// Render Blue Object

	g_pImmediateContext->DrawIndexed(6, 0, 0); //Set to number of Draw Vertices for First Object. 

	//GREEN OBJECT

	ConstantBuffer cb3;

	cb3.world = XMMatrixTranspose(theWorldGreen);
	cb3.view = XMMatrixTranspose(theView);
	cb3.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = 0.0f; theColor.y = 1.0f; theColor.z = 0.0f; theColor.w = 1.0f;

	cb3.Color = theColor;
	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb3, 0, 0);

	// Render Green Object

	g_pImmediateContext->DrawIndexed(3, 0, 0); //Set to number of Draw Vertices for First Object. 

	//RED OBJECT

	ConstantBuffer cb4;

	cb4.world = XMMatrixTranspose(theWorldRed);
	cb4.view = XMMatrixTranspose(theView);
	cb4.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = 1.0f; theColor.y = 0.0f; theColor.z = 0.0f; theColor.w = 1.0f;

	cb4.Color = theColor;
	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb4, 0, 0);

	// Render Green Object

	g_pImmediateContext->DrawIndexed(3, 3, 0); //Set to number of Draw Vertices for First Object. 

	//LIVES

	//Life 1

	ConstantBuffer cb5;

	cb5.world = XMMatrixTranspose(theWorldPlayerLives1);
	cb5.view = XMMatrixTranspose(theView);
	cb5.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = playerLife1; theColor.y = playerLife1; theColor.z = playerLife1; theColor.w = 1.0f;
	cb5.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb5, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 0, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//Life 2

	ConstantBuffer cb6;

	cb6.world = XMMatrixTranspose(theWorldPlayerLives2);
	cb6.view = XMMatrixTranspose(theView);
	cb6.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = playerLife2; theColor.y = playerLife2; theColor.z = playerLife2; theColor.w = 1.0f;
	cb6.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb6, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 0, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//Life 3

	ConstantBuffer cb7;

	cb7.world = XMMatrixTranspose(theWorldPlayerLives3);
	cb7.view = XMMatrixTranspose(theView);
	cb7.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = playerLife3; theColor.y = playerLife3; theColor.z = playerLife3; theColor.w = 1.0f;
	cb7.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb7, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 0, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.


	//FALL THRESHOLD

	ConstantBuffer cb8;

	cb8.world = XMMatrixTranspose(theWorldThreshold);
	cb8.view = XMMatrixTranspose(theView);
	cb8.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = 0.8f; theColor.y = 0.8f; theColor.z = 0.8f; theColor.w = 1.0f;
	cb8.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb8, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 6, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//BORDERS

	//Width Bottom

	ConstantBuffer cb9;

	cb9.world = XMMatrixTranspose(theWorldBorderTop);
	cb9.view = XMMatrixTranspose(theView);
	cb9.projection = XMMatrixTranspose(theProjection);

	//Colors

	theColor.x = 0.0f; theColor.y = 0.5f; theColor.z = 0.5f; theColor.w = 1.0f; //All Border Colours
	cb9.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb9, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 12, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//Width Top
	ConstantBuffer cb10;

	cb10.world = XMMatrixTranspose(theWorldBorderBottom);
	cb10.view = XMMatrixTranspose(theView);
	cb10.projection = XMMatrixTranspose(theProjection);

	//Colors

	cb10.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb10, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 12, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//COLUMN LEFT

	ConstantBuffer cb11;

	cb11.world = XMMatrixTranspose(theWorldBorderColumnLeft);
	cb11.view = XMMatrixTranspose(theView);
	cb11.projection = XMMatrixTranspose(theProjection);

	//Colors

	cb11.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb11, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 18, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//COLUMN RIGHT

	ConstantBuffer cb12;

	cb12.world = XMMatrixTranspose(theWorldBorderColumnRight);
	cb12.view = XMMatrixTranspose(theView);
	cb12.projection = XMMatrixTranspose(theProjection);

	//Colors

	cb12.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb12, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 18, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//COLUMN LEFT MIDDLE

	ConstantBuffer cb13;

	cb13.world = XMMatrixTranspose(theWorldBorderColumnLeftMiddle);
	cb13.view = XMMatrixTranspose(theView);
	cb13.projection = XMMatrixTranspose(theProjection);

	//Colors

	cb13.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb13, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 18, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.

	//COLUMN MIDDLE RIGHT

	ConstantBuffer cb14;

	cb14.world = XMMatrixTranspose(theWorldBorderColumnRightMiddle);
	cb14.view = XMMatrixTranspose(theView);
	cb14.projection = XMMatrixTranspose(theProjection);

	//Colors

	cb14.Color = theColor;

	g_pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb14, 0, 0);

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer); // VSSetConstantBuffers( Register Number/Order of Set Constant Buffer , Number of Buffers , Set Constant Buffer )
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &constantBuffer); // *PS* Set Buffers. Allow for Set Colour because Pixel Shader.

	g_pImmediateContext->DrawIndexed(6, 18, 0); //Set (x, 0, 0) to number of Draw Vertices for First Object.


	//Check Score

	//Score Count
	playerScore = std::to_string(playerPoints);



    // Present the information rendered to the back buffer to the front buffer (the screen)
    g_pSwapChain->Present( 0, 0 );
}
