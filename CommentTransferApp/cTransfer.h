#pragma once
#include "libxl.h"
#include <string>

class cTransfer
{
public:
	cTransfer(std::wstring srcPath, std::wstring destPath, 
		unsigned int headRow);
	~cTransfer();

	void CopyBook();

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
	int getCommentCol(libxl::Sheet* sheet);
};

