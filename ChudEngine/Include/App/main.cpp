#include "ChudEngineApp.hpp"
#include "Utils/FileSystem.hpp"
#include "Core/Logger.h"


int main ()
	{
	CE::FileSystem::GetLogsDirectory ();

	CE::Logger::Init ( "ChudLog.log", true );

#ifdef _DEBUG
	CE::Logger::SetLogLevel ( CE::LogLevel::Trace );
#else
	CE::Logger::SetLogLevel ( CE::LogLevel::Info );
#endif // _DEBUG

	CE_CORE_DEBUG ( "ChudEngine Starting - Vulkan Renderer Test" );


	try
		{
		ChudEngineApp app;
		app.Run ();
		}
		catch (const std::exception & e)
			{
			CE_CORE_ERROR ( "Exception: {}", e.what () );
			return 1;
			}


		CE_CORE_DEBUG ( "ChudEngine Shutdown..." );
		CE::Logger::Shutdown ();
		
		return 0;
	}