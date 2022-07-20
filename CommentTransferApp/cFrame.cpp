#include "cFrame.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <locale>
#include <codecvt>

wxBEGIN_EVENT_TABLE(cFrame, wxFrame)
	EVT_BUTTON(10001, PerformTransfer)
wxEND_EVENT_TABLE()

cFrame::cFrame() : wxFrame(nullptr, wxID_ANY, "Comment Transfer - Erasca", wxPoint(100, 100), wxSize(340, 400))
{
	btn1 = new wxButton(this, 10001, "Copy Comments", wxPoint(45, 10), wxSize(150, 30));
	rowText = new wxStaticText(this, wxID_ANY, "Start row: ", wxPoint(205, 17));
	rowInput = new wxTextCtrl(this, wxID_ANY, "6", wxPoint(260, 15), wxSize(30, 20), 0L, wxIntegerValidator<unsigned int>());
	srcText = new wxStaticText(this, wxID_ANY, "Commented Spreadsheet: ", wxPoint(10, 47));
	srcFile = new wxFilePickerCtrl(this, wxID_ANY, "", "", "XLSX and XLS files (*.xlsx;*.xls)|*.xlsx;*.xls", wxPoint(10, 65), wxSize(300, 20));
	srcText = new wxStaticText(this, wxID_ANY, "Destination Spreadsheet: ", wxPoint(10, 87));
	dstFile = new wxFilePickerCtrl(this, wxID_ANY, "", "", "XLSX and XLS files (*.xlsx;*.xls)|*.xlsx;*.xls", wxPoint(10, 105), wxSize(300, 20));
	output = new wxTextCtrl (this, wxID_ANY, "", wxPoint(10, 135), wxSize(300, 200), wxTE_READONLY + wxTE_MULTILINE);

	wxStreamToTextRedirector redirect(output);

	std::cout << "ENSURE DATA IS ORDERED IN ALL SHEETS" << std::endl;
}

cFrame::~cFrame()
{

}


void cFrame::PerformTransfer(wxCommandEvent& evt)
{
	wxStreamToTextRedirector redirect(output);

	auto srcName = srcFile->GetPath().ToStdWstring();
	auto destName = dstFile->GetPath().ToStdWstring();
	output->Clear();

	libxl::Book* src = nullptr;
	libxl::Book* dest = nullptr;

	if (srcName.find(L".xlsx") != std::wstring::npos)
		src = xlCreateXMLBook();
	else if (srcName.find(L".xls") != std::wstring::npos)
		src = xlCreateBook();
	else {
		std::cout << "Invaid source filetype" << std::endl;
		goto skip;
	}
	std::cout << "Created source book" << std::endl;

	if (destName.find(L".xlsx") != std::wstring::npos)
		dest = xlCreateXMLBook();
	else if (destName.find(L".xls") != std::wstring::npos)
		dest = xlCreateBook();
	else {
		std::cout << "Invaid destination filetype" << std::endl;
		goto skip;
	}
	src->setKey(L"Iain Weissburg", L"windows-2a242a0d01cfe90a6ab8666baft2map2");
	dest->setKey(L"Iain Weissburg", L"windows-2a242a0d01cfe90a6ab8666baft2map2");
	std::cout << "Created destination book" << std::endl;

	if (src->load(srcName.c_str()) && dest->load(destName.c_str()))
	{
		std::cout << "Loaded books" << std::endl;
		int numSrcSheets = src->sheetCount();
		int numDestSheets = dest->sheetCount();
		if (numSrcSheets != numDestSheets)
			std::cout << "ERROR: Mismatched sheet counts" << std::endl;
		else for (int sheet = 0; sheet < numSrcSheets; sheet++) 
		{
			auto srcSheet = src->getSheet(sheet);
			auto destSheet = dest->getSheet(sheet);
			if (srcSheet && destSheet)
			{
				std::cout << "Loaded sheet " << sheet << ": " << std::wstring(srcSheet->name()) << std::endl;
				CopySheet(srcSheet, destSheet);
			}
			else 
			{
				std::cout << "ERROR: Sheet " << sheet << " not loaded" << std::endl;
			}
		}
	}

	dest->save(destName.replace(destName.find(L".xls"), 4, L"_processed.xls").c_str());
	std::cout << "Output saved as: " << destName  << std::endl;

skip:
	src->release();
	dest->release();

	evt.Skip();
}


void cFrame::CopySheet(libxl::Sheet* srcSheet, libxl::Sheet* destSheet) 
{
	wxStreamToTextRedirector redirect(output);
	
	int col = 0;
	if ((col = getCommentCol(srcSheet)) != getCommentCol(destSheet))
	{
		std::cout << "ERROR: Mismatched sheets " << srcSheet->name() << " and " << destSheet->name() << std::endl;
	}
	else
	{
		std::cout << "Inserting comments at column " << (char)(col + 'A') << std::endl;
		for (int row = srcSheet->firstRow(); row < srcSheet->lastRow(); ++row)
		{
			CopyCell(srcSheet, destSheet, row, col);
		}
	}
}

void cFrame::CopyCell(libxl::Sheet* srcSheet, libxl::Sheet* destSheet, int row, int col) {
	wxStreamToTextRedirector redirect(output);

	auto cellType = srcSheet->cellType(row, col);
	auto srcFormat = srcSheet->cellFormat(row, col);
	if (srcSheet->isFormula(row, col))
	{
		const wchar_t* s = srcSheet->readFormula(row, col);
		destSheet->writeFormula(row, col, s, srcFormat);
		std::cout << std::wstring(s ? s : L"null") << " [formula]" << std::endl;
	}
	else
	{
		switch (cellType)
		{
		case libxl::CELLTYPE_EMPTY:
		{
			//std::cout << "[empty]" << std::endl;
			destSheet->writeStr(row, col, L"", srcFormat, libxl::CELLTYPE_EMPTY);
			break;
		}
		case libxl::CELLTYPE_NUMBER:
		{
			double d = srcSheet->readNum(row, col);
			std::cout << d << " [number] << std::endl";
			destSheet->writeNum(row, col, d, srcFormat);
			break;
		}
		case libxl::CELLTYPE_STRING:
		{
			const wchar_t* s = srcSheet->readStr(row, col);
			std::cout << std::wstring(s ? s : L"null") << " [string]" << std::endl;
			destSheet->writeStr(row, col, s, srcFormat);
			break;
		}
		case libxl::CELLTYPE_BOOLEAN:
		{
			bool b = srcSheet->readBool(row, col);
			std::cout << (b ? "true" : "false") << " [boolean]" << std::endl;
			destSheet->writeBool(row, col, b, srcFormat);
			break;
		}
		case libxl::CELLTYPE_BLANK:
		{
			//std::cout << "[blank]" << std::endl;
			destSheet->writeBlank(row, col, srcFormat);
			break;
		}
		case libxl::CELLTYPE_ERROR:
		{
			auto e = srcSheet->readError(row, col);
			std::cout << "[error]" << std::endl;
			destSheet->writeError(row, col, e, srcFormat);
			break;
		}
		}
	}
}

int cFrame::getCommentCol(libxl::Sheet* sheet)
{
	wxStreamToTextRedirector redirect(output);
	
	bool commentsFound = false;
	int row = std::stoi(rowInput->GetLineText(0).ToStdString()) - 1;
	int col = 0;
	for (col = sheet->firstFilledCol(); col < sheet->lastFilledCol(); col++)
	{
		if (sheet->cellType(row, col) == libxl::CELLTYPE_STRING)
		{
			std::wstring cellData(sheet->readStr(row, col));
			std::transform(cellData.begin(), cellData.end(), cellData.begin(),
				[](wchar_t c) { return tolower(c); });
			if (cellData.compare(L"comments") == 0)
				return col;
		}
	}
	return sheet->lastFilledCol();
}