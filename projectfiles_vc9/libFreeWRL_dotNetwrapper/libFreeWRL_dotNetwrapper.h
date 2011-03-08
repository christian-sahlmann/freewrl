// libFreeWRL_dotNetwrapper.h

#pragma once

using namespace System;

using namespace System::Runtime::InteropServices;
#include "dllFreeWRL.h"
[DllImport("Kernel32.dll")]    
extern int AllocConsole();    
[DllImport("Kernel32.dll")]    
extern int FreeConsole();    
[DllImport("Kernel32.dll")]    
extern int AttachConsole(UInt32 dwProcessId);

namespace libFreeWRL_dotNetwrapper {


	public ref class FreewrlLib
	{
	private:
		CdllFreeWRL *dllfreewrl;
public: 
		// TODO: Add your methods for this class here.

		enum class KeyAction {KEYDOWN=2,KEYUP=3,KEYPRESS=1};
		enum class MouseAction {MOUSEMOVE=6,MOUSEDOWN=4,MOUSEUP=5};
		enum class MouseButton {LEFT=1,MIDDLE=2,RIGHT=3,NONE=0};
		System::String ^message;
		FreewrlLib()
		{
		}
		void onInit(IntPtr handle,int width, int height)
		{
			// Show a console window    
			dllfreewrl = new CdllFreeWRL();
			dllfreewrl->onInit((int)handle,width,height);
			message = handle.ToString();
			// Hide the console window    FreeConsole();
			//AllocConsole();    
			Console::WriteLine("Write to the console!");   
		}
        void onResize(int width,int height)
		{
			dllfreewrl->onResize(width,height);
		}
        void onMouse(MouseAction a,MouseButton b,int x, int y)
		{
			message = x.ToString()+y.ToString();
			dllfreewrl->onMouse((int)a,(int)b,x,y);
		}
        void onKey(KeyAction a,int keyValue)
		{
			int act = (int)a;
			if(keyValue != 'q')
				dllfreewrl->onKey(act,keyValue);
		}
        void onTick(int interval)
		{
			dllfreewrl->onTick(interval);
		}
		// http://msdn.microsoft.com/en-us/library/ms177197.aspx
		// somehow we need to deterministically release unmanaged resources
		// is this the right way, or will this way skip Dispose?
		~FreewrlLib() 
		{
			this->!FreewrlLib();
		}
		!FreewrlLib() 
		{
			//Flush();
			//fclose(file);
			message += "closing";
			dllfreewrl->onClose();
			//delete dllfreewrl;
		}
	};
}
