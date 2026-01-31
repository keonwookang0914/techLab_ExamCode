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


//forward declarations
class UPrimitive;
class UBall;
class URenderer;
struct FVector3;
struct FVertexSimple;

// UBall 객체를 보관할 배열 -> 많은 함수에서 접근하기 때문에 전역 변수로 선언
UPrimitive** PrimitiveList;


// 정점 정의 구조체
struct FVertexSimple
{
	float x, y, z;     // position
	float r, g, b, a;  // color
};

// 3차원 Vector 구조체
struct FVector3
{
	float x, y, z;
	FVector3(float _x = 0.f, float _y = 0.f, float _z = 0.f) : x(_x), y(_y), z(_z) {}

	static float DotProduct(const FVector3& lhs, const FVector3& rhs)
	{
		return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
	}

	static FVector3 CrossProduct(const FVector3& lhs, const FVector3& rhs)
	{
		return {
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		};
	}

	float Dot(const FVector3& rhs)
	{
		return DotProduct(*this, rhs);
	}

	FVector3 Cross(const FVector3& rhs)
	{
		return CrossProduct(*this, rhs);
	}

	FVector3 operator+(const FVector3& rhs) const
	{
		return { x + rhs.x, y + rhs.y, z + rhs.z };
	}
	FVector3& operator+=(const FVector3& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
	FVector3 operator-(const FVector3& rhs) const
	{
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}

	FVector3 operator-() const
	{
		return { -x, -y, -z };
	}

	FVector3& operator-=(const FVector3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	FVector3 operator*(const float rhs) const
	{
		return { x * rhs, y * rhs, z * rhs };
	}

	friend FVector3 operator*(const float lhs, const FVector3& rhs)
	{
		return rhs * lhs;
	}

	FVector3& operator*=(const float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	FVector3 operator/(const float rhs) const
	{
		return { x / rhs, y / rhs, z / rhs };
	}

	FVector3& operator/=(const float rhs)
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

	FVector3& Normalize()
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
};

class UPrimitive
{
public:
	UPrimitive() = default;

	virtual ~UPrimitive() = default; //안전한 자식 소멸자 호출을 위해 부모 소멸자를 가상함수로 선언
};

class UBall : public UPrimitive
{
public:
	FVector3 Location;	// 공의 위치
	FVector3 Velocity;	// 공의 속도
	float Radius;		// 공의 크기
	float Mass;			// 공의 질량

	static int TotalNumBalls;	// 생성된 총 UBall의 개수를 보관하는 변수
	static bool bApplyGravity;	// 중력 적용 여부 체크용 bool 변수

	UBall() :
		Location{ //위치의 범위는 -0.5 ~ 0.5로 설정
			(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX))) * 1.0f - 0.5f,
			(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX))) * 1.0f - 0.5f,
			0.f}
		, Velocity{ //속도는 -0.5 ~ 0.5로 설정
			(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX))) * 1.f - 0.5f,
			(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX))) * 1.f - 0.5f,
			0.f}
		, Radius{ // 공의 크기는 0.1~0.2로 설정
			(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX))) * 0.1f + 0.1f }
		, Mass{ //질량이 크기에 비례하게 설정
			Radius 
		}
	{
		++TotalNumBalls; // 생성자 호출시 카운트 1 증가
	}

	virtual ~UBall() override
	{
		--TotalNumBalls;  // 소멸자 호출 시 카운트 1 감소
	}

	// 벽과의 충돌 처리 + 위치 보정(위치를 지정하지 않으면 화면 밖으로 공이 밀려난다)
	void HandleBoundaryCollision()
	{
		if (Location.x > 1.f - Radius)
		{
			Velocity.x *= -1;
			Location.x = 1.f - Radius;
		}
		if (Location.x < -1.f + Radius)
		{
			Velocity.x *= -1;
			Location.x = -1.f + Radius;
		}
		if (Location.y > 1.f - Radius)
		{
			Velocity.y *= -1;
			Location.y = 1.f - Radius;
		}
		if (Location.y < -1.f + Radius)
		{
			Velocity.y *= -1;
			Location.y = -1.f + Radius;
		}
	}

	//공을 움직이게 하는 함수 (등가속도 또는 등속도 운동 수행)
	void MoveAccelerate()
	{
		constexpr float FixedTimeStep = 1.f / 60.f; //물리 연산이기 때문에 규칙적인 호출인 Fixed Update 사용(일정 간격)

		if (bApplyGravity) //중력 적용 ( 중력 계수 1.0f)
		{
			Velocity.y -= 1.f * FixedTimeStep;
		}

		Location += Velocity * FixedTimeStep;

		HandleBoundaryCollision();
	}

	// 공을 움직이게 하는 함수(각속도 운동)
	void MoveAngular()
	{
		/*
		* 회전 중심으로부터 r만큼떨어진 물체의 속도 v와 각속도 w사이의 관계
		* v = w x r
		* 물체의 운동이 평면상에서 이루어지는 경우 r과 w가 수직이 되어 아래와 같이 각속도에 대해 식을 쓸 수 있다
		* w = r x v / (abs(r) * abs(r)) 
		* 이를 사용해 velocity를 각속도 운동으로 변환
		*/

		//중점은 FVector(0, 0, 0) 이라고 가정 -> Location을 r로 간주.

		constexpr float FixedTimeStep = 1.f / 60.f; //물리 연산이기 때문에 고정 업데이트 사용

		// 현재 속도값을 사용해 각속도 계산 
		// w = r x v / (abs(r) * abs(r))
		FVector3 AngularVelocity = FVector3::CrossProduct(Location, Velocity);
		AngularVelocity /= Location.LengthSquare();

		// 각속도를 기반으로 원운동 궤적 생성하기
		// v = ω × r
		Velocity = FVector3::CrossProduct(AngularVelocity, Location);

		// 이를 Location에 적용
		Location += Velocity * FixedTimeStep;

		HandleBoundaryCollision();
	}
};
int UBall::TotalNumBalls = 0; // 0으로 초기화(이후 생성자에서 1 증가, 소멸자에서 1 감소)
bool UBall::bApplyGravity = true;


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
		FVector3 Offset;	// 공 위치 오프셋
		float Scale;		// 공의 크기
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
	/*
	* UBall의 크기 정보 처리 필요 -> constant buffer를 16바이트로 맞추기 위해 억지로 넣은 Pad 변수를 활용
	* - Scale값을 넘겨 VertexShader에서 크기를 처리
	*/
	void UpdateConstant(FVector3 Offset, float Scale)
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
				constants->Scale = Scale;
			}
			DeviceContext->Unmap(ConstantBuffer, 0);
		}
	}
};
#pragma endregion

// UBall::TotalNumBalls > LastNumberOfBalls (사용자가 입력한 공의 개수가 현재 배열에 있는 공의 개수보다 적은 경우, 공 삭제)
void RemoveBalls(int diff)
{
	if (UBall::TotalNumBalls - diff <= 0) return; //만약 diff개 만큼 삭제 후 남은 공의 개수가 0 이하라면 삭제 불가능(조건 위반)

	int CacheTotalNumBalls;

	for (int i = 0; i < diff; ++i) // diff개 공을 해제해야함.
	{
		CacheTotalNumBalls = UBall::TotalNumBalls; // 현재 배열의 길이 캐싱
		//삭제할 공 index 선정
		int idx = rand() % (UBall::TotalNumBalls); // 현재 배열에서 삭제할 원소 하나 선택.
		
		//해당 원소 할당 해제 (가상함수로 소멸자 선언했기 때문에 적절한 소멸자가 호출됨)
		delete PrimitiveList[idx]; //객체 즉시 소멸

		UPrimitive** NewPrimitiveList = new UPrimitive * [UBall::TotalNumBalls]; //감소된 갯수만큼의 동적 배열 할당.

		//copy 작업 수행
		int NewIdx = 0;
		for (int j = 0; j < CacheTotalNumBalls; ++j)
		{
			if (j == idx) continue;
			NewPrimitiveList[NewIdx] = PrimitiveList[j];
			++NewIdx;
		}
		delete[] PrimitiveList;

		PrimitiveList = NewPrimitiveList;
	}
}

// LastNumberOfBalls > UBall::TotalNumBalls (사용자가 입력한 공의 개수가 현재 배열에 있는 공의 개수보다 많은 경우, 공 추가)
void AddBalls(int diff)
{
	//기존 배열에 diff개 만큼의 공을 추가로 할당해야함.
	if (diff <= 0) return;

	UPrimitive** NewPrimitiveList = new UPrimitive*[UBall::TotalNumBalls + diff];

	int PrevNumberOfBalls = UBall::TotalNumBalls; //TotalNumBalls 값 복사

	for (int i = 0; i < PrevNumberOfBalls; ++i)
	{
		NewPrimitiveList[i] = PrimitiveList[i];
	}

	for (int i = PrevNumberOfBalls; i < PrevNumberOfBalls + diff; ++i)
	{
		NewPrimitiveList[i] = new UBall();
	}

	delete[] PrimitiveList;

	PrimitiveList = NewPrimitiveList;
}

// 두 공의 충돌 여부 판단 및 탄성 충돌로 인해 새로 생긴 속도를 게산하는 함수
void CheckElasticCollision()
{
	//두 공을 각각 보면서 충돌이 발생했는지 체크
	for (int i = 0; i < UBall::TotalNumBalls; ++i) for (int j = i + 1; j < UBall::TotalNumBalls; ++j)
	{
		UBall* Ball1 = static_cast<UBall*>(PrimitiveList[i]);
		UBall* Ball2 = static_cast<UBall*>(PrimitiveList[j]);

		//두 공의 중심 거리의 제곱 (연산 편의성)
		float DistanceSquare = (Ball1->Location - Ball2->Location).LengthSquare();
		if (DistanceSquare > (Ball1->Radius + Ball2->Radius) * (Ball1->Radius + Ball2->Radius)) //두 구의 반지름의 합 보다 거리가 크다면 두 원은 떨어져 있는 것이다.
		{
			continue; //충돌하지 않음.
		}
		
		// Ball2->Ball1 단위 벡터
		FVector3 Normal = Ball1->Location - Ball2->Location; 
		Normal.Normalize();

		// 두 공이 겹치는 부분의 절반씩 밀어내기 (겹침 현상 해결)
		float Distance = sqrtf(DistanceSquare);
		float Overlap = ((Ball1->Radius + Ball2->Radius) - Distance) / 2.f;

		//각 공을 반대방향으로 밀치기
		Ball1->Location += Normal * Overlap;
		Ball2->Location -= Normal * Overlap;

		// 1차원 뉴턴 충돌 계산을 위한 각 공의 속도를 단위벡터로 투영하여 1차원 상의 속도 구하기
		float v1 = FVector3::DotProduct(Ball1->Velocity, Normal);
		float v2 = FVector3::DotProduct(Ball2->Velocity, Normal);

		//각 공의 질량
		float m1 = Ball1->Mass;
		float m2 = Ball2->Mass;

		// 새로운 속도 계산
		float NewVelocity1 = ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
		float NewVelocity2 = ((m2 - m1) * v2 + 2 * m1 * v1) / (m1 + m2);

		// 법선 방향의 속도만 수정(법선 방향)
		/*
		* 왜 v1, v2을 빼야하는가?
		* - Ball->Velocity는 Normal(법선) 벡터 와 Tangent(접선) 벡터의 합으로 이루어져있음
		* - 따라서 Ball->Velocity = v1 * Normal + Tangent임.
		* - 근데 새로 계산된 속도를 법선 방향에 적용해야함.
		* - 따라서 v1 * Normal값을 제거하기 위해 NewVelocity1 - v1을 대입
		*/
		Ball1->Velocity += Normal * (NewVelocity1 - v1);
		Ball2->Velocity += Normal * (NewVelocity2 - v2);
	}
}

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
	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);

	//Vertex Buffer 선언(sphere)
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

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

	//초기 1개의 구체 생성
	UBall* initBall = new UBall();

	// 생성된 공을 PrimitiveList에서 가리키게 지정.
	PrimitiveList = new UPrimitive * [UBall::TotalNumBalls];	
	PrimitiveList[0] = initBall;
	
	//더이상 사용하지않는 initBall 포인터 변수 처리
	initBall = nullptr;

	//이전 프레임의 공의 개수를 보관하는 변수
	int LastNumberOfBalls = UBall::TotalNumBalls;

	bool bApplyAngularVelocity = false;

	bool bIsExit = false;
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
		}

		/* 
		* 이 부분에서 TotalNumBalls 개수를 체크해서 객체 할당 또는 해제 진행
		*/
		//UBall 클래스의 생성자와 소멸자에서 TotalNumBalls 관리. 따로 신경X
		if (LastNumberOfBalls < UBall::TotalNumBalls) // 사용자가 더 적은 개수의 공을 입력 -> 차이 만큼 공을 제거
		{
			RemoveBalls(UBall::TotalNumBalls - LastNumberOfBalls);
		}
		else if (LastNumberOfBalls > UBall::TotalNumBalls) // 사용자가 더 많은 개수의 공을 입력 -> 차이만큼 공을 추가
		{
			AddBalls(LastNumberOfBalls - UBall::TotalNumBalls);
		}

		// 공 움직임 적용
		for (int i = 0; i < UBall::TotalNumBalls; ++i)
		{
			UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);

			if (bApplyAngularVelocity)
			{
				Ball->MoveAngular();
			}
			else
			{
				Ball->MoveAccelerate();
			}
		}

		//충돌 검사 및 탄성 충돌
		CheckElasticCollision();

		///////////////////////////////////////
		// 매번 실행되는 코드를 여기에 추가합니다

		// 준비 작업
		renderer.Prepare();
		renderer.PrepareShader();

		for (int i = 0; i < UBall::TotalNumBalls; ++i)
		{
			// 상수 버퍼 갱신
			UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);
			renderer.UpdateConstant(Ball->Location, Ball->Radius);

			// 생성한 버텍스 버퍼를 넘겨 실질적인 렌더링 요청(Draw 요청)
			renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render()
		// 사이인 이곳에 위치
		ImGui::Begin("Jungle Property Window");
		//**********************IMGUI section Start**********************
		ImGui::Text("Hello Jungle World!");
		
		ImGui::BeginDisabled(bApplyAngularVelocity);
		ImGui::Checkbox("Gravity", &UBall::bApplyGravity);
		ImGui::EndDisabled();

		ImGui::BeginDisabled(UBall::bApplyGravity);
		ImGui::Checkbox("Angular Velocity", &bApplyAngularVelocity);
		ImGui::EndDisabled();

		ImGui::InputInt("Number of Balls", &LastNumberOfBalls);
		LastNumberOfBalls = max(LastNumberOfBalls, 1); //공의 개수가 1 이하로 떨어지지 않게 조절	
		

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
	renderer.ReleaseVertexBuffer(vertexBufferSphere);

	// 쉐이더 소멸 직전 상수 버퍼 소멸 함수 호출
	renderer.ReleaseConstantBuffer();

	// 렌더러 소멸 직전 쉐이더 소멸 함수 호출
	renderer.ReleaseShader();

	// D3D11 소멸시키는 함수를 호출
	renderer.Release();
	

	// 남은 Ball 객체 소멸
	// delete를 할 때 마다 UBall::TotalNumBalls 값이 변하기 때문에 값을 복사해서 사용
	int TotalBallCnt = UBall::TotalNumBalls;

	for (int i = 0; i < TotalBallCnt; i++)
	{
		delete PrimitiveList[i];
	}
	delete[] PrimitiveList;

	return 0;
}