#include "pch.h"
#include "util.h"
#include <string.h>
#include <stringapiset.h>
using namespace std;

wstring String2Wstring(string s)
{
    if (s.empty())
        return wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), NULL, 0);
    wstring ws(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), &ws[0], size_needed);
    return ws;
}

string Wstring2String(wstring ws)
{
    if (ws.empty())
        return string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
    string s(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), &s[0], size_needed, NULL, NULL);
    return s;
}