#include <windows.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_internal.h"


// 어떤 도형을 렌더링하는지 나타내는 열거형
enum ETypePrimitive
{
	EPT_Triangle,
	EPT_Cube,
	EPT_Sphere,
	EPT_MAX
};

// 정점 정의 구조체
struct FVertexSimple
{
	float x, y, z;     // position
	float r, g, b, a;  // color
};

struct FVector
{
	float x, y, z;
	FVector(float _x = 0.f, float _y = 0.f, float _z = 0.f) : x(_x), y(_y), z(_z) {}
	static float DotProduct(const FVector& lhs, const FVector& rhs)
	{
		return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
	}

	static FVector CrossProduct(const FVector& lhs, const FVector& rhs)
	{
		return {
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		};
	}

	float Dot(const FVector& rhs)
	{
		return DotProduct(*this, rhs);
	}

	FVector Cross(const FVector& rhs)
	{
		return CrossProduct(*this, rhs);
	}

	FVector operator+(const FVector& rhs) const
	{
		return { x + rhs.x, y + rhs.y, z + rhs.z };
	}
	FVector& operator+=(const FVector& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
	FVector operator-(const FVector& rhs) const
	{
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}

	FVector& operator-=(const FVector& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	FVector operator*(const float rhs) const
	{
		return { x * rhs, y * rhs, z * rhs };
	}

	FVector& operator*=(const float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	FVector operator/(const float rhs) const
	{
		return { x / rhs, y / rhs, z / rhs };
	}

	FVector& operator/=(const float rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}

	float LengthSquare() const
	{
		return (x * x + y * y + z * z);
	}

	float Length() const
	{
		return sqrtf(LengthSquare());
	}

	FVector& Normalize()
	{
		float Len = Length();
		if (Len > 0.0f)
		{
			x /= Len;
			y /= Len;
			z /= Len;
		}
		return *this;
	}

	static const FVector UnitX;
	static const FVector UnitY;
	static const FVector UnitZ;
	static const FVector One;
	static const FVector Zero;
};

const FVector FVector::UnitX = { 1.f, 0.f, 0.f };
const FVector FVector::UnitY = { 0.f, 1.f, 0.f };
const FVector FVector::UnitZ = { 0.f, 0.f, 1.f };
const FVector FVector::Zero = { 0.f, 0.f, 0.f };
const FVector FVector::One = { 1.f, 1.f, 1.f };
#pragma region Primitive Vertex Data
/**********************************************
*              Primitive Vertex Data
**********************************************/

//삼각형 하드코딩
FVertexSimple triangle_vertices[] =
{
	{ 0.f,  1.f, 0.f,   1.f, 0.f, 0.f, 1.f},    //TOP Vertex (red)
	{ 1.f, -1.f, 0.f,   0.f, 1.f, 0.f, 1.f},    //Bottom-right vertex (green)
	{-1.f, -1.f, 0.f,   0.f, 0.f, 1.f, 1.f}     // Bottom-left vertex (blue)
};

//큐브 하드코딩
FVertexSimple cube_vertices[] =
{
	// Front face (Z+)
	{ -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f }, // Bottom-left (red)
	{ -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-left (yellow)
	{  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)
	{ -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-left (yellow)
	{  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-right (blue)
	{  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)

	// Back face (Z-)
	{ -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f }, // Bottom-left (cyan)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-right (magenta)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-right (magenta)
	{  0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-right (yellow)

	// Left face (X-)
	{ -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-left (purple)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-right (yellow)
	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)

	// Right face (X+)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f, 1.0f }, // Bottom-left (orange)
	{  0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 1.0f }, // Bottom-right (gray)
	{  0.5f,  0.5f, -0.5f,  0.5f, 0.0f, 0.5f, 1.0f }, // Top-left (purple)
	{  0.5f,  0.5f, -0.5f,  0.5f, 0.0f, 0.5f, 1.0f }, // Top-left (purple)
	{  0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 1.0f }, // Bottom-right (gray)
	{  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // Top-right (dark blue)

	// Top face (Y+)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.5f, 1.0f }, // Bottom-left (light green)
	{ -0.5f,  0.5f,  0.5f,  0.0f, 0.5f, 1.0f, 1.0f }, // Top-left (cyan)
	{  0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f }, // Bottom-right (white)
	{ -0.5f,  0.5f,  0.5f,  0.0f, 0.5f, 1.0f, 1.0f }, // Top-left (cyan)
	{  0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.0f, 1.0f }, // Top-right (brown)
	{  0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f }, // Bottom-right (white)

	// Bottom face (Y-)
	{ -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.0f, 1.0f }, // Bottom-left (brown)
	{ -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top-left (red)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.5f, 1.0f }, // Bottom-right (purple)
	{ -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top-left (red)
	{  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Top-right (green)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.5f, 1.0f }, // Bottom-right (purple)
};

//구는 헤더파일로 대체
#include "Sphere.h"

#pragma endregion

#pragma region 렌더러 클래스
// 렌더링 담당 클래스
class URenderer
{
public:
	// Direct3D 장치(Device)와 장치 컨텍스트(Device Context) 및 스왑
	// 체인(SwapChain) 관리를 위한 포인터
	ID3D11Device*			Device = nullptr;									// GPU와 통신하기 위한 Direct3D 장치
	ID3D11DeviceContext*	DeviceContext = nullptr;							// GPU 명령 실행을 담당할 컨텍스트
	IDXGISwapChain*			SwapChain = nullptr;								// 프레임 버퍼 교체에 사용되는 스왑 체인

	// 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수
	ID3D11Texture2D*		FrameBuffer = nullptr;								// 화면 출력용 텍스쳐
	ID3D11RenderTargetView* FrameBufferRTV = nullptr;							// 텍스쳐를 렌더 타겟으로 사용하는 뷰
	ID3D11RasterizerState*	RasterizerState = nullptr;							// 래스터라이저 상태(컬링, 채우기 모드 정의)
	ID3D11Buffer*			ConstantBuffer = nullptr;							// 쉐이더에 데이터를 전달하기 위한 상수 버퍼

	FLOAT					ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.f };	// 화면을 초기화(clear) 할때 사용할 색상 RGBA
	D3D11_VIEWPORT			ViewportInfo;										// 렌더링 영역 정의하는 뷰포트 정보

public:
	// Renderer 초기화 함수
	void Create(HWND hWindow)
	{
		// Direct3D 장치 및 스왑 체인 생성
		CreateDeviceAndSwapChain(hWindow);

		// 프레임 버퍼 생성
		CreateFrameBuffer();

		// 래스터라이저 상태 설정
		CreateRasterizerState();

		// 깊이 스텐실 버퍼 및 블렌드 상태는 다루지 않음.
	}
	void CreateDeviceAndSwapChain(HWND hWindow)
	{
		// 지원하는 Direct3D 기능 레벨을 정의
		D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

		// 스왑 체인 설정 구조체 초기화
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc.Width = 0;							// 창 크기에 맞게 자동으로 설정
		swapChainDesc.BufferDesc.Height = 0;						// 창 크기에 맞게 자동으로 설정
		swapChainDesc.BufferDesc.Format =
			DXGI_FORMAT_B8G8R8A8_UNORM;								// 색상 포맷
		swapChainDesc.SampleDesc.Count = 1;							// 멀티 샘플링 비활성화
		swapChainDesc.BufferUsage =
			DXGI_USAGE_RENDER_TARGET_OUTPUT;						// 렌더 타겟으로 사용
		swapChainDesc.BufferCount = 2;								// 더블 버퍼링
		swapChainDesc.OutputWindow = hWindow;						// 렌더링할 창 핸들
		swapChainDesc.Windowed = TRUE;								// 창 모드
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 스왑 방식

		// Direct3D 장치와 스왑 체인 생성
		D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
			featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
			&swapChainDesc, OUT &SwapChain, OUT &Device, nullptr, OUT &DeviceContext);

		// 생성된 스왑 체인의 정보 가져오기
		SwapChain->GetDesc(&swapChainDesc);

		// 뷰포트 정보 설정
		ViewportInfo = { 
			0.0f,									// TopLeftX
			0.0f,									// TopLeftY
			(float)swapChainDesc.BufferDesc.Width,	// Width
			(float)swapChainDesc.BufferDesc.Height, // Height
			0.0f,									// MinDepth
			1.0f									// MaxDepth
		};
	}

	// Direct3D 장치 및 스왑 체인을 해제하는 함수
	void ReleaseDeviceAndSwapChain()
	{
		if (DeviceContext)
		{
			DeviceContext->Flush();  // 남아있는 gpu 명령 실행
		}

		if (SwapChain)
		{
			SwapChain->Release();
			SwapChain = nullptr;
		}
		if (Device)
		{
			Device->Release();
			Device = nullptr;
		}
		if (DeviceContext)
		{
			DeviceContext->Release();
			DeviceContext = nullptr;
		}
	}

	// 프레임 버퍼를 생성하는 함수
	void CreateFrameBuffer()
	{
		// 스왑 체인으로부터 백 버퍼 텍스쳐 가져오기
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), OUT (void**)&FrameBuffer);

		// 렌더 타겟 뷰 설정
		D3D11_RENDER_TARGET_VIEW_DESC frameBufferRTVDesc = {};
		frameBufferRTVDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;		// 색상 포맷
		frameBufferRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;	// 2D 텍스처

		Device->CreateRenderTargetView(FrameBuffer, &frameBufferRTVDesc, OUT &FrameBufferRTV);
	}
	// 프레임 버퍼 해제하는 함수
	void ReleaseFrameBuffer()
	{
		if (FrameBuffer)
		{
			FrameBuffer->Release();
			FrameBuffer = nullptr;
		}

		if (FrameBufferRTV)
		{
			FrameBufferRTV->Release();
			FrameBufferRTV = nullptr;
		}
	}

	// 래스터라이저 상태를 생성하는 함수
	void CreateRasterizerState()
	{
		D3D11_RASTERIZER_DESC rasterizerdesc = {};
		rasterizerdesc.FillMode = D3D11_FILL_SOLID;  // 채우기 모드
		rasterizerdesc.CullMode = D3D11_CULL_BACK;   // 백 페이스 컬링

		Device->CreateRasterizerState(&rasterizerdesc, OUT &RasterizerState);
	}

	// 래스터라이저 상태를 해제하는 변수
	void ReleaseRasterizerState()
	{
		if (RasterizerState)
		{
			RasterizerState->Release();
			RasterizerState = nullptr;
		}
	}

	// 렌더링에 사용된 모든 리소스를 해제하는 함수
	void Release()
	{
		RasterizerState->Release();

		// 렌더 타겟 초기화
		DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		ReleaseFrameBuffer();
		ReleaseDeviceAndSwapChain();
	}

	// 스왑체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
	void SwapBuffer()
	{
		SwapChain->Present(1, 0);  // 1: VSync 활성화
	}

	/******************************************
	 *     vertex, pixel Shader Section
	 ******************************************/
public:
	ID3D11VertexShader* SimpleVertexShader;
	ID3D11PixelShader* SimplePixelShader;
	ID3D11InputLayout* SimpleInputLayout;
	unsigned int Stride;

	void CreateShader()
	{
		ID3DBlob* vertexShaderCSO;
		ID3DBlob* pixelShaderCSO;

		HRESULT br =
			D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS",
				"vs_5_0", 0, 0, &vertexShaderCSO, nullptr);

		Device->CreateVertexShader(vertexShaderCSO->GetBufferPointer(), vertexShaderCSO->GetBufferSize(), nullptr,
			OUT &SimpleVertexShader);

		D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS",
			"ps_5_0", 0, 0, &pixelShaderCSO, nullptr);

		Device->CreatePixelShader(pixelShaderCSO->GetBufferPointer(), pixelShaderCSO->GetBufferSize(), nullptr,
			OUT &SimplePixelShader);

		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
			  D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		Device->CreateInputLayout(
			layout, ARRAYSIZE(layout), vertexShaderCSO->GetBufferPointer(),
			vertexShaderCSO->GetBufferSize(), OUT &SimpleInputLayout);

		Stride = sizeof(FVertexSimple);

		// 사용한 CSO 해제
		vertexShaderCSO->Release();
		pixelShaderCSO->Release();
	}

	void ReleaseShader()
	{
		if (SimpleInputLayout)
		{
			SimpleInputLayout->Release();
			SimpleInputLayout = nullptr;
		}

		if (SimplePixelShader)
		{
			SimplePixelShader->Release();
			SimplePixelShader = nullptr;
		}
		if (SimpleVertexShader)
		{
			SimpleVertexShader->Release();
			SimpleVertexShader = nullptr;
		}
	}

	/******************************************
	 *     Render Helper Function
	 ******************************************/

	void Prepare()
	{
		DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

		// TriangleList 사용(이미 sphere이 Triangle list 형식으로 하드코딩됨)
		DeviceContext->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DeviceContext->RSSetViewports(1, &ViewportInfo);
		DeviceContext->RSSetState(RasterizerState);

		DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
		DeviceContext->OMSetBlendState(nullptr, nullptr, 0xfffffff);
	}

	void PrepareShader()
	{
		DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
		DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
		DeviceContext->IASetInputLayout(SimpleInputLayout);

		// 버텍스 셰이더에 상수 버퍼를 설정한다
		if (ConstantBuffer)
		{
			DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
		}
	}

	void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
	{
		UINT offset = 0;
		DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);

		DeviceContext->Draw(numVertices, 0);
	}

	/******************************************
	 *     Vertex Buffer Helper function
	 ******************************************/
	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth)
	{
		D3D11_BUFFER_DESC vertexbufferdesc = {};
		vertexbufferdesc.ByteWidth = byteWidth;
		vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;  // will never be
		// updated
		vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

		ID3D11Buffer* vertexBuffer;
		Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD,
			OUT &vertexBuffer);

		return vertexBuffer;
	}

	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
	{
		vertexBuffer->Release();
	}

	/******************************************
	 *          Constant Buffer Section
	 ******************************************/
	struct FConstants
	{
		FVector Offset;
		float Pad;
	};

	void CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC constantbufferdesc = {};
		// ensure constant buffer size is multiple of 16 bytes
		constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0;
		// will be updated from CPU every frame
		constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
		constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		Device->CreateBuffer(&constantbufferdesc, nullptr, OUT &ConstantBuffer);
	}

	void ReleaseConstantBuffer()
	{
		if (ConstantBuffer)
		{
			ConstantBuffer->Release();
			ConstantBuffer = nullptr;
		}
	}

	// 상수 버퍼 갱신 함수
	void UpdateConstant(FVector Offset)
	{
		if (ConstantBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE constantBufferMSR;
			DeviceContext->Map(
				ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
				&constantBufferMSR);  // update constant buffer every frame
			FConstants* constants = (FConstants*)constantBufferMSR.pData;
			{
				constants->Offset = Offset;
			}
			DeviceContext->Unmap(ConstantBuffer, 0);
		}
	}
};
#pragma endregion

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam);

/*
 * 각종 메세지를 처리할 함수
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
	case WM_DESTROY:
		// Signal that the app should quit
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/*
 * 가장 기본이 되는 메인 함수
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// 윈도우 클래스 지정
	WCHAR WindowClass[] = L"JungleWindowClass";

	// 윈도우 타이틀바에 표시될 이름
	WCHAR Title[] = L"Game Tech Lab";

	// 각종 메세지를 처리할 함수인 WinProc의 함수 포인터를 WindowClass구조체에
	// 넣는다.

	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// 1024 x 1024 크기에 윈도우 생성
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024, nullptr, nullptr, hInstance, nullptr);

	// Renderer 클래스 생성
	URenderer renderer;

	// D3D11 생성하는 함수 호출
	renderer.Create(hWnd);

	// 렌더러 생성 직후 쉐이더 생성 함수 호출
	renderer.CreateShader();

	// 상수함수 생성
	renderer.CreateConstantBuffer();


	// ImGui 생성 및 초기화
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	// Renderer와 Shader 생성 이후에 Vertex Buffer 생성
	UINT numVerticesTriangle = sizeof(triangle_vertices) / sizeof(FVertexSimple);
	UINT numVerticesCube = sizeof(cube_vertices) / sizeof(FVertexSimple);
	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);

	// 구 크기 조절 (주어진 구의 radius가 1.f라서 NDC 영억 꽉 채움 -> 크기 1/10 감소
	float scaleMod = 0.1f;

	for (UINT i = 0; i < numVerticesSphere; ++i)
	{
		sphere_vertices[i].x *= scaleMod;
		sphere_vertices[i].y *= scaleMod;
		sphere_vertices[i].z *= scaleMod;
	}

	//Vertex Buffer 선언(triangle, cube, sphere)
	ID3D11Buffer* vertexBufferTriangle = renderer.CreateVertexBuffer(triangle_vertices, sizeof(triangle_vertices));
	ID3D11Buffer* vertexBufferCube = renderer.CreateVertexBuffer(cube_vertices, sizeof(cube_vertices));
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

	// 도형의 움직임 정도를 담을 offset 변수.
	FVector offset(0.f);

	// 도형의 속도를 담을 변수
	FVector velocity(0.f);

	bool bIsExit = false;

	// 각종 생성하는 코드를 여기에 추가한다.
	ETypePrimitive typePrimitive = EPT_Sphere;

	//NDC 경계 영역
	const float leftBorder = -1.f;
	const float rightBorder = 1.f;
	const float topBorder = 1.f;
	const float bottomBorder = -1.f;
	const float sphereRadius = 1.f;

	bool bBoundBallToScreen = true;
	// 핀볼 움직임 여부를 나타내는 bPinballMovement 정의
	bool bPinballMovement = true;
	// 핀볼에 임의의속도를 부여
	//  0.001f 를 조절해공의 초기 속도 조절
	float ballSpeed = 0.0005f;

	velocity.x = ((float)(rand() % 100 - 50)) * ballSpeed;
	velocity.y = ((float)(rand() % 100 - 50)) * ballSpeed;

	// FPS 제한을 위한 설정 (60FPS)
	const int targetFPS = 60;
	const double targetFrameTime = 1000.0 / targetFPS;  // 한 프레임의 목표 시간(밀리초 단위)

	// 고성능 타이머 초기화
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	// FPS 체크를 위한 시작/종료시간
	LARGE_INTEGER startTime, endTime;

	// 경과시간(End - Start)
	double elapsedTime = 0.0;

	// Main Loop(Quit Message가 들어오기 전가지 아래 Loop를 무한히 실행한다)
	while (bIsExit == false)
	{
		// 루프 시간 기록
		QueryPerformanceCounter(&startTime);

		MSG msg;
		// 처리할 메시지가 더 이상 없을 때 까지 수행
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{                            // 키 입력 메시지를 번역
			TranslateMessage(&msg);  // 메세지를 적절한 윈도우 프로시저에 전달,

			// 메시지가 위에서 등록한 WndProc 으로 전달됨
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				bIsExit = true;
				break;
			}
			else if (msg.message == WM_KEYDOWN)  // 키보드 눌렀을 때
			{
				if (msg.wParam == VK_LEFT)
				{
					offset.x -= 0.01f;
				}
				if (msg.wParam == VK_RIGHT)
				{
					offset.x += 0.01f;
				}
				if (msg.wParam == VK_UP)
				{
					offset.y += 0.01f;
				}
				if (msg.wParam == VK_DOWN)
				{
					offset.y -= 0.01f;
				}
			}
			// 키보드 처리 직후에 화면 밖을 벗어났다면 화면 안쪽으로 위치시킨다
			// 화면을 벗어나지 않게 처리
			if (bBoundBallToScreen)
			{
				float renderRadius = sphereRadius * scaleMod;
				if (offset.x < leftBorder + renderRadius)
				{
					offset.x = leftBorder + renderRadius;
				}
				if (offset.x > rightBorder - renderRadius)
				{
					offset.x = rightBorder - renderRadius;
				}
				if (offset.y > topBorder - renderRadius)
				{
					offset.y = topBorder - renderRadius;
				}
				if (offset.y < bottomBorder + renderRadius)
				{
					offset.y = bottomBorder + renderRadius;
				}
			}
		}
		// 핀볼 움직임이 켜져있다면
		if (bPinballMovement)
		{
			// 속도를 공 위치에 더해 공을 실질적으로 움직임
			offset += velocity;

			// 벽과 충돌 여부를 체크하고 충돌 시 속도에 음수를 곱해 방향을 바꿈
			float renderRadius = sphereRadius * scaleMod;
			if (offset.x < leftBorder + renderRadius)
			{
				velocity.x *= -1;
			}
			if (offset.x > rightBorder - renderRadius)
			{
				velocity.x *= -1;
			}
			if (offset.y > topBorder - renderRadius)
			{
				velocity.y *= -1;
			}
			if (offset.y < bottomBorder + renderRadius)
			{
				velocity.y *= -1;
			}
		}

		///////////////////////////////////////
		// 매번 실행되는 코드를 여기에 추가합니다

		// 준비 작업
		renderer.Prepare();
		renderer.PrepareShader();
		renderer.UpdateConstant(offset);

		// 생성한 버텍스 버퍼를 넘겨 실질적인 렌더링 요청(Draw 요청)
		switch (typePrimitive)
		{
		case EPT_Triangle:
			renderer.RenderPrimitive(vertexBufferTriangle,
				numVerticesTriangle);
			break;
		case EPT_Cube:
			renderer.RenderPrimitive(vertexBufferCube, numVerticesCube);
			break;
		case EPT_Sphere:
			renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);
			break;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render()
		// 사이인 이곳에 위치
		ImGui::Begin("Jungle Property Window");
		//**********************IMGUI section Start**********************
		ImGui::Text("Hello Jungle World!");

		ImGui::Checkbox("Bound Ball To Screen", &bBoundBallToScreen);
		// 핀볼 움직임 켜고 끌 수 있는 UI와 연결
		ImGui::Checkbox("PinBall Movement", &bPinballMovement);
		ImGui::Text("Speed: (%f, %f, %f)", velocity.x, velocity.y, velocity.z);

		ImGui::Text("%.3f ms/Frame \n%.1f FPS", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		//**********************IMGUI section End**********************
		ImGui::End();

		ImGui::Render();

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// 현재 화면에 보여지는 버퍼와 그리기 작업을 위한 버퍼를 서로 교환(double-buffering)
		renderer.SwapBuffer();
		///////////////////////////////////////
		
		do
		{
			Sleep(0);

			QueryPerformanceCounter(&endTime);
			elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
		} while (elapsedTime < targetFrameTime);
	}

	// 여기에서 ImGui 소멸
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// 소멸하는 코드를 여기에 추가합니다

	// renderer소멸 전에 vertex buffer소멸 처리
	renderer.ReleaseVertexBuffer(vertexBufferTriangle);
	renderer.ReleaseVertexBuffer(vertexBufferCube);
	renderer.ReleaseVertexBuffer(vertexBufferSphere);

	// 쉐이더 소멸 직전 상수 버퍼 소멸 함수 호출
	renderer.ReleaseConstantBuffer();

	// 렌더러 소멸 직전 쉐이더 소멸 함수 호출
	renderer.ReleaseShader();

	// D3D11 소멸시키는 함수를 호출
	renderer.Release();

	return 0;
}