#include <windows.h>
#include <Tlhelp32.h>
#include <wingdi.h>
#include <stdio.h>
#include <stdbool.h>
#include <cstdint>
#include <string>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <ctype.h>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif


#pragma comment(lib,"User32.lib")
#pragma comment(lib,"d3d9.lib")

// Returns the milliseconds since system start
int64_t GetTickCountInt64(void) {
	return (int64_t) GetTickCount64();
}

int digits_only(const char *s) {
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

int ExecuteFrameProcess(int x, int y, int w, int h, int screen_width, int screen_height);

void set_stdout_to_binary_mode() {
	#ifdef _WIN32
	setmode(fileno(stdout),O_BINARY);
	setmode(fileno(stdin),O_BINARY);
	#endif
}

int main(int argn, char ** argv) {
	if (argn != 5 && argn != 6) {
		printf(
			"This program takes RGB data from the display and write it to stdout\n"
			"Usage:\n"
			"Stream raw RGB binary data from screen in 3 channels of 1 byte each\n"
			"\n"
			"\t%s <left> <top> <width> <height>\n"
			"\n"
			"Get a single frame of RGB data from screen instead of a continous stream\n"
			"\n"
			"\t%s -s <x> <y> <width> <height>\n"
			"\n",
			argv[0],
			argv[0]
		);
		return __LINE__;
	}
	int left = -1, top = -1, width = -1, height = -1;

	for (int arg_index = (argn == 6) ? 2 : 1; arg_index < argn; arg_index++) {
		if (!digits_only(argv[arg_index]) || argv[arg_index][0] == '\0') {
			fprintf(stderr, "Expected %dth argument to be number, got '%s'\n", arg_index - 1, argv[arg_index]);
			return __LINE__;
		}
		if (left == -1) {
			left = atoi(argv[arg_index]);
			continue;
		}
		if (top == -1) {
			top = atoi(argv[arg_index]);
			continue;
		}
		if (width == -1) {
			width = atoi(argv[arg_index]);
			continue;
		}
		if (height == -1) {
			height = atoi(argv[arg_index]);
			continue;
		}
	}

	int screen_width = (int) GetSystemMetrics(SM_CXSCREEN);
	int screen_height = (int) GetSystemMetrics(SM_CYSCREEN);

	if (argn == 6) {
		if (argv[1][0] = '-' && argv[1][1] == 's' && argv[1][2] == '\0') {
			set_stdout_to_binary_mode();
			return ExecuteFrameProcess(left, top, width, height, screen_width, screen_height);
		} else {
			fprintf(stderr, "Unknown first parameter, expected '-s', got '%s'\n", argv[1]);
			return __LINE__;
		}
	}
	set_stdout_to_binary_mode();
	while (true) {
		int line = ExecuteFrameProcess(left, top, width, height, screen_width, screen_height);
		if (line != 0) {
			return line;
		}
		fflush(stdout);
	}
	return 0;
}

// DirectX method to capture screen
#include <D3D9.h>

#ifdef D3DADAPTER_DEFAULT

HRESULT Direct3D9TakeScreenshot(uint32_t adapter, LPBYTE *pBuffer, uint32_t *pStride, const RECT *pInputRc);

#define GetBGRAPixel(b,s,x,y)       (((LPDWORD)(((LPBYTE)b) + y * s))[x])
#define GetBGRAPixelBlue(p)         (LOBYTE(p))
#define GetBGRAPixelGreen(p)        (HIBYTE(p))
#define GetBGRAPixelRed(p)          (LOBYTE(HIWORD(p)))
#define GetBGRAPixelAlpha(p)        (HIBYTE(HIWORD(p)))
#ifndef HRCHECK
	#define HRCHECK(__expr) {hr=(__expr);if(FAILED(hr)){wprintf(L"FAILURE 0x%08X (%i)\n\tline: %u file\n", hr, hr, __LINE__);goto cleanup;}}
#endif

LPBYTE pixel_buffer = NULL;

int ExecuteFrameProcess(int x, int y, int w, int h, int screen_width, int screen_height) {
	LONG left = x;
	LONG top = y;
	LONG width = w;
	LONG height = h;
	uint32_t stride;
	RECT rc = { left, top, left + width, top + height };

	int ret = Direct3D9TakeScreenshot((uint32_t) D3DADAPTER_DEFAULT, &pixel_buffer, &stride, &rc);
	if (ret != 0) {
		return ret;
	}

	uint8_t buffer[3];
	DWORD pixel;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {

			if (x > screen_width || x < 0 || y > screen_height || y < 0) {
				buffer[0] = 0;
				buffer[1] = 0;
				buffer[2] = 0;
			} else {
				pixel = GetBGRAPixel(pixel_buffer, stride, x, y);
				buffer[0] = (uint8_t) GetBGRAPixelRed(pixel);
				buffer[1] = (uint8_t) GetBGRAPixelGreen(pixel);
				buffer[2] = (uint8_t) GetBGRAPixelBlue(pixel);
			}
			// fprintf(stdout, "%d %d = %d %d %d\n", (int32_t) x, (int32_t) y, (int32_t) buffer[0], (int32_t) buffer[1], (int32_t) buffer[2]);
			fwrite(buffer, 1, 3, stdout);
		}
	}
	return 0;
}

HRESULT hr = S_OK;
IDirect3D9 *d3d = nullptr;
IDirect3DDevice9 *device = nullptr;
IDirect3DSurface9 *surface = nullptr;
D3DPRESENT_PARAMETERS parameters = { 0 };
D3DDISPLAYMODE mode;
D3DLOCKED_RECT rc;

HRESULT Direct3D9TakeScreenshot(uint32_t adapter, LPBYTE *pBuffer, uint32_t *pStride, const RECT *pInputRc) {
	static int has_initialized = 0;
	*pStride = 0;
	if (has_initialized == 0) {
		// init D3D and get screen size
		d3d = Direct3DCreate9(D3D_SDK_VERSION);
		HRCHECK(d3d->GetAdapterDisplayMode(adapter, &mode));
	}

	LONG width = pInputRc ? (pInputRc->right - pInputRc->left) : mode.Width;
	LONG height = pInputRc ? (pInputRc->bottom - pInputRc->top) : mode.Height;

	if (has_initialized == 0) {
		parameters.Windowed = TRUE;
		parameters.BackBufferCount = 1;
		parameters.BackBufferHeight = height;
		parameters.BackBufferWidth = width;
		parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
		parameters.hDeviceWindow = NULL;
	}

	if (has_initialized == 0) {
		// create device & capture surface (note it needs desktop size, not our capture size)
		HRCHECK(d3d->CreateDevice(adapter, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &device));
		HRCHECK(device->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, nullptr));
		has_initialized = 1;
	}

	// get pitch/stride to compute the required buffer size
	HRCHECK(surface->LockRect(&rc, pInputRc, 0));
	*pStride = rc.Pitch;
	HRCHECK(surface->UnlockRect());

	if (!*pBuffer) {
		// allocate buffer
		*pBuffer = (LPBYTE)LocalAlloc(0, *pStride * height);
	}
	if (!*pBuffer) {
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	// get the data
	HRCHECK(device->GetFrontBufferData(0, surface));

	// copy it into our buffer
	HRCHECK(surface->LockRect(&rc, pInputRc, 0));
	CopyMemory(*pBuffer, rc.pBits, rc.Pitch * height);
	HRCHECK(surface->UnlockRect());

	cleanup:
	if (FAILED(hr)) {
		if (*pBuffer) {
			LocalFree(*pBuffer);
			*pBuffer = NULL;
		}
		*pStride = 0;
		if (surface) {
			surface->Release();
			surface = nullptr;
		}
		if (device) {
			device->Release();
			device = nullptr;
		}
		if (d3d) {
			d3d->Release();
			d3d = nullptr;
		}
		return hr;
	}
	return 0;
}

#endif
