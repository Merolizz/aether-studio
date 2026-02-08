#include "FileDialog.h"
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <shlobj.h>
#include <sstream>

#else
// Linux/Mac implementation would use native dialogs
#include <iostream>
#endif

namespace aether {

std::string FileDialog::openFile(
    const std::string &title,
    const std::vector<std::pair<std::string, std::string>> &filters,
    const std::string &defaultPath) {

#ifdef _WIN32
  (void)title; // Title shown in native dialog is standard
  OPENFILENAMEA ofn;
  char szFile[260] = {0};

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "All Files\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = defaultPath.empty() ? NULL : defaultPath.c_str();
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  // Build filter string
  if (!filters.empty()) {
    std::stringstream filterStream;
    for (const auto &[name, pattern] : filters) {
      filterStream << name << "\0" << pattern << "\0";
    }
    filterStream << "All Files\0*.*\0\0";
    std::string filterStr = filterStream.str();
    ofn.lpstrFilter = filterStr.c_str();
  }

  if (GetOpenFileNameA(&ofn) == TRUE) {
    return std::string(szFile);
  }

  return "";
#else
  // Linux/Mac implementation
  // Would use GTK file dialog on Linux, NSOpenPanel on Mac
  (void)title;
  (void)filters;
  (void)defaultPath;
  return "";
#endif
}

std::string FileDialog::saveFile(
    const std::string &title,
    const std::vector<std::pair<std::string, std::string>> &filters,
    const std::string &defaultPath, const std::string &defaultName) {

#ifdef _WIN32
  (void)title; // Title shown in native dialog is standard
  OPENFILENAMEA ofn;
  char szFile[260] = {0};

  if (!defaultName.empty()) {
    strncpy_s(szFile, sizeof(szFile), defaultName.c_str(), _TRUNCATE);
  }

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "All Files\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = defaultPath.empty() ? NULL : defaultPath.c_str();
  ofn.Flags = OFN_OVERWRITEPROMPT;

  // Build filter string
  if (!filters.empty()) {
    std::stringstream filterStream;
    for (const auto &[name, pattern] : filters) {
      filterStream << name << "\0" << pattern << "\0";
    }
    filterStream << "All Files\0*.*\0\0";
    std::string filterStr = filterStream.str();
    ofn.lpstrFilter = filterStr.c_str();
  }

  if (GetSaveFileNameA(&ofn) == TRUE) {
    return std::string(szFile);
  }

  return "";
#else
  // Linux/Mac implementation
  (void)title;
  (void)filters;
  (void)defaultPath;
  (void)defaultName;
  return "";
#endif
}

std::string FileDialog::openFolder(const std::string &title,
                                   const std::string &defaultPath) {

#ifdef _WIN32
  BROWSEINFOA bi;
  char szPath[MAX_PATH];

  ZeroMemory(&bi, sizeof(bi));
  bi.hwndOwner = NULL;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = szPath;
  bi.lpszTitle = title.c_str();
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  bi.lpfn = NULL;
  bi.lParam = 0;

  LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
  if (pidl != NULL) {
    if (SHGetPathFromIDListA(pidl, szPath)) {
      CoTaskMemFree(pidl);
      return std::string(szPath);
    }
    CoTaskMemFree(pidl);
  }

  return "";
#else
  // Linux/Mac implementation
  (void)title;
  (void)defaultPath;
  return "";
#endif
}

} // namespace aether
