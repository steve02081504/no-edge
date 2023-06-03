//msedge.cpp
//接受命令行参数并唤起默认浏览器

#include <windows.h>
#include <string>
#include <vector>

using namespace std;

int wmain(int argc, wchar_t *_argv[])
{
	vector<wstring> argv;
	for (int i = 0; i < argc; i++)
		argv.push_back(_argv[i]);
	
	//删除第一个参数
	wstring self = argv[0];
	argv.erase(argv.begin());
	argc--;
	//删除所有以'-'开头的选项：我们不处理它
	for (int i = 0; i < argc; i++)
	{
		if (argv[i][0] == L'-')
		{
			argv.erase(argv.begin() + i);
			argc--;i--;
		}
	}
	//解析默认浏览器
	wstring browser;
	{
		//读取注册表
		HKEY hKey;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD dwType = REG_SZ;
			wchar_t szBuf[1024];
			DWORD dwBufSize = sizeof(szBuf);
			if (RegQueryValueExW(hKey, L"ProgId", NULL, &dwType, (LPBYTE)szBuf, &dwBufSize) == ERROR_SUCCESS)
				browser = szBuf;
			RegCloseKey(hKey);
		}
		//browser现在是html的默认class名
		//读取注册表
		if (!browser.empty())
		{
			HKEY hKey;
			if (RegOpenKeyExW(HKEY_CLASSES_ROOT, (browser + L"\\shell\\open\\command").c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				DWORD dwType = REG_SZ;
				wchar_t szBuf[1024];
				DWORD dwBufSize = sizeof(szBuf);
				if (RegQueryValueExW(hKey, NULL, NULL, &dwType, (LPBYTE)szBuf, &dwBufSize) == ERROR_SUCCESS)
					browser = szBuf;
				RegCloseKey(hKey);
			}
			else
				browser.clear();
		}
		//browser现在是浏览器的路径+打开参数
		//去掉打开参数
		if (!browser.empty()){
			int i = 0;
			bool inquote = false;
			for (; i < browser.size(); i++)
			{
				if (inquote && browser[i] == L'"')
					inquote = !inquote;
				if (!inquote && browser[i] == L' ')
					break;
			}
			browser = browser.substr(0, i);
		}
	}
	//如果和self相同，报错
	if (browser == self)
	{
		MessageBoxW(NULL, L"更新你的默认浏览器到其他浏览器", L"错误", MB_ICONERROR);
		return 1;
	}
	//如果没有默认浏览器，报错
	if (browser.empty())
	{
		MessageBoxW(NULL, L"你没有设置默认浏览器", L"错误", MB_ICONERROR);
		return 1;
	}
	//启动
	{
		wstring cmd = L"cmd /c start \"\" \"" + browser + L"\"";
		for (int i = 0; i < argc; i++)
		{
			cmd += L" \"";
			cmd += argv[i];
			cmd += L"\"";
		}
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        auto aret=CreateProcessW(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		if(aret){
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else{
			MessageBoxW(NULL, L"无法启动浏览器", L"错误", MB_ICONERROR);
			return 1;
		}
	}
}
