#include "cTransfer.h"
#include <algorithm>
#include <cctype>
#include <locale>
#include <codecvt>
#include <iostream>

using namespace libxl;

cTransfer::cTransfer(std::wstring sourcePath, std::wstring destinationPath, unsigned int headRow)
	: srcPath(sourcePath), destPath(destinationPath), headRow(headRow)
{
	if (srcPath.compare(destPath) == 0) {
		std::wcout << "ERROR: Duplicate input paths" << std::endl;
		return;
	}

	if (srcPath.find(L".xlsx") != std::wstring::npos)
		src = xlCreateXMLBook();
	else if (srcPath.find(L".xls") != std::wstring::npos)
		src = xlCreateBook();
	else {
		std::wcout << "ERROR: Invaid source filetype" << std::endl;
		return;
	}
	src->setKey(L"Iain Weissburg", L"windows-2a242a0d01cfe90a6ab8666baft2map2");
	std::wcout << "Created source book" << std::endl;

	if (destPath.find(L".xlsx") != std::wstring::npos)
		dest = xlCreateXMLBook();
	else if (destPath.find(L".xls") != std::wstring::npos)
		dest = xlCreateBook();
	else {
		std::wcout << "ERROR: Invaid destination filetype" << std::endl;
		return;
	}
	dest->setKey(L"Iain Weissburg", L"windows-2a242a0d01cfe90a6ab8666baft2map2");
	std::wcout << "Created destination book" << std::endl;

	src->load(srcPath.c_str());
	dest->load(destPath.c_str());
	std::wcout << "Loaded books" << std::endl;
}

cTransfer::~cTransfer() {
	dest->save(destPath.replace(destPath.find(L".xls"), 4, L"_processed.xls").c_str());
	std::wcout << "Output saved as: " << destPath << std::endl;

	src->release();
	dest->release();
}

void cTransfer::CopyBook()
{
	int numSrcSheets = src->sheetCount();
	int numDestSheets = dest->sheetCount();
	if (numSrcSheets != numDestSheets)
		std::wcout << "ERROR: Mismatched sheet counts" << std::endl;
	else for (int sheet = 0; sheet < numSrcSheets; sheet++)
	{
		srcSheet = src->getSheet(sheet);
		destSheet = dest->getSheet(sheet);
		if (srcSheet && destSheet)
		{
			std::wcout << "Loaded sheet " << sheet << ": " << std::wstring(srcSheet->name()) << std::endl;
			CopySheet();
		}
		else
		{
			std::wcout << "ERROR: Sheet " << sheet << " not loaded" << std::endl;
		}
	}
}


void cTransfer::CopySheet()
{
	int srcCol = getCol(srcSheet, L"comment");
	int destCol = getCol(destSheet, L"comment");
	if ( srcCol != destCol )
		std::wcout << "WARNING: Mismatched sheets " << srcSheet->name() << " and " << destSheet->name() << std::endl;
	
	std::wcout << "Inserting comments at column " << (char)(destCol + 'A') << " from column " << (char)(srcCol + 'A') << std::endl;
	for (int row = srcSheet->firstRow(); row < srcSheet->lastRow(); ++row)
	{
		CopyCell(row, row, srcCol, destCol);
	}
	
}

void cTransfer::CopyCell(int row, int col)
{
	CopyCell(row, row, col, col);
}

void cTransfer::CopyCell(int srcRow, int destRow, int srcCol, int destCol)
{
	auto cellType = srcSheet->cellType(srcRow, srcCol);
	auto srcFormat = srcSheet->cellFormat(srcRow, srcCol);
	if (srcSheet->isFormula(srcRow, srcCol))
	{
		const wchar_t* s = srcSheet->readFormula(srcRow, srcCol);
		destSheet->writeFormula(destRow, destCol, s, srcFormat);
		std::wcout << std::wstring(s ? s : L"null") << " [formula]" << std::endl;
	}
	else
	{
		switch (cellType)
		{
		case CELLTYPE_EMPTY:
		{
			//std::wcout << "[empty]" << std::endl;
			destSheet->writeStr(destRow, destCol, L"", srcFormat, CELLTYPE_EMPTY);
			break;
		}
		case CELLTYPE_NUMBER:
		{
			double d = srcSheet->readNum(srcRow, srcCol);
			std::wcout << d << " [number] << std::endl";
			destSheet->writeNum(destRow, destCol, d, srcFormat);
			break;
		}
		case CELLTYPE_STRING:
		{
			const wchar_t* s = srcSheet->readStr(srcRow, srcCol);
			std::wcout << std::wstring(s ? s : L"null") << " [string]" << std::endl;
			destSheet->writeStr(destRow, destCol, s, srcFormat);
			break;
		}
		case CELLTYPE_BOOLEAN:
		{
			bool b = srcSheet->readBool(srcRow, srcCol);
			std::wcout << (b ? "true" : "false") << " [boolean]" << std::endl;
			destSheet->writeBool(destRow, destCol, b, srcFormat);
			break;
		}
		case CELLTYPE_BLANK:
		{
			//std::wcout << "[blank]" << std::endl;
			destSheet->writeBlank(destRow, destCol, srcFormat);
			break;
		}
		case CELLTYPE_ERROR:
		{
			auto e = srcSheet->readError(srcRow, srcCol);
			std::wcout << "[error]" << std::endl;
			destSheet->writeError(destRow, destCol, e, srcFormat);
			break;
		}
		}
	}
}

int cTransfer::getCol(Sheet* sheet, std::wstring label)
{
	bool commentsFound = false;
	int col = 0;
	for (col = sheet->firstFilledCol(); col < sheet->lastFilledCol(); col++)
	{
		if (sheet->cellType(headRow, col) == CELLTYPE_STRING)
		{
			std::wstring cellData(sheet->readStr(headRow, col));
			std::transform(cellData.begin(), cellData.end(), cellData.begin(),
				[](wchar_t c) { return tolower(c); });
			if (cellData.find(label) != std::wstring::npos)
				return col;
		}
	}
	return sheet->lastFilledCol();
}