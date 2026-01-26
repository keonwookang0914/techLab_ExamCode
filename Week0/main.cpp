#include <windows.h>

// 먼저 D3D11 관련한 라이브러리와 헤더를 Main 소스 파일에 추가합니다.

// D3D 사용에 필요한 라이브러리들을 링크합니다.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D에 사용할 헤더파일들을 포함합니다.
#include <d3d11.h>
#include <d3dcompiler.h>

// 렌더링 담당 클래스
class URenderer
{
 public:
    // Direct3D 장치(Device)와 장치
};

/*
 * 각종 메세지를 처리할 함수
 * @param hWnd:
 * @param message
 * @param wParam
 * @param lParam
 * @return 콜백함수
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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

//

/*
 * 가장 기본이 되는 메인 함수
 * @param hIntance: 인스턴스
 * @param hPrevInstance: 이전 인스턴스
 * @param lpCmdLine:
 * @param nShowCmd:
 * @return 프로그램 안전 종료 체크 (0이면 안전 아니면 비정상적인 종료)
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd)
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
    HWND hWnd = CreateWindowExW(0, WindowClass, Title,
                                WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
                                nullptr, nullptr, hInstance, nullptr);

    bool bIsExit = false;

    // 각종 생성하는 코드를 여기에 추가한다.
    // Main Loop(Quit Message가 들어오기 전가지 아래 Loop를 무한히 실행한다)
    while (bIsExit == false)
    {
        MSG msg;
        // 처리할 메시지가 더 이상 없을 때 까지 수행
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {                            // 키 입력 메시지를 번역
            TranslateMessage(&msg);  // 메세지를 적절한 윈도우 프로시저에 전달,
                                     // 메시지가 위에서 등록한
            // WndProc 으로 전달됨
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }
        ///////////////////////////////////////
        // 매번 실행되는 코드를 여기에 추가합니다

        ///////////////////////////////////////
    }

    // 소멸하는 코드를 여기에 추가합니다

    return 0;
}