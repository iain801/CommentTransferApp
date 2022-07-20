#pragma once

#include "wx/wx.h"
#include "wx/filepicker.h"
#include "wx/valnum.h"
#include "libxl.h"
#include <iostream>

class cFrame : public wxFrame
{
public:
	cFrame();
	~cFrame();

	wxButton* btn1 = nullptr;
	wxFilePickerCtrl* srcFile = nullptr;
	wxFilePickerCtrl* dstFile = nullptr;
	wxTextCtrl* rowInput = nullptr;
	wxTextCtrl* output = nullptr;

	wxStaticText* srcText = nullptr;
	wxStaticText* dstText = nullptr;
	wxStaticText* rowText = nullptr;

	void PerformTransfer(wxCommandEvent& evt);
	

	wxDECLARE_EVENT_TABLE();

private:
	void CopySheet(libxl::Sheet* srcSheet, libxl::Sheet* destSheet);
	void CopyCell(libxl::Sheet* srcSheet, libxl::Sheet* destSheet, int row, int col);
	int getCommentCol(libxl::Sheet* sheet);
};

