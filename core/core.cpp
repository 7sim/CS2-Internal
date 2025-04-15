#include <windows.h>
#include <thread>
#include <ntstatus.h>
#include <winternl.h>
#include "utils/utils.h"
#include "math/math.h"
#include "sdk/sdk.h"
#include "utils/hook/hooks.h"
#include "helpers/crt/crt.h"
#include "utils/render/render.h"
#include "utils/render/ui/menu.h"
#include "utils/configs/config.h"

#include "cheats/skins/skins.h"
#include "cheats/chams/chams.h"
#include "sdk/interfaces/game_event.h"

static BOOL CALLBACK EnumWindowsCallback( HWND handle, LPARAM lParam )
{
	const auto is_main_window = [ handle ] ( )
	{
		return GetWindow( handle, GW_OWNER ) == nullptr && IsWindowVisible( handle ) && handle != GetConsoleWindow( );
	};

	DWORD window_pid = 0;
	GetWindowThreadProcessId( handle, &window_pid );

	if ( GetCurrentProcessId( ) != window_pid || !is_main_window( ) )
		return TRUE;

	*reinterpret_cast< HWND* >( lParam ) = handle;
	return FALSE;
}

DWORD WINAPI LoadClient( PVOID module_ptr )
{
	try
	{
		g::store_shot_input_history_index = 0;

		
#ifdef _DEBUG
		if (!L::AttachConsole(xorstr_(L"cs2 developer-mode")))
		{
			return false;
		}
		// create console
    	utils::CreateConsole(L"Debug console");
#endif


		// get hwnd
		while (g::m_hwnd == nullptr)
		{
			// loop through all windows until we find one that matched what we want
			EnumWindows(::EnumWindowsCallback, reinterpret_cast<LPARAM>(&g::m_hwnd));

			// wait 200ms before trying again
			std::this_thread::sleep_for(200ms);
		}
		// init math
		if (!math::Init())
		{
			// tell the user and the developer we failed.
			CS2_LOG(xorstr_("Failed to initialize math.\n"));
			throw std::runtime_error(xorstr_("1A"));
		}

		// attempt to find math functions was successful
		CS2_LOG(xorstr_("A1\n"));

		// init util functions
		if (!utils::Init())
		{
			// tell the user and the developer we failed.
			CS2_LOG(xorstr_("Failed to initialize utils.\n"));
			throw std::runtime_error(xorstr_("5F"));
		}
		
		// attempt to find utils functions was successful
		CS2_LOG(xorstr_("F5\n"));

		// init sdk
		if (!sdk::Init())
		{
			// tell the user and the developer we failed.
			CS2_LOG(xorstr_("Failed to initialize sdk.\n"));
			throw std::runtime_error(xorstr_("2B"));
		}

		
		CS2_LOG(xorstr_("4SDK\n"));
		if (!chams::init())
		{
			CS2_LOG(xorstr_("Failed to initialize chams.\n"));
			throw std::runtime_error(xorstr_("chanms"));
		}

		// attempt to find schemas 
		CS2_LOG(xorstr_("schema\n"));
		if (!SCHEMA::Setup(xorstr_(L"schema_client.txt"), xorstr_("client.dll")))
		{
			CS2_LOG(xorstr_("Failed to initialize schema.\n"));
			throw std::runtime_error(xorstr_("schema"));
		}	

		if (!SCHEMA::Setup(xorstr_(L"schema_sv.txt"), xorstr_("server.dll")))
		{
			CS2_LOG(xorstr_("Failed to initialize schema.\n"));
			throw std::runtime_error(xorstr_("schema"));
		}
		// attempt to init renderer
		CS2_LOG(xorstr_("DSCHEMA5\n"));
		

		// init hooks
		if (!hooks::Init())
		{
			// tell the user and the developer we failed.
			CS2_LOG(xorstr_("Failed to initialize hooks.\n"));
			throw std::runtime_error(xorstr_("3C"));
		}

		// attempt to attach hooks
		CS2_LOG(xorstr_("3C\n"));
		

		// setup values to save/load cheat variables into/from files and load default configuration
		if (!C::Setup(xorstr_(CS_CONFIGURATION_DEFAULT_FILE_NAME)))
		{
			CS2_LOG(xorstr_("found no configuration to load\n"));
		}	

		Items::skins->parse();


		_log(LOG_INFO) << "done";
	}

	// catch exceptions
	catch ( const std::exception& exception )
	{
		// show exception text in a popup
		MessageBox( nullptr, exception.what( ), xorstr_( "cs2" ), MB_OK | MB_ICONERROR );

		// stop running
		return 0;
	}
			
	// allow for unload in debug
#ifdef CS2_UNLOAD
	// wait for VK_END

	while ( !GetAsyncKeyState( VK_END ) )
		std::this_thread::sleep_for( 200ms );
#ifdef _DEBUG
	sdk::m_pvs->set(false);
	// tell hooks to not run
	g::m_unloading = true;
	game_events->Destroy();

	chams::unload();
	Items::skins->remove();

	// restore m_release_mouse variable
	//if ( sdk::m_csgo_input->m_release_mouse )
	//	sdk::m_csgo_input->m_release_mouse = false;

	// wait for active hooks to finish
	std::this_thread::sleep_for( 200ms );


	// destroy render
	render::Destroy( );

	// close GUI
	menu::m_opened = false;
	L::DetachConsole();
	L::CloseFile();
	// wait for menu to close
	std::this_thread::sleep_for( 200ms );

	// check if old wndproc is valid
	if ( hooks::m_old_wndproc != nullptr )
	{
		// restore wnd proc
		SetWindowLongPtrW( g::m_hwnd, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( hooks::m_old_wndproc ) );

		// invalidate old wnd proc
		hooks::m_old_wndproc = nullptr;
	}

	// destroy minhook
	MH_DisableHook( MH_ALL_HOOKS );
	MH_RemoveHook( MH_ALL_HOOKS );
	MH_Uninitialize( );

	// close thread
	FreeLibraryAndExitThread( reinterpret_cast< HMODULE >( module_ptr ), EXIT_SUCCESS );
#endif
#endif

	return 1;
}
typedef NTSTATUS(NTAPI* pNtCreateThreadEx)(
	OUT PHANDLE ThreadHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
	IN HANDLE ProcessHandle,
	IN PVOID StartRoutine,
	IN PVOID Argument OPTIONAL,
	IN ULONG CreateFlags,
	IN ULONG_PTR ZeroBits,
	IN SIZE_T StackSize OPTIONAL,
	IN SIZE_T MaximumStackSize OPTIONAL,
	IN PVOID AttributeList OPTIONAL);
extern "C" BOOL WINAPI _CRT_INIT(HMODULE module, DWORD reason, LPVOID reserved);

BOOL WINAPI DllMain( HMODULE module, DWORD reason, LPVOID reserved )
{
	// process destroy of the cheat before crt calls atexit table

	// on injection
	if ( reason == DLL_PROCESS_ATTACH )
	{	
	
		// Call CRT initialization
		if (!_CRT_INIT(module, reason, reserved)) {
			return FALSE;
		}

		g::m_module = module;

		// Locate NtCreateThreadEx
		HMODULE hNtdll = GetModuleHandleW(xorstr_(L"ntdll.dll"));
		if (!hNtdll) {
			return FALSE;
		}

		pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(hNtdll, xorstr_("NtCreateThreadEx"));
		if (!NtCreateThreadEx) {
			return FALSE;
		}

		// Use NtCreateThreadEx to create a new thread
		HANDLE hThread = NULL;
		NTSTATUS status = NtCreateThreadEx(
			&hThread,                         // Thread handle
			THREAD_ALL_ACCESS,                // Desired access
			nullptr,                          // Object attributes
			GetCurrentProcess(),              // Handle to current process
			(LPTHREAD_START_ROUTINE)LoadClient, // Thread start address (your function)
			module,                           // Argument to pass to the thread function
			FALSE,                            // Create suspended?
			NULL,                             // Zero bits
			NULL,                             // Stack size
			NULL,                             // Maximum stack size
			nullptr);                         // Attribute list

		// If thread creation was successful, close handle
		if (NT_SUCCESS(status) && hThread != nullptr) {
			CloseHandle(hThread);
		}
	}

	return 1;
}
