#pragma once

#include "wx/wx.h"
#include "cFrame.h"

class cApp : public wxApp
{
public:
	cApp();
	~cApp();
	virtual bool OnInit();

private:
	cFrame* frame1 = nullptr;

};

