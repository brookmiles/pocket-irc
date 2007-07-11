#include <windows.h>
#include <tchar.h>
#include <imgdecmp.h>

static DWORD CALLBACK GetImageData(LPSTR szBuffer, DWORD dwBufferMax, LPARAM lParam)
{
	DWORD dwNumberOfBytesRead;

	if((HANDLE)lParam != INVALID_HANDLE_VALUE)
	{
		ReadFile((HANDLE)lParam, szBuffer, dwBufferMax, &dwNumberOfBytesRead, NULL );
	}
	else
	{
		return 0;
	}

	return dwNumberOfBytesRead;
}

static void CALLBACK ImageProgress(IImageRender *pRender, BOOL bComplete, LPARAM lParam)
{
	if(bComplete)
	{
		// rah rah
	}
}

HBITMAP ImageFileLoad(LPCTSTR pszFileName)
{
	HBITMAP hbm = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BYTE    szBuffer[1024];
	DecompressImageInfo	dii;
	HDC hdc;

	hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	hdc = GetDC(NULL);

//Step 4: Fill in the 'DecompressImageInfo' structure
	dii.dwSize = sizeof( DecompressImageInfo );		// Size of this structure
	dii.pbBuffer = szBuffer;											// Pointer to the buffer to use for data
	dii.dwBufferMax = sizeof(szBuffer);												// Size of the buffer
	dii.dwBufferCurrent = 0;											// The amount of data which is current in the buffer
	dii.phBM = &hbm;										// Pointer to the bitmap returned (can be NULL)
	dii.ppImageRender = NULL;											// Pointer to an IImageRender object (can be NULL)
	dii.iBitDepth = GetDeviceCaps(hdc,BITSPIXEL);// Bit depth of the output image
	dii.lParam = ( LPARAM ) hFile;								// User parameter for callback functions
	dii.hdc = hdc;																// HDC to use for retrieving palettes
	dii.iScale = 100;												// Scale factor (1 - 100)
	dii.iMaxWidth = GetSystemMetrics(SM_CXSCREEN);									// Maximum width of the output image
	dii.iMaxHeight = GetSystemMetrics(SM_CYSCREEN);								// Maxumum height of the output image
	dii.pfnGetData = GetImageData;								// Callback function to get image data
	dii.pfnImageProgress = ImageProgress;					// Callback function to notify caller of progress decoding the image
	dii.crTransparentOverride = ( UINT ) -1;			// If this color is not (UINT)-1, it will override the
																								// transparent color in the image with this color. (GIF ONLY)
//Step 6: Call DecompressImageIndirect

	DecompressImageIndirect( &dii );

	// Clean up 
	CloseHandle(hFile);
	ReleaseDC(NULL, hdc);

	return hbm;
}

