#pragma once

#include "wx/wx.h"
#include "wx/filepicker.h"
#include "wx/valnum.h"
#include "cTransfer.h"

class cFrame : public wxFrame
{
public:
	cFrame();
	~cFrame();

	wxButton* btn1 = nullptr;
	wxFilePickerCtrl* srcFile = nullptr;
	wxFilePickerCtrl* dstFile = nullptr;
	wxTextCtrl* rowInput = nullptr;
	//wxTextCtrl* output = nullptr;

	wxStaticText* srcText = nullptr;
	wxStaticText* dstText = nullptr;
	wxStaticText* rowText = nullptr;

	void PerformTransfer(wxCommandEvent& evt);
	void ResetButton(wxFileDirPickerEvent& evt);

	wxDECLARE_EVENT_TABLE();

};

