#pragma once
#pragma unmanaged

template <class DERIVED_TYPE> 
class McBaseW
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE *pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	McBaseW() : m_hwnd(NULL) {}

	BOOL Create( PCWSTR lpWindowName, WCHAR *classSuffix = NULL, BOOL hwAccel = TRUE )
	{
		WNDCLASS wc = {0};

		isActivated = FALSE;

		wc.style = NULL; 
		wc.lpfnWndProc   = DERIVED_TYPE::WindowProc;
		wc.hInstance	 = GetModuleHandle(NULL);

		if (!classSuffix)
			classSuffix = L"ToolWindow";
		
		WCHAR b[1000];
		wsprintf( b, L"Emcee.UI.%s", classSuffix );
		wc.lpszClassName = b;

		RegisterClass(&wc);

		// Use WS_EX_COMPOSITED for hardware-accelerated double buffering when enabled
		DWORD exStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
		if ( hwAccel )
			exStyle |= WS_EX_COMPOSITED;

		m_hwnd = CreateWindowEx(
			exStyle, 
			b, 
			lpWindowName,
			WS_CHILD,
			0,0,1,1,
			GetDesktopWindow(), 
			NULL, 
			GetModuleHandle(NULL), 
			this
			);

		unsigned pva1 = DWMFLIP3D_EXCLUDEBELOW;
		DwmSetWindowAttribute( m_hwnd, DWMWA_FLIP3D_POLICY, &pva1, sizeof( pva1 ) );

		// Enable DWM-based hardware acceleration features when setting is on
		if ( hwAccel )
		{
			// Extend frame into client area for DWM composited rendering
			MARGINS margins = { -1 };
			DwmExtendFrameIntoClientArea( m_hwnd, &margins );
		}

		return (m_hwnd ? TRUE : FALSE);
	}

	static DWORD WINAPI ThreadProc( LPVOID obj )
	{
		if ( obj )
		{
			DERIVED_TYPE *pThis = (DERIVED_TYPE *)obj;
			return pThis->runThread();
		}
		return 1;
	}

	HWND getHwnd() const { return m_hwnd; }
	BOOL isActivated;

protected:

	virtual ~McBaseW() 
	{ 
	}

	virtual PCWSTR  ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual DWORD runThread() = 0;

	HWND m_hwnd;

	int lbX;
	int lbY;
	
	void clearLbInfo()
	{
		if ( useLbInfo() ) lbX=lbY=-1;
	}
	
	BOOL useLbInfo()
	{
		return (GetMessageExtraInfo( ) & 0xFFFFFF00) == 0xFF515700;
	}

	void setLbInfo( int _x, int _y )
	{
		if ( useLbInfo() )
		{
			lbX=_x;
			lbY=_y; 
		}
	}

	BOOL checkLbInfo( int _x, int _y ) 
	{ 
		if (useLbInfo() )
			return (lbX==_x) && (lbY==_y);
		return TRUE;
	}


};
 
