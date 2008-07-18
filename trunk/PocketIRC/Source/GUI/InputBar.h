#ifndef _INPUTBAR_H_INCLUDED_
#define _INPUTBAR_H_INCLUDED_

#define POCKETIRC_INPUT_COMBO_WIDTH		150

class InputBar 
{	
		static LRESULT CALLBACK EditStubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK MenuBarStubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	public: 
		enum INPUTBAR_IDC {IDC_INPUTBOX = 100};

		InputBar();
		~InputBar();

		HRESULT Create(HWND hMenuBar, HWND hCmdParent, UINT uID);
		HWND GetWindow() { return m_hCombo; }

		void RestoreSipState();

		// for switching tabs using left/right when no text is entered
		enum { WM_SWITCH_WINDOW = WM_USER + 1 };

		enum WindowSwitchType : WPARAM
		{
			WINDOW_SWITCH_LEFT = 0,
			WINDOW_SWITCH_RIGHT = 1
		};
	private:
		WNDPROC m_pfnDefEditProc;
		WNDPROC m_pfnDefMenuBarProc;
		LRESULT CALLBACK EditSubClassProc(UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK MenuBarSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		HWND m_hCombo;
		HWND m_hEdit;

		HWND m_hCmdParent;
		UINT m_uID;
		bool m_bLastSipState;
};

#endif//_INPUTBAR_H_INCLUDED_

