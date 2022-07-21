#include "cFrame.h"
#include <string>
#include <iostream>

wxBEGIN_EVENT_TABLE(cFrame, wxFrame)
EVT_BUTTON(10001, PerformTransfer)
wxEND_EVENT_TABLE()



//if using output, change to wxSize(340, 400)
cFrame::cFrame() : wxFrame(nullptr, wxID_ANY, "Comment Transfer - Erasca", wxPoint(100, 100), wxSize(340, 160), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
	btn1 = new wxButton(this, 10001, "Copy Comments", wxPoint(10, 85), wxSize(150, 30));
	rowText = new wxStaticText(this, wxID_ANY, "Start row: ", wxPoint(205, 92));
	rowInput = new wxTextCtrl(this, wxID_ANY, "6", wxPoint(260, 90), wxSize(30, 20), 0L, wxIntegerValidator<unsigned int>());
	srcText = new wxStaticText(this, wxID_ANY, "Commented Spreadsheet: ", wxPoint(10, 2));
	srcFile = new wxFilePickerCtrl(this, wxID_ANY, "", "", "XLSX and XLS files (*.xlsx;*.xls)|*.xlsx;*.xls", wxPoint(10, 20), wxSize(300, 20));
	srcText = new wxStaticText(this, wxID_ANY, "Data Spreadsheet: ", wxPoint(10, 42));
	dstFile = new wxFilePickerCtrl(this, wxID_ANY, "", "", "XLSX and XLS files (*.xlsx;*.xls)|*.xlsx;*.xls", wxPoint(10, 60), wxSize(300, 20));
	//output = new wxTextCtrl (this, wxID_ANY, "", wxPoint(10, 135), wxSize(300, 200), wxTE_READONLY + wxTE_MULTILINE);

	//wxStreamToTextRedirector redirect(output);

	//std::cout << "ENSURE DATA IS ORDERED IN ALL SHEETS" << std::endl;
}

cFrame::~cFrame()
{

}

void cFrame::PerformTransfer(wxCommandEvent& evt)
{
	//wxStreamToTextRedirector redirect(output, std::wcout);

	std::wstring srcPath = srcFile->GetPath().ToStdWstring();
	std::wstring destPath = dstFile->GetPath().ToStdWstring();
	//output->Clear();
	int row = std::stoi(rowInput->GetLineText(0).ToStdString()) - 1;

	cTransfer* transfer = new cTransfer(srcPath, destPath, row);
	transfer->CopyBook();
	delete transfer;

	evt.Skip();
}