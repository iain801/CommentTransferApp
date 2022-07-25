#pragma once
#include "libxl.h"
#include <string>
#include <list>

class cTransfer
{
public:
	cTransfer(std::wstring srcPath, std::wstring destPath, 
		unsigned int headRow);
	~cTransfer();

	int CopyBook();
	bool isID();

private:
	unsigned int headRow;
	std::wstring srcPath;
	std::wstring destPath;
	libxl::Book* src = nullptr;
	libxl::Book* dest = nullptr;
	libxl::Sheet* srcSheet = nullptr; 
	libxl::Sheet* destSheet = nullptr;
	
	void CopySheet();
	void CopyCell(int row, int col);
	void CopyCell(int srcRow, int destRow, int srcCol, int destCol);
	int getSheet(libxl::Book* book, std::wstring label);
	int getRow(libxl::Sheet* sheet, std::wstring label, int idCol);
	int getCol(libxl::Sheet* sheet, std::wstring label, bool comment=true);
	std::list<int> getColList(libxl::Sheet* sheet, std::wstring label);
};

