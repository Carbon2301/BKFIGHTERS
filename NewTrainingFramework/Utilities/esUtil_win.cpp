#include "stdafx.h"
#include <windows.h>
#include <cstring>
#include "esUtil.h"



// Main window procedure
LRESULT WINAPI ESWindowProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
{
   LRESULT  lRet = 1; 
   static POINT WDpoint = {0, 0}; // Window position for relative mouse coordinates

   switch (uMsg) 
   { 
      case WM_CREATE:
         break;

      case WM_PAINT:
         {
            ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );
            
            if ( esContext && esContext->drawFunc )
               esContext->drawFunc ( esContext );
            
            ValidateRect( esContext->hWnd, NULL );
         }
         break;

      case WM_DESTROY:
         PostQuitMessage(0);             
         break; 
      
	  case WM_KEYDOWN:
		  {
			  ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );

			  if ( esContext && esContext->keyFunc )
				  esContext->keyFunc ( esContext, (unsigned char) wParam, true );
		  }
		  break;

	  case WM_KEYUP:
		  {
			  ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );

			  if ( esContext && esContext->keyFunc )
				  esContext->keyFunc ( esContext, (unsigned char) wParam, false );
		  }
		  break;

	  case WM_SYSKEYDOWN:
		  {
			  ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );

			  if ( esContext && esContext->keyFunc )
				  esContext->keyFunc ( esContext, (unsigned char) wParam, true );
		  }
		  break;

	  case WM_SYSKEYUP:
		  {
			  ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );

			  if ( esContext && esContext->keyFunc )
				  esContext->keyFunc ( esContext, (unsigned char) wParam, false );
		  }
		  break;

      case WM_LBUTTONDOWN:
         {
            ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(hWnd, &point);
            
            if ( esContext && esContext->mouseFunc )
               esContext->mouseFunc ( esContext, point.x, point.y, true );
         }
         break;

      case WM_LBUTTONUP:
         {
            ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(hWnd, &point);
            
            if ( esContext && esContext->mouseFunc )
               esContext->mouseFunc ( esContext, point.x, point.y, false );
         }
         break;

      case WM_MOUSEMOVE:
         {
            ESContext *esContext = (ESContext*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(hWnd, &point);
            
            if ( esContext && esContext->mouseMoveFunc )
               esContext->mouseMoveFunc ( esContext, point.x, point.y );
         }
         break;

      case WM_MOVE:
         {
            WDpoint.x = (int)LOWORD(lParam); // horizontal position
            WDpoint.y = (int)HIWORD(lParam); // vertical position
         }
         break;
         
      default: 
         lRet = DefWindowProc (hWnd, uMsg, wParam, lParam); 
         break; 
   } 

   return lRet; 
}

//      Create Win32 instance and window
GLboolean WinCreate ( ESContext *esContext, const char *title )
{
   WNDCLASS wndclass = {0}; 
   DWORD    wStyle   = 0;
   RECT     windowRect;
   HINSTANCE hInstance = GetModuleHandle(NULL);


   wndclass.style         = CS_OWNDC;
   wndclass.lpfnWndProc   = (WNDPROC)ESWindowProc; 
   wndclass.hInstance     = hInstance; 
   wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); 
   wndclass.lpszClassName = "opengles2.0"; 

   if (!RegisterClass (&wndclass) ) 
      return FALSE; 

   // Default style for windowed mode; may be overridden for fullscreen scaling
   wStyle = WS_VISIBLE | WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION;
   
   // Adjust the window rectangle so that the client area has
   // the correct number of pixels
   windowRect.left = 0;
   windowRect.top = 0;
   windowRect.right = esContext->width;
   windowRect.bottom = esContext->height;

   AdjustWindowRect ( &windowRect, wStyle, FALSE );

   esContext->hWnd = CreateWindow(
                         "opengles2.0",
                         title,
                         wStyle,
                          0,
                          0,
                          windowRect.right - windowRect.left,
                          windowRect.bottom - windowRect.top,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);

   // Set the ESContext* to the GWL_USERDATA so that it is available to the 
   // ESWindowProc
   SetWindowLongPtr (  esContext->hWnd, GWL_USERDATA, (LONG) (LONG_PTR) esContext );

   if ( esContext->hWnd == NULL )
      return GL_FALSE;

   ShowWindow ( esContext->hWnd, TRUE );

   {
       size_t titleLen = strlen(title);
       bool wantFullscreenBorderless = (titleLen >= 4 && strcmp(title + (titleLen - 4), "[FS]") == 0);
       if (wantFullscreenBorderless) {
           RECT desktopRect;
           const HWND hDesktop = GetDesktopWindow();
           if (GetWindowRect(hDesktop, &desktopRect)) {
               LONG fullWidth = desktopRect.right - desktopRect.left;
               LONG fullHeight = desktopRect.bottom - desktopRect.top;

               // Make window borderless and top-level
               LONG style = GetWindowLong(esContext->hWnd, GWL_STYLE);
               style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU | WS_BORDER | WS_OVERLAPPED | WS_OVERLAPPEDWINDOW);
               style |= WS_POPUP | WS_VISIBLE;
               SetWindowLong(esContext->hWnd, GWL_STYLE, style);

               SetWindowPos(esContext->hWnd, HWND_TOP, 0, 0, fullWidth, fullHeight, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
           }
       }
   }

   return GL_TRUE;
}



//      Start main windows loop
void WinLoop ( ESContext *esContext )
{
   MSG msg = { 0 };
   int done = 0;
   DWORD lastTime = GetTickCount();
   
   while (!done)
   {
      int gotMsg = (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
      DWORD curTime = GetTickCount();
      float deltaTime = (float)( curTime - lastTime ) / 1000.0f;
      lastTime = curTime;

      if ( gotMsg )
      {
         if (msg.message==WM_QUIT)
         {
             done=1; 
         }
         else
         {
             TranslateMessage(&msg); 
             DispatchMessage(&msg); 
         }
      }
      else
         SendMessage( esContext->hWnd, WM_PAINT, 0, 0 );

      // Call update function if registered
      if ( esContext->updateFunc != NULL )
         esContext->updateFunc ( esContext, deltaTime );
   }
}
