#include "cTransfer.h"
#include <algorithm>
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

	for (int sheet = 0; sheet < numSrcSheets; sheet++)
	{
		srcSheet = src->getSheet(sheet);
		destSheet = dest->getSheet(getSheet(dest, srcSheet->name()));
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

int cTransfer::getSheet(libxl::Book* book, std::wstring label)
{
	for (int sheet = 0; sheet < book->sheetCount(); sheet++)
	{
		std::wstring sheetName(book->getSheetName(sheet));
		if (sheetName.compare(label) == 0)
			return sheet;
	}
	return -1;
}

void cTransfer::CopySheet()
{
	int srcIDCol = getCol(srcSheet, L"unique");
	int destIDCol = getCol(destSheet, L"unique");

	std::list<int> srcCommList = getColList(srcSheet, L"comment");
	for (int srcCommCol : srcCommList)
	{
		std::wstring destCommHeader(srcSheet->readStr(headRow, srcCommCol));
		int destCommCol = getCol(destSheet, destCommHeader);
		CopyCell(headRow, headRow, srcCommCol, destCommCol);
		for (int srcRow = headRow + 1; srcRow < srcSheet->lastRow(); ++srcRow)
		{
			int destRow = getRow(destSheet, srcSheet->readStr(srcRow, srcIDCol), destIDCol);
			if (destRow != -1)
				CopyCell(srcRow, destRow, srcCommCol, destCommCol);
		}

		auto srcFormat = srcSheet->cellFormat(headRow + 1, srcCommCol);
		for (int destRow = headRow + 1; destRow < destSheet->lastRow(); ++destRow)
		{
			auto cellType = destSheet->cellType(destRow, destCommCol);
			if (cellType == CELLTYPE_BLANK)
				destSheet->writeBlank(destRow, destCommCol, srcFormat);
			else if (cellType == CELLTYPE_EMPTY)
				destSheet->writeStr(destRow, destCommCol, L"", srcFormat, CELLTYPE_EMPTY);	
		}
		destSheet->setCol(destCommCol,destCommCol, srcSheet->colWidth(srcCommCol));
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

int cTransfer::getRow(libxl::Sheet* sheet, std::wstring label, int idCol)
{
	for (int row = headRow; row < sheet->lastFilledRow(); row++)
	{
		std::wstring cellData(sheet->readStr(row, idCol));
		if (cellData.compare(label) == 0)
			return row;
	}
	return -1;
}


int cTransfer::getCol(Sheet* sheet, std::wstring label)
{
	auto colList = getColList(sheet, label);
	if (colList.empty())
		return sheet->lastFilledCol();
	else
		return colList.front();
}

std::list<int> cTransfer::getColList(Sheet* sheet, std::wstring label)
{
	std::list<int> colList(0);
	for (int col = sheet->firstFilledCol(); col < sheet->lastFilledCol(); col++)
	{
		if (sheet->cellType(headRow, col) == CELLTYPE_STRING)
		{
			std::wstring cellData(sheet->readStr(headRow, col));
			std::transform(cellData.begin(), cellData.end(), cellData.begin(),
				[](wchar_t c) { return tolower(c); });
			if (cellData.find(label) != std::wstring::npos)
				colList.push_back(col);
		}
	}
	return colList;
}