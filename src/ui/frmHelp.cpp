//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// frmHelp.cpp - SQL Help Screen
//
//////////////////////////////////////////////////////////////////////////


#include "pgAdmin3.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/fs_inet.h>
#include <wx/file.h>

// App headers
#include "copyright.h"
#include "frmHelp.h"
#include "frmMain.h"
#include "menu.h"

// Icons
#include "images/pgAdmin3.xpm"
#include "images/reload.xpm"
#include "images/forward.xpm"
#include "images/back.xpm"
#include "images/help.xpm"




BEGIN_EVENT_TABLE(frmHelp, wxFrame)
    EVT_MENU(MNU_BACK,                  frmHelp::OnBack)
    EVT_MENU(MNU_FORWARD,               frmHelp::OnForward)
    EVT_MENU(MNU_REFRESH,               frmHelp::OnRefresh)
END_EVENT_TABLE();



frmHelp::frmHelp(wxWindow *fr) 
: wxFrame(fr, -1, wxT(""))
{
    SetIcon(wxIcon(pgAdmin3_xpm));

    wxWindowBase::SetFont(settings->GetSystemFont());
    wxToolBar *toolBar=CreateToolBar();
    toolBar->SetToolBitmapSize(wxSize(24, 24));

    toolBar->AddTool(MNU_BACK, _("Back"), wxBitmap(back_xpm), _("Back to previous page."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_FORWARD, _("Forward"), wxBitmap(forward_xpm), _("Forward to next page."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_REFRESH, _("Reload"), wxBitmap(reload_xpm), _("Reload current page."), wxITEM_NORMAL);
    toolBar->Realize();

    wxAcceleratorEntry entries[2];

    entries[0].Set(wxACCEL_ALT, WXK_LEFT,   MNU_BACK);
    entries[1].Set(wxACCEL_ALT, WXK_RIGHT,  MNU_FORWARD);
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);

    
    CreateStatusBar();
    htmlWindow = new ctlHelpWindow(this);
    htmlWindow->SetRelatedFrame(this, APPNAME_L wxT(" - %s"));
    htmlWindow->SetRelatedStatusBar(0);

    wxPoint pos(settings->Read(wxT("frmHelp"), wxPoint(100, 100)));
    wxSize size(settings->Read(wxT("frmHelp"), wxSize(400, 500)));
    CheckOnScreen(pos, size, 200, 150);
    Move(pos);
    SetSize(size);

    CheckToolBar();
};


frmHelp::~frmHelp()
{
    settings->Write(wxT("frmHelp"), GetSize(), GetPosition());
}


bool frmHelp::LoadSqlDoc(wxWindow *wnd, const wxString &page, char **icon)
{
    wxString helpSite=settings->GetSqlHelpSite();

    frmHelp *h=new frmHelp(wnd);
    h->Show(true);
    bool loaded=h->Load(helpSite + page, icon);
    if (!loaded)
        h->Destroy();

    return loaded;
}


bool frmHelp::LoadLocalDoc(wxWindow *wnd, const wxString &page, char **icon)
{
    extern wxString docPath;
    wxString cn=settings->GetCanonicalLanguage();
    if (cn.IsEmpty())
        cn=wxT("en_US");

    wxString file=docPath + wxT("/") + cn + wxT("/") + page;

    if (!wxFile::Exists(file))
        file = docPath + wxT("/en_US/") + page;
    if (!wxFile::Exists(file))
        return false;

    frmHelp *h=new frmHelp(wnd);
    h->Show(true);
    bool loaded=h->Load(wxT("file:") + file, icon);
    if (!loaded)
        h->Destroy();

    return loaded;
}


bool frmHelp::Load(const wxString &page, char **icon)
{
    if (currentPage.IsEmpty())
    {
        htmlWindow->SetPage(wxString::Format(_("<html><body>Loading %s</body></html>"), page.c_str()));
        if (icon)
            SetIcon(wxIcon(icon));
        else
            SetIcon(wxIcon(help_xpm));
    }


    currentPage=page;
    bool loaded=htmlWindow->LoadPage(page);
    if (loaded)
        CheckToolBar();
    return loaded;
}


void frmHelp::CheckToolBar()
{
    wxToolBar *tb=GetToolBar();
    tb->EnableTool(MNU_REFRESH, !currentPage.IsEmpty());
    tb->EnableTool(MNU_BACK, htmlWindow->HistoryCanBack());
    tb->EnableTool(MNU_FORWARD, htmlWindow->HistoryCanForward());
}


void frmHelp::OnLinkClicked(const wxHtmlLinkInfo& link)
{
    wxString newPage=link.GetHref();
    long dp=currentPage.Find(wxT(":"));

    if (newPage.StartsWith(wxT("/")))
    {
        // relative to root
        if (dp < 0)
            currentPage=newPage;
        else {
            long sp = currentPage.Mid(dp+3).Find(wxT("/"));
            currentPage=currentPage.Left(sp+7) + newPage;
        }
    }
    else if (dp < 0)
    {
        // relativ to current page
        currentPage=currentPage.BeforeLast('/') + wxT("/") + newPage;
    }
    else
        currentPage=newPage;

    htmlWindow->LoadPage(currentPage);
    CheckToolBar();
}


void frmHelp::OnRefresh(wxCommandEvent &ev)
{
    htmlWindow->LoadPage(currentPage);
    CheckToolBar();
}


void frmHelp::OnBack(wxCommandEvent &ev)
{
    htmlWindow->HistoryBack();
    currentPage=htmlWindow->GetOpenedPage();
    CheckToolBar();
}


void frmHelp::OnForward(wxCommandEvent &ev)
{
    htmlWindow->HistoryForward();
    currentPage=htmlWindow->GetOpenedPage();
    CheckToolBar();
}


ctlHelpWindow::ctlHelpWindow(frmHelp *frm) 
: wxHtmlWindow(frm) 
{
    helpForm = frm; 
    m_Parser->GetFS()->AddHandler(new wxInternetFSHandler());
}


void ctlHelpWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
    helpForm->OnLinkClicked(link);
}
