// PSPWatcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <Windows.h>
#include <assert.h>
//#include <iostream>
//#include <list>
// #include <boost/shared_ptr.hpp>
// #include "subprocess.h"
// #include <ppl/mswin/file.h>
#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <string>
//#include <algorithm>
/*
#define _Console
#define _NO_LINE_BREAK
#include <MyOutput.h>
#define printf MyOutputDebugString
*/

#include "framework/Framework.h"
//#include "framework/logger/Logger.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

#include <boost/filesystem.hpp>
#include <sstream>

const static unsigned int csMaxProcessCount = 640;
#include "AsyncLilo.h"

#ifdef WIN32

#include <tlhelp32.h>

bool TerminateProcessTree(HANDLE hProcess)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD dwProcessId;
	DWORD dwExitCode;

	if ( ::GetExitCodeProcess(hProcess, &dwExitCode) && dwExitCode == STILL_ACTIVE)
	{
		dwProcessId = ::GetProcessId(hProcess);
		::TerminateProcess(hProcess, 0);
	}
	//	::CloseHandle(hProcess);

	// Take a snapshot of all processes in the system.
	hProcessSnap = ::CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !::Process32First( hProcessSnap, &pe32 ) )
	{
		::CloseHandle( hProcessSnap );          // clean the snapshot object
		return false;
	}

	// Now walk the snapshot of processes, and
	// kill each son process tree in turn
	do
	{
		if (dwProcessId == pe32.th32ParentProcessID)
		{
			HANDLE hKidProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
			if( hKidProcess )
			{

				if ( ::GetExitCodeProcess(hKidProcess, &dwExitCode) && dwExitCode == STILL_ACTIVE)
				{
					TerminateProcessTree(hKidProcess);
				}
				::CloseHandle( hKidProcess );
			}
		}

	} while( ::Process32Next( hProcessSnap, &pe32 ) );

	::CloseHandle( hProcessSnap );
	return true;
}

#define ReStartProcess(x) do { \
	if (x >= processCount) { \
	assert(0); logger.printf(lilogger::kLevelError, "Error processIndex[%d] > processCount[%d]", nIdx, processCount); \
	} \
	else ::CloseHandle(hProcesses[x]); \
	for (map<string, std::pair<short, string>>::iterator iter = workers.begin(); iter != workers.end(); ++iter) { \
	if (iter->second.first == x) { \
	if ( false == ::CreateProcess( NULL, (char*)iter->first.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE/*|CREATE_NO_WINDOW*/, NULL, NULL, &si, &pi) ) { \
	logger.printf(lilogger::kLevelError, "Failed to create process: %s. Err[%d]", iter->first.c_str(), ::GetLastError()); \
	do{ \
	hProcesses[x] = hProcesses[x + 1]; \
	++x; \
	}while(hProcesses[x] != 0 && x < csMaxProcessCount - 1); \
	if (x == csMaxProcessCount - 1) hProcesses[csMaxProcessCount - 1] = 0; \
	break; \
	} \
	logger.printf(lilogger::kLevelInfor, "* Task ReStart => PID[%d]:%s", pi.dwProcessId, iter->first.c_str());\
	::CloseHandle(pi.hThread); \
	hProcesses[x] = pi.hProcess; \
	break; \
	} \
	} \
}while(false);

#else

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include <errno.h>
extern int errno;

#define ReStartProcess(x) do { \
	if (x >= processCount) { \
	assert(0); logger.printf(lilogger::kLevelError, "Error processIndex[%d] > processCount[%d]", nIdx, processCount); \
	} \
	for (map<string, std::pair<short, string> >::iterator iter = workers.begin(); iter != workers.end(); ++iter) { \
		if (iter->second.first == x) { \
			hProc = fork(); \
			if (hProc < 0) { \
				logger.printf(lilogger::kLevelError, "Failed to create process: %s. Err[%d]", iter->first.c_str(), errno); \
					do{ \
						hProcesses[x] = hProcesses[x + 1]; \
						++x; \
					}while(hProcesses[x] != 0 && x < csMaxProcessCount - 1); \
					if (x == csMaxProcessCount - 1) hProcesses[csMaxProcessCount - 1] = 0; \
						break; \
			} else if (hProc == 0) { \
				if (exec_line(iter->first.c_str()) < 0) { \
					logger.printf(lilogger::kLevelError, "Failed to start worker: %s. Err[%d]", cmd_line.c_str(), errno); \
						do{ \
							hProcesses[x] = hProcesses[x + 1]; \
							++x; \
						}while(hProcesses[x] != 0 && x < csMaxProcessCount - 1); \
						if (x == csMaxProcessCount - 1) hProcesses[csMaxProcessCount - 1] = 0; \
							break; \
				} \
				exit(0); \
			} else { \
				logger.printf(lilogger::kLevelInfor, "* Task ReStart => PID[%d]:%s", hProc, iter->first.c_str());\
					hProcesses[x] = hProc; \
					break; \
			} \
		} \
	} \
}while(false);

inline int exec_line(const char *line)
{
	char *pArg;
	char *exec_path = NULL;
	char *argv[100 + 1];
	int argc = 0;
/*
	if( ( pArg = strrchr( cmd_line, '/' ) ) != NULL )
		pArg++;
	else
		pArg = cmd_line;
	argv[0] = pArg;
	argc = 1;
*/	
	char *cmd_line;

	if( line != NULL && *line != '\0' )
	{
		cmd_line = new char[strlen(line) + 1];
		strcpy(cmd_line, line);

		exec_path = pArg = strtok(cmd_line, " ");
		argv[0] = pArg;
		argc = 1;
		while((pArg = strtok(NULL, " "))!= NULL )
		{
			argv[argc] = pArg;
			argc++;
			if( argc >= 100 )
				break;
		}
	}
	argv[argc] = NULL;

	return execv(exec_path, argv);
}

void alarm_handler(int code)
{
	printf("!!!! ALARM !!!! %d\n", code);
}

#endif // WIN32

using namespace std;

int main(int argc, char* argv[])
{
	printf("Cutter Processor\n");
	printf("ver.1.5 released by Tady, 03122012.\n");

	map<string, std::pair<short,string> > workers;
	set<string> channel_id_set;
#ifdef WIN32
	STARTUPINFO si = { 0 };
	si.cb = sizeof( si );
	PROCESS_INFORMATION pi = { 0 };
	HANDLE hProcesses[csMaxProcessCount] = { 0 };
	HANDLE hProc;
#else
	pid_t hProcesses[csMaxProcessCount] = { 0 };
	pid_t hProc;
	//signal(SIGALRM, SIG_IGN);
	//signal(SIGALRM, alarm_handler);
#endif
	int processCount(0);
	string cmd_line;
	vector<string> channel_list;

	string work_dir("./");
	string cache_time("30");
	string head_cache_time("");
	int channels_update_scale(5);
	framework::configure::Config conf("./cutter_base.ini");  
	conf.register_module("base")
		<< CONFIG_PARAM_NAME_NOACC("work_dir", work_dir)
		<< CONFIG_PARAM_NAME_NOACC("cache_time", cache_time)
		<< CONFIG_PARAM_NAME_NOACC("head_cache_time", head_cache_time)
		<< CONFIG_PARAM_NAME_NOACC("channels_update_scale", channels_update_scale);	

	boost::filesystem::path work_path(work_dir);
	boost::filesystem::path channel_list_path(work_dir);
	channel_list_path /= "cutter_channel_list";

	unsigned int print_level = lilogger::kLevelDebug;
	unsigned int file_level = lilogger::kLevelNostr;
	std::string file_path;
	unsigned int file_cut_period = 0;
	unsigned int file_save_period = 0;
	conf.register_module("log")
		<< CONFIG_PARAM_NAME_NOACC("print_level", print_level)
		<< CONFIG_PARAM_NAME_NOACC("file_level", file_level)
		<< CONFIG_PARAM_NAME_NOACC("file_path", file_path)
		<< CONFIG_PARAM_NAME_NOACC("file_cut_period", file_cut_period)
		<< CONFIG_PARAM_NAME_NOACC("file_save_period", file_save_period);

	lilo logger("processor", print_level, file_level, 0, file_path.c_str(), file_cut_period, file_save_period);
	logger.printf(lilogger::kLevelInfor, "Started.");

	while(true)
	{
		//////////////////////////////////////////////////////////////////////////
		// Read push menu file and start workers.
		{
			ifstream channel_list_file;
			channel_list_file.open(channel_list_path.string().c_str());
			if (!channel_list_file.is_open())
			{
//				printf("Failed to open channel_list_file file.\n");
				logger.printf(lilogger::kLevelError, "Failed to open channel_list_file file.");
				goto watching;
			}

			channel_list.clear();
			channel_id_set.clear();
			processCount = 0;

			while(!channel_list_file.eof())
			{
				string channel_line;
				std::getline(channel_list_file, channel_line);
				if (channel_line == "" || channel_line[0] == '#')
					continue;

				istringstream ss(channel_line);
				string guid, url, interval;
				ss >> guid >> url >> interval;

				work_path /= guid;

				cmd_line = "./cutterworker " + url + " ";
				cmd_line += work_path.string();
				cmd_line += " " + cache_time + " " + head_cache_time;
				cmd_line += " " + interval;

				if (std::find(channel_list.begin(), channel_list.end(), cmd_line) != channel_list.end()
					|| channel_id_set.find(guid) != channel_id_set.end())
				{
					//printf("Repeated channel_id or channel_line[%d]: %s\n", channel_list.size(), channel_line.c_str());
					logger.printf(lilogger::kLevelAlarm, "Repeated channel_id or channel_line[%d]: %s", channel_list.size(), channel_line.c_str());
					work_path.remove_leaf();
					continue;
				}
				channel_list.push_back(cmd_line);

				if (workers.find(cmd_line) == workers.end())
				{	
					// Prepare work dir.
					try
					{
						if (!boost::filesystem::exists(work_path))
							boost::filesystem::create_directory( work_path );
					}
					catch (const boost::filesystem::filesystem_error& e)
					{
						//printf("Exception: %s\n", e.what());
						logger.printf(lilogger::kLevelError, "Exception: %s", e.what());
						work_path.remove_leaf();
						continue;
					}
#ifdef WIN32
					if ( false == ::CreateProcess( NULL, (char*)cmd_line.c_str(), 
						NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE/*|CREATE_NO_WINDOW*/, 
						NULL, NULL, &si, &pi) )
					{
						//printf("Failed to create process: %s. Err[%d]\n", cmd_line.c_str(), ::GetLastError());
						logger.printf(lilogger::kLevelError, "Failed to create process: %s. Err[%d]", cmd_line.c_str(), ::GetLastError());
						work_path.remove_leaf();
						continue;
					}
					//printf("+ Task begin => PID[%d]: %s\n", pi.dwProcessId, cmd_line.c_str());
					logger.printf(lilogger::kLevelInfor, "+ Task begin => PID[%d]: %s", pi.dwProcessId, cmd_line.c_str());
					::CloseHandle(pi.hThread);
					hProc = pi.hProcess;
#else // !WIN32
					hProc = fork();
					if (hProc < 0)                            
					{
						logger.printf(lilogger::kLevelError, "Failed to create process: %s. Err[%d]", cmd_line.c_str(), errno);
						work_path.remove_leaf();
						continue;
					}
					else if (hProc == 0)                                                          
					{
						if (exec_line(cmd_line.c_str()) < 0)
							logger.printf(lilogger::kLevelError, "Failed to start worker: %s. Err[%d]", cmd_line.c_str(), errno);
						exit(0);
					} 

					logger.printf(lilogger::kLevelInfor, "+ Task begin => PID[%d]: %s", hProc, cmd_line.c_str());
#endif // WIN32
					short i = 0;
					for (; i < csMaxProcessCount - 1; ++i)
					{
						if (hProcesses[i] == 0)
						{
							hProcesses[i] = hProc;
							workers[cmd_line] = std::make_pair(i, guid);
							break;
						}
					}
					if (i == csMaxProcessCount - 1)
					{
						assert(false);
#ifdef WIN32
						::TerminateProcess(hProc, 0);
						::CloseHandle(hProc);
#else
						kill(hProc,SIGTERM);
#endif
						work_path.remove_leaf();
						continue;
					}
					
					channel_id_set.insert(guid);
				}
				++processCount;

				work_path.remove_leaf();
			}
			channel_list_file.close();
		}

		//////////////////////////////////////////////////////////////////////////
		// Close deleted Processes.
		
		for (map<string, std::pair<short, string> >::iterator iter = workers.begin(); iter != workers.end(); )
		{
			if (std::find(channel_list.begin(), channel_list.end(), iter->first) == channel_list.end())
			{
				int processIdx = iter->second.first;
#ifdef WIN32
				ULONG exitCode = 0;
				if ( ::GetExitCodeProcess(hProcesses[processIdx], &exitCode) && exitCode == STILL_ACTIVE)
					::TerminateProcess(hProcesses[processIdx], 0);
				::CloseHandle(hProcesses[processIdx]);
#else
				kill(hProcesses[processIdx], SIGTERM);
#endif
				logger.printf(lilogger::kLevelInfor, "- Task stopped: %s", iter->first.c_str());

				if (processIdx < (int)(workers.size() - 1))
				{
					for (map<string, std::pair<short, string> >::iterator iter2 = workers.begin(); iter2 != workers.end(); ++iter2)
					{
						if (iter2->second.first > processIdx)
						{
							iter2->second.first--;
						}
					}
				}
				do{
					hProcesses[processIdx] = hProcesses[processIdx + 1];
					++processIdx;
				}while(hProcesses[processIdx] != 0 && processIdx < csMaxProcessCount - 1);
				if (processIdx == csMaxProcessCount - 1) hProcesses[csMaxProcessCount - 1] = 0;

// 				istringstream ss(iter->first);
// 				string guid;
// 				ss >> guid;
				if (channel_id_set.find(iter->second.second) == channel_id_set.end())
				{ // Remove all channel files.
					work_path /= iter->second.second;
					try
					{
						if (boost::filesystem::exists(work_path))
							boost::filesystem::remove_all(work_path);
					}
					catch (const boost::filesystem::filesystem_error& e)
					{
						logger.printf(lilogger::kLevelError, "Exception: %s", e.what());
					}
					work_path.remove_leaf();
				}
				workers.erase(iter++);

				continue;
			}
			else
				++iter;
		}

		//////////////////////////////////////////////////////////////////////////
		// Watching
watching:
		if (processCount > 0)
		{
#ifdef WIN32
			DWORD dwRet;
			dwRet = ::WaitForMultipleObjects(processCount, hProcesses, false, channels_update_scale * 1000);
			switch(dwRet)
			{
			case WAIT_TIMEOUT:
			case WAIT_FAILED:
				break;
			default:
				{
					int nIdx = (int)(dwRet - WAIT_OBJECT_0);
					ReStartProcess(nIdx);
					//同时检测其他的事件
					while(nIdx < processCount)
					{
						dwRet = WaitForMultipleObjects(processCount - nIdx, hProcesses, false, 0);
						switch(dwRet)
						{
						case WAIT_TIMEOUT:
						case WAIT_FAILED:
							nIdx = processCount; //退出检测,因为没有被触发的对象了.
							break;
						default:
							{
								nIdx = dwRet - WAIT_OBJECT_0;
								ReStartProcess(nIdx);
							}
							break;
						}
					}
					::Sleep(channels_update_scale * 1000);
				}
				break;
			}
#else // !win32
//			alarm(5);
			int wait_ticks = 5;
			do 
			{
				pid_t stopedID = waitpid(0, NULL, WNOHANG);
				//pid_t stopedID = wait(NULL);
				if (stopedID > 0)
				{
					for (int nIdx = 0; nIdx < processCount; ++nIdx)
					{
						if (hProcesses[nIdx] == stopedID)
						{
							ReStartProcess(nIdx);
							break;
						}
					}
				}
				else
				{
					sleep(1);
					--wait_ticks;
				}
			} while (wait_ticks > 0);
#endif // win32
		}
		else // processCount == 0
		{
#ifdef WIN32
			::Sleep(channels_update_scale * 1000);
#else
			sleep(channels_update_scale);
#endif		
		} // processCount == 0

	} // while(true);

	return 0;
};
