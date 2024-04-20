// XYSlider.cpp - Implementation of CXySlider
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
// Copyright (C) 2004-2024 WinDirStat Team (windirstat.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "stdafx.h"
#include "SelectObject.h"
#include "XYSlider.h"

#include "resource.h"

IMPLEMENT_DYNAMIC(CXySlider, CStatic)

const UINT CXySlider::XY_SETPOS = WM_USER + 100;
const UINT CXySlider::XY_GETPOS = WM_USER + 101;

void AFXAPI DDX_XySlider(CDataExchange* pDX, int nIDC, CPoint& value)
{
    pDX->PrepareCtrl(nIDC);
    HWND hWndCtrl;
    pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
    if (pDX->m_bSaveAndValidate)
    {
        ::SendMessage(hWndCtrl, CXySlider::XY_GETPOS, 0, reinterpret_cast<LPARAM>(&value));
    }
    else
    {
        ::SendMessage(hWndCtrl, CXySlider::XY_SETPOS, 0, reinterpret_cast<LPARAM>(&value));
    }
}

void CXySlider::Initialize()
{
    if (!m_Inited && ::IsWindow(m_hWnd))
    {
        // Make size odd, so that zero lines are central
        CRect rc;
        GetWindowRect(rc);
        GetParent()->ScreenToClient(rc);
        if (rc.Width() % 2 == 0)
        {
            rc.right--;
        }
        if (rc.Height() % 2 == 0)
        {
            rc.bottom--;
        }
        MoveWindow(rc);

        // Initialize constants
        CalcSizes();

        m_Inited = true;
    }
}

void CXySlider::GetRange(CSize& range) const
{
    range = m_ExternalRange;
}

void CXySlider::SetRange(const CSize range)
{
    m_ExternalRange = range;
}

CPoint CXySlider::GetPos() const
{
    return m_ExternalPos;
}

LRESULT CXySlider::OnSetPos(WPARAM, const LPARAM lparam)
{
    const POINT* point = reinterpret_cast<POINT*>(lparam);
    SetPos(*point);
    return 0;
}

LRESULT CXySlider::OnGetPos(WPARAM, const LPARAM lparam)
{
    const auto point = reinterpret_cast<POINT*>(lparam);
    *point = GetPos();
    return 0;
}

void CXySlider::SetPos(const CPoint pt)
{
    Initialize();

    m_ExternalPos = pt;
    ExternToIntern();
    Invalidate();
}

void CXySlider::CalcSizes()
{
    static constexpr int _gripperRadius = 8;

    GetClientRect(m_RcAll);

    ASSERT(m_RcAll.left == 0);
    ASSERT(m_RcAll.top == 0);
    ASSERT(m_RcAll.Width() % 2 == 1);
    ASSERT(m_RcAll.Height() % 2 == 1);
    ASSERT(m_RcAll.Width() >= _gripperRadius * 2); // Control must be large enough
    ASSERT(m_RcAll.Height() >= _gripperRadius * 2);

    m_Zero.x = m_RcAll.Width() / 2;
    m_Zero.y = m_RcAll.Height() / 2;

    m_Radius.cx = m_RcAll.Width() / 2 - 1;
    m_Radius.cy = m_RcAll.Height() / 2 - 1;

    m_RcInner = m_RcAll;
    m_RcInner.DeflateRect(_gripperRadius - 3, _gripperRadius - 3);

    m_GripperRadius.cx = _gripperRadius;
    m_GripperRadius.cy = _gripperRadius;

    m_Range = m_Radius - m_GripperRadius;
}

CRect CXySlider::GetGripperRect() const
{
    CRect rc(
        -m_GripperRadius.cx,
        -m_GripperRadius.cy,
        m_GripperRadius.cx + 1,
        m_GripperRadius.cy + 1
    );
    rc.OffsetRect(m_Zero);
    rc.OffsetRect(m_Pos);
    return rc;
}

void CXySlider::CheckMinMax(LONG& val, const int min, const int max)
{
    ASSERT(min <= max);

    if (val < min)
    {
        val = min;
    }
    if (val > max)
    {
        val = max;
    }
}

void CXySlider::InternToExtern()
{
    m_ExternalPos.x = static_cast<int>(static_cast<double>(abs(m_Pos.x)) * m_ExternalRange.cx / m_Range.cx + 0.5) * signum(m_Pos.x);
    m_ExternalPos.y = static_cast<int>(static_cast<double>(abs(m_Pos.y)) * m_ExternalRange.cy / m_Range.cy + 0.5) * signum(m_Pos.y);
}

void CXySlider::ExternToIntern()
{
    m_Pos.x = static_cast<int>(static_cast<double>(abs(m_ExternalPos.x)) * m_Range.cx / m_ExternalRange.cx + 0.5) * signum(m_ExternalPos.x);
    m_Pos.y = static_cast<int>(static_cast<double>(abs(m_ExternalPos.y)) * m_Range.cy / m_ExternalRange.cy + 0.5) * signum(m_ExternalPos.y);
}

void CXySlider::NotifyParent()
{
    NMHDR hdr;
    hdr.hwndFrom = m_hWnd;
    hdr.idFrom   = GetDlgCtrlID();
    hdr.code     = XYSLIDER_CHANGED;

    GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), reinterpret_cast<LPARAM>(&hdr));
}

void CXySlider::PaintBackground(CDC* pdc)
{
    pdc->FillSolidRect(m_RcAll, ::GetSysColor(COLOR_BTNFACE));

    CRect rc = m_RcInner;
    pdc->DrawEdge(rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);

    pdc->FillSolidRect(rc, RGB(255, 255, 255));

    CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DLIGHT));
    CSelectObject sopen(pdc, &pen);

    pdc->MoveTo(rc.left, m_Zero.y);
    pdc->LineTo(rc.right, m_Zero.y);
    pdc->MoveTo(m_Zero.x, rc.top);
    pdc->LineTo(m_Zero.x, rc.bottom);

    CRect circle = m_RcAll;
    circle.DeflateRect(m_GripperRadius);

    CSelectStockObject sobrush(pdc, NULL_BRUSH);
    pdc->Ellipse(circle);

    if (GetFocus() == this)
    {
        pdc->DrawFocusRect(m_RcAll);
    }
}


void CXySlider::PaintGripper(CDC* pdc)
{
    CRect rc = GetGripperRect();

    COLORREF color = ::GetSysColor(COLOR_BTNFACE);
    if (m_GripperHighlight)
    {
        int r = RGB_GET_RVALUE(color);
        int g = RGB_GET_GVALUE(color);
        int b = RGB_GET_BVALUE(color);
        r += (255 - r) / 3;
        g += (255 - g) / 3;
        b += (255 - b) / 3;
        color = RGB(r, g, b);
    }
    pdc->FillSolidRect(rc, color);
    pdc->DrawEdge(rc, EDGE_RAISED, BF_RECT);

    CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
    CSelectObject sopen(pdc, &pen);

    pdc->MoveTo(rc.left, rc.top + rc.Height() / 2);
    pdc->LineTo(rc.right, rc.top + rc.Height() / 2);
    pdc->MoveTo(rc.left + rc.Width() / 2, rc.top);
    pdc->LineTo(rc.left + rc.Width() / 2, rc.bottom);
}

void CXySlider::DoMoveBy(const int cx, const int cy)
{
    m_Pos.x += cx;
    CheckMinMax(m_Pos.x, -m_Range.cx, m_Range.cx);

    m_Pos.y += cy;
    CheckMinMax(m_Pos.y, -m_Range.cy, m_Range.cy);

    RedrawWindow();

    const CPoint oldpos = m_ExternalPos;
    InternToExtern();
    if (m_ExternalPos != oldpos)
    {
        NotifyParent();
    }
}

void CXySlider::DoDrag(const CPoint point)
{
    CPoint pt0 = point;

    HighlightGripper(true);

    const CSize inGripper = pt0 - GetGripperRect().CenterPoint();
    const CPoint ptMin(m_Zero - m_Range + inGripper);
    const CPoint ptMax(m_Zero + m_Range + inGripper);

    SetCapture();
    while (true)
    {
        MSG msg;
        if (!GetMessage(&msg, nullptr, 0, 0))
        {
            break;
        }

        if (msg.message == WM_LBUTTONUP)
        {
            break;
        }

        if (GetCapture() != this)
        {
            break;
        }

        if (msg.message == WM_MOUSEMOVE)
        {
            CPoint pt = msg.pt;
            ScreenToClient(&pt);

            CheckMinMax(pt.x, ptMin.x, ptMax.x);
            CheckMinMax(pt.y, ptMin.y, ptMax.y);

            const int dx = pt.x - pt0.x;
            const int dy = pt.y - pt0.y;

            DoMoveBy(dx, dy);

            pt0 = pt;
        }
        else
        {
            DispatchMessage(&msg);
        }
    }
    ReleaseCapture();

    HighlightGripper(false);
}

void CXySlider::DoPage(const CPoint point)
{
    const CSize sz = point - (m_Zero + m_Pos);

    ASSERT(sz.cx != 0 || sz.cy != 0);

    const double len = sqrt(static_cast<double>(sz.cx * sz.cx + sz.cy * sz.cy));

    constexpr double d = 10;

    const int dx = static_cast<int>(d * sz.cx / len);
    const int dy = static_cast<int>(d * sz.cy / len);

    DoMoveBy(dx, dy);
}

void CXySlider::HighlightGripper(const bool on)
{
    m_GripperHighlight = on;
    RedrawWindow();
}

void CXySlider::InstallTimer()
{
    RemoveTimer();
    m_Timer = SetTimer(ID_WDS_CONTROL, 500, nullptr);
}

void CXySlider::RemoveTimer()
{
    if (m_Timer != 0)
    {
        KillTimer(m_Timer);
    }
    m_Timer = 0;
}

BEGIN_MESSAGE_MAP(CXySlider, CStatic)
    ON_WM_DESTROY()
    ON_WM_GETDLGCODE()
    ON_WM_NCHITTEST()
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
    ON_WM_PAINT()
    ON_WM_KEYDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_MESSAGE(CXySlider::XY_SETPOS, OnSetPos)
    ON_MESSAGE(CXySlider::XY_GETPOS, OnGetPos)
END_MESSAGE_MAP()

void CXySlider::OnDestroy()
{
    RemoveTimer();
    CStatic::OnDestroy();
}

UINT CXySlider::OnGetDlgCode()
{
    return DLGC_WANTARROWS;
}

LRESULT CXySlider::OnNcHitTest(CPoint /*point*/)
{
    return HTCLIENT;
}

void CXySlider::OnSetFocus(CWnd* pOldWnd)
{
    CStatic::OnSetFocus(pOldWnd);
    Invalidate();
}

void CXySlider::OnKillFocus(CWnd* pNewWnd)
{
    CStatic::OnKillFocus(pNewWnd);
    Invalidate();
}

void CXySlider::OnPaint()
{
    Initialize();
    const int w = m_RcAll.Width();
    const int h = m_RcAll.Height();

    CPaintDC dc(this);
    CDC dcmem;
    dcmem.CreateCompatibleDC(&dc);
    CBitmap bm;
    bm.CreateCompatibleBitmap(&dc, w, h);
    CSelectObject sobm(&dcmem, &bm);

    PaintBackground(&dcmem);
    // PaintValues(&dcmem); This is too noisy
    PaintGripper(&dcmem);

    dc.BitBlt(0, 0, w, h, &dcmem, 0, 0, SRCCOPY);
}

void CXySlider::OnKeyDown(const UINT nChar, const UINT nRepCnt, const UINT nFlags)
{
    switch (nChar)
    {
        case VK_LEFT:  DoMoveBy(-1, 0); break;
        case VK_RIGHT: DoMoveBy(1, 0);  break;
        case VK_UP:    DoMoveBy(0, -1); break;
        case VK_DOWN:  DoMoveBy(0, 1);  break;
        default: CStatic::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}

void CXySlider::OnLButtonDown(UINT /*nFlags*/, const CPoint point)
{
    SetFocus();
    DoDrag(point);

    if (const CRect rc = GetGripperRect(); !rc.PtInRect(point))
    {
        InstallTimer();
    }
}

void CXySlider::OnLButtonDblClk(UINT /*nFlags*/, const CPoint point)
{
    SetFocus();

    if (GetGripperRect().PtInRect(point))
    {
        DoMoveBy(-m_Pos.x, -m_Pos.y);
    }
    else
    {
        DoPage(point);
        InstallTimer();
    }
}

void CXySlider::OnLButtonUp(const UINT nFlags, const CPoint point)
{
    RemoveTimer();
    CStatic::OnLButtonUp(nFlags, point);
}

void CXySlider::OnTimer(UINT_PTR /*nIDEvent*/)
{
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);

    const CRect rc = GetGripperRect();
    if (!rc.PtInRect(point))
    {
        DoPage(point);
    }
}
