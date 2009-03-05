#ifndef __QTCURVE_H__
#define __QTCURVE_H__

/*
  QtCurve (C) Craig Drummond, 2003 - 2009 craig_p_drummond@yahoo.co.uk

  ----

  Based upon B???Curve style (Copyright (c) 2002 R?d H?t, Inc)
      Bernhard Rosenkrazer <bero@r?dh?t.com>
      Preston Brown <pbrown@r?dh?t.com>
      Than Ngo <than@r?dh?t.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  ----

  B???Curve is based on the KDE Light style, 2nd revision:
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com). The light style
  license is as follows:

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "config.h"
#include <kdeversion.h>
#include <kstyle.h>
#include <qcolor.h>
#include <qpoint.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qcache.h>
#include <qvaluelist.h>
#include "common.h"

class QTimer;
class QSlider;

class QtCurveStyle : public KStyle
{
    Q_OBJECT

    public:

    enum EApp
    {
        APP_KICKER,
        APP_KORN,
        APP_OPENOFFICE,
        APP_MACTOR,
        APP_KPRESENTER,
        APP_KONQUEROR,
        APP_SKIP_TASKBAR,
        APP_KPRINTER,
        APP_KDIALOG,
        APP_KDIALOGD,
        APP_TORA,
        APP_KONTACT,
        APP_OPERA,
        APP_SYSTEMSETTINGS,
        APP_KATE,
        APP_OTHER
    };

    enum EHover
    {
        HOVER_NONE,
        HOVER_CHECK,
        HOVER_RADIO,
        HOVER_SB_ADD,
        HOVER_SB_SUB,
        HOVER_SB_SUB2,
        HOVER_SB_SLIDER,
        HOVER_HEADER,
        HOVER_KICKER,
        HOVER_SW_UP,
        HOVER_SW_DOWN,
        HOVER_CB_ARROW
    };

    QtCurveStyle(const QString &name=QString());
    virtual ~QtCurveStyle();

    void polish(QApplication *app);
    void polish(QPalette &pal);
    QColorGroup setColorGroup(const QColorGroup &old);
    void polish(QWidget *widget);
    void unPolish(QWidget *widget);
    void drawLightBevel(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags,
                        int round, const QColor &fill, const QColor *custom=NULL,
                        bool doBorder=true, bool doCorners=true, EWidget w=WIDGET_OTHER) const
    { drawLightBevel(cg.background(), p, r, cg, flags, round, fill, custom, doBorder, doCorners, w); }
    void drawLightBevel(const QColor &bgnd, QPainter *p, const QRect &r, const QColorGroup &cg,
                        SFlags flags, int round, const QColor &fill, const QColor *custom=NULL,
                        bool doBorder=true, bool doCorners=true, EWidget w=WIDGET_OTHER) const;
    void drawGlow(QPainter *p, const QRect &r, const QColorGroup &cg, EWidget w) const;
    void drawEtch(QPainter *p, const QRect &r, const QColorGroup &cg, bool raised=false) const;
    void drawBorder(const QColor &bgnd, QPainter *p, const QRect &r, const QColorGroup &cg,
                    SFlags flags, int round, const QColor *custom=NULL, EWidget w=WIDGET_OTHER,
                    bool doCorners=true, EBorder borderProfile=BORDER_FLAT, bool blendBorderColors=true, int borderVal=QT_STD_BORDER) const;
    void drawMdiIcon(QPainter *painter, const QColor &color, const QColor &shadow, const QRect &r, bool sunken, int margin,
                     SubControl button) const;
    void drawWindowIcon(QPainter *painter, const QColor &color, const QRect &r, bool sunken, int margin, SubControl button) const;
    void drawEntryField(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags,
                        bool highlight, int round, EWidget=WIDGET_ENTRY) const;
    void drawArrow(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags,
                   QStyle::PrimitiveElement pe,  bool small=false, bool checkActive=false) const;
    void drawPrimitive(PrimitiveElement, QPainter *, const QRect &, const QColorGroup &,
                       SFlags = Style_Default, const QStyleOption & = QStyleOption::Default) const;
    void drawKStylePrimitive(KStylePrimitive kpe, QPainter* p, const QWidget* widget, const QRect &r,
                             const QColorGroup &cg, SFlags flags, const QStyleOption &opt ) const;
    void drawControl(ControlElement, QPainter *, const QWidget *, const QRect &, const QColorGroup &,
                     SFlags = Style_Default, const QStyleOption & = QStyleOption::Default) const;
    void drawControlMask(ControlElement, QPainter *, const QWidget *, const QRect &,
                         const QStyleOption & = QStyleOption::Default) const;
    void drawComplexControlMask(ComplexControl control, QPainter *p, const QWidget *widget,
                                const QRect &r, const QStyleOption &data) const;
    QRect subRect(SubRect, const QWidget *) const;
    void drawComplexControl(ComplexControl, QPainter *, const QWidget *, const QRect &,
                            const QColorGroup &, SFlags = Style_Default, SCFlags = SC_All,
                            SCFlags = SC_None, const QStyleOption & = QStyleOption::Default) const;
    QRect querySubControlMetrics(ComplexControl, const QWidget *, SubControl,
                                 const QStyleOption & = QStyleOption::Default) const;
    int pixelMetric(PixelMetric, const QWidget *widget= 0) const;
    int kPixelMetric(KStylePixelMetric kpm, const QWidget *widget) const;
    QSize sizeFromContents(ContentsType, const QWidget *, const QSize &,
                           const QStyleOption & = QStyleOption::Default) const;
    int styleHint(StyleHint, const QWidget *widget= 0, const QStyleOption & = QStyleOption::Default,
                  QStyleHintReturn *returnData= 0) const;
    void drawItem(QPainter *p, const QRect &r, int flags, const QColorGroup &cg, bool enabled,
                  const QPixmap *pixmap, const QString &text, int len=-1, const QColor *penColor=0) const;

    protected:

    bool appIsNotEmbedded(QDialog *dlg);
    bool eventFilter(QObject *object, QEvent *event);
    void drawMenuItem(QPainter *p, const QRect &r, int flags, const QColorGroup &cg,
                      bool mbi, int round, const QColor &bgnd, const QColor *cols) const;
    void drawProgress(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags, int round,
                      const QWidget *widget) const;
    void drawBevelGradient(const QColor &base, QPainter *p, QRect const &r,
                           bool horiz, bool sel, EAppearance bevApp, EWidget w=WIDGET_OTHER) const;
    void drawGradient(const QColor &top, const QColor &bot, QPainter *p, const QRect &r, bool horiz=true) const;
    void drawSbSliderHandle(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags, bool slider=false) const;
    void drawSliderHandle(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags, QSlider *slider, bool tb=false) const;
    void drawSliderGroove(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags,
                          const QWidget *widget) const;
    void drawMenuOrToolBarBackground(QPainter *p, const QRect &r, const QColorGroup &cg, bool menu=true,
                                     bool horiz=true) const;
    void drawHandleMarkers(QPainter *p, const QRect &r, SFlags flags, bool tb, ELine handles) const;

    private:

    static QColor shadowColor(const QColor col)
    {
        return qGray(col.rgb()) < 100 ? QColor(255, 255, 255) : QColor(0, 0, 0);
    }

    void           shadeColors(const QColor &base, QColor *vals) const;
    const QColor * buttonColors(const QColorGroup &cg) const;
    const QColor * sliderColors(/*const QColorGroup &cg, */SFlags flags) const;
    const QColor * backgroundColors(const QColor &c) const;
    const QColor * backgroundColors(const QColorGroup &cg) const
                                { return backgroundColors(cg.background()); }
    const QColor * borderColors(SFlags flags, const QColor *use) const;
    const QColor * getSidebarButtons() const;
    void           setMenuColors(const QColorGroup &cg);
    const QColor * getMdiColors(const QColorGroup &cg, bool active) const;
#ifdef SET_MDI_WINDOW_BUTTON_POSITIONS
    void           readMdiPositions() const;
#endif
    bool           redrawHoverWidget(const QPoint &pos);
    const QColor & getFill(SFlags flags, const QColor *use, bool cr=false) const;
    const QColor & getListViewFill(SFlags flags, const QColor *use) const;
    const QColor & getTabFill(bool current,  bool highlight, const QColor *use) const;
    const QColor & menuStripeCol() const;
    QPixmap *      getPixelPixmap(const QColor col) const;
    QPixmap *      getPixmap(const QColor col, EPixmap pix, double shade=1.0) const;
    void           setSbType();
    bool           isFormWidget(const QWidget *w) const { return itsKhtmlWidgets.contains(w); }
    void           resetHover();

    private slots:

    void updateProgressPos();
    void progressBarDestroyed(QObject *bar);
    void sliderThumbMoved(int val);
    void khtmlWidgetDestroyed(QObject *o);
    void hoverWidgetDestroyed(QObject *o);

    private:

    Options                    opts;
    QColor                     itsMenuitemCols[TOTAL_SHADES+1],
                               itsBackgroundCols[TOTAL_SHADES+1],
                               itsMenubarCols[TOTAL_SHADES+1],
                               *itsSliderCols,
                               *itsDefBtnCols,
                               *itsMouseOverCols,
                               itsButtonCols[TOTAL_SHADES+1],
                               itsLighterPopupMenuBgndCol,
                               itsCheckRadioCol;
    mutable QColor             *itsSidebarButtonsCols;
    mutable QColor             *itsActiveMdiColors;
    mutable QColor             *itsMdiColors;
    mutable QColor             itsActiveMdiTextColor;
    mutable QColor             itsMdiTextColor;
    mutable QColor             itsColoredButtonCols[TOTAL_SHADES+1];
    mutable QColor             itsColoredBackgroundCols[TOTAL_SHADES+1];
    EApp                       itsThemedApp;
    mutable QCache<QPixmap>    itsPixmapCache;
#if KDE_VERSION >= 0x30200
    bool                       itsIsTransKicker;
#endif
    EHover                     itsHover;
    QPoint                     itsOldPos;
    mutable bool               itsFormMode;
    QWidget                    *itsHoverWidget;
    int                        itsHoverSect;
    QTab                       *itsHoverTab;
    QPalette                   *itsMactorPal;
    QMap<QWidget*, int>        itsProgAnimWidgets;
    QMap<const QWidget*, bool> itsKhtmlWidgets;
    QTimer                     *itsAnimationTimer;
    mutable bool               itsActive,
                               itsIsSpecialHover;
    mutable QValueList<int>    itsMdiButtons[2]; // 0=left, 1=right
};

#endif
