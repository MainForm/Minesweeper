#include <Windows.h>
#include <time.h>

#include "resource.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("지뢰 찾기");
HWND hMain;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

#define BLOCK_SIZE_X (20)
#define BLOCK_SIZE_Y (20)
#define START_HEGHIT (50)

POINT max_pt;
int max_mines;
BOOL bFirstClick;
BOOL bGameStart;

int BlockColor[100][100];
TCHAR BlockText[100][100];
TCHAR Mines[100][100];

void OnLButtonDown(int x, int y);
int GetCountAround(int x, int y, TCHAR ch);
int GetRemainBlock();
void SetMines(int x,int y,int cnt_mines);

void GameStart();

POINT pt;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	static LPMINMAXINFO pmmi;

	//WM_PAINT 변수들
	PAINTSTRUCT ps;
	HBRUSH OldBrush,Brushs[2];

	switch (iMessage) {
	case WM_CREATE:
		hMain = hWnd;

		srand(time(NULL));

		GameStart();
		return 0;
	case WM_GETMINMAXINFO:	//블럭 갯수에 맞게 윈도우 크기 고정
		pmmi = (LPMINMAXINFO)lParam;
		pmmi->ptMaxSize = pmmi->ptMinTrackSize = pmmi->ptMaxTrackSize = pt;
		pmmi->ptMaxPosition.x = 100;
		pmmi->ptMaxPosition.y = 100;
		return 0;
	case WM_LBUTTONDOWN:
		if (HIWORD(lParam) > START_HEGHIT) {

			if (!bGameStart)
				return 0;
			
			int x = LOWORD(lParam) / BLOCK_SIZE_X;
			int y = (HIWORD(lParam) - START_HEGHIT) / BLOCK_SIZE_Y;

			if (bFirstClick) {
				SetMines(x, y, max_mines);
				bFirstClick = FALSE;
			}

			if (BlockColor[x][y] == 0 && BlockText[x][y] == 0) {
				OnLButtonDown(x, y);
			}
			else {
				if (GetCountAround(x, y, '!') == BlockText[x][y] - '0') {
					for (int iy = -1; iy <= 1; iy++) {
						for (int ix = -1; ix <= 1; ix++) {
							int mx = x + ix;
							int my = y + iy;

							if (mx < 0 || my < 0 || mx > max_pt.x || my > max_pt.y)
								continue;

							if (BlockText[mx][my] == '!')
								continue;

							OnLButtonDown(mx, my);
						}
					}
				}
			}
			

			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_RBUTTONUP:
		if (HIWORD(lParam) > START_HEGHIT) {

			if (!bGameStart)
				return 0;

			int x = LOWORD(lParam) / BLOCK_SIZE_X;
			int y = (HIWORD(lParam) - START_HEGHIT) / BLOCK_SIZE_Y;

			if (BlockColor[x][y] == 0) {
				switch (BlockText[x][y]) {
				case 0:
					BlockText[x][y] = '!';
					break;
				case '!':
					BlockText[x][y] = '?';
					break;
				case '?':
					BlockText[x][y] = 0;
					break;
				}
			}

			InvalidateRect(hWnd,NULL, TRUE);
		}
		return 0;
	case WM_PAINT:
		TCHAR Msg[20] = TEXT("");
		int cnt = 0;

		BeginPaint(hWnd, &ps);

		Brushs[0] = CreateSolidBrush(RGB(70, 150, 240));
		Brushs[1] = CreateSolidBrush(RGB(100, 215, 155));
		OldBrush = (HBRUSH)SelectObject(ps.hdc, Brushs[1]);

		SetBkMode(ps.hdc, TRANSPARENT);


		for (int y = 0; y < max_pt.y; y++)
			for (int x = 0; x < max_pt.x; x++)
				if (BlockText[x][y] == '!')
					cnt++;

		wsprintf(Msg, TEXT("남은 지뢰 갯수 : %d"), max_mines - cnt);
		TextOut(ps.hdc, 10, 20, Msg, lstrlen(Msg));

		for (int ix = 0; ix < max_pt.x; ix++) {
			for (int iy = 0; iy < max_pt.y; iy++) {
				TCHAR txt[2] = TEXT("");

				SelectObject(ps.hdc, Brushs[BlockColor[ix][iy]]);
				Rectangle(ps.hdc, ix * BLOCK_SIZE_X, iy * BLOCK_SIZE_Y + START_HEGHIT,
					(ix * BLOCK_SIZE_X) + BLOCK_SIZE_X, (iy * BLOCK_SIZE_Y + START_HEGHIT) + BLOCK_SIZE_Y);

				if (BlockText[ix][iy]) {
					txt[0] = BlockText[ix][iy];
					TextOut(ps.hdc, ix * BLOCK_SIZE_X + 5, iy * BLOCK_SIZE_Y + START_HEGHIT + 2, txt, 1);
				}
			}
		}

		SelectObject(ps.hdc, OldBrush);

		EndPaint(hWnd, &ps);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDM_RESTART:
			GameStart();
			break;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON1:
			SetDlgItemInt(hDlg, IDC_EDIT1, 9, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT2, 9, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT3, 10, FALSE);
			return TRUE;
		case IDC_BUTTON2:
			SetDlgItemInt(hDlg, IDC_EDIT1, 16, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT2, 16, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT3, 40, FALSE);
			return TRUE;
		case IDC_BUTTON3:
			SetDlgItemInt(hDlg, IDC_EDIT1, 30, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT2, 16, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT3, 99, FALSE);
			return TRUE;
		case IDOK:
			max_pt.x = GetDlgItemInt(hDlg, IDC_EDIT1, NULL,FALSE);
			max_pt.y = GetDlgItemInt(hDlg, IDC_EDIT2, NULL,FALSE);
			pt.x = (max_pt.x * BLOCK_SIZE_X) + 16;
			pt.y = (max_pt.y * BLOCK_SIZE_Y) + 39 + START_HEGHIT + 20;

			max_mines = GetDlgItemInt(hDlg, IDC_EDIT3, NULL, FALSE);
			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void GameStart() {
	int iDialogReturn = 0;

	iDialogReturn = DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hMain, DlgProc);

	if (iDialogReturn == IDCANCEL)
		return;

	memset(Mines, 0, sizeof(Mines));
	memset(BlockColor, 0, sizeof(BlockColor));
	memset(BlockText, 0, sizeof(BlockText));

	bGameStart = TRUE;
	bFirstClick = TRUE;

	InvalidateRect(hMain, NULL, TRUE);

	SetWindowPos(hMain, 0, 0, 0, pt.x, pt.y, SWP_NOMOVE);
}

void OnLButtonDown(int x, int y) {
	int cnt_mines = 0;

	if (BlockColor[x][y] == 1)
		return;

	BlockColor[x][y] = 1;

	if (Mines[x][y] == '*') {
		bGameStart = FALSE;
		for (int iy = 0; iy < max_pt.y; iy++)
			for (int ix = 0; ix < max_pt.x; ix++) 
				if (Mines[ix][iy] == '*')
					BlockText[ix][iy] = Mines[ix][iy];

		MessageBox(hMain, TEXT("게임 끝"), TEXT(""), 0);
		return;
	}

	cnt_mines = GetCountAround(x, y, '*');

	if (cnt_mines) {
		BlockText[x][y] = cnt_mines + '0';
	}

	if (GetRemainBlock() == max_mines) {
		bGameStart = FALSE;
		MessageBox(hMain, TEXT("게임 승리"), TEXT("알림"), 0);
		return;
	}
	
	if (cnt_mines == 0) {
		for (int iy = -1; iy <= 1; iy++) {
			for (int ix = -1; ix <= 1; ix++) {
				int mx = x + ix;
				int my = y + iy;

				if (mx < 0 || my < 0 || mx > max_pt.x || my > max_pt.y)
					continue;

				OnLButtonDown(mx, my);
			}
		}
	}
}

int GetCountAround(int x, int y,TCHAR ch) {
	int cnt = 0;

	for (int iy = -1; iy <= 1; iy++) {
		for (int ix = -1; ix <= 1; ix++) {
			int mx = x + ix;
			int my = y + iy;

			if (ix == 0 && iy == 0)
				continue;

			if (mx < 0 || my < 0 || mx > max_pt.x || my > max_pt.y)
				continue;

			switch (ch) {
			case '*':
				if (ch == Mines[mx][my])
					cnt++;
				break;
			case '!':
				if (ch == BlockText[mx][my])
					cnt++;
				break;
			}
		}
	}

	return cnt;
}

void SetMines(int x,int y,int cnt_mines) {
	while (cnt_mines--) {
		int rx = rand() % max_pt.x;
		int ry = rand() % max_pt.y;

		if (Mines[rx][ry] == '*') {
			cnt_mines++;
			continue;
		}

		if (x - 1 <= rx && rx <= x + 1) {
			cnt_mines++;
			continue;
		}

		if (y - 1 <= ry && ry <= y + 1) {
			cnt_mines++;
			continue;
		}

		Mines[rx][ry] = '*';
	}
}

int GetRemainBlock() {
	int cnt = 0;

	for (int y = 0; y < max_pt.y; y++)
		for (int x = 0; x < max_pt.x; x++)
			if (BlockColor[x][y] == 0)
				cnt++;

	return cnt;
}