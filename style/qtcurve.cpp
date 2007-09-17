/*
  QtCurve (C) Craig Drummond, 2003 - 2007 Craig.Drummond@lycos.co.uk

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
  Copyright(c)2000-2001 Trolltech AS (info@trolltech.com). The light style
  license is as follows:

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files(the "Software"),
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

/*
KHTML
-----
Srollbars:

For some reason Scrollbars in KHTML seem to lose the bottom/left pixels. As if KHTML expects
the scrollbar to be 1 pixel smaller/thinner than it actually is. To 'fix' this, the pixelMetric
function will return 1 scrollbar with 1 greater than standard for form widgets, or where widget==NULL

In the CC_ScrollBar draw code, the rects used for each component are shrunk by 1, in the appropriate
dimension, so as to draw the scrollbar at the correct size.
*/

#include <kdeversion.h>
#include <qsettings.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qdrawutil.h>
#include <qscrollbar.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qtable.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qslider.h>
#include <qcleanuphandler.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qprogressbar.h>
#include <qheader.h>
#include <qwidgetstack.h>
#include <qsplitter.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtimer.h>
#include <qdatetimeedit.h>
#include <qobjectlist.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#define QTC_COMMON_FUNCTIONS
#include "qtcurve.h"
#define CONFIG_READ
#include "config_file.c"
#include "pixmaps.h"
#include <qdialog.h>
#include <qprogressdialog.h>
#include <qstyleplugin.h>
#include <qgroupbox.h>                      
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>

class QtCurveStylePlugin : public QStylePlugin
{
    public:

    QtCurveStylePlugin();

    QStringList   keys() const;
    QStyle      * create(const QString &);
};

QtCurveStylePlugin::QtCurveStylePlugin()
                  : QStylePlugin()
{
}

QStringList QtCurveStylePlugin::keys() const
{
    QStringList list;
    list << "QtCurve";

    return list;
}

QStyle * QtCurveStylePlugin::create(const QString &s)
{
    return (s.lower() == "qtcurve") ? new QtCurveStyle : 0;
}

Q_EXPORT_PLUGIN(QtCurveStylePlugin)

#define QTC_SKIP_TASKBAR (APP_SKIP_TASKBAR==itsThemedApp || APP_KPRINTER==itsThemedApp || APP_KDIALOG==itsThemedApp)

#if KDE_VERSION >= 0x30200
#include <qfile.h>
#include <qdir.h>
#endif

#define QTC_NO_SECT -1

#define QTC_VERTICAL_TB_BUTTON 0x01000000
#define QTC_CHECK_BUTTON       0x02000000
#define QTC_STD_TOOLBUTTON     0x04000000
#define QTC_TOGGLE_BUTTON      0x08000000
#define QTC_NO_ETCH_BUTTON     0x10000000
#define QTC_LISTVIEW_ITEM      0x20000000
#define QTC_MENU_ITEM          0x40000000

#if KDE_VERSION >= 0x30200
// Try to read $KDEHOME/share/config/kickerrc to find out if kicker is transparent...

static QString readEnvPath(const char *env)
{
   QCString path=getenv(env);

   return path.isEmpty() ? QString::null : QFile::decodeName(path);
}

static bool kickerIsTrans()
{
    QString kdeHome(readEnvPath(getuid() ? "KDEHOME" : "KDEROOTHOME")),
            cfgFileName(kdeHome.isEmpty() ? QDir::homeDirPath()+"/.kde/share/config/kickerrc"
                                          : kdeHome+"/share/config/kickerrc");
    bool    trans(false);
    QFile   cfgFile(cfgFileName);

    if (cfgFile.open(IO_ReadOnly))
    {
        QTextStream stream(&cfgFile);
        QString     line;
        bool        stop(false),
                    inGen(false);

        while (!stream.atEnd() && !stop)
        {
            line=stream.readLine();

            if(inGen)
            {
                if(0==line.find("Transparent="))  // Found it!
                {
                    if(-1!=line.find("true"))
                        trans=true;
                    stop=true;
                }
                else if(line[0]==QChar('['))   // Then wasn't in General section...
                    stop=true;
            }
            else if(0==line.find("[General]"))
                inGen=true;
        }
        cfgFile.close();
    }

    return trans;
}
#endif

inline QColor midColor(const QColor &a, const QColor &b, double factor=1.0)
{
    return QColor((a.red()+limit(b.red()*factor))>>1, 
                  (a.green()+limit(b.green()*factor))>>1, 
                  (a.blue()+limit(b.blue()*factor))>>1);
}

static bool isKhtmlWidget(const QWidget *w, int level=1)
{
    return w && ((w->name() && 0==strcmp(w->name(), "__khtml")) ||
                 (level && isKhtmlWidget(w->parentWidget(), --level)));
}

static bool isKhtmlFormWidget(const QWidget *widget)
{
    if(isKhtmlWidget(widget))
        return true;

    // Copied from Keramik...

    //Form widgets are in the KHTMLView, but that has 2 further inner levels
    //of widgets - QClipperWidget, and outside of that, QViewportWidget
    QWidget *potentialClipPort(widget ? widget->parentWidget() : NULL);

    if (!potentialClipPort || potentialClipPort->isTopLevel())
        return false;

    QWidget *potentialViewPort(potentialClipPort->parentWidget());

    if (!potentialViewPort || potentialViewPort->isTopLevel() ||
        qstrcmp(potentialViewPort->name(), "qt_viewport"))
        return false;

    QWidget *potentialKHTML(potentialViewPort->parentWidget());

    if (!potentialKHTML || potentialKHTML->isTopLevel() ||
        qstrcmp(potentialKHTML->className(), "KHTMLView"))
        return false;

    return true;
}

QColor shade(const QColor &a, float k)
{
    QColor mod;

    shade(a, &mod, k);
    return mod;
}

static void drawLines(QPainter *p, const QRect &r, bool horiz, int nLines, int offset,
                      const QColor *cols, int startOffset, int dark, int etchedDisp=1)
{
    int space((nLines*2)+(etchedDisp ? (nLines-1) : 0)),
        step(etchedDisp ? 3 : 2),
        x(horiz ? r.x(): r.x()+((r.width()-space)>>1)),
        y(horiz ? r.y()+((r.height()-space)>>1): r.y()),
        x2(r.x()+r.width()-1),
        y2(r.y()+r.height()-1),
        i;

    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=step)
            p->drawLine(x+offset, y+i, x2-(offset+etchedDisp), y+i);

        p->setPen(cols[0]);
        for(i=1; i<space; i+=step)
            p->drawLine(x+offset+etchedDisp, y+i, x2-offset, y+i);
    }
    else
    {
        if(startOffset && x+startOffset>0)
            x+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=step)
            p->drawLine(x+i, y+offset, x+i, y2-(offset+etchedDisp));

        p->setPen(cols[0]);
        for(i=1; i<space; i+=step)
            p->drawLine(x+i, y+offset+etchedDisp, x+i, y2-offset);
    }
}

static void drawDots(QPainter *p, const QRect &r, bool horiz, int nLines, int offset,
                     const QColor *cols, int startOffset, int dark)
{
    int space((nLines*2)+(nLines-1)),
        x(horiz ? r.x() : r.x()+((r.width()-space)>>1)),
        y(horiz ? r.y()+((r.height()-space)>>1) : r.y()),
        i, j,
        numDots((horiz ? (r.width()-(2*offset))/3 : (r.height()-(2*offset))/3)+1);

    if(horiz)
    {
        if(startOffset && y+startOffset>0)
            y+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+offset+(3*j), y+i);

        p->setPen(cols[0]);
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+offset+1+(3*j), y+i);
    }
    else
    {
        if(startOffset && x+startOffset>0)
            x+=startOffset;

        p->setPen(cols[dark]);
        for(i=0; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+i, y+offset+(3*j));

        p->setPen(cols[0]);
        for(i=1; i<space; i+=3)
            for(j=0; j<numDots; j++)
                p->drawPoint(x+i, y+offset+1+(3*j));
    }
}

static bool onToolBar(QWidget *widget, int l=0)
{
    return l<3 && widget && widget->parentWidget()
               ? widget->parentWidget()->inherits("QToolBar") ||
                 onToolBar(widget->parentWidget(), ++l)
               : false;
}

enum ECacheAppearance
{
    CACHE_APPEARANCE_SELECTED=APPEARANCE_BEVELLED+1
};

inline int app2App(EAppearance app, bool sel)
{
    return sel ? (int)CACHE_APPEARANCE_SELECTED : (int)app;
}

#define QTC_PIXMAP_DIMENSION 10

static int     double2int(double d) { return (int)(d*100); }
static QString createKey(int size, QRgb color, bool horiz, bool increase=false, int app=0,
                         EWidget w=WIDGET_OTHER, double shadeTop=0.0, double shadeBot=0.0)
{
    QString key;

    if(WIDGET_DEF_BUTTON==w && !IS_GLASS(app)) // Glass uses different shading for def button...
        w=WIDGET_STD_BUTTON;

    QTextOStream(&key) << size << color << horiz << increase << app << (int)w
                       << ' ' << double2int(shadeTop) << ' ' << double2int(shadeBot);

    return key;
}

static QString createKey(QRgb color)
{
    QString key;

    QTextOStream(&key) << 'p' << color;

    return key;
}

static QString createKey(QRgb color, EPixmap p)
{
    QString key;

    QTextOStream(&key) << 'P' << color << p;

    return key;
}

static void readPal(QString &line, QPalette::ColorGroup grp, QPalette &pal)
{
    QStringList cols(QStringList::split(", ", line.mid(line.find("=#")+1)));

    if(17==cols.count())
    {
        QStringList::ConstIterator it(cols.begin()),
                                   end(cols.end());
        QColorGroup                group;

        for(int i=0; it!=end, i<16; ++it, ++i)
        {
            QColor col;

            setRgb(&col, (*it).latin1());
            group.setColor((QColorGroup::ColorRole)i, col);
        }

        switch(grp)
        {
            case QPalette::Active:
                pal.setActive(group);
                break;
            case QPalette::Disabled:
                pal.setDisabled(group);
                break;
            case QPalette::Inactive:
                pal.setInactive(group);
            default:
                break;
        }
    }
}

static bool readQt4(QFile &f, QPalette &pal, QFont &font)
{
    bool inSect(false),
         gotPal(false),
         gotFont(false);

    if(f.open(IO_ReadOnly))
    {
        QTextStream in(&f);

        while (!in.atEnd())
        {
            QString line(in.readLine());

            if(inSect)
            {
                gotPal=true;
                if(0==line.find("Palette\\active=#"))
                {
                    readPal(line, QPalette::Active, pal);
                    gotPal=true;
                }
                else if(0==line.find("Palette\\disabled=#"))
                {
                    readPal(line, QPalette::Disabled, pal);
                    gotPal=true;
                }
                else if(0==line.find("Palette\\inactive=#"))
                {
                    readPal(line, QPalette::Inactive, pal);
                    gotPal=true;
                }
                else if(0==line.find("font=\""))
                    gotFont=font.fromString(line.mid(6, line.findRev('\"')-6));
                else if (0==line.find('['))
                    break;
            }
            else if(0==line.find("[Qt]"))
                inSect=true;
        }
        f.close();
    }

    return gotPal && gotFont;
}

static bool useQt4Settings()
{
    static const char *vers=getenv("KDE_SESSION_VERSION");

    return vers && atoi(vers)>=4;
}

static bool readQt4(QPalette &pal, QFont &font)
{
    if(useQt4Settings())
    {
        QFile file(xdgConfigFolder()+QString("Trolltech.conf"));

        if(file.exists())
            return readQt4(file, pal, font);
    }
    return false;
}

static bool isCheckBoxOfGroupBox(const QObject *w)
{
    return w && w->parent() &&
           ::qt_cast<QCheckBox *>(w) && ::qt_cast<QGroupBox *>(w->parent()) &&
           !qstrcmp(w->name(), "qt_groupbox_checkbox");
}

//
// OO.o 2.x checks to see whether the used theme "inherits" from HighContrastStyle,
// if so it uses the highlightedText color to draw highlighted menubar and popup menu
// items. Otherwise it uses the standard color. So faking this inheritance, solves this
// problem.
HighContrastStyle::HighContrastStyle()
                 : KStyle(AllowMenuTransparency, WindowsStyleScrollBar)
{
}

QtCurveStyle::QtCurveStyle()
            : itsSliderCols(NULL),
              itsDefBtnCols(NULL),
              itsMouseOverCols(NULL),
              itsSidebarButtonsCols(NULL),
              itsThemedApp(APP_OTHER),
              itsPixmapCache(150000, 499),
#if KDE_VERSION >= 0x30200
              itsIsTransKicker(false),
#endif
              itsHover(HOVER_NONE),
              itsOldPos(-1, -1),
              itsFormMode(false),
              itsHoverWidget(NULL),
              itsHoverSect(QTC_NO_SECT),
              itsHoverTab(NULL),
              itsMactorPal(NULL),
              itsActive(true),
              itsIsSpecialHover(false)
{
    defaultSettings(&opts);
    readConfig(NULL, &opts, &opts);
    opts.contrast=QSettings().readNumEntry("/Qt/KDE/contrast", 7);
    if(opts.contrast<0 || opts.contrast>10)
        opts.contrast=7;
    itsPixmapCache.setAutoDelete(true);

    if ((SHADE_CUSTOM==opts.shadeMenubars || SHADE_BLEND_SELECTED==opts.shadeMenubars) &&
        "soffice.bin"==QString(qApp->argv()[0]) && TOO_DARK(SHADE_CUSTOM==opts.shadeMenubars
                                                       ? opts.customMenubarsColor
                                                       : itsMenuitemCols[ORIGINAL_SHADE]))
        opts.shadeMenubars=SHADE_DARKEN;

    shadeColors(QApplication::palette().active().highlight(), itsMenuitemCols);
    shadeColors(QApplication::palette().active().background(), itsBackgroundCols);
    shadeColors(QApplication::palette().active().button(), itsButtonCols);
    if(SHADE_SELECTED==opts.shadeSliders)
        itsSliderCols=itsMenuitemCols;
    else if(SHADE_NONE!=opts.shadeSliders)
    {
        itsSliderCols=new QColor [TOTAL_SHADES+1];
        shadeColors(SHADE_BLEND_SELECTED==opts.shadeSliders
                        ? midColor(itsMenuitemCols[ORIGINAL_SHADE],
                                   itsButtonCols[ORIGINAL_SHADE])
                        : opts.customSlidersColor,
                    itsSliderCols);
    }

    if(IND_COLORED==opts.defBtnIndicator)
        if(SHADE_BLEND_SELECTED==opts.shadeSliders)
            itsDefBtnCols=itsSliderCols;
        else
        {
            itsDefBtnCols=new QColor [TOTAL_SHADES+1];
            shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                                 itsButtonCols[ORIGINAL_SHADE]), itsDefBtnCols);
        }

    if(opts.coloredMouseOver)
        if(itsDefBtnCols)
            itsMouseOverCols=itsDefBtnCols;
        else
        {
            itsMouseOverCols=new QColor [TOTAL_SHADES+1];
            shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                                 itsButtonCols[ORIGINAL_SHADE]), itsMouseOverCols);
        }

    setMenuColors(QApplication::palette().active());

    if(opts.lighterPopupMenuBgnd)
        itsLighterPopupMenuBgndCol=shade(itsBackgroundCols[ORIGINAL_SHADE],
                                         POPUPMENU_LIGHT_FACTOR);

    switch(opts.shadeCheckRadio)
    {
        default:
            itsCheckRadioCol=QApplication::palette().active().text();
            break;
        case SHADE_BLEND_SELECTED:
        case SHADE_SELECTED:
            itsCheckRadioCol=QApplication::palette().active().highlight();
            break;
        case SHADE_CUSTOM:
            itsCheckRadioCol=opts.customCheckRadioColor;
    }

    if (opts.animatedProgress)
    {
        itsAnimationTimer = new QTimer(this);
        connect(itsAnimationTimer, SIGNAL(timeout()), this, SLOT(updateProgressPos()));
    }

    setSbType();

    // Set palette from Qt4 settings. NOTE: This will only affect Qt-only apps. KApplication, sets the applications
    // palette after style is constructed...
    QPalette pal;
    QFont    font;

    if(readQt4(pal, font))
    {
        QApplication::setPalette(pal);
        QApplication::setFont(font);
    }
}

QtCurveStyle::~QtCurveStyle()
{
    if(itsSidebarButtonsCols!=itsSliderCols &&
       itsSidebarButtonsCols!=itsDefBtnCols)
        delete [] itsSidebarButtonsCols;
    if(itsMouseOverCols && itsMouseOverCols!=itsDefBtnCols &&
       itsMouseOverCols!=itsSliderCols)
        delete [] itsMouseOverCols;
    if(itsDefBtnCols && itsDefBtnCols!=itsSliderCols)
        delete [] itsDefBtnCols;
    if(itsSliderCols && itsSliderCols!=itsMenuitemCols)
        delete [] itsSliderCols;
    delete itsMactorPal;
}

static QString getFile(const QString &f)
{
    QString d(f);

    int slashPos(d.findRev('/'));

    if(slashPos!=-1)
        d.remove(0, slashPos+1);

    return d;
}

void QtCurveStyle::polish(QApplication *app)
{
    QString appName(getFile(app->argv()[0]));

    if ("kicker"==appName || "appletproxy"==appName)
    {
        itsThemedApp=APP_KICKER;
#if KDE_VERSION >= 0x30200
        itsIsTransKicker=kickerIsTrans();
#endif
    }
    else if ("kontact"==appName)
        itsThemedApp=APP_KONTACT;
    else if ("konqueror"==appName)
        itsThemedApp=APP_KONQUEROR;
    else if ("kpresenter"==appName)
        itsThemedApp=APP_KPRESENTER;
    else if ("soffice.bin"==appName)
    {
        itsThemedApp=APP_OPENOFFICE;
        opts.framelessGroupBoxes=false;
    }
    else if ("kdefilepicker"==appName)
        itsThemedApp=APP_SKIP_TASKBAR;
    else if ("kprinter"==appName)
        itsThemedApp=APP_KPRINTER;
    else if ("kdialog"==appName)
        itsThemedApp=APP_KDIALOG;
    else if ("kdialogd"==appName)
        itsThemedApp=APP_KDIALOGD;
    else if ("tora"==appName)
        itsThemedApp=APP_TORA;
    else if ("opera"==appName)
        itsThemedApp=APP_OPERA;
    else if ("korn"==appName)
    {
        itsThemedApp=APP_KORN;
#if KDE_VERSION >= 0x30200
        itsIsTransKicker=kickerIsTrans();
#endif
    }
    else if ("mactor"==appName)
    {
        if(!itsMactorPal)
            itsMactorPal=new QPalette(QApplication::palette());
        itsThemedApp=APP_MACTOR;
    }
    else
        itsThemedApp=APP_OTHER;

    if(APP_OPENOFFICE==itsThemedApp)
    {
        if(opts.scrollbarType==SCROLLBAR_NEXT)
            opts.scrollbarType=SCROLLBAR_KDE;
        else if(opts.scrollbarType==SCROLLBAR_NONE)
            opts.scrollbarType=SCROLLBAR_WINDOWS;
        setSbType();
    }
}

void QtCurveStyle::polish(QPalette &pal)
{
    if(APP_MACTOR==itsThemedApp && itsMactorPal &&
       pal.active().background()!=itsMactorPal->active().background())
        return;

    int  c(QSettings().readNumEntry("/Qt/KDE/contrast", 7));
    bool newContrast(false);

    if(c<0 || c>10)
        c=7;

    if(c!=opts.contrast)
    {
        opts.contrast=c;
        newContrast=true;
    }

    bool newMenu(newContrast ||
                 itsMenuitemCols[ORIGINAL_SHADE]!=QApplication::palette().active().highlight()),
         newGray(newContrast ||
                 itsBackgroundCols[ORIGINAL_SHADE]!=QApplication::palette().active().background()),
         newButton(newContrast ||
                   itsButtonCols[ORIGINAL_SHADE]!=QApplication::palette().active().button()),
         newSlider(itsSliderCols && SHADE_BLEND_SELECTED==opts.shadeSliders &&
                   (newContrast || newButton || newMenu)),
         newDefBtn(itsDefBtnCols && IND_COLORED==opts.defBtnIndicator &&
                   SHADE_BLEND_SELECTED!=opts.shadeSliders &&
                   (newContrast || newButton || newMenu)),
         newMouseOver(itsMouseOverCols && itsMouseOverCols!=itsDefBtnCols &&
                      itsMouseOverCols!=itsSliderCols &&
                     (newContrast || newButton || newMenu));

    if(newGray)
        shadeColors(QApplication::palette().active().background(), itsBackgroundCols);

    if(newButton)
        shadeColors(QApplication::palette().active().button(), itsButtonCols);

    if(newMenu)
        shadeColors(QApplication::palette().active().highlight(), itsMenuitemCols);

    setMenuColors(QApplication::palette().active());

    if(newSlider)
        shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                    itsButtonCols[ORIGINAL_SHADE]), itsSliderCols);

    if(newDefBtn)
        shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                    itsButtonCols[ORIGINAL_SHADE]), itsDefBtnCols);

    if(newMouseOver)
        shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                    itsButtonCols[ORIGINAL_SHADE]), itsMouseOverCols);

    if(itsSidebarButtonsCols && SHADE_BLEND_SELECTED!=opts.shadeSliders &&
       IND_COLORED!=opts.defBtnIndicator)
        shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE],
                   itsButtonCols[ORIGINAL_SHADE]), itsSidebarButtonsCols);

    if(opts.lighterPopupMenuBgnd && newGray)
        itsLighterPopupMenuBgndCol=shade(itsBackgroundCols[ORIGINAL_SHADE],
                                         POPUPMENU_LIGHT_FACTOR);

    const QColorGroup      &actGroup(pal.active()),
                           &inactGroup(pal.inactive()),
                           &disGroup(pal.disabled());
    const QColor           *use(backgroundColors(actGroup));
    QColorGroup            newAct(actGroup.foreground(), actGroup.button(), use[0], use[5],
                                  actGroup.mid(), actGroup.text(), actGroup.brightText(),
                                  actGroup.base(), actGroup.background());
    QColorGroup::ColorRole roles[]={QColorGroup::Midlight, QColorGroup::ButtonText,
                                    QColorGroup::Shadow, QColorGroup::Highlight,
                                    QColorGroup::HighlightedText,
                                    QColorGroup::NColorRoles };
    int                    r(0);

    for(r=0; roles[r]!=QColorGroup::NColorRoles; ++r)
        newAct.setColor(roles[r], actGroup.color(roles[r]));
    pal.setActive(newAct);

    use=backgroundColors(inactGroup);

    QColorGroup newInact(inactGroup.foreground(), inactGroup.button(), use[0], use[5],
                         inactGroup.mid(), inactGroup.text(), inactGroup.brightText(),
                         inactGroup.base(), inactGroup.background());

    for(r=0; roles[r]!=QColorGroup::NColorRoles; ++r)
        newInact.setColor(roles[r], inactGroup.color(roles[r]));
    pal.setInactive(newInact);

    use=backgroundColors(disGroup);

    QColorGroup newDis(disGroup.foreground(), disGroup.button(), use[0], use[5],
                       disGroup.mid(), disGroup.text(), disGroup.brightText(),
                       disGroup.base(), disGroup.background());

    for(r=0; roles[r]!=QColorGroup::NColorRoles; ++r)
        newDis.setColor(roles[r], disGroup.color(roles[r]));
    pal.setDisabled(newDis);

    switch(opts.shadeCheckRadio)
    {
        default:
            itsCheckRadioCol=QApplication::palette().active().text();
            break;
        case SHADE_SELECTED:
        case SHADE_BLEND_SELECTED:
            itsCheckRadioCol=QApplication::palette().active().highlight();
            break;
        case SHADE_CUSTOM:
            itsCheckRadioCol=opts.customCheckRadioColor;
    }

    if(itsMactorPal)
        *itsMactorPal=pal;
}

static const char * kdeToolbarWidget="kde toolbar widget";

void QtCurveStyle::polish(QWidget *widget)
{
    bool enableFilter(!equal(opts.highlightFactor, 1.0) || opts.coloredMouseOver);

    if(::isKhtmlFormWidget(widget))
    {
        itsKhtmlWidgets[widget]=true;
        connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(khtmlWidgetDestroyed(QObject *)));
    }

    if(widget->parentWidget() && ::qt_cast<QScrollView *>(widget) && ::qt_cast<QComboBox *>(widget->parentWidget()))
    {
        QPalette    pal(widget->palette());
#if 0
        QPalette    orig(pal);
#endif
        QColorGroup act(pal.active());

#if 0
        if(opts.gtkComboMenus)
            act.setColor(QColorGroup::Base, opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol : itsBackgroundCols[ORIGINAL_SHADE]);
        act.setColor(QColorGroup::Background, opts.gtkComboMenus
                                            ? opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol : itsBackgroundCols[ORIGINAL_SHADE]
                                            : QApplication::palette().active().base());
#endif
        act.setColor(QColorGroup::Foreground, itsBackgroundCols[QT_STD_BORDER]);

        pal.setActive(act);
        widget->setPalette(pal);
#if 0
        //((QScrollView *)widget)->setMargin(1);

        const QObjectList *children(widget->children());

        if(children)
        {
            QObjectList::Iterator it(children->begin()),
                                  end(children->end());

            for(; it!=end; ++it)
                if(::qt_cast<QWidget *>(*it))
                    ((QWidget *)(*it))->setPalette(orig);
        }
#endif
    }

    if (APP_MACTOR==itsThemedApp && itsMactorPal && !widget->inherits("QTipLabel"))
        widget->setPalette(*itsMactorPal);

    // Get rid of Kontact's frame...
    if(APP_KONTACT==itsThemedApp && ::qt_cast<QHBox *>(widget) && widget->parentWidget() &&
       0==qstrcmp(widget->parentWidget()->className(), "Kontact::MainWindow"))
        ((QHBox *)widget)->setLineWidth(0);

    if (::qt_cast<QRadioButton *>(widget) || ::qt_cast<QCheckBox *>(widget))
    {
        bool framelessGroupBoxCheckBox=(opts.framelessGroupBoxes && isCheckBoxOfGroupBox(widget));

        if(framelessGroupBoxCheckBox || enableFilter)
        {
#if QT_VERSION >= 0x030200
            if(!isFormWidget(widget))
                widget->setMouseTracking(true);
#endif
            if(framelessGroupBoxCheckBox)
            {
                QFont fnt(widget->font());

                fnt.setBold(true);
                widget->setFont(fnt);
            }
            widget->installEventFilter(this);
        }
    }
    else if (::qt_cast<QHeader *>(widget) || ::qt_cast<QTabBar *>(widget) || ::qt_cast<QSpinWidget *>(widget)/* ||
             ::qt_cast<QDateTimeEditBase*>(widget)*/)
    {
        if(enableFilter)
        {
            widget->setMouseTracking(true);
            widget->installEventFilter(this);
        }
    }
    else if (::qt_cast<QToolButton *>(widget))
    {
        if(NoBackground!=widget->backgroundMode()) //  && onToolBar(widget))
            widget->setBackgroundMode(PaletteBackground);
        if(enableFilter)
        {
            widget->installEventFilter(this);
#if KDE_VERSION >= 0x30400 && KDE_VERSION < 0x30500
            widget->setMouseTracking(true);
#endif
        }
    }
    else if (::qt_cast<QButton *>(widget) || widget->inherits("QToolBarExtensionWidget"))
    {
        /*if(onToolBar(widget))
            widget->setBackgroundMode(NoBackground);
        else*/ if(NoBackground!=widget->backgroundMode()) //  && onToolBar(widget))
            widget->setBackgroundMode(PaletteBackground);
        if(enableFilter)
            widget->installEventFilter(this);
    }
    else if (::qt_cast<QComboBox *>(widget))
    {
        if(NoBackground!=widget->backgroundMode()) //  && onToolBar(widget))
            widget->setBackgroundMode(PaletteBackground);
        widget->installEventFilter(this);

        if(QTC_DO_EFFECT && onToolBar(widget))
            widget->setName(kdeToolbarWidget);

        if(enableFilter)
            widget->setMouseTracking(true);

        if(((QComboBox *)widget)->listBox())
            ((QComboBox *)widget)->listBox()->installEventFilter(this);
    }
    else if(::qt_cast<QMenuBar *>(widget) || ::qt_cast<QToolBar *>(widget))
    {
        if(NoBackground!=widget->backgroundMode())
            widget->setBackgroundMode(PaletteBackground);
        if(SHADE_NONE!=opts.shadeMenubars)
            widget->installEventFilter(this);
    }
    else if(::qt_cast<QPopupMenu *>(widget))
            widget->setBackgroundMode(NoBackground); // PaletteBackground);
    else if (widget->inherits("KToolBarSeparator") ||
             (widget->inherits("KListViewSearchLineWidget") &&
              widget->parent() && ::qt_cast<QToolBar *>(widget->parent())))
    {
        widget->setName(kdeToolbarWidget);
        widget->setBackgroundMode(NoBackground);
        widget->installEventFilter(this);
    }
    else if (::qt_cast<QScrollBar *>(widget))
    {
        if(enableFilter)
        {
            widget->setMouseTracking(true);
            widget->installEventFilter(this);
        }
        //widget->setBackgroundMode(NoBackground);
    }
    else if (::qt_cast<QSlider *>(widget))
    {
        if(enableFilter)
            widget->installEventFilter(this);
        if(widget->parent() && ::qt_cast<QToolBar *>(widget->parent()))
        {
            widget->setName(kdeToolbarWidget);
            widget->setBackgroundMode(NoBackground);  // We paint whole background.
        }

        // This bit stolen form polyester...
        connect(widget, SIGNAL(sliderMoved(int)), this, SLOT(sliderThumbMoved(int)));
        connect(widget, SIGNAL(valueChanged(int)), this, SLOT(sliderThumbMoved(int)));
    }
    else if (::qt_cast<QLineEdit *>(widget) || ::qt_cast<QTextEdit *>(widget))
    {
        widget->installEventFilter(this);
        if(onToolBar(widget))
            widget->setName(kdeToolbarWidget);
        if(widget && widget->parentWidget() &&
           widget->inherits("KLineEdit") && widget->parentWidget()->inherits("KIO::DefaultProgress") &&
           ::qt_cast<QFrame *>(widget))
            ((QFrame *)widget)->setLineWidth(0);
    }
    else if (widget->inherits("QSplitterHandle") || widget->inherits("QDockWindowHandle"))
    {
        if(enableFilter)
            widget->installEventFilter(this);
    }
    else if (0==qstrcmp(widget->name(), kdeToolbarWidget))
    {
        if(!widget->parent() ||
           0!=qstrcmp(static_cast<QWidget *>(widget->parent())->className(),
                                             "KListViewSearchLineWidget") ||
           onToolBar(widget))
        {
            widget->installEventFilter(this);
            widget->setBackgroundMode(NoBackground);  // We paint whole background.
        }
    }

    if (widget->parentWidget() && ::qt_cast<QMenuBar *>(widget->parentWidget()) && !qstrcmp(widget->className(), "QFrame"))
    {
        widget->installEventFilter(this);
        widget->setBackgroundMode(NoBackground);  // We paint whole background.
    }
    else if (Qt::X11ParentRelative!=widget->backgroundMode() &&
              (::qt_cast<QLabel *>(widget) || ::qt_cast<QHBox *>(widget) ||
              ::qt_cast<QVBox *>(widget)) &&
              widget->parent() &&
              ( 0==qstrcmp(static_cast<QWidget *>(widget->parent())->className(),
                           "MainWindow") || onToolBar(widget)))
    {
        widget->setName(kdeToolbarWidget);
        widget->installEventFilter(this);
        widget->setBackgroundMode(NoBackground);  // We paint the whole background.
    }
    else if(opts.animatedProgress && ::qt_cast<QProgressBar*>(widget))
    {
        widget->installEventFilter(this);
        itsProgAnimWidgets[widget] = 0;
        connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(progressBarDestroyed(QObject *)));
        if (!itsAnimationTimer->isActive())
            itsAnimationTimer->start(PROGRESS_ANIMATION, false);
    }
#ifdef QTC_HIGHLIGHT_SCROLVIEWS
    else if(::qt_cast<QScrollView*>(widget))
        widget->installEventFilter(this);
#endif
    else if(!qstrcmp(widget->className(), "KonqFrameStatusBar"))
    {
        // This disables the white background of the KonquerorFrameStatusBar.
        // When the green led is shown the background is set to
        // applications cg.midlight() so we override it to standard background.
        // Thanks Comix! (because this was ugly from day one!)
        // NOTE: Check if we can set it earlier (before painting), cause
        // on slow machines we can see the repainting of the bar (from white to background...)
        QPalette pal(QApplication::palette());

        pal.setColor(QColorGroup::Midlight, pal.active().background());
        QApplication::setPalette(pal);
    }
    else if(widget->inherits("KTabCtl") || (opts.framelessGroupBoxes && ::qt_cast<QGroupBox *>(widget)))
        widget->installEventFilter(this);
    else if(opts.fixParentlessDialogs && ::qt_cast<QDialog *>(widget))
    {
        QDialog *dlg=(QDialog *)widget;

        // The parent->isShown is needed for KWord. It's insert picure file dialog is a child of
        // the insert picture dialog - but the file dialog is shown *before* the picture dialog!
        if( (QTC_SKIP_TASKBAR && !dlg->parentWidget()) ||
            ( (!dlg->parentWidget() || !dlg->parentWidget()->isShown())// &&
              /*(dlg->isModal() || ::qt_cast<QProgressDialog *>(widget))*/) )
            widget->installEventFilter(this);
    }

    if(opts.fixParentlessDialogs && (APP_KPRINTER==itsThemedApp || APP_KDIALOG==itsThemedApp ||
                                     APP_KDIALOGD==itsThemedApp))
    {
        QString cap(widget->caption());
        int     index=-1;

        // Remove horrible "Open - KDialog" titles...
        if( cap.length() &&
            ( (APP_KPRINTER==itsThemedApp && (-1!=(index=cap.find(" - KPrinter"))) &&
               (index+11)==(int)cap.length()) ||
              (APP_KDIALOG==itsThemedApp && (-1!=(index=cap.find(" - KDialog"))) &&
               (index+10)==(int)cap.length()) ||
              (APP_KDIALOGD==itsThemedApp && (-1!=(index=cap.find(" - KDialog Daemon"))) &&
               (index+17)==(int)cap.length())) )
            widget->QWidget::setCaption(cap.left(index));
    }

    KStyle::polish(widget);
}

void QtCurveStyle::unPolish(QWidget *widget)
{
    if(isFormWidget(widget))
        itsKhtmlWidgets.remove(widget);

    if (::qt_cast<QRadioButton *>(widget) || ::qt_cast<QCheckBox *>(widget))
    {
#if QT_VERSION >= 0x030200
        widget->setMouseTracking(false);
#endif
        widget->removeEventFilter(this);
    }
    else if (::qt_cast<QHeader *>(widget) || ::qt_cast<QTabBar *>(widget) || ::qt_cast<QSpinWidget *>(widget) /*||
             ::qt_cast<QDateTimeEditBase*>(widget)*/)
    {
        widget->setMouseTracking(false);
        widget->removeEventFilter(this);
    }
    else if (::qt_cast<QButton *>(widget) || widget->inherits("QToolBarExtensionWidget"))
    {
        if(NoBackground!=widget->backgroundMode()) //  && onToolBar(widget))
            widget->setBackgroundMode(PaletteButton);
        widget->removeEventFilter(this);
    }
    else if (::qt_cast<QToolButton *>(widget))
    {
        if(NoBackground!=widget->backgroundMode()) //  && onToolBar(widget))
            widget->setBackgroundMode(PaletteButton);
        widget->removeEventFilter(this);
#if KDE_VERSION >= 0x30400 && KDE_VERSION < 0x30500
        widget->setMouseTracking(false);
#endif
    }
    else if (::qt_cast<QComboBox *>(widget))
    {
        if(NoBackground!=widget->backgroundMode()) //  && onToolBar(widget))
            widget->setBackgroundMode(PaletteButton);
        widget->removeEventFilter(this);
        widget->setMouseTracking(false);
        if(((QComboBox *)widget)->listBox())
            ((QComboBox *)widget)->listBox()->removeEventFilter(this);
    }
    else if (::qt_cast<QToolBar *>(widget) || ::qt_cast<QPopupMenu *>(widget))
    {
        if(NoBackground!=widget->backgroundMode())
           widget->setBackgroundMode(PaletteBackground);
    }
    else if (::qt_cast<QMenuBar *>(widget))
    {
        if(NoBackground!=widget->backgroundMode())
           widget->setBackgroundMode(PaletteBackground);
        if(SHADE_NONE!=opts.shadeMenubars)
            widget->removeEventFilter(this);
    }
    else if (widget->inherits("KToolBarSeparator"))
    {
        widget->setBackgroundMode(PaletteBackground);
        widget->removeEventFilter(this);
    }
    else if (::qt_cast<QScrollBar *>(widget))
    {
        widget->setMouseTracking(false);
        widget->removeEventFilter(this);
        widget->setBackgroundMode(PaletteButton);
    }
    else if (::qt_cast<QSlider *>(widget))
    {
        widget->removeEventFilter(this);
        if(widget->parent() && ::qt_cast<QToolBar *>(widget->parent()))
            widget->setBackgroundMode(PaletteBackground);
    }
    else if (::qt_cast<QLineEdit*>(widget) || ::qt_cast<QTextEdit*>(widget))
        widget->removeEventFilter(this);
    else if (widget->inherits("QSplitterHandle") || widget->inherits("QDockWindowHandle"))
        widget->removeEventFilter(this);
    else if (::qt_cast<QProgressBar*>(widget))
    {
        itsProgAnimWidgets.remove(widget);
        widget->removeEventFilter(this);
    }
#ifdef QTC_HIGHLIGHT_SCROLVIEWS
    else if(::qt_cast<QScrollView*>(widget))
        widget->removeEventFilter(this);
#endif
    else if(0==qstrcmp(widget->name(), kdeToolbarWidget))
    {
        widget->removeEventFilter(this);
        widget->setBackgroundMode(PaletteBackground);
    }
    if (widget->parentWidget() && ::qt_cast<QMenuBar *>(widget->parentWidget()) && !qstrcmp(widget->className(), "QFrame"))
    {
        widget->removeEventFilter(this);
        widget->setBackgroundMode(PaletteBackground);  // We paint whole background.
    }
    else if(widget->inherits("KTabCtl") || (opts.framelessGroupBoxes && ::qt_cast<QGroupBox *>(widget)))
        widget->removeEventFilter(this);
    else if(opts.fixParentlessDialogs && ::qt_cast<QDialog *>(widget))
        widget->removeEventFilter(this);

    KStyle::unPolish(widget);
}

static void sendXEvent(QDialog *dlg, const char *msg)
{
    static Atom msgTypeAtom = XInternAtom(qt_xdisplay(), "_NET_WM_STATE", False);

    XEvent xev;
    Atom   atom=XInternAtom(qt_xdisplay(), msg, False);

    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.window = dlg->winId();
    xev.xclient.message_type = msgTypeAtom;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = atom;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    XSendEvent(qt_xdisplay(), qt_xrootwin(), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

bool QtCurveStyle::appIsNotEmbedded(QDialog *dlg)
{
    Window win;

    if(!XGetTransientForHint(qt_xdisplay(), dlg->winId(), &win) || (int)win < 1000)
        return true;

    // OK, dialog has been set transient, so there is no need for this event filter anymore :-)
    dlg->removeEventFilter(this);
    return false;
}

bool QtCurveStyle::eventFilter(QObject *object, QEvent *event)
{
    if(itsHoverWidget && object==itsHoverWidget && (QEvent::Destroy==event->type() || QEvent::Hide==event->type()))
    {
        itsOldPos.setX(-1);
        itsOldPos.setY(-1);
        itsHoverWidget=NULL;
        itsIsSpecialHover=false;
        itsHoverSect=QTC_NO_SECT;
        itsHover=HOVER_NONE;
    }

    if(object->parent() && 0==qstrcmp(object->name(), kdeToolbarWidget))
    {
        // Draw background for custom widgets in the toolbar that have specified a
        // "kde toolbar widget" name.
        if(QEvent::Paint==event->type())
        {
            QWidget *widget(static_cast<QWidget*>(object)),
                    *parent(static_cast<QWidget*>(object->parent()));

            if(IS_FLAT(opts.toolbarAppearance))
                QPainter(widget).fillRect(widget->rect(), parent->colorGroup().background());
            else
            {
                int y_offset(widget->y());

                while(parent && parent->parent() && 0==qstrcmp(parent->name(), kdeToolbarWidget))
                {
                    y_offset += parent->y();
                    parent = static_cast<QWidget*>(parent->parent());
                }

                QRect    r(widget->rect()),
                         pr(parent->rect());
                bool     horiz=pr.width() > pr.height();
                QPainter p(widget);
                QToolBar *tb(::qt_cast<QToolBar*>(parent));

                // If parent is a QToolbar use its orientation, else just base on width>height.
                if (tb)
                    horiz = Qt::Horizontal==tb->orientation();

                QRect bgndRect(r.x(), r.y()-y_offset, r.width(), pr.height());

                if(!IS_FLAT(opts.toolbarAppearance))
                    if(horiz)
                        bgndRect.addCoords(0, -1, 0, 1);
                    else
                        bgndRect.addCoords(-1, 0, 1, 0);

                drawMenuOrToolBarBackground(&p, bgndRect, parent->colorGroup(), false, horiz);
            }

            if(!::qt_cast<QLineEdit*>(object) && !::qt_cast<QTextEdit*>(object) &&
               !(QTC_DO_EFFECT && ::qt_cast<QComboBox*>(object)))
                return false;   // Now draw the contents
        }
    }
    else if (opts.framelessGroupBoxes && QEvent::Move==event->type() && isCheckBoxOfGroupBox(object))
    {
        QCheckBox *cb=static_cast<QCheckBox *>(object);
        QRect     r(cb->geometry());

        cb->removeEventFilter(this);
        if(QApplication::reverseLayout())
            r.setWidth(r.width()+8);
        else
            r.setX(0);
        cb->setGeometry(r);
        cb->installEventFilter(this);
        return false;
    }
    else if (QEvent::Paint==event->type())
    {
        if (opts.framelessGroupBoxes && ::qt_cast<QGroupBox *>(object))
        {
            QGroupBox *box=static_cast<QGroupBox*>(object);

            if (!box->isCheckable())
            {
                QString title(box->title());

                if(title.length())
                {
                    int          left,
                                 right,
                                 top,
                                 bottom,
                                 width,
                                 height;
                    QPainter     p(box);
                    QFontMetrics fm(p.fontMetrics());
                    QRect        r(box->rect());
                    int          th(fm.height()+2);
                    QFont        f(p.font());

                    r.rect(&left, &top, &width, &height);
                    r.coords(&left, &top, &right, &bottom);
                    f.setBold(true);
                    p.setPen(box->colorGroup().foreground());
                    p.setFont(f);
                    p.drawText(QRect(left, top, width, th),
                               (QApplication::reverseLayout() ? AlignRight : AlignLeft)|AlignVCenter|ShowPrefix|SingleLine, title);
                    return true;
                }
            }
        }
        else if (object->inherits("KToolBarSeparator"))
        {
            QFrame *frame(::qt_cast<QFrame *>(object));

            if(frame && QFrame::NoFrame!=frame->frameShape())
            {
                QPainter painter(frame);
                if (QFrame::VLine==frame->frameShape())
                    drawPrimitive(PE_DockWindowSeparator, &painter, frame->rect(),
                                  frame->colorGroup(), Style_Horizontal);
                else if (QFrame::HLine==frame->frameShape())
                    drawPrimitive(PE_DockWindowSeparator, &painter, frame->rect(),
                                  frame->colorGroup());
                else
                    return false;
                return true; // been drawn!
            }
        }
        else if(object->inherits("KTabCtl") && ::qt_cast<QWidget*>(object))
        {
            QWidget  *widget((QWidget*)object);
            QObject  *child(object->child("_tabbar"));
            QTabBar  *tb(child ? ::qt_cast<QTabBar *>(child) : NULL);
            QPainter painter(widget);
            QRect    r(widget->rect());
            int      tbHeight(tb ? tb->height()-1 : 28);

            if(tb && (QTabBar::RoundedBelow == tb->shape() ||
                      QTabBar::TriangularBelow == tb->shape()))
                r.addCoords(0, 0, 0, -tbHeight);
            else
                r.addCoords(0, tbHeight, 0, 0);
            drawPrimitive(PE_PanelTabWidget, &painter, r, widget->colorGroup(),
                          Style_Horizontal|Style_Enabled);
            return true;
        }
    }

    // Fix mdi buttons in menubars...
    if(::qt_cast<QWidget*>(object) && ((QWidget *)object)->parentWidget() &&
       ::qt_cast<QMenuBar*>(((QWidget *)object)->parentWidget()))
    {
        bool drawMenubar=false;

        switch(event->type())
        {
            case QEvent::Paint:
                drawMenubar=true;
                break;
            case QEvent::WindowActivate:
                itsActive=true;
                drawMenubar=opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars;
                break;
            case QEvent::WindowDeactivate:
                itsActive=false;
                drawMenubar=opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars;
            default:
                break;
        }

        if(drawMenubar)
        {

            QWidget  *widget(static_cast<QWidget*>(object)),
                     *parent(static_cast<QWidget*>(object->parent()));
            QRect    r(widget->rect());
            QPainter p(widget);
            int      y_offset(widget->y()+parent->y());

            r.setY(r.y()-y_offset);
            r.setHeight(parent->rect().height());

            drawMenuOrToolBarBackground(&p, r, parent->colorGroup(), true, true);
            return true;
        }
    }

    // Taken from plastik...
    // focus highlight
    if (::qt_cast<QLineEdit*>(object) || ::qt_cast<QTextEdit*>(object)/* || ::qt_cast<QDateTimeEditBase*>(object)*/)
    {
        if((QEvent::FocusIn==event->type() || QEvent::FocusOut==event->type()))
        {
            QWidget *widget(static_cast<QWidget*>(object));

            if (::qt_cast<QSpinWidget*>(widget->parentWidget()))
            {
                widget->parentWidget()->repaint(false);
                return false;
            }

            widget->repaint(false);
        }
        return false;
    }

    if(opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars &&
       ::qt_cast<QMenuBar *>(object))
        switch(event->type())
        {
            case QEvent::WindowActivate:
                itsActive=true;
                ((QWidget *)object)->repaint(false);
                return false;
            case QEvent::WindowDeactivate:
                itsActive=false;
                ((QWidget *)object)->repaint(false);
                return false;
            default:
                break;
        }

    if(opts.fixParentlessDialogs && ::qt_cast<QDialog *>(object))
    {
        QDialog *dlg=(QDialog *)object;

        switch(event->type())
        {
            case QEvent::ShowMinimized:
                if(QTC_SKIP_TASKBAR && appIsNotEmbedded(dlg))
                {
                    // Ugly hack :-( Cant seem to get KWin to remove the minimize button. So when
                    // the dialog gets minimized, restore.
                    dlg->setWindowState(dlg->windowState() & ~WindowMinimized | WindowActive);
                    return true;
                }
                break;
            case QEvent::WindowActivate:
                if(QTC_SKIP_TASKBAR && appIsNotEmbedded(dlg))
                {
                    // OO.o's filepicker is a spawned process - but is not set transient :-(
                    // --plus no reliable way of finding which widget to make it transient for...
                    sendXEvent(dlg, "_NET_WM_STATE_SKIP_PAGER");
                    sendXEvent(dlg, "_NET_WM_STATE_SKIP_TASKBAR");
                    sendXEvent(dlg, "_NET_WM_STATE_ABOVE");
                    sendXEvent(dlg, "_NET_WM_STATE_STAYS_ON_TOP");
                    //setActions(dlg);
                }
                break;
            case QEvent::Show:
                // The parent->isShown is needed for KWord. It's insert picure file dialog is a
                // child of the insert picture dialog - but the file dialog is shown *before* the
                // picture dialog!
                if((!dlg->parentWidget() || !dlg->parentWidget()->isShown())) // &&
                   //(dlg->isModal() || ::qt_cast<QProgressDialog *>(object)))
                {
                    QWidget *activeWindow=qApp->activeWindow();

                    if(activeWindow)
                    {
                        XWindowAttributes attr;
                        int               rx, ry;
                        Window            win;

                        if(!XGetTransientForHint(qt_xdisplay(), dlg->winId(), &win) ||
                           win!=activeWindow->winId())
                        {
                            XSetTransientForHint(qt_xdisplay(), dlg->winId(),
                                                 activeWindow->winId());

                            if(XGetWindowAttributes(qt_xdisplay(), activeWindow->winId(), &attr))
                            {
                                XTranslateCoordinates(qt_xdisplay(), activeWindow->winId(),
                                                      attr.root, -attr.border_width, -16,
                                                      &rx, &ry, &win);

                                rx=(rx+(attr.width/2))-(dlg->width()/2);
                                if(rx<0)
                                    rx=0;
                                ry=(ry+(attr.height/2))-(dlg->height()/2);
                                if(ry<0)
                                    ry=0;
                                dlg->move(rx, ry);
                                if(!dlg->isModal())
                                    dlg->setModal(true);
                            }
                        }
                    }
                }
            default:
                break;
        }
        return false;
    }

    // Track show events for progress bars
    if (opts.animatedProgress && ::qt_cast<QProgressBar*>(object))
    {
        if(QEvent::Show==event->type() && !itsAnimationTimer->isActive())
            itsAnimationTimer->start(PROGRESS_ANIMATION, false);
        return false;
    }

    switch(event->type())
    {
#ifdef QTC_HIGHLIGHT_SCROLVIEWS
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            if(object->isWidgetType() && ::qt_cast<QScrollView*>(object))
                ((QWidget *)object)->repaint(false);
            break;
#endif
        case QEvent::Hide:
        case QEvent::Show:
            if(::qt_cast<QListBox *>(object) &&
              (((QListBox *)object)->parentWidget() &&
               ::qt_cast<QComboBox *>(((QListBox *)object)->parentWidget())))
                ((QComboBox *)(((QListBox *)object)->parentWidget()))->repaint(false);
//             else if(::qt_cast<QFrame *>(object) &&
//                (QFrame::Box==((QFrame *)object)->frameShape() || QFrame::Panel==((QFrame *)object)->frameShape() ||
//                 QFrame::WinPanel==((QFrame *)object)->frameShape()))
//                 ((QFrame *)object)->setFrameShape(QFrame::StyledPanel);
            break;
        case QEvent::Enter:
            if(object->isWidgetType())
            {
                itsHoverWidget=(QWidget *)object;

                if(itsHoverWidget && itsHoverWidget->isEnabled())
                {
                    if(::qt_cast<QTabBar*>(object) && static_cast<QWidget*>(object)->isEnabled())
                    {
                        itsHoverTab=NULL;
                        itsHoverWidget->repaint(false);
                    }
                    else if(!itsHoverWidget->hasMouseTracking() ||
                            (itsFormMode=isFormWidget(itsHoverWidget)))
                    {
                        itsHoverWidget->repaint(false);
                        itsFormMode=false;
                    }
                }
                else
                    itsHoverWidget=NULL;

                if(itsHoverWidget && !itsIsSpecialHover &&
                    (
#if QT_VERSION >= 0x030200
                    ::qt_cast<QRadioButton *>(itsHoverWidget) ||
                    ::qt_cast<QCheckBox *>(itsHoverWidget) ||
#endif
                    ::qt_cast<QScrollBar *>(itsHoverWidget) ||
#if KDE_VERSION >= 0x30400 && KDE_VERSION < 0x30500
                    ::qt_cast<QToolButton *>(itsHoverWidget) ||

#endif
                    ::qt_cast<QHeader *>(itsHoverWidget) ||
                    ::qt_cast<QSpinWidget *>(itsHoverWidget) ||
                    ::qt_cast<QComboBox *>(itsHoverWidget) ||
                    ::qt_cast<QTabBar *>(itsHoverWidget)))

                    itsIsSpecialHover=true;
            }
            break;
        case QEvent::Leave:
            if(itsHoverWidget && object==itsHoverWidget)
            {
                itsOldPos.setX(-1);
                itsOldPos.setY(-1);
                itsHoverWidget=NULL;
                itsHoverSect=QTC_NO_SECT;
                itsHover=HOVER_NONE;
                itsHoverTab=NULL;
                ((QWidget *)object)->repaint(false);
            }
            break;
        case QEvent::MouseMove:  // Only occurs for widgets with mouse tracking enabled
        {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);

            if(me && itsHoverWidget && object->isWidgetType())
            {
                if(!me->pos().isNull() && itsIsSpecialHover && redrawHoverWidget(me->pos()))
                    itsHoverWidget->repaint(false);
                itsOldPos=me->pos();
            }
            break;
        }
        default:
            break;
    }

    return KStyle::eventFilter(object, event);
}

void QtCurveStyle::drawLightBevel(const QColor &bgnd, QPainter *p, const QRect &rOrig,
                                  const QColorGroup &cg, SFlags flags,
                                  int round, const QColor &fill, const QColor *custom,
                                  bool doBorder, bool doCorners, EWidget w) const
{
    EAppearance  app(widgetApp(w, &opts));
    QRect        r(rOrig),
                 br(r);
    bool         bevelledButton(WIDGET_BUTTON(w) && APPEARANCE_BEVELLED==app),
                 sunken(flags &(Style_Down|Style_On|Style_Sunken)),
                 lightBorder(QTC_DRAW_LIGHT_BORDER(sunken , w, app)),
                 doColouredMouseOver(doBorder &&
                                    opts.coloredMouseOver && flags&Style_MouseOver &&
                                    (!IS_SLIDER(w) || (WIDGET_SB_SLIDER==w && MO_PLASTIK==opts.coloredMouseOver)) &&
                                    (flags&QTC_CHECK_BUTTON || flags&QTC_TOGGLE_BUTTON || !sunken)),
                 plastikMouseOver(doColouredMouseOver && MO_PLASTIK==opts.coloredMouseOver),
                 colouredMouseOver(doColouredMouseOver && MO_COLORED==opts.coloredMouseOver),
                 doEtch(!itsFormMode && doBorder && ETCH_WIDGET(w) && !(flags&QTC_CHECK_BUTTON) &&
                        QTC_DO_EFFECT),
                 horiz(flags&Style_Horizontal);
    int          dark(bevelledButton ? 2 : 4),
                 c1(sunken ? dark : 0);
    const QColor *cols(custom ? custom : itsBackgroundCols),
                 *border(colouredMouseOver ? borderColors(flags, cols) : cols);

    p->save();

    if(doEtch)
    {
        r.addCoords(0, 1, 0, -1);
        br=r;
    }

    if(!colouredMouseOver && lightBorder)
        br.addCoords(1, 1,-1,-1);
    else if(colouredMouseOver || (!IS_GLASS(app) && !sunken && flags&Style_Raised))
    {
        if(colouredMouseOver)
            p->setPen(border[QTC_MO_STD_LIGHT(w, sunken)]);
        else
            p->setPen(border[c1]);
        if(colouredMouseOver || bevelledButton || APPEARANCE_RAISED==app)
        {
            //Left & top
            p->drawLine(br.x()+1, br.y()+2, br.x()+1, br.y()+br.height()-3);
            p->drawLine(br.x()+1, br.y()+1, br.x()+br.width()-2, br.y()+1);

            if(colouredMouseOver)
                p->setPen(border[QTC_MO_STD_DARK(w)]);
            else
                p->setPen(border[sunken ? 0 : dark]);
            //Right & bottom
            p->drawLine(br.x()+br.width()-2, br.y()+1, br.x()+br.width()-2, br.y()+br.height()-3);
            p->drawLine(br.x()+1, br.y()+br.height()-2, br.x()+br.width()-2, br.y()+br.height()-2);
            br.addCoords(2, 2,-2,-2);
        }
        else
        {
            //Left & top
            p->drawLine(br.x()+1, br.y()+2, br.x()+1, br.y()+br.height()-2);
            p->drawLine(br.x()+1, br.y()+1, br.x()+br.width()-2, br.y()+1);
            br.addCoords(2, 2,-1,-1);
        }
    }
    else
        br.addCoords(1, 1,-1,-1);

    if(!colouredMouseOver && lightBorder && br.width()>0 && br.height()>0)
    {
        QColor col(cols[APPEARANCE_DULL_GLASS==app ? 1 : 0]);

        p->setPen(col);
        br=r;
        br.addCoords(1,1,-1,-1);
        p->drawRect(br);

        if(WIDGET_PROGRESSBAR==w && !IS_GLASS(app))
            br.addCoords(1,1,-1,-1);
        else if(horiz)
            br.addCoords(1,0,-1,-1);
        else
            br.addCoords(0,1,-1,-1);
    }

    // fill
    if(br.width()>0 && br.height()>0)
    {
        drawBevelGradient(fill, !sunken, p, br, horiz, getWidgetShade(w, true, sunken, app),
                          getWidgetShade(w, false, sunken, app), sunken, app, w);

        if(plastikMouseOver)
        {
            if(WIDGET_SB_SLIDER==w)
            {
                int len(QTC_SB_SLIDER_MO_LEN(horiz ? r.width() : r.height())),
                    so(lightBorder ? QTC_SLIDER_MO_BORDER : 1),
                    eo(len+so),
                    col(QTC_SLIDER_MO_SHADE);

                if(horiz)
                {
                    drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x()+so, r.y(), len, r.height()),
                                      horiz, getWidgetShade(w, true, sunken, app),
                                      getWidgetShade(w, false, sunken, app), sunken, app, w);
                    drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x()+r.width()-eo, r.y(), len, r.height()),
                                      horiz, getWidgetShade(w, true, sunken, app),
                                      getWidgetShade(w, false, sunken, app), sunken, app, w);
                }
                else
                {
                    drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x(), r.y()+so, r.width(), len),
                                      horiz, getWidgetShade(w, true, sunken, app),
                                      getWidgetShade(w, false, sunken, app), sunken, app, w);
                    drawBevelGradient(itsMouseOverCols[col], !sunken, p, QRect(r.x(), r.y()+r.height()-eo, r.width(), len),
                                      horiz, getWidgetShade(w, true, sunken, app),
                                      getWidgetShade(w, false, sunken, app), sunken, app, w);
                }
            }
            else
            {
                bool horizontal((horiz && WIDGET_SB_BUTTON!=w)|| (!horiz && WIDGET_SB_BUTTON==w)),
                     thin(WIDGET_SB_BUTTON==w || WIDGET_SPIN==w || ((horiz ? r.height() : r.width())<16));

                p->setPen(itsMouseOverCols[QTC_MO_PLASTIK_DARK(w)]);
                if(horizontal)
                {
                    p->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-2, r.y()+1);
                    p->drawLine(r.x()+1, r.y()+r.height()-2, r.x()+r.width()-2, r.y()+r.height()-2);
                }
                else
                {
                    p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.y()+r.height()-2);
                    p->drawLine(r.x()+r.width()-2, r.y()+1, r.x()+r.width()-2, r.y()+r.height()-2);
                }
                if(!thin)
                {
                    p->setPen(itsMouseOverCols[QTC_MO_PLASTIK_LIGHT(w)]);
                    if(horizontal)
                    {
                        p->drawLine(r.x()+1, r.y()+2, r.x()+r.width()-2, r.y()+2);
                        p->drawLine(r.x()+1, r.y()+r.height()-3, r.x()+r.width()-2, r.y()+r.height()-3);
                    }
                    else
                    {
                        p->drawLine(r.x()+2, r.y()+1, r.x()+2, r.y()+r.height()-2);
                        p->drawLine(r.x()+r.width()-3, r.y()+1, r.x()+r.width()-3, r.y()+r.height()-2);
                    }
                }
            }
        }
    }

    if(doBorder)
        drawBorder(bgnd, p, r, cg, flags, round, cols, w, doCorners);

    if(doEtch)
        drawEtch(p, rOrig, cg, true, true, EFFECT_SHADOW==opts.buttonEffect && WIDGET_BUTTON(w) && !sunken);

    p->restore();
}

void QtCurveStyle::drawEtch(QPainter *p, const QRect &r, const QColorGroup &cg, bool top, bool bot, bool raised) const
{
    if(top && !raised)
    {
        QColor darkCol(shade(cg.background(), QTC_ETCHED_DARK));

        p->setPen(darkCol);
        p->drawLine(r.x()+2, r.y(), r.x()+r.width()-3, r.y());
        p->setPen(midColor(darkCol, cg.background()));
        p->drawPoint(r.x()+1, r.y());
        p->drawPoint(r.x()+r.width()-2, r.y());
        p->drawPoint(r.x(), r.y()+1);
        p->drawPoint(r.x()+r.width()-1, r.y()+1);
    }
    if(bot)
    {
        QColor col(raised ? shade(cg.background(), QTC_ETCHED_DARK) : itsBackgroundCols[1]);

        p->setPen(col);
        p->drawLine(r.x()+2, r.y()+r.height()-1, r.x()+r.width()-3, r.y()+r.height()-1);
        p->setPen(midColor(raised ? col : itsBackgroundCols[0], cg.background()));
        p->drawPoint(r.x()+1, r.y()+r.height()-1);
        p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-1);
        p->drawPoint(r.x(), r.y()+r.height()-2);
        p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-2);
    }
}

void QtCurveStyle::drawBorder(const QColor &bgnd, QPainter *p, const QRect &r, const QColorGroup &cg,
                              SFlags flags, int round, const QColor *custom, EWidget w, bool doCorners,
                              EBorder borderProfile, bool blendBorderColors, int borderVal) const
{
    EAppearance  app(widgetApp(w, &opts));
    const QColor *cols(custom ? custom : itsBackgroundCols);
    QColor       border(flags&Style_ButtonDefault && IND_FONT_COLOR==opts.defBtnIndicator &&
                        flags&Style_Enabled
                          ? cg.buttonText()
                          : cols[!(flags&Style_Enabled) && (WIDGET_BUTTON(w) || WIDGET_SLIDER_TROUGH==w || flags&QTC_CHECK_BUTTON)
                                    ? QT_DISABLED_BORDER : borderVal]);

    switch(borderProfile)
    {
        case BORDER_FLAT:
            break;
        case BORDER_RAISED:
        case BORDER_SUNKEN:
            p->setPen(flags&Style_Enabled && (BORDER_RAISED==borderProfile || APPEARANCE_FLAT!=app)
                         ? blendBorderColors
                               ? midColor(cg.background(), cols[BORDER_RAISED==borderProfile
                                                                   ? 0 : QT_FRAME_DARK_SHADOW]) // Was base???
                               : cols[BORDER_RAISED==borderProfile ? 0 : QT_FRAME_DARK_SHADOW]
                         : cg.background());
            p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.y()+r.height()-2);
            p->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-2, r.y()+1);
            p->setPen(flags&Style_Enabled && (BORDER_SUNKEN==borderProfile || APPEARANCE_FLAT!=app)
                         ? blendBorderColors
                               ? midColor(cg.background(), cols[BORDER_RAISED==borderProfile
                                                                   ? QT_FRAME_DARK_SHADOW : 0]) // Was base???
                               : cols[BORDER_RAISED==borderProfile ? QT_FRAME_DARK_SHADOW : 0]
                         : cg.background());
            p->drawLine(r.x()+r.width()-2, r.y()+1, r.x()+r.width()-2, r.y()+r.height()-2);
            p->drawLine(r.x()+1, r.y()+r.height()-2, r.x()+r.width()-2, r.y()+r.height()-2);
    }

    if(QTC_ROUNDED && ROUNDED_NONE!=round)
    {
        bool largeArc(ROUND_FULL==opts.round && !(flags&QTC_CHECK_BUTTON) &&
                      r.width()>=QTC_MIN_BTN_SIZE && r.height()>=QTC_MIN_BTN_SIZE);

        p->setPen(border);
        if(itsFormMode)
        {
            // If we're itsFormMode (KHTML) then we need to draw the aa borders using pixmaps
            //  - so we need to draw 2 pixels away from each corner (so that the alpha
            // blend lets through the background color...
            p->drawLine(r.x()+2, r.y(), r.x()+r.width()-3, r.y());
            p->drawLine(r.x()+2, r.y()+r.height()-1, r.x()+r.width()-3, r.y()+r.height()-1);
            p->drawLine(r.x(), r.y()+2, r.x(), r.y()+r.height()-3);
            p->drawLine(r.x()+r.width()-1, r.y()+2, r.x()+r.width()-1, r.y()+r.height()-3);

            // If not rounding a corner need to draw the missing pixels!
            if(!(round&CORNER_TL) || !largeArc)
            {
                p->drawPoint(r.x()+1, r.y());
                p->drawPoint(r.x(), r.y()+1);
            }
            if(!(round&CORNER_TR) || !largeArc)
            {
                p->drawPoint(r.x()+r.width()-2, r.y());
                p->drawPoint(r.x()+r.width()-1, r.y()+1);
            }
            if(!(round&CORNER_BR) || !largeArc)
            {
                p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-1);
                p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-2);
            }
            if(!(round&CORNER_BL) || !largeArc)
            {
                p->drawPoint(r.x()+1, r.y()+r.height()-1);
                p->drawPoint(r.x(), r.y()+r.height()-2);
            }
        }
        else
        {
            // If we're not itsFormMode (ie. not KHTML) then we can just draw 1 pixel in - as
            // we can overwrite with the alpha colour.
            p->drawLine(r.x()+1, r.y(), r.x()+r.width()-2, r.y());
            p->drawLine(r.x()+1, r.y()+r.height()-1, r.x()+r.width()-2, r.y()+r.height()-1);
            p->drawLine(r.x(), r.y()+1, r.x(), r.y()+r.height()-2);
            p->drawLine(r.x()+r.width()-1, r.y()+1, r.x()+r.width()-1, r.y()+r.height()-2);
        }

        QColor  largeArcMid(midColor(border, bgnd)),
                aaColor(midColor(custom ? custom[3] : itsBackgroundCols[3], bgnd));
        QPixmap *pix=itsFormMode ? getPixelPixmap(border) : NULL;

        if(round&CORNER_TL)
        {
            if(largeArc)
            {
                p->drawPoint(r.x()+1, r.y()+1);
                if(itsFormMode)
                {
                    p->drawPixmap(r.x(), r.y()+1, *pix);
                    p->drawPixmap(r.x()+1, r.y(), *pix);
                }
                else
                {
                    p->setPen(largeArcMid);
                    p->drawLine(r.x(), r.y()+1, r.x()+1, r.y());
                }
            }
            if(doCorners)
                if(itsFormMode)
                {
                    if(!largeArc)
                        p->drawPixmap(r.x(), r.y(), *pix);
                }
                else
                {
                    p->setPen(largeArc ? bgnd : aaColor);
                    p->drawPoint(r.x(), r.y());
                }
        }
        else
            p->drawPoint(r.x(), r.y());

        p->setPen(border);
        if(round&CORNER_TR)
        {
            if(largeArc)
            {
                p->drawPoint(r.x()+r.width()-2, r.y()+1);
                if(itsFormMode)
                {
                    p->drawPixmap(r.x()+r.width()-2, r.y(), *pix);
                    p->drawPixmap(r.x()+r.width()-1, r.y()+1, *pix);
                }
                else
                {
                    p->setPen(largeArcMid);
                    p->drawLine(r.x()+r.width()-2, r.y(), r.x()+r.width()-1, r.y()+1);
                }
            }
            if(doCorners)
                if(itsFormMode)
                {
                    if(!largeArc)
                        p->drawPixmap(r.x()+r.width()-1, r.y(), *pix);
                }
                else
                {
                    p->setPen(largeArc ? bgnd : aaColor);
                    p->drawPoint(r.x()+r.width()-1, r.y());
                }
        }
        else
            p->drawPoint(r.x()+r.width()-1, r.y());

        p->setPen(border);
        if(round&CORNER_BR)
        {
            if(largeArc)
            {
                p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-2);
                if(itsFormMode)
                {
                    p->drawPixmap(r.x()+r.width()-2, r.y()+r.height()-1, *pix);
                    p->drawPixmap(r.x()+r.width()-1, r.y()+r.height()-2, *pix);
                }
                else
                {
                    p->setPen(largeArcMid);
                    p->drawLine(r.x()+r.width()-2, r.y()+r.height()-1, r.x()+r.width()-1,
                                r.y()+r.height()-2);
                }
            }
            if(doCorners)
                if(itsFormMode)
                {
                    if(!largeArc)
                        p->drawPixmap(r.x()+r.width()-1, r.y()+r.height()-1, *pix);
                }
                else
                {
                    p->setPen(largeArc ? bgnd : aaColor);
                    p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-1);
                }
        }
        else
            p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-1);

        p->setPen(border);
        if(round&CORNER_BL)
        {
            if(largeArc)
            {
                p->drawPoint(r.x()+1, r.y()+r.height()-2);
                if(itsFormMode)
                {
                    p->drawPixmap(r.x(), r.y()+r.height()-2, *pix);
                    p->drawPixmap(r.x()+1, r.y()+r.height()-1, *pix);
                }
                else
                {
                    p->setPen(largeArcMid);
                    p->drawLine(r.x(), r.y()+r.height()-2, r.x()+1, r.y()+r.height()-1);
                }
            }
            if(doCorners)
                if(itsFormMode)
                {
                    if(!largeArc)
                        p->drawPixmap(r.x(), r.y()+r.height()-1, *pix);
                }
                else
                {
                    p->setPen(largeArc ? bgnd : aaColor);
                    p->drawPoint(r.x(), r.y()+r.height()-1);
                }
        }
        else
            p->drawPoint(r.x(), r.y()+r.height()-1);
    }
    else
    {
        p->setPen(border);
        p->setBrush(NoBrush);
        p->drawRect(r);
    }
}

void QtCurveStyle::drawEntryField(QPainter *p, const QRect &rx, const QColorGroup &cg,
                                  SFlags flags, bool highlight, int round, EWidget w) const
{
    const QColor *use(highlight ? itsMenuitemCols : buttonColors(cg));
    bool         isSpin(WIDGET_SPIN==w),
                 doEtch(!itsFormMode && QTC_DO_EFFECT);

    QRect r(rx);

    if(doEtch)
        r.addCoords(0, 1, 0, -1);

    if(isSpin)
    {
        if(QApplication::reverseLayout())
            r.addCoords(-1, 0, 0, 0);

        p->setPen(flags&Style_Enabled ? cg.base() : cg.background());
        p->drawLine(r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
        p->drawLine(r.x()+r.width()-3, r.y(), r.x()+r.width()-3, r.y()+r.height()-1);
    }

    if(!itsFormMode)
        p->fillRect(rx, cg.background());
    p->fillRect(QRect(rx.x()+2, rx.y()+2, rx.x()+rx.width()-3, rx.y()+rx.height()-3),
                flags&Style_Enabled ? cg.base() : cg.background());

    p->setPen(APPEARANCE_FLAT!=opts.appearance || highlight
                ? midColor(flags&Style_Enabled ? cg.base() : cg.background(), use[3])
                : cg.base());
    p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.y()+r.height()-2);
    p->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-(isSpin ? 0 : 2), r.y()+1);

    p->setPen(flags&Style_Enabled ? midColor(cg.base(), use[0]) : cg.background());
    p->drawLine(r.x()+r.width()-(isSpin && highlight ? 3 : 2), r.y()+1,
                r.x()+r.width()-(isSpin && highlight ? 3 : 2), r.y()+r.height()-2);
    p->drawLine(r.x()+1, r.y()+r.height()-2, r.x()+r.width()-(isSpin ? 0 : 2), r.y()+r.height()-2);

    if(highlight && isSpin)
    {
        p->setPen(use[5]);
        p->drawLine(r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
    }

    drawBorder(cg.background(), p, r, cg, (SFlags)(flags|Style_Horizontal), round, use);
    if(doEtch)
    {
        r=rx;
        p->setClipRegion(r);

        if(!(round&CORNER_TR) && !(round&CORNER_BR))
            r.addCoords(0, 0, 2, 0);
        if(!(round&CORNER_TL) && !(round&CORNER_BL))
            r.addCoords(-2, 0, 0, 0);
        drawEtch(p, r, cg, true, true, EFFECT_SHADOW==opts.buttonEffect && WIDGET_BUTTON(w) &&
                 !(flags &(Style_Down | Style_On | Style_Sunken)));
        p->setClipping(false);
    }
}

static void drawArrow(QPainter *p, const QRect &r, const QColor &col, QStyle::PrimitiveElement pe, const Options &opts, bool small=false)
{
    QPointArray a;

    if(small)
        switch(pe)
        {
            case QStyle::PE_ArrowUp:
                a.setPoints(opts.vArrows ? 7 : 3,  2,0,  0,-2,  -2,0,   -2,1, -1,0, 1,0, 2,1);
                break;
            case QStyle::PE_ArrowDown:
                a.setPoints(opts.vArrows ? 7 : 3,  2,0,  0,2,  -2,0,   -2,-1, -1,0, 1,0, 2,-1);
                break;
            case QStyle::PE_ArrowRight:
                a.setPoints(opts.vArrows ? 7 : 3,  0,-2,  2,0,  0,2,   -1,2, 0,1, 0,-1, -1,-2);
                break;
            case QStyle::PE_ArrowLeft:
                a.setPoints(opts.vArrows ? 7 : 3,  0,-2,  -2,0,  0,2,   1,2, 0,1, 0,-1, 1,-2);
                break;
            default:
                return;
        }
    else // Large arrows...
        switch(pe)
        {
            case QStyle::PE_ArrowUp:
                a.setPoints(opts.vArrows ? 6 : 3,  3,1,  0,-2,  -3,1,    -2, 2,  0,0,  2,2);
                break;
            case QStyle::PE_ArrowDown:
                a.setPoints(opts.vArrows ? 6 : 3,  3,-1,  0,2,  -3,-1,   -2,-2,  0,0, 2,-2);
                break;
            case QStyle::PE_ArrowRight:
                a.setPoints(opts.vArrows ? 6 : 3,  -1,-3,  2,0,  -1,3,   -2,2, 0,0, -2,-2);
                break;
            case QStyle::PE_ArrowLeft:
                a.setPoints(opts.vArrows ? 6 : 3,  1,-3,  -2,0,  1,3,    2,2, 0,0, 2,-2);
                break;
            default:
                return;
        }

    if(a.isNull())
        return;

    p->save();
    a.translate((r.x()+(r.width()>>1)),(r.y()+(r.height()>>1)));
    p->setBrush(col);
    p->setPen(col);
    p->drawPolygon(a);
    p->restore();
}

void QtCurveStyle::drawArrow(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags,
                             PrimitiveElement pe, bool small, bool checkActive) const
{
    const QColor &col(flags&Style_Enabled
                          ? checkActive && flags&Style_Active
                                ? cg.highlightedText()
                                : cg.text()
                          : cg.mid());

    ::drawArrow(p, r, col, pe, opts, small);
}

void QtCurveStyle::drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r,
                                 const QColorGroup &cg, SFlags flags, const QStyleOption &data) const
{
    switch(pe)
    {
        case PE_HeaderSection:
        {
            // Is it a taskbar button? Kicker uses PE_HeaderSection for these! :-(
            // If the painter device is a QWidger, assume its not a taskbar button...
            if(APP_KICKER==itsThemedApp && (!p->device() || !dynamic_cast<QWidget *>(p->device())))
            {
                const QColor *use(buttonColors(cg));

                if(flags&Style_Down)
                    flags=((flags|Style_Down)^Style_Down)| Style_Sunken;
                flags|=Style_Enabled;
#if KDE_VERSION >= 0x30200
#if KDE_VERSION >= 0x30400 && KDE_VERSION < 0x30500
                if(HOVER_KICKER==itsHover && itsHoverWidget) //  && itsHoverWidget==p->device())
                    flags|=Style_MouseOver;
#endif
                itsFormMode=itsIsTransKicker;
#endif
                drawLightBevel(p, r, cg, flags|Style_Horizontal, ROUNDED_ALL,
                               getFill(flags, use), use, true, false);
#if KDE_VERSION >= 0x30200
                itsFormMode=false;
#endif
            }
            else
            {
                bool    isFirst(false), isLast(false), isTable(false);
                QHeader *header(p && p->device() ? dynamic_cast<QHeader*>(p->device())
                                                 : NULL);

                if (header)
                {
                    if(header->parent() && ::qt_cast<const QTable *>(header->parent()))
                    {
                        QTable *tbl((QTable *)(header->parent()));

                        isTable=true;
                        if(flags&Style_Horizontal)
                           isFirst=tbl->columnAt(r.x()+header->offset())==0;
                        else
                           isLast=tbl->rowAt(r.y()+header->offset())==(tbl->numRows()-1);
                    }
                    else
                        isFirst = header->mapToIndex(header->sectionAt(r.x()+header->offset())) == 0;
                }
                else if(0==flags) // Header on popup menu?
                {
                    const QColor *use(buttonColors(cg));

                    drawLightBevel(p, r, cg, flags|Style_Horizontal, ROUNDED_ALL,
                                   getFill(flags, use), use);
                    break;
                }

                flags=((flags|Style_Sunken)^Style_Sunken)| Style_Raised;

                if(QTC_NO_SECT!=itsHoverSect && HOVER_HEADER==itsHover && itsHoverWidget)
                {
                    QHeader *hd(::qt_cast<QHeader *>(itsHoverWidget));

                    if(hd && hd->isClickEnabled(itsHoverSect) && r==hd->sectionRect(itsHoverSect))
                        flags|=Style_MouseOver;
                }

                bool sunken(flags &(Style_Down | Style_On | Style_Sunken));

                drawBevelGradient(getFill(flags, itsBackgroundCols), !sunken, p, r, flags&Style_Horizontal,
                                  sunken ? SHADE_BEVEL_GRAD_SEL_LIGHT : SHADE_BEVEL_GRAD_LIGHT,
                                  sunken ? SHADE_BEVEL_GRAD_SEL_DARK : SHADE_BEVEL_GRAD_DARK,
                                  sunken, opts.lvAppearance, WIDGET_LISTVIEW_HEADER);

                if(APPEARANCE_RAISED==opts.lvAppearance)
                {
                    p->setPen(itsBackgroundCols[4]);
                    if(flags&Style_Horizontal)
                        p->drawLine(r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);
                    else
                        p->drawLine(r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
                }

                const QColor *border(borderColors(flags, NULL));

                if(flags&Style_Horizontal)
                {
                    if(border)
                    {
                        p->setPen(border[ORIGINAL_SHADE]);
                        p->drawLine(r.x(), r.y()+r.height()-2, r.x()+r.width()-1,
                                    r.y()+r.height()-2);
                        p->setPen(border[QT_STD_BORDER]);
                    }
                    else
                        p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);

                    if(!isFirst)
                    {
                        p->setPen(itsBackgroundCols[5]);
                        p->drawLine(r.x(), r.y()+5, r.x(), r.y()+r.height()-6);
                        p->setPen(itsBackgroundCols[0]);
                        p->drawLine(r.x()+1, r.y()+5, r.x()+1, r.y()+r.height()-6);
                    }
                }
                else
                {
                    if(border)
                    {
                        p->setPen(border[ORIGINAL_SHADE]);
                        p->drawLine(r.x()+r.width()-2, r.y(), r.x()+r.width()-2, r.y()+r.height()-1);
                        p->setPen(border[QT_STD_BORDER]);
                    }
                    else
                        p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);

                    if(!isLast)
                    {
                        p->setPen(itsBackgroundCols[5]);
                        p->drawLine(r.x()+5, r.y()+r.height()-2, r.x()+r.width()-6,
                                    r.y()+r.height()-2);
                        p->setPen(itsBackgroundCols[0]);
                        p->drawLine(r.x()+5, r.y()+r.height()-1, r.x()+r.width()-6,
                                    r.y()+r.height()-1);
                    }
                }
            }
            break;
        }
        case PE_HeaderArrow:
            drawArrow(p, r, cg, flags, flags&Style_Up ? PE_ArrowUp : PE_ArrowDown);
            break;
        case PE_ButtonBevel:
            flags|=Style_Enabled;
        case PE_ButtonCommand:
        case PE_ButtonTool:
        case PE_ButtonDropDown:
        {
            const QColor *use(buttonColors(cg));
            bool         glassMod(PE_ButtonTool==pe && IS_GLASS(opts.appearance) &&
                                  IS_GLASS(opts.toolbarAppearance)),
                         mdi(!(flags&QTC_CHECK_BUTTON) && (!(flags&QTC_STD_TOOLBUTTON)||flags&QTC_NO_ETCH_BUTTON) &&
                               PE_ButtonTool==pe && r.width()<=16 && r.height()<=16),
                         operaMdi(PE_ButtonTool==pe && APP_OPERA==itsThemedApp && r.width()==16 && r.height()==16);

            // If its not sunken, its raised-don't want flat buttons.
            if(!(flags&Style_Sunken))
                flags|=Style_Raised;

            if(PE_ButtonTool==pe && flags&QTC_VERTICAL_TB_BUTTON)
            {
                flags-=QTC_VERTICAL_TB_BUTTON;
                if(flags&Style_Horizontal)
                    flags-=Style_Horizontal;
            }

            // Dont AA' MDI windows' control buttons...
            itsFormMode=itsFormMode || mdi || operaMdi;

            if(mdi || operaMdi)
                flags|=Style_Horizontal;

            drawLightBevel(p, r, cg, glassMod ? flags : flags|Style_Horizontal,
#if KDE_VERSION >= 0x30200
                           (APP_KORN==itsThemedApp && itsIsTransKicker && PE_ButtonTool==pe) ||
#endif
                           operaMdi || mdi
                               ? ROUNDED_NONE
                               : ROUNDED_ALL,
                           getFill(flags, use), use, true, true,
                           flags&QTC_NO_ETCH_BUTTON ? WIDGET_NO_ETCH_BTN : WIDGET_STD_BUTTON);

            if(IND_COLORED==opts.defBtnIndicator && (flags&Style_ButtonDefault))
            {
                QRegion outer(r);
                QRect   r2(r);

                if(!itsFormMode && QTC_DO_EFFECT)
                    r2.addCoords(0, 1, 0, -1);

                r2.addCoords(COLORED_BORDER_SIZE, COLORED_BORDER_SIZE, -COLORED_BORDER_SIZE,
                             -COLORED_BORDER_SIZE);

                QRegion inner(r2);

                p->setClipRegion(outer.eor(inner));

                drawLightBevel(p, r, cg, glassMod ? flags : flags|Style_Horizontal,
                               flags&QTC_CHECK_BUTTON
#if KDE_VERSION >= 0x30200
                                 || (APP_KORN==itsThemedApp && itsIsTransKicker && PE_ButtonTool==pe)
#endif
                                    ? ROUNDED_NONE : ROUNDED_ALL,
                               itsDefBtnCols[QTC_MO_DEF_BTN], itsDefBtnCols, true, true,
                               WIDGET_DEF_BUTTON);
                p->setClipping(false);
            }
            itsFormMode=false;
            break;
        }

        case PE_ButtonDefault:
            switch(opts.defBtnIndicator)
            {
                case IND_CORNER:
                {
                    const QColor *use(buttonColors(cg));
                    QPointArray  points;
                    bool         sunken(flags&Style_Down || flags&Style_Sunken);
                    int          offset(sunken ? 4 : 3);

                    points.setPoints(3, r.x()+offset, r.y()+offset+1, r.x()+offset+6, r.y()+offset+1,
                                        r.x()+offset, r.y()+offset+7);

                    p->setBrush(use[sunken ? 0 : 4]);
                    p->setPen(use[sunken ? 0 : 4]);
                    p->drawPolygon(points);
                    break;
                }
                default:
                    break;
            }
            break;
        case PE_CheckMark:
            if(flags&Style_On)
            {
                QPixmap *pix(getPixmap(flags&Style_Enabled
                                           ? (flags&Style_Selected && !(flags&QTC_LISTVIEW_ITEM)) ||
                                             (flags&Style_Active && flags&QTC_MENU_ITEM)
                                               ? cg.highlightedText()
                                               : itsCheckRadioCol
                                           : cg.mid(),
                                       PIX_CHECK, 1.0));

                p->drawPixmap(r.center().x()-(pix->width()/2), r.center().y()-(pix->height()/2),
                              *pix);
            }
            else if (!(flags&Style_Off))    // tri-state
            {
                int x(r.center().x()), y(r.center().y());

                p->setPen(flags&Style_Enabled
                              ? flags&Style_Selected && !(flags&QTC_LISTVIEW_ITEM)
                                  ? cg.highlightedText()
                                  : itsCheckRadioCol
                              : cg.mid());
                p->drawLine(x-3, y, x+3, y);
                p->drawLine(x-3, y+1, x+3, y+1);
            }
            break;
        case PE_CheckListController:
        {
            QCheckListItem *item(data.checkListItem());

            if(item)
            {
                const QColor *bc(borderColors(flags, NULL)),
                             *btn(buttonColors(cg)),
                             *use(bc ? bc : btn);
                int          x(r.x()+1), y(r.y()+2);

                p->drawPixmap(x, y, *getPixmap(use[opts.coloredMouseOver && flags&Style_MouseOver
                                                    ? 4 : QT_BORDER(flags&Style_Enabled)],
                                               PIX_RADIO_BORDER, 0.8));
                ::drawArrow(p, QRect(r.x()-1, r.y()-1, r.width(), r.height()),
                            use[opts.coloredMouseOver && flags&Style_MouseOver ? 4:5], PE_ArrowDown, opts);
            }
            break;
        }
        case PE_CheckListIndicator:
        {
            QCheckListItem *item(data.checkListItem());

            if(item)
            {
                QListView *lv(item->listView());

                p->setPen(QPen(flags&Style_Enabled ? cg.text()
                                                   : lv->palette().color(QPalette::Disabled,
                                                                         QColorGroup::Text)));
                if (flags&Style_Selected)
                {
                    flags-=Style_Selected;
                    if(!lv->rootIsDecorated() && !((item->parent() && 1==item->parent()->rtti() &&
                       QCheckListItem::Controller==((QCheckListItem*)item->parent())->type())))
                    {
                        p->fillRect(0, 0, r.x()+lv->itemMargin()+r.width()+4, item->height(),
                                    cg.brush(QColorGroup::Highlight));
                        if(item->isEnabled())
                        {
                            p->setPen(QPen(cg.highlightedText()));
                            flags+=Style_Selected;
                        }
                    }
                }

                QRect checkRect(r.x()+1, r.y()+1, QTC_CHECK_SIZE, QTC_CHECK_SIZE);
                drawPrimitive(PE_Indicator, p, checkRect, cg, flags|QTC_LISTVIEW_ITEM);
            }
            break;
        }
        case PE_IndicatorMask:
            if(QTC_ROUNDED)
            {
                p->fillRect(r, color0);
                p->fillRect(r.x()+1, r.y(), r.width()-2, r.height(), color1);
                p->setPen(color1);
                p->drawLine(r.x(), r.y()+1, r.x(), r.y()+r.height()-2);
                p->drawLine(r.x()+r.width()-1, r.y()+1, r.x()+r.width()-1, r.y()+r.height()-2);
            }
            else
                p->fillRect(r, color1);
            break;
        case PE_Indicator:
        {
            bool   on(flags&Style_On || !(flags&Style_Off));
            SFlags sflags(!(flags&Style_Off) ? flags|Style_On : flags);

            if(!itsFormMode && sflags&Style_MouseOver && HOVER_NONE==itsHover)
                sflags-=Style_MouseOver;

            const QColor *bc(borderColors(sflags, NULL)),
                         *btn(buttonColors(cg)),
                         *use(bc ? bc : btn),
                         &bgnd(sflags&Style_Enabled
                                ? sflags&Style_MouseOver
                                    ? use[QTC_CR_MO_FILL]
                                    : cg.base()
                                : cg.background());

            if(IS_FLAT(opts.appearance))
                p->fillRect(QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2), bgnd);
            else
                drawBevelGradient(bgnd, false, p, QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2), true,
                                    getWidgetShade(WIDGET_TROUGH, true, false, APPEARANCE_GRADIENT),
                                    getWidgetShade(WIDGET_TROUGH, false, false, APPEARANCE_GRADIENT),
                                    false, APPEARANCE_GRADIENT, WIDGET_TROUGH);

            p->setPen(midColor(sflags&Style_Enabled ? cg.base() : cg.background(), use[3]));
            p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.y()+r.height()-2);
            p->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-2, r.y()+1);

            drawBorder(cg.background(), p, r, cg, (SFlags)(sflags|Style_Horizontal|QTC_CHECK_BUTTON),
                       ROUNDED_ALL, use, WIDGET_OTHER, !(flags&QTC_LISTVIEW_ITEM));

            if(on)
                drawPrimitive(PE_CheckMark, p, r, cg, flags);
            break;
        }
        case PE_CheckListExclusiveIndicator:
        {
            QCheckListItem *item(data.checkListItem());

            if(item)
            {
                const QColor *bc(borderColors(flags, NULL)),
                             *btn(buttonColors(cg)),
                             *use(bc ? bc : btn),
                             &on(flags&Style_Enabled
                                     ? itsCheckRadioCol
                                    : cg.mid());

                int          x(r.x()), y(r.y()+2);

                p->drawPixmap(x, y, *getPixmap(use[opts.coloredMouseOver && flags&Style_MouseOver ? 4 : QT_BORDER(flags&Style_Enabled)],
                                               PIX_RADIO_BORDER, 0.8));

                if(flags&Style_On)
                    p->drawPixmap(x, y, *getPixmap(on, PIX_RADIO_ON, 1.0));
            }
            break;
        }
        case PE_ExclusiveIndicator:
        case PE_ExclusiveIndicatorMask:
        {
            int x(r.x()), y(r.y());

            if(PE_ExclusiveIndicatorMask==pe)
            {
                p->fillRect(r, color0);
                p->setPen(Qt::color1);
                p->setBrush(Qt::color1);
                p->drawPie(r, 0, 5760);
            }
            else
            {
                QPointArray clipRegion;

                clipRegion.setPoints(8,  x,    y+8,     x,    y+4,     x+4, y,      x+8, y,
                                         x+12, y+4,     x+12, y+8,     x+8, y+12,   x+4, y+12);

                SFlags sflags(flags);

                if(!itsFormMode && sflags&Style_MouseOver && HOVER_NONE==itsHover)
                    sflags-=Style_MouseOver;

                const QColor *bc(borderColors(sflags, NULL)),
                             *btn(buttonColors(cg)),
                             *use(bc ? bc : btn);
                const QColor &on(sflags&Style_Enabled
                                     ? sflags&Style_Selected && !(flags&QTC_LISTVIEW_ITEM)
                                         ? cg.highlightedText()
                                         : itsCheckRadioCol
                                     : cg.mid()),
                             &bgnd(sflags&Style_Enabled
                                    ? sflags&Style_MouseOver
                                        ? use[QTC_CR_MO_FILL]
                                        : cg.base()
                                    : cg.background());
                bool         set(sflags&Style_On);

                p->save();
                p->fillRect(r, cg.background());

                p->setClipRegion(QRegion(clipRegion));
                if(IS_FLAT(opts.appearance))
                    p->fillRect(QRect(x+1, y+1, r.width()-2, r.height()-2), bgnd);
                else
                    drawBevelGradient(bgnd, false, p, QRect(x+1, y+1, r.width()-2, r.height()-2), true,
                                      getWidgetShade(WIDGET_TROUGH, true, false, APPEARANCE_GRADIENT),
                                      getWidgetShade(WIDGET_TROUGH, false, false, APPEARANCE_GRADIENT),
                                      false, APPEARANCE_GRADIENT, WIDGET_TROUGH);

                p->setClipping(false);

                p->drawPixmap(r.x(), r.y(),
                              *getPixmap(use[opts.coloredMouseOver && sflags&Style_MouseOver ? 4 : QT_BORDER(flags&Style_Enabled)],
                                         PIX_RADIO_BORDER, 0.8));

                if(QApplication::NormalColor!=QApplication::colorSpec() || itsFormMode)
                {
                    p->setPen(QPen(use[opts.coloredMouseOver && sflags&Style_MouseOver ? 4 : QT_BORDER(flags&Style_Enabled)], 1));
                    p->drawArc(r, 0, 5760);
                }

                if(set)
                    p->drawPixmap(r.x(), r.y(), *getPixmap(on, PIX_RADIO_ON, 1.0));
                if(QApplication::NormalColor==QApplication::colorSpec() || itsFormMode)
                    p->drawPixmap(r.x(), r.y(), *getPixmap(btn[sflags&Style_MouseOver ? 3 : 4], PIX_RADIO_LIGHT));
                p->restore();
            }
            break;
        }
        case PE_DockWindowSeparator:
        {
            QRect r2(r);
            int   light=0,
                  dark=3;

            r2.addCoords(-1, -1, 2, 2);
            drawMenuOrToolBarBackground(p, r2, cg, false, flags&Style_Horizontal);

            switch(opts.toolbarSeparators)
            {
                case LINE_NONE:
                    break;
                case LINE_SUNKEN:
                    if(r.width()<r.height())
                    {
                        int x(r.x()+((r.width()-2) / 2));

                        p->setPen(itsBackgroundCols[dark]);
                        p->drawLine(x, r.y()+6, x, r.y()+r.height()-7);
                        p->setPen(itsBackgroundCols[light]);
                        p->drawLine(x+1, r.y()+6, x+1, r.y()+r.height()-7);
                    }
                    else
                    {
                        int y(r.y()+((r.height()-2) / 2));

                        p->setPen(itsBackgroundCols[3]);
                        p->drawLine(r.x()+6, y, r.x()+r.width()-7, y);
                        p->setPen(itsBackgroundCols[0]);
                        p->drawLine(r.x()+6, y+1, r.x()+r.width()-7, y+1);
                    }
                    break;
                case LINE_DOTS:
                    drawDots(p, r, !(flags & Style_Horizontal), 1, 5, itsBackgroundCols, 0, 5);
            }
            break;
        }
        case PE_Splitter:
        {
            if(itsHoverWidget && itsHoverWidget == p->device())
                flags|=Style_MouseOver;

            const QColor *use(buttonColors(cg));
            const QColor *border(borderColors(flags, use));

            p->fillRect(r, QColor(flags&Style_MouseOver
                                      ? shade(cg.background(), opts.highlightFactor)
                                      : cg.background()));
            switch(opts.splitters)
            {
                default:
                case LINE_DOTS:
                    drawDots(p, r, flags&Style_Horizontal, NUM_SPLITTER_DASHES, 1, border, 0, 5);
                    break;
                case LINE_SUNKEN:
                    drawLines(p, r, flags&Style_Horizontal, NUM_SPLITTER_DASHES, 1, border, 0, 3);
                    break;
                case LINE_DASHES:
                    drawLines(p, r, flags&Style_Horizontal, NUM_SPLITTER_DASHES, 1, border, 0, 3, 0);
            }
            break;
        }
        case PE_DockWindowResizeHandle:
        {
            const QColor *use(backgroundColors(cg));

            drawBorder(cg.background(), p, r, cg, (SFlags)(flags|Style_Horizontal),
                       ROUNDED_ALL, use, WIDGET_OTHER, true, BORDER_RAISED, false);
            break;
        }
        case PE_GroupBoxFrame:
        case PE_PanelGroupBox:
            if (!opts.framelessGroupBoxes)
                if(APP_OPENOFFICE==itsThemedApp || data.lineWidth()>0 || data.isDefault())
                {
                    const QColor *use(backgroundColors(cg));

                    drawBorder(cg.background(), p, r, cg, (SFlags)(flags|Style_Horizontal),
                            ROUNDED_ALL, use, WIDGET_OTHER, true, BORDER_FLAT);
                }
                else
                    QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
            break;
        case PE_Panel:
        case PE_WindowFrame:
            if(APP_OPENOFFICE==itsThemedApp || data.lineWidth()>0 || data.isDefault())
            {
                const QColor *use(
#ifdef QTC_HIGHLIGHT_SCROLVIEWS
                                    flags&Style_HasFocus ? itsMenuitemCols :
#endif
                                    backgroundColors(cg));

                itsFormMode=itsIsTransKicker;
                drawBorder(cg.background(), p, r, cg,
                           (SFlags)(flags|Style_Horizontal|Style_Enabled),
                           ROUNDED_ALL, use, WIDGET_OTHER, APP_KICKER!=itsThemedApp, itsIsTransKicker
                                                                           ? BORDER_FLAT
                                                                           : flags&Style_Sunken
                                                                               ? BORDER_SUNKEN
                                                                               : BORDER_RAISED);
                itsFormMode=false;
            }
            else
                QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
            break;
        case PE_PanelTabWidget:
        {
            const QColor *use(backgroundColors(cg));

            drawBorder(cg.background(), p, r, cg,
                        (SFlags)(flags|Style_Horizontal|Style_Enabled),
                        ROUNDED_ALL, use, WIDGET_OTHER, true, BORDER_RAISED, false);
            break;
        }
        case PE_PanelPopup:
        {
            const QColor *use(backgroundColors(cg));

            p->setPen(use[5]);
            p->setBrush(NoBrush);
            p->drawRect(r);
            if(opts.lighterPopupMenuBgnd)
            {
                p->setPen(/*opts.lighterPopupMenuBgnd ? */itsLighterPopupMenuBgndCol/* : cg.background()*/);
                p->drawRect(QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2));
            }
            else
            {
                p->setPen(use[0]);
                p->drawLine(r.x()+1, r.y()+1, r.x()+r.width()-2,  r.y()+1);
                p->drawLine(r.x()+1, r.y()+1, r.x()+1,  r.y()+r.height()-2);
                p->setPen(use[QT_FRAME_DARK_SHADOW]);
                p->drawLine(r.x()+1, r.y()+r.height()-2, r.x()+r.width()-2,  r.y()+r.height()-2);
                p->drawLine(r.x()+r.width()-2, r.y()+1, r.x()+r.width()-2,  r.y()+r.height()-2);
            }
            break;
        }
        case PE_TabBarBase:
        {
            const QColor *use(backgroundColors(cg));
            bool  flat(APPEARANCE_FLAT==opts.appearance);

            if(data.isDefault() || data.lineWidth()>1)
            {
                p->setPen(use[5]);
                p->setBrush(NoBrush);
                p->drawRect(r);
                qDrawShadePanel(p, r.x()+1, r.y()+1, r.width()-2, r.height()-2,
                                QColorGroup(use[flat ? ORIGINAL_SHADE : 4], use[ORIGINAL_SHADE],
                                use[0], use[flat ? ORIGINAL_SHADE : 4], use[2],
                                cg.text(), use[ORIGINAL_SHADE]), flags & Style_Sunken,
                                data.isDefault() ? 1 : data.lineWidth()-1);
            }
            else
                qDrawShadePanel(p, r, QColorGroup(use[flat ? ORIGINAL_SHADE : 5],
                                use[ORIGINAL_SHADE], use[0], use[flat ? ORIGINAL_SHADE : 5], use[2],
                                cg.text(), use[ORIGINAL_SHADE]), flags & Style_Sunken,
                                data.isDefault() ? 2 : data.lineWidth());
            break;
        }
        case PE_PanelDockWindow:
        case PE_PanelMenuBar:
        {
            // fix for toolbar lag (from Mosfet Liquid) 
            QWidget *w = dynamic_cast<QWidget *>(p->device());

            if(w)
            {
                if(PaletteButton==w->backgroundMode())
                    w->setBackgroundMode(PaletteBackground);

                if(itsActive && opts.shadeMenubarOnlyWhenActive && SHADE_NONE!=opts.shadeMenubars)
                {
                    QWidget *top=w->topLevelWidget();

                    if(top && !top->isActiveWindow())
                        itsActive=false;
                }
            }

            drawMenuOrToolBarBackground(p, r, cg, PE_PanelMenuBar==pe,
                                        PE_PanelMenuBar==pe || r.width()>r.height());

            if(TB_NONE!=opts.toolbarBorders)
            {
                const QColor *use=PE_PanelMenuBar==pe && itsActive
                                      ? itsMenubarCols
                                      : backgroundColors(cg.background());
                bool         dark(TB_DARK==opts.toolbarBorders || TB_DARK_ALL==opts.toolbarBorders);

                if(TB_DARK_ALL==opts.toolbarBorders || TB_LIGHT_ALL==opts.toolbarBorders)
                {
                    p->setPen(use[0]);
                    p->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                    p->drawLine(r.x(), r.y(), r.x(), r.y()+r.width()-1);
                    p->setPen(use[dark ? 3 : 4]);
                    p->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    p->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                }
                else if(PE_PanelMenuBar==pe || r.width()>r.height())
                {
                    if(PE_PanelMenuBar!=pe)
                    {
                        p->setPen(use[0]);
                        p->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                    }
                    p->setPen(use[dark ? 3 : 4]);
                    p->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                }
                else
                {
                    p->setPen(use[0]);
                    p->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                    p->setPen(use[dark ? 3 : 4]);
                    p->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                }
            }
            break;
        }
        case PE_ScrollBarAddLine:
        case PE_ScrollBarSubLine:
        {
            QRect        br(r),
                         ar(r);
            bool         down((flags &(Style_Down|Style_On|Style_Sunken)));
            const QColor *use(itsButtonCols); // buttonColors(cg));

            pe=flags&Style_Horizontal
                   ? PE_ScrollBarAddLine==pe ? PE_ArrowRight : PE_ArrowLeft
                   : PE_ScrollBarAddLine==pe ? PE_ArrowDown : PE_ArrowUp;

            int round=PE_ArrowRight==pe ? ROUNDED_RIGHT :
                      PE_ArrowLeft==pe ? ROUNDED_LEFT :
                      PE_ArrowDown==pe ? ROUNDED_BOTTOM :
                      PE_ArrowUp==pe ? ROUNDED_TOP : ROUNDED_NONE;

            if(flags&Style_Down)
                ar.addCoords(1, 1, 1, 1);

            switch(opts.scrollbarType)
            {
                default:
                case SCROLLBAR_WINDOWS:
                    break;
                case SCROLLBAR_KDE:
                case SCROLLBAR_PLATINUM:
                    if(PE_ArrowLeft==pe && r.x()>3)
                    {
                        round=ROUNDED_NONE;
                        br.addCoords(0, 0, 1, 0);
                        ar.addCoords(1, 0, 1, 0);
                    }
                    else if(PE_ArrowUp==pe && r.y()>3)
                    {
                        round=ROUNDED_NONE;
                        br.addCoords(0, 0, 0, 1);
                        ar.addCoords(0, 1, 0, 1);
                    }
                    break;
                case SCROLLBAR_NEXT:
                    if(PE_ArrowRight==pe)
                    {
                        round=ROUNDED_NONE;
                        br.addCoords(-1, 0, 0, 0);
                        ar.addCoords(-1, 0, 0, -1);
                    }
                    else if(PE_ArrowDown==pe)
                    {
                        round=ROUNDED_NONE;
                        br.addCoords(0, -1, 0, 0);
                        ar.addCoords(0, -1, 0, -1);
                    }
                    break;
            }
            drawLightBevel(p, br, cg, down ? flags
                                           : flags|(flags&Style_Enabled
                                                        ? Style_Raised
                                                        : Style_Default),
                           round, getFill(flags, use), use, true, true, WIDGET_SB_BUTTON);

            drawPrimitive(pe, p, ar, cg, flags);
            break;
        }
        case PE_ScrollBarSlider:
            drawSbSliderHandle(p, r, cg, flags);
            break;
        case PE_FocusRect:
#if 0
            // Menu item style selection...
            if(opts.gtkComboMenus)
            {
                QWidget *widget(dynamic_cast<QWidget*>(p->device()));

                if(widget && 0==qstrcmp(widget->className(), "QViewportWidget") &&
                   widget->parentWidget() && ::qt_cast<QListBox *>(widget->parentWidget()) &&
                   widget->parentWidget()->parentWidget() && ::qt_cast<QComboBox *>(widget->parentWidget()->parentWidget()))
                {
                    struct QtCurveListBoxItem : public QListBoxItem
                    {
                        void paintContents(QPainter *p)
                        {
                            paint(p);
                        }
                    };

                    QListBox     *box=(QListBox *)widget->parentWidget();
                    QtCurveListBoxItem *item=(QtCurveListBoxItem *)(box->item(box->currentItem()));

                    if(item)
                    {
                        //p->fillRect(r, Qt::black);
                        drawMenuItem(p, r, cg, false, ROUNDED_ALL,
                                     opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol
                                                               : itsBackgroundCols[ORIGINAL_SHADE], itsMenuitemCols);
                        item->paintContents(p);
                        break;
                    }
                }
            }
#endif
#ifndef QTC_PLAIN_FOCUS_ONLY
            if(opts.stdFocus)
            {
#endif
                p->setPen(Qt::black);
                p->drawWinFocusRect(r);
#ifndef QTC_PLAIN_FOCUS_ONLY
            }
            else
            {
                //Figuring out in what beast we are painting...
                const QColor *use(backgroundColors(cg));
                QWidget      *widget(dynamic_cast<QWidget*>(p->device()));

                if(r.width()<4 || r.height()<4 ||
                   (widget && (dynamic_cast<QScrollView*>(widget->parent()) ||
                               dynamic_cast<QListBox*>(widget->parent()))))
                {
                    p->setPen(use[QT_FOCUS]);
                    p->drawRect(r);
                }
                else
                    drawBorder(cg.background(), p, r, cg, Style_Horizontal,
                               ROUNDED_ALL, use, WIDGET_OTHER, false, BORDER_FLAT, true, QT_FOCUS);
            }
#endif
            break;
        case PE_ArrowUp:
        case PE_ArrowDown:
        case PE_ArrowRight:
        case PE_ArrowLeft:
            drawArrow(p, r, cg, flags, pe);
            break;
        case PE_SpinWidgetUp:
        case PE_SpinWidgetDown:
        case PE_SpinWidgetPlus:
        case PE_SpinWidgetMinus:
        {
            QRect        sr(r);
            const QColor *use(buttonColors(cg));
            bool         reverse(QApplication::reverseLayout()),
                         doEtch(!itsFormMode && QTC_DO_EFFECT);

            if(doEtch)
                if(PE_SpinWidgetDown==pe || PE_SpinWidgetMinus==pe)
                    sr.addCoords(0, 0, 0, -1);
                else
                    sr.addCoords(0, 1, 0, 0);

            drawLightBevel(p, sr, cg, flags|Style_Horizontal, PE_SpinWidgetDown==pe || PE_SpinWidgetMinus==pe
                                                                ? reverse
                                                                    ? ROUNDED_BOTTOMLEFT
                                                                    : ROUNDED_BOTTOMRIGHT
                                                                : reverse
                                                                    ? ROUNDED_TOPLEFT
                                                                    : ROUNDED_TOPRIGHT,
                           getFill(flags, use), use, true, true, WIDGET_SPIN);

            if(doEtch)
            {
                QRect er(r);
                p->setClipRegion(er);
                if(reverse)
                    er.addCoords(0, 0, 2, 0);
                else
                    er.addCoords(-2, 0, 0, 0);
                drawEtch(p, er, cg, PE_SpinWidgetUp==pe || PE_SpinWidgetPlus==pe,
                         PE_SpinWidgetDown==pe || PE_SpinWidgetMinus==pe);
                p->setClipping(false);
            }

            if(PE_SpinWidgetUp==pe || PE_SpinWidgetDown==pe)
            {
                sr.setY(sr.y()+(PE_SpinWidgetDown==pe ? -2 : 1));

                if(flags&Style_Sunken)
                    sr.addCoords(1, 1, 1, 1);

                drawArrow(p, sr, cg, flags, PE_SpinWidgetUp==pe ? PE_ArrowUp : PE_ArrowDown, true);
            }
            else
            {
                int    l(QMIN(r.width()-6, r.height()-6));
                QPoint c(r.x()+(r.width()/2), r.y()+(r.height()/2));

                l/=2;
                if(l%2 != 0)
                    --l;

                if(flags&Style_Sunken)
                    c+=QPoint(1, 1);

                    p->setPen(cg.buttonText());
                p->drawLine(c.x()-l, c.y(), c.x()+l, c.y());
                if(PE_SpinWidgetPlus==pe)
                    p->drawLine(c.x(), c.y()-l, c.x(), c.y()+l);
            }
            break;
        }
        case PE_PanelLineEdit:
        {
            bool isReadOnly(false),
                 isEnabled(true);
            // panel is highlighted by default if it has focus, but if we have access to the
            // widget itself we can try to avoid highlighting in case it's readOnly or disabled.
            if (p->device() && dynamic_cast<QLineEdit*>(p->device()))
            {
                QLineEdit *lineEdit(dynamic_cast<QLineEdit*>(p->device()));
                isReadOnly = lineEdit->isReadOnly();
                isEnabled = lineEdit->isEnabled();

                if(flags&Style_Enabled && isReadOnly)
                    flags-=Style_Enabled;
            }

            // HACK!!  (From Plastik)
            //
            // In this place there is no reliable way to detect if we are in khtml; the
            // only thing we know is that khtml buffers its widgets into a pixmap. So
            // when the paint device is a QPixmap, chances are high that we are in khtml.
            // It's possible that this breaks other things, so let's see how it works...
            if (p->device() && dynamic_cast<QPixmap*>(p->device()))
                itsFormMode=true;

            drawEntryField(p, r, cg, flags, !isReadOnly && isEnabled && (flags&Style_HasFocus),
                           ROUNDED_ALL);
            itsFormMode=false;
            break;
        }
        case PE_StatusBarSection:
            if(opts.drawStatusBarFrames)
                KStyle::drawPrimitive(pe, p, r, cg, flags, data);
            break;
        default:
            KStyle::drawPrimitive(pe, p, r, cg, flags, data);
    }
}

static QString elliditide(const QString &text, const QFontMetrics &fontMetrics, const QRect &rect)
{
    // Chop and insert ellide into title if text is too wide
    QString title(text);

    if (fontMetrics.width(text) > rect.width())
    {
        QString ellipsis("...");

        while (fontMetrics.width(title+ellipsis)>rect.width())
            title=title.left(title.length()-1);
        return title+ellipsis;
    }

    return title;
}

void QtCurveStyle::drawKStylePrimitive(KStylePrimitive kpe, QPainter *p, const QWidget *widget,
                                       const QRect &r, const QColorGroup &cg, SFlags flags,
                                       const QStyleOption &opt) const
{
    ELine handles(kpe!=KPE_ToolBarHandle && LINE_DASHES==opts.handles ? LINE_SUNKEN
                                                                      : opts.handles);

    switch(kpe)
    {
        case KPE_ToolBarHandle:
        {
            QRect r2(r);
            r2.addCoords(-1, -1, 2, 2);
            drawMenuOrToolBarBackground(p, r2, cg, false, flags&Style_Horizontal);
            drawHandleMarkers(p, r, flags, true, handles);
            break;
        }
        case KPE_DockWindowHandle:
        {
/*
            QRect left(r);

            if(flags&Style_Horizontal)
                left.addCoords(0, 2, 0, -2);
            else
                left.addCoords(2, 0, -2, 0);
            drawHandleMarkers(p, left, flags, false, handles);
*/
/*
            if(IS_FLAT(opts.appearance))
*/
                p->fillRect(r, cg.background().dark(110));
/*
            else
            {
                const QColor *use(backgroundColors(cg));

                drawBevelGradient(use[ORIGINAL_SHADE], true, p, r,
                                  !(flags&Style_Horizontal) || r.width()>r.height(),
                                  getWidgetShade(WIDGET_STD_BUTTON, true, false, opts.appearance),
                                  getWidgetShade(WIDGET_STD_BUTTON, false, false, opts.appearance),
                                  false, opts.appearance, WIDGET_STD_BUTTON);
            }
*/
            if(widget && widget->parentWidget())
            {
                QString title(widget->parentWidget()->caption());

                if(!title.isEmpty())
                {
                    QRect r2(r);
                    QFont font(QApplication::font(widget));

                    font.setPointSize(font.pointSize()-2);
                    r2.addCoords(2, -2, -2, -2);
                    p->save();
                    p->setFont(font);
                    p->drawText(r, AlignLeft|AlignVCenter, elliditide(title, QFontMetrics(font), r2));
                    p->restore();
                }
            }
            break;
        }
        case KPE_GeneralHandle:
            drawHandleMarkers(p, r, flags, false, handles);
            break;
        case KPE_SliderGroove:
            drawSliderGroove(p, r, cg, flags, widget);
            break;
        case KPE_SliderHandle:
            drawSliderHandle(p, r, cg, flags);
            break;
        case KPE_ListViewExpander:
        {
            QRect ar(r.x()+((r.width()-(QTC_LV_SIZE+4))>>1), r.y()+((r.height()-(QTC_LV_SIZE+4))>>1), QTC_LV_SIZE+4,
                     QTC_LV_SIZE+4);

            if(opts.lvLines)
            {
                int lo(QTC_ROUNDED ? 2 : 0);

                p->setPen(cg.mid());
                p->drawLine(ar.x()+lo, ar.y(), (ar.x()+ar.width()-1)-lo, ar.y());
                p->drawLine(ar.x()+lo, ar.y()+ar.height()-1, (ar.x()+ar.width()-1)-lo,
                            ar.y()+ar.height()-1);
                p->drawLine(ar.x(), ar.y()+lo, ar.x(), (ar.y()+ar.height()-1)-lo);
                p->drawLine(ar.x()+ar.width()-1, ar.y()+lo, ar.x()+ar.width()-1,
                            (ar.y()+ar.height()-1)-lo);

                if(QTC_ROUNDED)
                {
                    p->drawPoint(ar.x()+1, ar.y()+1);
                    p->drawPoint(ar.x()+1, ar.y()+ar.height()-2);
                    p->drawPoint(ar.x()+ar.width()-2, ar.y()+1);
                    p->drawPoint(ar.x()+ar.width()-2, ar.y()+ar.height()-2);
                    p->setPen(midColor(cg.mid(), cg.background()));
                    p->drawLine(ar.x(), ar.y()+1, ar.x()+1, ar.y());
                    p->drawLine(ar.x()+ar.width()-2, ar.y(), ar.x()+ar.width()-1, ar.y()+1);
                    p->drawLine(ar.x(), ar.y()+ar.height()-2, ar.x()+1, ar.y()+ar.height()-1);
                    p->drawLine(ar.x()+ar.width()-2, ar.y()+ar.height()-1, ar.x()+ar.width()-1,
                                ar.y()+ar.height()-2);
                }
            }

            drawArrow(p, ar, cg, flags|Style_Enabled, flags&Style_On // Collapsed = On
                                            ?  QApplication::reverseLayout()
                                                 ? PE_ArrowLeft
                                                 : PE_ArrowRight
                                            : PE_ArrowDown);
            break;
        }
        case KPE_ListViewBranch:
            if(opts.lvLines)
            {
                p->setPen(cg.mid());
                if (flags&Style_Horizontal)
                {
                    if(r.width()>0)
                        p->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                }
                else
                    if(r.height()>0)
                        p->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
            }
            break;
        default:
            KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);
    }
}

void QtCurveStyle::drawControl(ControlElement control, QPainter *p, const QWidget *widget,
                               const QRect &r, const QColorGroup &cg, SFlags flags,
                               const QStyleOption &data) const
{
    if(widget==itsHoverWidget)
        flags|=Style_MouseOver;

    switch(control)
    {
        case CE_TabBarTab:
        {
            const QTabBar *tb((const QTabBar *)widget);
            int           tabIndex(tb->indexOf(data.tab()->identifier())),
                          dark(APPEARANCE_FLAT==opts.appearance ? ORIGINAL_SHADE : QT_FRAME_DARK_SHADOW);
            bool          cornerWidget(false),
                          bottomCornerWidget(false),
                          reverse(QApplication::reverseLayout()),
                          firstTab(0==tabIndex),
                          lastTab((tb->count()-1)==tabIndex),
//                           isFirstKTabCtlTab(firstTab && widget->parent()
//                                          ? 0==qstrcmp("KTabCtl", widget->parent()->className())
//                                          : false),
                          active(flags & Style_Selected),
                          itsHover(itsHoverTab && itsHoverTab->isEnabled() && data.tab()==itsHoverTab &&
                                          !(flags&Style_Selected) &&
                                          tb->currentTab()!=tabIndex);
            const QColor  &fill(getTabFill(flags&Style_Selected, itsHover, itsBackgroundCols));

            if(reverse)
            {
                bool oldLast=lastTab;

                lastTab=firstTab;
                firstTab=oldLast;
            }

            if(::qt_cast<const QTabWidget *>(tb->parent()))
            {
                const QTabWidget *tw((const QTabWidget*)tb->parent());

                // is there a corner widget in the (top) left edge?
                if(tw->cornerWidget(Qt::TopLeft))
                    cornerWidget=true;
                if(tw->cornerWidget(Qt::BottomLeft))
                    bottomCornerWidget=true;
            }

            QRect tr(r);
            bool  top(QTabBar::TriangularAbove==tb->shape() || QTabBar::RoundedAbove==tb->shape());

            if(!active)
                if(top)
                    tr.addCoords(0, 2, 0, 0);
                else
                    tr.addCoords(0, 0, 0, -2);

            if(!firstTab && top && (APP_TORA==itsThemedApp || (APP_OPENOFFICE==itsThemedApp && !active)))
                tr.addCoords(-1, 0, 0, 0);

            p->setClipRect(QRect(tr.x(), top ? tr.y() : tr.y()+2, tr.width(), top ? tr.height()-2 : tr.height()),
                           QPainter::CoordPainter);

            if(APPEARANCE_INVERTED==opts.appearance && active)
                p->fillRect(tr, cg.background());
            else
                drawBevelGradient(fill, top, p, tr, true,
                                    top ? SHADE_TAB_SEL_LIGHT
                                        : SHADE_BOTTOM_TAB_SEL_DARK,
                                    top ? SHADE_TAB_SEL_DARK
                                        : SHADE_BOTTOM_TAB_SEL_LIGHT,
                                  false, active ? QTC_SEL_TAB_APP : QTC_NORM_TAB_APP, top ? WIDGET_TAB_TOP : WIDGET_TAB_BOT);

            drawBorder(cg.background(), p, tr, cg, flags|Style_Horizontal|Style_Enabled,
                           active
                            ? (top ? ROUNDED_TOP : ROUNDED_BOTTOM)
                            : firstTab
                                ? (top ? ROUNDED_TOPLEFT : ROUNDED_BOTTOMLEFT)
                                : lastTab
                                    ? (top ? ROUNDED_TOPRIGHT : ROUNDED_BOTTOMRIGHT)
                                    : ROUNDED_NONE, NULL, top ? WIDGET_TAB_TOP : WIDGET_TAB_BOT, true,
                       active ? BORDER_RAISED : BORDER_FLAT, false);
            p->setClipping(false);

            if(top)
            {
                if(active)
                {
                    p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawPoint(r.x(), r.y()+r.height()-2);
                    p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-2);
                    p->setPen(itsBackgroundCols[0]);
                    p->drawLine(r.x()+1, r.y()+r.height()-3, r.x()+1, r.y()+r.height()-1);
                    //p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-1);
                    p->setPen(itsBackgroundCols[QT_FRAME_DARK_SHADOW]);
                    p->drawPoint(r.x()+r.width()-2, r.y()+r.height()-2);
                }
                else
                {
                    p->setPen(itsBackgroundCols[0]);
                    p->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawLine(r.x(), r.y()+r.height()-2, r.x()+r.width()-1, r.y()+r.height()-2);

                    if(opts.coloredMouseOver && itsHover)
                    {
                        p->setPen(itsMouseOverCols[ORIGINAL_SHADE]);
                        p->drawLine(tr.x()+(firstTab ? opts.round : 1), tr.y()+1,
                                    tr.x()+tr.width()-((lastTab ? opts.round : 0)+1), tr.y()+1);

                        p->setPen(itsMouseOverCols[QT_STD_BORDER]);
                        p->drawLine(tr.x()+(firstTab ? opts.round : 1), tr.y(),
                                    tr.x()+tr.width()-((lastTab ? opts.round : 0)+1), tr.y());
                    }
                }

                if(((!reverse && firstTab) || (lastTab && reverse)) && !cornerWidget)
                {
                    int x(reverse ? r.x()+r.width()-1 : r.x()),
                        x2(reverse ? x-1 : x+1);

                    p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawLine(x, r.y()+r.height()-1, x, r.height()-2);
                    if(active)
                    {
                        p->setPen(itsBackgroundCols[reverse ? dark : 0]);
                        p->drawLine(x2, r.y()+r.height()-1, x2, r.height()-2);
                    }
                }

                if(active && opts.highlightTab)
                {
                    p->setPen(itsMenuitemCols[0]);
                    p->drawLine(tr.left(), tr.top()+1, tr.right(), tr.top()+1);
                    p->drawLine(tr.left(), tr.top()+2, tr.right(), tr.top()+2);

                    p->setClipRect(QRect(tr.x(), tr.y(), tr.width(), 3), QPainter::CoordPainter);
                    drawBorder(cg.background(), p, tr, cg, flags|Style_Horizontal|Style_Enabled,
                               ROUNDED_ALL, itsMenuitemCols, top ? WIDGET_TAB_TOP : WIDGET_TAB_BOT,
                               true, BORDER_FLAT, false, 3);
                    p->setClipping(false);
                }

                // Round top-left corner...
                if(ROUND_FULL==opts.round && APP_TORA!=itsThemedApp && firstTab && !active && !cornerWidget && !reverse) // && !isFirstKTabCtlTab)
                {
                    p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawPoint(r.x()+1, r.y()+r.height()-1);
                    p->setPen(midColor(itsBackgroundCols[QT_STD_BORDER], cg.background()));
                    p->drawPoint(r.x()+1, r.y()+r.height()-2);
                }
            }
            else
            {
                if(active)
                {
                    p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawPoint(r.x(), r.y()+1);
                    p->drawPoint(r.x()+r.width()-1, r.y()+1);
                    p->setPen(itsBackgroundCols[0]);
                    p->drawLine(r.x()+1, r.y()+2, r.x()+1, r.y());
                    p->setPen(itsBackgroundCols[QT_FRAME_DARK_SHADOW]);
                    p->drawLine(r.x()+r.width()-2, r.y()+1, r.x()+r.width()-2, r.y());
                    p->drawPoint(r.x()+r.width()-1, r.y());
                }
                else
                {
                    p->setPen(itsBackgroundCols[dark]);
                    p->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                    p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawLine(r.x(), r.y()+1, r.x()+r.width()-1, r.y()+1);

                    if(opts.coloredMouseOver && itsHover)
                    {
                        p->setPen(itsMouseOverCols[ORIGINAL_SHADE]);
                        p->drawLine(tr.x()+(firstTab ? opts.round : 1), tr.y()+tr.height()-2,
                                    tr.x()+tr.width()-((lastTab ? opts.round : 0)+1), tr.y()+tr.height()-2);

                        p->setPen(itsMouseOverCols[3]);
                        p->drawLine(tr.x()+(firstTab ? opts.round : 1), tr.y()+tr.height()-1,
                                    tr.x()+tr.width()-((lastTab ? opts.round : 0)+1), tr.y()+tr.height()-1);
                    }
                }

                if(active && opts.highlightTab)
                {
                    p->setPen(itsMenuitemCols[0]);
                    p->drawLine(tr.left(), tr.bottom()-1, tr.right(), tr.bottom()-1);
                    p->drawLine(tr.left(), tr.bottom()-2, tr.right(), tr.bottom()-2);
                    p->setClipRect(QRect(tr.x(), tr.y()+r.height()-3, tr.width(), 3), QPainter::CoordPainter);
                    drawBorder(cg.background(), p, tr, cg, flags|Style_Horizontal|Style_Enabled,
                               ROUNDED_ALL, itsMenuitemCols, top ? WIDGET_TAB_TOP : WIDGET_TAB_BOT,
                               true, BORDER_FLAT, false, 3);
                    p->setClipping(false);
                }

                if(ROUND_FULL==opts.round && APP_TORA!=itsThemedApp && firstTab && !bottomCornerWidget)// && !isFirstKTabCtlTab)
                {
                    p->setPen(itsBackgroundCols[QT_STD_BORDER]);
                    p->drawPoint(r.x(), reverse ? r.y()+r.width()-1 : r.y());
                    // Round bottom-left corner...
                    if(!active&& !reverse)
                    {
                        p->drawPoint(r.x()+1, r.y()-1);
                        p->setPen(midColor(itsBackgroundCols[QT_STD_BORDER], cg.background()));
                        p->drawPoint(r.x()+1, r.y());
                    }
                }
            }
            break;
        }
#if QT_VERSION >= 0x030200
        case CE_TabBarLabel:
        {
            if (data.isDefault())
                break;

            const QTabBar *tb((const QTabBar *) widget);
            QTab          *t(data.tab());
            QRect         tr(r);
            int           shift(pixelMetric(PM_TabBarTabShiftVertical, tb));

            if (t->identifier() == tb->currentTab())
            {
                if(QTabBar::RoundedAbove==tb->shape() || QTabBar::TriangularAbove==tb->shape())
                    tr.addCoords(0, -shift, 0, -shift);
            }
            else
                if(QTabBar::RoundedBelow==tb->shape() || QTabBar::TriangularBelow==tb->shape())
                    tr.addCoords(0, shift, 0, shift);

            if(APP_MACTOR==itsThemedApp)
            {
                drawControl(CE_TabBarTab, p, widget, t->rect(), cg, flags, data);

                if(t->iconSet())
                {
                    QIconSet::Mode mode((t->isEnabled() && tb->isEnabled())
                                            ? QIconSet::Normal : QIconSet::Disabled);

                    if (mode == QIconSet::Normal && (flags&Style_HasFocus))
                        mode = QIconSet::Active;

                    QPixmap pixmap(t->iconSet()->pixmap(QIconSet::Small, mode));
                    int     pixh(pixmap.height()),
                            xoff(0),
                            yoff(0);

                    if(!(flags&Style_Selected))
                    {
                        xoff = pixelMetric(PM_TabBarTabShiftHorizontal, widget);
                        yoff = pixelMetric(PM_TabBarTabShiftVertical, widget);
                    }
                    p->drawPixmap(t->rect().left()+8+xoff, t->rect().center().y()-pixh/2 + yoff,
                                  pixmap);
                }
            }

            drawItem(p, tr, AlignCenter | ShowPrefix, cg, flags & Style_Enabled, 0, t->text());

            if ((flags & Style_HasFocus) && !t->text().isEmpty())
            {
                QRect fr(r);

                if(QTabBar::RoundedAbove==tb->shape() || QTabBar::TriangularAbove==tb->shape())
                    fr.addCoords(0, 1, 0, -1);
                else
                    fr.addCoords(0, 0, 0, -1);

                drawPrimitive(PE_FocusRect, p, fr, cg);
            }
            break;
        }
#endif
        case CE_PushButtonLabel:  // Taken from Highcolor and Plastik...
        {
            int x, y, w, h;

            r.rect(&x, &y, &w, &h);

            const QPushButton *button(static_cast<const QPushButton *>(widget));
            bool              active(button->isOn() || button->isDown()),
                              cornArrow(false);

            // Shift button contents if pushed.
            if (active)
            {
                x += pixelMetric(PM_ButtonShiftHorizontal, widget);
                y += pixelMetric(PM_ButtonShiftVertical, widget);
                flags |= Style_Sunken;
            }

            // Does the button have a popup menu?
            if (button->isMenuButton())
            {
                int dx(pixelMetric(PM_MenuButtonIndicator, widget)),
                    margin(pixelMetric(PM_ButtonMargin, widget));

                if(button->iconSet() && !button->iconSet()->isNull() &&
                    (dx+button->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal, QIconSet::Off
                                                  ).width()) >= w )
                    cornArrow = true; //To little room. Draw the arrow in the corner, don't adjust
                                      //the widget
                else
                {
                    drawPrimitive(PE_ArrowDown, p,
                                  visualRect(QRect((x + w) - (dx + margin), y, dx, h), r), cg,
                                  flags, data);
                    w-=dx;
                }
            }

            // Draw the icon if there is one
            if (button->iconSet() && !button->iconSet()->isNull())
            {
                QIconSet::Mode  mode(QIconSet::Disabled);
                QIconSet::State state(QIconSet::Off);

                if (button->isEnabled())
                    mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
                if (button->isToggleButton() && button->isOn())
                    state = QIconSet::On;

                QPixmap pixmap = button->iconSet()->pixmap(QIconSet::Small, mode, state);

                static const int constSpace(2);

                int xo(0),
                    pw(pixmap.width()),
                    iw(0);

                if (button->text().isEmpty() && !button->pixmap())
                    p->drawPixmap(x + (w>>1) - (pixmap.width()>>1),
                                  y + (h>>1) - (pixmap.height()>>1),
                                  pixmap);
                else
                {
                    iw=button->pixmap() ? button->pixmap()->width()
                                        : widget->fontMetrics().size(Qt::ShowPrefix,
                                                                     button->text()).width();

                    int cw(iw+pw+constSpace);

                    xo=cw<w ? (w-cw)>>1 : constSpace; 
                    p->drawPixmap(x+xo, y + (h>>1) - (pixmap.height()>>1), pixmap);
                    xo+=pw;
                }

                if (cornArrow) //Draw over the icon
                    drawPrimitive(PE_ArrowDown, p, visualRect(QRect(x + w - 6, x + h - 6, 7, 7), r),
                                  cg, flags, data);

                if(xo && iw)
                {
                    x+= xo+constSpace;
                    w=iw;
                }
                else
                {
                    x+= pw+constSpace;
                    w-= pw+constSpace;
                }
            }

            // Make the label indicate if the button is a default button or not
            int          i,
                         j(opts.embolden && button->isDefault() ? 2 : 1);
            const QColor &textCol(!opts.stdSidebarButtons && button->isFlat() &&
                                  button->inherits("KMultiTabBarTab") &&
                                  (button->isOn() || flags&Style_On)
                                    ? QApplication::palette().active().highlightedText()
                                    : button->colorGroup().buttonText());

            for(i=0; i<j; i++)
                drawItem(p, QRect(x+i, y, w, h), AlignCenter|ShowPrefix, button->colorGroup(),
                         button->isEnabled(),
                         button->pixmap(), button->text(), -1, &textCol);

            // Draw a focus rect if the button has focus
            if (flags & Style_HasFocus)
               drawPrimitive(PE_FocusRect, p, visualRect(subRect(SR_PushButtonFocusRect,
                             widget), widget), cg, flags);
            break;
        }
        case CE_PopupMenuItem:
        {
            if(!widget || data.isDefault())
                break;

            const QPopupMenu *popupmenu((const QPopupMenu *)widget);
            QMenuItem        *mi(data.menuItem());
            int              tab(data.tabWidth()),
                             maxpmw(data.maxIconWidth()),
                             x, y, w, h;

            r.rect(&x, &y, &w, &h);

            if((flags & Style_Active)&&(flags & Style_Enabled))
                drawMenuItem(p, r, cg, false, ROUNDED_ALL,
                             opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol
                                                       : itsBackgroundCols[ORIGINAL_SHADE], itsMenuitemCols);
            else if(widget->erasePixmap() && !widget->erasePixmap()->isNull())
                p->drawPixmap(x, y, *widget->erasePixmap(), x, y, w, h);
            else
                p->fillRect(r, opts.lighterPopupMenuBgnd ? itsLighterPopupMenuBgndCol
                                                         : itsBackgroundCols[ORIGINAL_SHADE]);

            if(!mi)
                break;

            if(mi->isSeparator())
            {
                y=r.y()+((r.height()/2)-1);
                p->setPen(itsBackgroundCols[5]);
                p->drawLine(r.x()+4, y, r.x()+r.width()-5, y);
//                 p->setPen(itsBackgroundCols[0]);
//                 p->drawLine(r.x()+4, y+1, r.x()+r.width()-5, y+1);
                break;
            }

            maxpmw=QMAX(maxpmw, 16);

            QRect cr, ir, tr, sr;
            // check column
            cr.setRect(r.left(), r.top(), maxpmw, r.height());
            // submenu indicator column
            sr.setCoords(r.right()-maxpmw, r.top(), r.right(), r.bottom());
            // tab/accelerator column
            tr.setCoords(sr.left()-tab-4, r.top(), sr.left(), r.bottom());
            // item column
            ir.setCoords(cr.right()+4, r.top(), tr.right()-4, r.bottom());

            bool reverse(QApplication::reverseLayout());

            if(reverse)
            {
                cr=visualRect(cr, r);
                sr=visualRect(sr, r);
                tr=visualRect(tr, r);
                ir=visualRect(ir, r);
            }

            if(mi->iconSet())
            {
                // Select the correct icon from the iconset
                QIconSet::Mode mode=flags & Style_Active
                                        ? (mi->isEnabled() ? QIconSet::Active : QIconSet::Disabled)
                                        : (mi->isEnabled() ? QIconSet::Normal : QIconSet::Disabled);
                cr=visualRect(QRect(x, y, maxpmw, h), r);

                // Do we have an icon and are checked at the same time?
                // Then draw a "pressed" background behind the icon
                if(popupmenu->isCheckable() && mi->isChecked())
                    drawLightBevel((flags & Style_Active)&&(flags & Style_Enabled)
                                    ? itsMenuitemCols[ORIGINAL_SHADE]
                                    : cg.background(), p, QRect(cr.x()+1, cr.y()+2, cr.width()-2, cr.height()-4),
                                   cg, flags|Style_Sunken|Style_Horizontal, ROUNDED_ALL,
                                   getFill(flags|Style_Sunken|Style_Enabled, itsBackgroundCols),
                                   itsBackgroundCols, true, false, WIDGET_NO_ETCH_BTN);

                // Draw the icon
                QPixmap pixmap(mi->iconSet()->pixmap(QIconSet::Small, mode));
                QRect   pmr(0, 0, pixmap.width(), pixmap.height());

                pmr.moveCenter(cr.center());
                p->drawPixmap(pmr.topLeft(), pixmap);
            }
            else if(popupmenu->isCheckable() && mi->isChecked())
                drawPrimitive(PE_CheckMark, p, cr, cg,
                              (flags &(Style_Enabled|Style_Active))| Style_On|QTC_MENU_ITEM);

            QColor textcolor,
                   embosscolor;

            if(flags&Style_Active)
            {
                if(!(flags & Style_Enabled))
                {
                    textcolor=cg.text();
                    embosscolor=cg.light();
                }
                else
                {
                    textcolor=cg.highlightedText();
                    embosscolor=cg.midlight().light();
                }
            }
            else if(!(flags & Style_Enabled))
            {
                textcolor=cg.text();
                embosscolor=cg.light();
            }
            else
            {
                textcolor=cg.foreground();
                embosscolor=cg.light();
            }
            p->setPen(textcolor);

            if(mi->custom())
            {
                p->save();
                if(!(flags & Style_Enabled))
                {
                    p->setPen(cg.light());
                    mi->custom()->paint(p, cg,(flags & Style_Enabled)?(flags & Style_Active): 0,
                                        flags & Style_Enabled, ir.x()+1, ir.y()+1, ir.width()-1,
                                        ir.height()-1);
                    p->setPen(textcolor);
                }
                mi->custom()->paint(p, cg,(flags & Style_Enabled)?(flags & Style_Active): 0,
                                    flags & Style_Enabled, ir.x(), ir.y(), ir.width(), ir.height());
                p->restore();
            }

            QString text=mi->text();

            if(!text.isNull())
            {
                int t(text.find('\t'));

                // draw accelerator/tab-text
                if(t>=0)
                {
                    int alignFlag(AlignVCenter | ShowPrefix | DontClip | SingleLine);

                    alignFlag |=(reverse ? AlignLeft : AlignRight);

                    if(!(flags & Style_Enabled))
                    {
                        p->setPen(embosscolor);
                        tr.moveBy(1, 1);
                        p->drawText(tr, alignFlag, text.mid(t +1));
                        tr.moveBy(-1,-1);
                        p->setPen(textcolor);
                    }

                    p->drawText(tr, alignFlag, text.mid(t +1));
                }

                int alignFlag(AlignVCenter | ShowPrefix | DontClip | SingleLine);

                alignFlag |=(reverse ? AlignRight : AlignLeft);

                if(!(flags & Style_Enabled))
                {
                    p->setPen(embosscolor);
                    ir.moveBy(1, 1);
                    p->drawText(ir, alignFlag, text, t);
                    ir.moveBy(-1,-1);
                    p->setPen(textcolor);
                }

                p->drawText(ir, alignFlag, text, t);
            } 
            else if(mi->pixmap())
            {
                QPixmap *pixmap(mi->pixmap());

                if(1==pixmap->depth())
                    p->setBackgroundMode(OpaqueMode);
                int diffw(((r.width() - pixmap->width())/2) +
                          ((r.width() - pixmap->width())%2));
                p->drawPixmap(r.x()+diffw, r.y()+1, *pixmap );
                if(1==pixmap->depth())
                    p->setBackgroundMode(TransparentMode);
            }

            if(mi->popup())
                drawArrow(p, sr, cg, flags, reverse ? PE_ArrowLeft : PE_ArrowRight, false, true);
            break;
        }
        case CE_MenuBarItem:
        {
            bool down( (flags&Style_Enabled) && (flags&Style_Active) && (flags&Style_Down) ),
                 active( (flags&Style_Enabled) && (flags&Style_Active) &&
                       ( (flags&Style_Down) || opts.menubarMouseOver ) );

            if(!active || IS_GLASS(opts.menubarAppearance) || SHADE_NONE!=opts.shadeMenubars)
            {
                QMenuBar *mb((QMenuBar*)widget);
                QRect    r2(r);

                r2.setY(mb->rect().y()+1);
                r2.setHeight(mb->rect().height()-2);

                drawMenuOrToolBarBackground(p, r2, cg);
            }

            if(active)
                drawMenuItem(p, r, cg, true, down && opts.roundMbTopOnly ? ROUNDED_TOP : ROUNDED_ALL,
                             itsMenubarCols[ORIGINAL_SHADE],
                             opts.colorMenubarMouseOver || down ? itsMenuitemCols : itsBackgroundCols);

            if(data.isDefault())
                break;

            QMenuItem *mi(data.menuItem());

            if(mi->text().isEmpty()) // Draw pixmap...
                drawItem(p, r, AlignCenter|ShowPrefix|DontClip|SingleLine, cg, flags&Style_Enabled,
                     mi->pixmap(), QString::null);
            else
            {
                const QColor *col=(opts.colorMenubarMouseOver && active) || (!opts.colorMenubarMouseOver && down)
                                ? opts.customMenuTextColor
                                    ? &opts.customMenuSelTextColor
                                    : &cg.highlightedText()
                                : itsActive
                                    ? opts.customMenuTextColor
                                        ? &opts.customMenuNormTextColor
                                        : SHADE_BLEND_SELECTED==opts.shadeMenubars ||
                                            (SHADE_CUSTOM==opts.shadeMenubars &&
                                            TOO_DARK(itsMenubarCols[ORIGINAL_SHADE]))
                                            ? &cg.highlightedText()
                                            : &cg.foreground()
                                    : &cg.foreground();

                p->setPen(*col);
                p->drawText(r, AlignCenter|ShowPrefix|DontClip|SingleLine, mi->text());
            }

            break;
        }
        case CE_MenuBarEmptyArea:
            drawMenuOrToolBarBackground(p, r, cg);
            break;
        case CE_DockWindowEmptyArea:
            if(widget && widget->inherits("QToolBar"))
            {
                QDockWindow *wind((QDockWindow*)widget);

                drawMenuOrToolBarBackground(p, r, cg, false, Qt::Horizontal==wind->orientation());
            }
            else
                KStyle::drawControl(control, p, widget, r, cg, flags, data);
            break;
        case CE_ProgressBarGroove:
        {
            if(opts.gradientPbGroove)
                drawBevelGradient(flags & Style_Enabled ? cg.base() : cg.background(), false, p, r, true,
                                  getWidgetShade(WIDGET_TROUGH, true, false, opts.progressAppearance),
                                  getWidgetShade(WIDGET_TROUGH, false, false, opts.progressAppearance),
                                  false, APPEARANCE_GRADIENT, WIDGET_TROUGH);
            else
            {
                p->setBrush(flags & Style_Enabled ? cg.base() : cg.background());
                p->drawRect(r);
            }

            const QColor *use(backgroundColors(cg));

            drawBorder(cg.background(), p, r, cg, (SFlags)(flags|Style_Horizontal),
                       ROUNDED_ALL, use, WIDGET_OTHER, true, BORDER_SUNKEN);
            break;
        }
        case CE_ProgressBarContents:
        {
            const QProgressBar *pb((const QProgressBar*)widget);
            int                steps(pb->totalSteps());

            if(0==steps)//Busy indicator
            {
                static const int barWidth(10);

                int progress(pb->progress() % (2*(r.width()-barWidth)));

                if(progress < 0)
                    progress = 0;
                else if(progress > r.width()-barWidth)
                    progress = (r.width()-barWidth)-(progress-(r.width()-barWidth));

                p->fillRect(r, flags & Style_Enabled ? cg.base() : cg.background());
                drawLightBevel(cg.background(), p, QRect(r.x()+progress, r.y(), barWidth,
                               r.height()), cg, flags, ROUNDED_ALL, itsMenuitemCols[ORIGINAL_SHADE],
                               itsMenuitemCols, true, true, WIDGET_PROGRESSBAR);
            }
            else
            {
                QRect cr(subRect(SR_ProgressBarContents, widget));

                if(cr.isValid() && pb->progress()>0)
                {
                    double pg(((double)pb->progress()) / steps);
                    int    width(QMIN(cr.width(), (int)(pg * cr.width())));

                    if(QApplication::reverseLayout())
                        drawProgress(p, QRect(cr.x()+(cr.width()-width), cr.y(), width,
                                     cr.height()), cg, flags,
                                     width==cr.width() ? ROUNDED_NONE : ROUNDED_LEFT, widget);
                    else
                        drawProgress(p, QRect(cr.x(), cr.y(), width, cr.height()), cg, flags,
                                     width==cr.width() ?  ROUNDED_NONE : ROUNDED_RIGHT, widget);
                }
            }
            break;
        }
        case CE_PushButton:
        {
            const QPushButton *button(static_cast<const QPushButton *>(widget));
            bool              sidebar(button->isFlat() && button->inherits("KMultiTabBarTab"));

            if(!opts.stdSidebarButtons && sidebar)
            {
                QRect r2(r);

                flags|=QTC_TOGGLE_BUTTON;
                if(button->isOn())
                    flags|=Style_On;

                const QColor *use(flags&Style_On ? getSidebarButtons() : buttonColors(cg));

                if((flags&Style_On ) || flags&Style_MouseOver)
                {
                    r2.addCoords(-1, -1, 1, 1);
                    drawLightBevel(p, r2, cg, flags|Style_Horizontal, ROUNDED_NONE,
                                   getFill(flags, use), use, false, false, WIDGET_MENU_ITEM);
                }
                else
                    p->fillRect(r2, cg.background());

                if(flags&Style_MouseOver && opts.coloredMouseOver)
                {
                    r2=r;
                    if(MO_PLASTIK==opts.coloredMouseOver)
                        r2.addCoords(0, 1, 0, -1);
                    else
                        r2.addCoords(1, 1, -1, -1);
                    p->setPen(itsMouseOverCols[flags&Style_On ? 0 : 1]);
                    p->drawLine(r.x(), r.y(), r.x()+r.width()-1, r.y());
                    p->drawLine(r2.x(), r2.y(), r2.x()+r2.width()-1, r2.y());

                    if(MO_PLASTIK!=opts.coloredMouseOver)
                    {
                        p->drawLine(r.x(), r.y(), r.x(), r.y()+r.height()-1);
                        p->drawLine(r2.x(), r2.y(), r2.x(), r2.y()+r2.height()-1);
                        p->setPen(itsMouseOverCols[flags&Style_On ? 1 : 2]);
                    }

                    p->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1);
                    p->drawLine(r2.x(), r2.y()+r2.height()-1, r2.x()+r2.width()-1,
                                r2.y()+r2.height()-1);

                    if(MO_PLASTIK!=opts.coloredMouseOver)
                    {
                        p->drawLine(r.x()+r.width()-1, r.y(), r.x()+r.width()-1, r.y()+r.height()-1);
                        p->drawLine(r2.x()+r2.width()-1, r2.y(), r2.x()+r2.width()-1,
                                    r2.y()+r2.height()-1);
                    }
                }
            }
            else
            {
                itsFormMode = isFormWidget(widget);

                if(IND_FONT_COLOR==opts.defBtnIndicator && button->isDefault())
                    flags|=Style_ButtonDefault;

                if(button->isToggleButton())
                    flags|=QTC_TOGGLE_BUTTON;

                if(sidebar)
                    flags|=QTC_NO_ETCH_BUTTON;

                drawPrimitive(PE_ButtonCommand, p, r, cg, flags);
                if (button->isDefault() && IND_CORNER==opts.defBtnIndicator)
                    drawPrimitive(PE_ButtonDefault, p, r, cg, flags);
                itsFormMode = false;
            }
            break;
        }
        case CE_CheckBox:
            itsFormMode = isFormWidget(widget);
            drawPrimitive(PE_Indicator, p, r, cg, flags, data);
            itsFormMode = false;
            break;
        case CE_RadioButton:
            itsFormMode=isFormWidget(widget);
            drawPrimitive(PE_ExclusiveIndicator, p, r, cg, flags, data);
            itsFormMode=false;
            break;
        default:
            KStyle::drawControl(control, p, widget, r, cg, flags, data);
    }
}

void QtCurveStyle::drawControlMask(ControlElement control, QPainter *p, const QWidget *widget,
                                   const QRect &r, const QStyleOption &data) const
{
    switch(control)
    {
        case CE_PushButton:
        case CE_MenuBarItem:
        {
            int offset(r.width()<QTC_MIN_BTN_SIZE || r.height()<QTC_MIN_BTN_SIZE ? 1 : 2);

            p->fillRect(r, color0);
            p->fillRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2, color1);
            p->setPen(color1);
            p->drawLine(r.x()+offset, r.y(), r.x()+r.width()-(offset+1), r.y());
            p->drawLine(r.x()+offset, r.y()+r.height()-1, r.x()+r.width()-(offset+1),
                        r.y()+r.height()-1);
            p->drawLine(r.x(), r.y()+offset, r.x(), r.y()+r.height()-(offset+1));
            p->drawLine(r.x()+r.width()-1, r.y()+offset, r.x()+r.width()-1,
                        r.y()+r.height()-(offset+1));
            break;
        }
        default:
            KStyle::drawControlMask(control, p, widget, r, data);
    }
}

void QtCurveStyle::drawComplexControlMask(ComplexControl control, QPainter *p, const QWidget *widget,
                                          const QRect &r, const QStyleOption &data) const
{
    switch (control)
    {
        case CC_SpinWidget:
        case CC_ComboBox:
        case CC_ToolButton:
            drawControlMask(CE_PushButton, p, widget, r, data);
            break;
        default:
            KStyle::drawComplexControlMask(control, p, widget, r, data);
    }
}

QRect QtCurveStyle::subRect(SubRect subrect, const QWidget *widget)const
{
    QRect rect,
          wrect(widget->rect());

    switch(subrect)
    {
        case SR_PushButtonFocusRect:
        {
            int dbw1(pixelMetric(PM_ButtonDefaultIndicator, widget)),
                dbw2(dbw1*2),
                border(3),
                border2=(border*2);

            rect.setRect(wrect.x()+border +dbw1, wrect.y()+border +dbw1,
                         wrect.width()-border2-dbw2,
                         wrect.height()-border2-dbw2);


            if(!isFormWidget(widget) && QTC_DO_EFFECT)
                rect.addCoords(0, 1, 0, -1);

            break;
        }
        case SR_ProgressBarGroove:
            rect=QRect(widget->rect());
            break;
        case SR_ProgressBarContents:
        case SR_ProgressBarLabel:
            rect=QRect(wrect.left()+2, wrect.top()+2, wrect.width()-4, wrect.height()-4);
            break;
        default:
            rect=KStyle::subRect(subrect, widget);
    }

    return rect;
}

void QtCurveStyle::drawComplexControl(ComplexControl control, QPainter *p, const QWidget *widget,
                                      const QRect &r, const QColorGroup &cg, SFlags flags,
                                      SCFlags controls, SCFlags active,
                                      const QStyleOption &data) const
{
    if(widget==itsHoverWidget)
        flags |=Style_MouseOver;

    switch(control)
    {
        case CC_ToolButton:
        {
            const QToolButton *toolbutton((const QToolButton *)widget);
            QRect             button(querySubControlMetrics(control, widget, SC_ToolButton, data)), 
                              menuarea(querySubControlMetrics(control, widget, SC_ToolButtonMenu,
                                       data));
            SFlags            bflags(flags|QTC_STD_TOOLBUTTON),
                              mflags(flags);

            if (APP_KORN==itsThemedApp)
            {
                drawPrimitive(PE_ButtonTool, p, button, cg, bflags, data);
                break;
            }

            const QToolBar *tb(widget->parentWidget()
                                   ? ::qt_cast<const QToolBar *>(widget->parentWidget()) : NULL);
            bool onControlButtons(false),
                 onExtender(!tb &&
                            widget->parentWidget() &&
                            widget->parentWidget()->inherits( "QToolBarExtensionWidget") &&
                            ::qt_cast<QToolBar *>(widget->parentWidget()->parentWidget()));

            if (!tb && !onExtender && widget->parentWidget() &&
                !qstrcmp(widget->parentWidget()->name(), "qt_maxcontrols"))
                onControlButtons = true;

            if(active & SC_ToolButton)
                bflags |=Style_Down;
            if(active & SC_ToolButtonMenu)
                mflags |=Style_Down;

            itsFormMode = isFormWidget(widget);

            if(controls&SC_ToolButton)
            {
                if(onControlButtons ||
                   (toolbutton->parentWidget() && toolbutton->parentWidget()->parentWidget() &&
                    ::qt_cast<const QMenuBar *>(toolbutton->parentWidget()->parentWidget())))
                    bflags|=QTC_NO_ETCH_BUTTON;

                // If we're pressed, on, or raised...
#if KDE_VERSION >= 0x30200
                if(bflags &(Style_Down | Style_On | Style_Raised) || onControlButtons)
#else
                if(bflags &(Style_Down | Style_On | Style_Raised | Style_MouseOver) ||
                   onControlButtons)
#endif
                {
                    //Make sure the standalone toolbuttons have a gradient in the right direction
                    if (!tb && !onControlButtons)
                        bflags |= Style_Horizontal;

                    if(tb)
                        if(Qt::Vertical==tb->orientation())
                            bflags|=QTC_VERTICAL_TB_BUTTON;
                        else
                            bflags|=Style_Horizontal;

                    if(toolbutton->isToggleButton())
                        bflags|=QTC_TOGGLE_BUTTON;

                    drawPrimitive(PE_ButtonTool, p, button, cg, bflags, data);
                }

                // Check whether to draw a background pixmap
                else if(APP_MACTOR!=itsThemedApp && toolbutton->parentWidget() &&
                          toolbutton->parentWidget()->backgroundPixmap() &&
                          !toolbutton->parentWidget()->backgroundPixmap()->isNull())
                    p->drawTiledPixmap(r, *(toolbutton->parentWidget()->backgroundPixmap()),
                                       toolbutton->pos());
                else if(widget->parent())
                {
                    QToolBar *tb(NULL);

                    if(::qt_cast<const QToolBar *>(widget->parent()))
                        tb=(QToolBar*)widget->parent();
                    else if(widget->parent()->inherits("QToolBarExtensionWidget"))
                    {
                        QWidget *parent=(QWidget*)widget->parent();

                        tb=(QToolBar*)parent->parent();
                    }

                    if(tb)
                    {
                        QRect tbr(tb->rect());
                        bool  horiz(Qt::Horizontal==tb->orientation());

                        if(!IS_FLAT(opts.toolbarAppearance))
                            if(horiz)
                                tbr.addCoords(0, -1, 0, 0);
                            else
                                tbr.addCoords(-1, 0, 0, 0);

                        drawMenuOrToolBarBackground(p, tbr, cg, false, horiz);
                    }
                }
            }

            if(controls&SC_ToolButtonMenu)
            {
                if(mflags &(Style_Down | Style_On | Style_Raised))
                    drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, data);
                drawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, data);
            }

            if(toolbutton->hasFocus() && !toolbutton->focusProxy())
            {
                QRect fr(toolbutton->rect());
                fr.addCoords(3, 3,-3,-3);
                drawPrimitive(PE_FocusRect, p, fr, cg);
            }

            itsFormMode=false;
            break;
        }
        case CC_ComboBox:
        {
            const QComboBox *combobox((const QComboBox *)widget);
            QRect           frame(QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
                                                                            SC_ComboBoxFrame,
                                                                            data), widget)),
                            arrow(QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
                                                                            SC_ComboBoxArrow,
                                                                            data), widget)),
                            field(QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
                                                                            SC_ComboBoxEditField,
                                                                            data), widget));
            const QColor    *use(buttonColors(cg));
            bool            editable(combobox->editable()),
                            sunken(combobox->listBox() ? combobox->listBox()->isShown() : false),
                            reverse(QApplication::reverseLayout());
            SFlags          fillFlags(flags);

            itsFormMode = isFormWidget(widget);

            if(sunken)
            {
                fillFlags|=Style_Sunken;
                if(fillFlags&Style_MouseOver)
                    fillFlags-=Style_MouseOver;
            }

            if(editable ||(!itsFormMode && QTC_DO_EFFECT && qstrcmp(widget->name(), kdeToolbarWidget)))
            {
                p->setPen(cg.background());
                p->drawRect(r);
            }

            if(controls&SC_ComboBoxFrame && frame.isValid())
            {
                if(editable && HOVER_CB_ARROW!=itsHover && fillFlags&Style_MouseOver)
                    fillFlags-=Style_MouseOver;

                if(opts.coloredMouseOver && fillFlags&Style_MouseOver && editable && !sunken)
                    frame.addCoords(reverse ? 0 : 1, 0, reverse ? 1 : 0, 0);

                drawLightBevel(p, frame, cg, fillFlags|Style_Raised|Style_Horizontal,
                               controls&SC_ComboBoxEditField && field.isValid() && editable
                                   ? (reverse ? ROUNDED_LEFT : ROUNDED_RIGHT) : ROUNDED_ALL,
                               getFill(fillFlags, use), use, true, true, WIDGET_STD_BUTTON);
            }

            if(controls&SC_ComboBoxArrow && arrow.isValid())
            {
                if(sunken)
                    arrow.addCoords(1, 1, 1, 1);
                drawPrimitive(PE_ArrowDown, p, arrow, cg, flags & ~Style_MouseOver);
            }

            if(controls&SC_ComboBoxEditField && field.isValid())
            {
                if(editable)
                {
                    field.addCoords(-1,-1, 0, 1);
                    p->setPen(flags&Style_Enabled ? cg.base() : cg.background());
                    p->drawRect(field);
                    field.addCoords(-2,-2, 2, 2);
                    drawEntryField(p, field, cg, fillFlags, flags&Style_Enabled &&
                                   (flags&Style_HasFocus), reverse ? ROUNDED_RIGHT : ROUNDED_LEFT,
                                   WIDGET_STD_BUTTON);
                }
                else 
                {
                    field.addCoords(1, sunken ? 2 : 1, sunken ? 2 : 1, -1);
                    p->setPen(use[QT_BORDER(flags&Style_Enabled)]);
                    p->drawLine(reverse ? field.left()-3 : field.right(), field.top(),
                                reverse ? field.left()-3 : field.right(), field.bottom());
                    if(!sunken)
                    {
                        p->setPen(use[0]);
                        p->drawLine(reverse ? field.left()-2 : field.right()+1, field.top(),
                                    reverse ? field.left()-2 : field.right()+1, field.bottom());
                    }
                }

                if((flags & Style_HasFocus) && !editable)
                {
                    QRect fr(QStyle::visualRect(subRect(SR_ComboBoxFocusRect, widget), widget));

                    if(reverse)
                        fr.addCoords(3, 0, 0, 0);
                    else
                        fr.addCoords(0, 0, -2, 0);

                    if(!itsFormMode && QTC_DO_EFFECT)
                        fr.addCoords(0, 1, 0, -1);
                    drawPrimitive(PE_FocusRect, p, fr, cg, flags | Style_FocusAtBorder,
                                  QStyleOption(cg.highlight()));
                }
            }

            p->setPen(flags & Style_Enabled ? cg.buttonText() : cg.mid());
            itsFormMode = false;
            break;
        }
        case CC_SpinWidget:
        {
            const QSpinWidget *spinwidget((const QSpinWidget *)widget);
            QRect             frame(querySubControlMetrics(CC_SpinWidget, widget, SC_SpinWidgetFrame,
                                    data)),
                              up(spinwidget->upRect()),
                              down(spinwidget->downRect());
            bool              hw(itsHoverWidget && itsHoverWidget==spinwidget),
                              reverse(QApplication::reverseLayout()),
                              doFrame((controls&SC_SpinWidgetFrame) && frame.isValid());

            if(flags&Style_MouseOver)
                flags-=Style_MouseOver;

            if(!reverse && doFrame)
            {
                itsFormMode = isFormWidget(widget);
                frame.setWidth(frame.width()+1);

                drawEntryField(p, frame, cg, flags,
                               spinwidget ? spinwidget->hasFocus() && (flags&Style_Enabled) : false,
                               ROUNDED_LEFT, WIDGET_SPIN);
                itsFormMode=false;
            }

            if((controls&SC_SpinWidgetUp) && up.isValid())
            {
                PrimitiveElement pe(PE_SpinWidgetUp);
                SFlags           upflags(flags);

                if(hw && HOVER_SW_UP==itsHover)
                    upflags|=Style_MouseOver;
                up.setHeight(up.height()+1);
                if(spinwidget->buttonSymbols()==QSpinWidget::PlusMinus)
                    pe=PE_SpinWidgetPlus;
                if(!spinwidget->isUpEnabled())
                    upflags^=Style_Enabled;
                drawPrimitive(pe, p, up, cg,
                             upflags |((active==SC_SpinWidgetUp)
                                 ? Style_On | Style_Sunken : Style_Raised));
            }

            if((controls&SC_SpinWidgetDown) && down.isValid())
            {
                PrimitiveElement pe(PE_SpinWidgetDown);
                SFlags           downflags(flags);

                if(hw && HOVER_SW_DOWN==itsHover)
                    downflags|=Style_MouseOver;
                if(spinwidget->buttonSymbols()==QSpinWidget::PlusMinus)
                    pe=PE_SpinWidgetMinus;
                if(!spinwidget->isDownEnabled())
                    downflags^=Style_Enabled;
                drawPrimitive(pe, p, down, cg,
                              downflags |((active==SC_SpinWidgetDown)
                                  ? Style_On | Style_Sunken : Style_Raised));
            }

            if(reverse && doFrame)
            {
                itsFormMode = isFormWidget(widget);
                frame.setWidth(frame.width()+1);
                drawEntryField(p, frame, cg, flags,
                               spinwidget ? spinwidget->hasFocus() && (flags&Style_Enabled) : false,
                               ROUNDED_RIGHT, WIDGET_SPIN);
                itsFormMode=false;
            }
            break;
        }
        case CC_ScrollBar:
        {
            const QScrollBar *sb((const QScrollBar *)widget);
            bool             hw(itsHoverWidget && itsHoverWidget==sb),
                             useThreeButtonScrollBar(SCROLLBAR_KDE==opts.scrollbarType),
                             horiz(Qt::Horizontal==sb->orientation()),
                             maxed(sb->minValue() == sb->maxValue()),
                             atMin(maxed || sb->value()==sb->minValue()),
                             atMax(maxed || sb->value()==sb->maxValue());
            SFlags           sflags((horiz ? Style_Horizontal : Style_Default) |
                                    (maxed ? Style_Default : Style_Enabled));
            QRect            subline(querySubControlMetrics(control, widget, SC_ScrollBarSubLine,
                                                            data)),
                             addline(querySubControlMetrics(control, widget, SC_ScrollBarAddLine,
                                                            data)),
                             subpage(querySubControlMetrics(control, widget, SC_ScrollBarSubPage,
                                                            data)),
                             addpage(querySubControlMetrics(control, widget, SC_ScrollBarAddPage,
                                                            data)),
                             slider(querySubControlMetrics(control, widget, SC_ScrollBarSlider,
                                                           data)),
                             first(querySubControlMetrics(control, widget, SC_ScrollBarFirst,
                                                          data)),
                             last(querySubControlMetrics(control, widget, SC_ScrollBarLast,
                                                         data)),
                             subline2(addline),
                             sbRect(sb->rect());

            itsFormMode=isFormWidget(widget);

            if(itsFormMode)
            {
                // See KHTML note at top of file
                if(horiz)
                {
                    subline.addCoords(0, 0, 0, -1);
                    addline.addCoords(0, 0, 0, -1);
                    subpage.addCoords(0, 0, 0, -1);
                    addpage.addCoords(0, 0, 0, -1);
                    slider.addCoords(0, 0, 0, -1);
                    first.addCoords(0, 0, 0, -1);
                    last.addCoords(0, 0, 0, -1);
                    subline2.addCoords(0, 0, 0, -1);
                    sbRect.addCoords(0, 0, 0, -1);
                }
                else
                {
                    subline.addCoords(0, 0, -1, 0);
                    addline.addCoords(0, 0, -1, 0);
                    subpage.addCoords(0, 0, -1, 0);
                    addpage.addCoords(0, 0, -1, 0);
                    slider.addCoords(0, 0, -1, 0);
                    first.addCoords(0, 0, -1, 0);
                    last.addCoords(0, 0, -1, 0);
                    subline2.addCoords(0, 0, -1, 0);
                    sbRect.addCoords(0, 0, -1, 0);
                }

#ifndef QTC_SIMPLE_SCROLLBARS
                if(sbRect.isValid() && SCROLLBAR_NONE==opts.scrollbarType)
                    if(horiz)
                        sbRect.addCoords(0, 0, -1, 0);
                    else
                        sbRect.addCoords(0, 0, 0, -1);
#endif
            }

            if (useThreeButtonScrollBar)
                if (horiz)
                    subline2.moveBy(-addline.width(), 0);
                else
                    subline2.moveBy(0, -addline.height());

            // Draw trough...
            const QColor *trough(itsBackgroundCols); // backgroundColors(cg));
            bool  noButtons(SCROLLBAR_NONE==opts.scrollbarType);
            QRect s2(subpage), a2(addpage);

#ifndef QTC_SIMPLE_SCROLLBARS
            if(noButtons)
            {
                // Increase clipping to allow trough to "bleed" into slider corners...
                a2.addCoords(-3, -3, 3, 3);
                s2.addCoords(-3, -3, 3, 3);
            }
#endif

            p->setClipRegion(QRegion(s2)+QRegion(addpage));
            drawLightBevel(p, sbRect, cg, sflags|Style_Down,
#ifndef QTC_SIMPLE_SCROLLBARS
                           SCROLLBAR_NONE==opts.scrollbarType ? ROUNDED_ALL :
#endif
                           ROUNDED_NONE,
                           trough[2], trough, true, true, WIDGET_TROUGH);
            p->setClipping(false);

            if(/*(controls&SC_ScrollBarSubLine) && */subline.isValid())
            {
                bool enable=(!maxed && sb->value()!=sb->minValue());

                drawPrimitive(PE_ScrollBarSubLine, p, subline, cg, sflags |
                                 //(enable ? Style_Enabled : Style_Default) |
                                 (enable && hw && HOVER_SB_SUB==itsHover
                                       ? Style_MouseOver : Style_Default) |
                                 (enable && (!hw || HOVER_SB_SUB==itsHover || HOVER_NONE==itsHover)
                                   && SC_ScrollBarSubLine==active ? Style_Down : Style_Default));

                if (useThreeButtonScrollBar && subline2.isValid())
                    drawPrimitive(PE_ScrollBarSubLine, p, subline2, cg, sflags |
                                 //(enable ? Style_Enabled : Style_Default) |
                                 (enable && hw && HOVER_SB_SUB2==itsHover
                                     ? Style_MouseOver : Style_Default) |
                                 (enable && (!hw || HOVER_SB_SUB2==itsHover || HOVER_NONE==itsHover)
                                   && SC_ScrollBarSubLine==active ? Style_Down : Style_Default));
            }
            if(/*(controls&SC_ScrollBarAddLine) && */addline.isValid())
            {
                bool enable=(!maxed && sb->value()!=sb->maxValue());

                // See KHTML note at top of file
                if(itsFormMode && SCROLLBAR_NEXT!=opts.scrollbarType)
                    if(horiz)
                        addline.addCoords(0, 0, -1, 0);
                    else
                        addline.addCoords(0, 0, 0, -1);

                drawPrimitive(PE_ScrollBarAddLine, p, addline, cg, sflags |
                                 //(enable ? Style_Enabled : Style_Default) |
                                 (enable && hw && HOVER_SB_ADD==itsHover
                                     ? Style_MouseOver : Style_Default) |
                                 (enable && SC_ScrollBarAddLine==active
                                     ? Style_Down : Style_Default));
            }

            if((controls&SC_ScrollBarFirst) && first.isValid())
                drawPrimitive(PE_ScrollBarFirst, p, first, cg, sflags |
                                 //(maxed ? Style_Default : Style_Enabled) |
                                 (!maxed && SC_ScrollBarFirst==active ? Style_Down : Style_Default));

            if((controls&SC_ScrollBarLast) && last.isValid())
                drawPrimitive(PE_ScrollBarLast, p, last, cg, sflags |
                                 //(maxed ? Style_Default : Style_Enabled) |
                                 (!maxed && SC_ScrollBarLast==active ? Style_Down : Style_Default));

            if(((controls&SC_ScrollBarSlider) || noButtons) && slider.isValid())
            {
                // If "SC_ScrollBarSlider" wasn't pecified, then we only want to draw the portion
                // of the slider that overlaps with the tough. So, once again set the clipping
                // region...
                if(!(controls&SC_ScrollBarSlider))
                    p->setClipRegion(QRegion(s2)+QRegion(addpage));
#ifdef QTC_INCREASE_SB_SLIDER
                else
                {
                    if(atMax)
                        switch(opts.scrollbarType)
                        {
                            case SCROLLBAR_KDE:
                            case SCROLLBAR_WINDOWS:
                            case SCROLLBAR_PLATINUM:
                                if(horiz)
                                    slider.addCoords(0, 0, 1, 0);
                                else
                                    slider.addCoords(0, 0, 0, 1);
                            default:
                                break;
                        }
                    if(atMin)
                        switch(opts.scrollbarType)
                        {
                            case SCROLLBAR_KDE:
                            case SCROLLBAR_WINDOWS:
                            case SCROLLBAR_NEXT:
                                if(horiz)
                                    slider.addCoords(-1, 0, 0, 0);
                                else
                                    slider.addCoords(0, -1, 0, 0);
                            default:
                                break;
                        }
                }
#endif

                drawPrimitive(PE_ScrollBarSlider, p, slider, cg, sflags |
                                 //(maxed ? Style_Default : Style_Enabled) |
                                 (!maxed && hw && HOVER_SB_SLIDER==itsHover
                                     ? Style_MouseOver : Style_Default) |
                                 (!maxed && SC_ScrollBarSlider==active
                                     ? Style_Down : Style_Default));

                // ### perhaps this should not be able to accept focus if maxedOut?
                if(sb->hasFocus())
                    drawPrimitive(PE_FocusRect, p, QRect(slider.x()+2, slider.y()+2,
                                  slider.width()-5, slider.height()-5), cg, Style_Default);

#ifndef QTC_SIMPLE_SCROLLBARS
                if(noButtons && (!atMin || !atMax))
                {
                    p->setPen(backgroundColors(cg)[5]);

                    if(horiz)
                    {
                        if(!atMin)
                        {
                            p->drawLine(slider.x(), slider.y(),
                                        slider.x()+1, slider.y());
                            p->drawLine(slider.x(), slider.y()+slider.height()-1,
                                        slider.x()+1, slider.y()+slider.height()-1);
                        }
                        if(!atMax)
                        {
                            p->drawLine(slider.x()+slider.width()-1, slider.y(),
                                        slider.x()+slider.width()-2, slider.y());
                            p->drawLine(slider.x()+slider.width()-1,
                                        slider.y()+slider.height()-1,
                                        slider.x()+slider.width()-2,
                                        slider.y()+slider.height()-1);
                        }
                    }
                    else
                    {
                        if(!atMin)
                        {
                            p->drawLine(slider.x(), slider.y(),
                                        slider.x(), slider.y()+1);
                            p->drawLine(slider.x()+slider.width()-1, slider.y(),
                                        slider.x()+slider.width()-1, slider.y()+1);
                        }
                        if(!atMax)
                        {
                            p->drawLine(slider.x(), slider.y()+slider.height()-1,
                                        slider.x(), slider.y()+slider.height()-2);
                            p->drawLine(slider.x()+slider.width()-1,
                                        slider.y()+slider.height()-1,
                                        slider.x()+slider.width()-1,
                                        slider.y()+slider.height()-2);
                        }
                    }
                }
#endif
                if(!(controls&SC_ScrollBarSlider))
                    p->setClipping(false);
            }
            break;
        }
        case CC_Slider:
        //
        // Note: Can't use KStyle's drawing routine, as this doesnt work for sliders on gradient
        //       toolbars. It also draws groove, focus, slider - wherease QtCurve needs groove,
        //       slider, focus. We also ony double-buffer if not on a toolbar, as we dont know
        //       the background, etc, if on a toolbar - and that is handled in eventFilter
        {
            bool     tb(!IS_FLAT(opts.toolbarAppearance) && widget &&
                        0==qstrcmp(widget->name(), kdeToolbarWidget));
            QRect    groove=querySubControlMetrics(CC_Slider, widget, SC_SliderGroove, data),
                     handle=querySubControlMetrics(CC_Slider, widget, SC_SliderHandle, data);
            QPixmap  pix(widget->size());
            QPainter p2,
                     *paint(tb ? p : &p2);

            if(!tb)
            {
                paint->begin(&pix);
                if (widget->parentWidget() && widget->parentWidget()->backgroundPixmap() &&
                    !widget->parentWidget()->backgroundPixmap()->isNull())
                    paint->drawTiledPixmap(r, *(widget->parentWidget()->backgroundPixmap()), widget->pos());
                else
                    pix.fill(cg.background());
            }

            if((controls & SC_SliderGroove)&& groove.isValid())
                drawSliderGroove(paint, groove, cg, flags, widget);
            if((controls & SC_SliderHandle)&& handle.isValid())
                drawSliderHandle(paint, handle, cg, flags, tb);
            if(controls & SC_SliderTickmarks)
                QCommonStyle::drawComplexControl(control, paint, widget, r, cg, flags, SC_SliderTickmarks,
                                                 active, data);

            if(flags & Style_HasFocus)
                drawPrimitive(PE_FocusRect, paint, groove, cg);

            if(!tb)
            {
                paint->end();
                bitBlt((QWidget*)widget, r.x(), r.y(), &pix);
            }
            break;
        }
        default:
            KStyle::drawComplexControl(control, p, widget, r, cg, flags, controls, active, data);
    }
}

QRect QtCurveStyle::querySubControlMetrics(ComplexControl control, const QWidget *widget,
                                           SubControl sc, const QStyleOption &data) const
{
    bool reverse(QApplication::reverseLayout());

    switch(control)
    {
        case CC_SpinWidget:
        {
            if(!widget)
                return QRect();

            int   fw(pixelMetric(PM_SpinBoxFrameWidth, 0));
            QSize bs;

            bs.setHeight(widget->height()>>1);
            if(bs.height()< 8)
                bs.setHeight(8);
            bs.setWidth(15);
            bs=bs.expandedTo(QApplication::globalStrut());

            int extra(bs.height()*2==widget->height()? 0 : 1),
                y(0), x(widget->width()-bs.width()),
                rx(x-fw*2);

            switch(sc)
            {
                case SC_SpinWidgetUp:
                    return QRect(x, y, bs.width(), bs.height());
                case SC_SpinWidgetDown:
                    return QRect(x, y+bs.height(), bs.width(), bs.height()+extra);
                case SC_SpinWidgetButtonField:
                    return QRect(x, y, bs.width(), widget->height()-2*fw);
                case SC_SpinWidgetEditField:
                    return QRect(fw, fw, rx, widget->height()-2*fw);
                case SC_SpinWidgetFrame:
                    return reverse
                              ? QRect(widget->x()+bs.width(), widget->y(),
                                      widget->width()-bs.width()-1, widget->height())
                              : QRect(widget->x(), widget->y(),
                                      widget->width()-bs.width(),widget->height());
                default:
                    break; // Remove compiler warnings...
            }
        }
        case CC_ComboBox:
        {
            QRect r(KStyle::querySubControlMetrics(control, widget, sc, data));

            if(SC_ComboBoxFrame==sc)
            {
                const QComboBox *cb(::qt_cast<const QComboBox *>(widget));

                if(cb && cb->editable())
                    r=QRect((r.x()+r.width()-1)-18, r.y(), 19, r.height());
            }
            else if (reverse && SC_ComboBoxEditField==sc)
                r.addCoords(-1, 0, -2, 0);
            return r;
        }
        case CC_ScrollBar:
        {
            // Taken from kstyle.cpp, and modified so as to allow for no scrollbar butttons...
            bool  threeButtonScrollBar(SCROLLBAR_KDE==opts.scrollbarType),
                  platinumScrollBar(SCROLLBAR_PLATINUM==opts.scrollbarType),
                  nextScrollBar(SCROLLBAR_NEXT==opts.scrollbarType),
                  noButtons(SCROLLBAR_NONE==opts.scrollbarType);
            QRect ret;
            const QScrollBar *sb((const QScrollBar*)widget);
            bool  horizontal(sb->orientation() == Qt::Horizontal);
            int   sliderstart(sb->sliderStart()),
                  sbextent(pixelMetric(PM_ScrollBarExtent, widget)),
                  maxlen((horizontal ? sb->width() : sb->height())
                                - (noButtons ? 0 : (sbextent * (threeButtonScrollBar ? 3 : 2)))),
                  sliderlen;

            // calculate slider length
            if (sb->maxValue() != sb->minValue())
            {
                uint range = sb->maxValue() - sb->minValue();
                sliderlen = (sb->pageStep() * maxlen) / (range + sb->pageStep());

                int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
                if ( sliderlen < slidermin || range > INT_MAX / 2 )
                    sliderlen = slidermin;
                if ( sliderlen > maxlen )
                    sliderlen = maxlen;
            }
            else
                sliderlen = maxlen;

            // Subcontrols
            switch(sc)
            {
                case SC_ScrollBarSubLine:
                    if(noButtons)
                        return QRect();

                    // top/left button
                    if (platinumScrollBar)
                        if (horizontal)
                            ret.setRect(sb->width() - 2 * sbextent, 0, sbextent, sbextent);
                        else
                            ret.setRect(0, sb->height() - 2 * sbextent, sbextent, sbextent);
                    else
                        ret.setRect(0, 0, sbextent, sbextent);
                    break;
                case SC_ScrollBarAddLine:
                    if(noButtons)
                        return QRect();

                    // bottom/right button
                    if (nextScrollBar)
                        if (horizontal)
                            ret.setRect(sbextent, 0, sbextent, sbextent);
                        else
                            ret.setRect(0, sbextent, sbextent, sbextent);
                    else
                        if (horizontal)
                            ret.setRect(sb->width() - sbextent, 0, sbextent, sbextent);
                        else
                            ret.setRect(0, sb->height() - sbextent, sbextent, sbextent);
                    break;
                case SC_ScrollBarSubPage:
                    // between top/left button and slider
                    if (platinumScrollBar)
                        if (horizontal)
                            ret.setRect(0, 0, sliderstart, sbextent);
                        else
                            ret.setRect(0, 0, sbextent, sliderstart);
                    else if (nextScrollBar)
                        if (horizontal)
                            ret.setRect(sbextent*2, 0, sliderstart-2*sbextent, sbextent);
                        else
                            ret.setRect(0, sbextent*2, sbextent, sliderstart-2*sbextent);
                    else
                        if (horizontal)
                            ret.setRect(noButtons ? 0 : sbextent, 0,
                                        noButtons ? sliderstart
                                                  : (sliderstart - sbextent), sbextent);
                        else
                            ret.setRect(0, noButtons ? 0 : sbextent, sbextent,
                                        noButtons ? sliderstart : (sliderstart - sbextent));
                    break;
                case SC_ScrollBarAddPage:
                {
                    // between bottom/right button and slider
                    int fudge;

                    if (platinumScrollBar)
                        fudge = 0;
                    else if (nextScrollBar)
                        fudge = 2*sbextent;
                    else if(noButtons)
                        fudge = 0;
                    else
                        fudge = sbextent;

                    if (horizontal)
                        ret.setRect(sliderstart + sliderlen, 0,
                                    maxlen - sliderstart - sliderlen + fudge, sbextent);
                    else
                        ret.setRect(0, sliderstart + sliderlen, sbextent,
                                    maxlen - sliderstart - sliderlen + fudge);
                    break;
                }
                case SC_ScrollBarGroove:
                    if(noButtons)
                    {
                        if (horizontal)
                            ret.setRect(0, 0, sb->width(), sb->height());
                        else
                            ret.setRect(0, 0, sb->width(), sb->height());
                    }
                    else
                    {
                        int multi = threeButtonScrollBar ? 3 : 2,
                            fudge;

                        if (platinumScrollBar)
                            fudge = 0;
                        else if (nextScrollBar)
                            fudge = 2*sbextent;
                        else
                            fudge = sbextent;

                        if (horizontal)
                            ret.setRect(fudge, 0, sb->width() - sbextent * multi, sb->height());
                        else
                            ret.setRect(0, fudge, sb->width(), sb->height() - sbextent * multi);
                    }
                    break;
                case SC_ScrollBarSlider:
                    if (horizontal)
                        ret.setRect(sliderstart, 0, sliderlen, sbextent);
                    else
                        ret.setRect(0, sliderstart, sbextent, sliderlen);
                    break;
                default:
                    ret = QCommonStyle::querySubControlMetrics(control, widget, sc, data);
                    break;
            }
            return ret;
        }
        default:
            break; // Remove compiler warnings...
    }

    return KStyle::querySubControlMetrics(control, widget, sc, data);
}

int QtCurveStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    switch(metric)
    {
        case PM_MenuButtonIndicator:
            return 7;
        case PM_ButtonMargin:
            return 3;
#if QT_VERSION >= 0x030200
        case PM_TabBarTabShiftVertical:
        {
            const QTabBar *tb=widget ? ::qt_cast<const QTabBar *>(widget) : 0;

            return tb
                    ? QTabBar::RoundedAbove==tb->shape() || QTabBar::TriangularAbove==tb->shape()
                       ? 1
                       : -1
                    : KStyle::pixelMetric(metric, widget);
        }
        case PM_TabBarTabShiftHorizontal:
            return 0;
#endif
        case PM_ButtonShiftHorizontal:
        case PM_ButtonShiftVertical:
            return 1;
        case PM_ButtonDefaultIndicator:
            return 0;
        case PM_DefaultFrameWidth:
            if(QTC_DO_EFFECT && widget && !isFormWidget(widget) &&
               (::qt_cast<const QLineEdit *>(widget) || ::qt_cast<QDateTimeEditBase*>(widget) ||
                ::qt_cast<QTextEdit*>(widget)))
                return 3;
            else
                return 2;
        case PM_SpinBoxFrameWidth:
            return QTC_DO_EFFECT && !isFormWidget(widget) ? 3 : 2;
        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            return QTC_CHECK_SIZE;
        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
            return QTC_RADIO_SIZE;
        case PM_TabBarTabOverlap:
            return 1;
        case PM_ProgressBarChunkWidth:
            return 4;
        case PM_DockWindowSeparatorExtent:
            return 4;
        case PM_DockWindowHandleExtent:
            return 10;
        case PM_SplitterWidth:
            return 6;
        case PM_ScrollBarSliderMin:
            return 16;
        case PM_SliderThickness:
            return 18;
        case PM_SliderControlThickness:
            return 15; // This equates to 13, as we draw the handle 2 pix smaller for focus rect...
        case PM_SliderLength:
            return 21;
        case PM_ScrollBarExtent:
            // See KHTML note at top of file
            return APP_KPRESENTER==itsThemedApp ||
                   ((APP_KONQUEROR==itsThemedApp || APP_KONTACT==itsThemedApp) && (!widget || isFormWidget(widget)))
                        ? 16 : 15;
        case PM_MaximumDragDistance:
            return -1;
        case PM_TabBarTabVSpace:
            return opts.highlightTab ? 11 : 9;
        default:
            return KStyle::pixelMetric(metric, widget);
    }
}

int QtCurveStyle::kPixelMetric(KStylePixelMetric kpm, const QWidget *widget) const
{
    switch(kpm)
    {
        case KPM_MenuItemSeparatorHeight:
            return 2;
        default:
            return KStyle::kPixelMetric(kpm, widget);
    }
}

QSize QtCurveStyle::sizeFromContents(ContentsType contents, const QWidget *widget,
                                     const QSize &contentsSize, const QStyleOption &data) const
{
    switch(contents)
    {
        case CT_PushButton:
        {
            const QPushButton *button(static_cast<const QPushButton *>(widget));

            if (button && !button->text().isEmpty())
            {
                bool allowEtch(QTC_DO_EFFECT && !isFormWidget(widget));

                const int constMinH(allowEtch ? 29 : 27);

                int margin(2*pixelMetric(PM_ButtonMargin, widget)),
                    mbi(button->isMenuButton() ? pixelMetric(PM_MenuButtonIndicator, widget) : 0),
                    w(contentsSize.width() + margin + mbi + 16),
                    h(contentsSize.height() + margin);

//                 if(button->text()=="...")
//                     w+=24;
//                 else
                if("..."!=button->text())
                {
                    const int constMinW(84);

                    if(opts.embolden)
                        w+=6; // add room for bold font - Assume all buttons can be default!
                    if(w<constMinW)
                        w=constMinW;
                }

                if(allowEtch)
                    h+=2;

                return QSize(w, h < constMinH ? constMinH : h);
            }
            break;
        }
        case CT_ComboBox:
        {
            bool allowEtch(QTC_DO_EFFECT && !isFormWidget(widget));

            const int constMinH(allowEtch ? 26 : 24);

            QSize sz(KStyle::sizeFromContents(contents, widget, contentsSize, data));
            int   h(sz.height());

            if(allowEtch)
                h+=2;

            return QSize(sz.width(), h<constMinH ? constMinH : h);
        }
        case CT_PopupMenuItem:
        {
            if (!widget || data.isDefault())
                break;

            const int        constMinH(opts.thinnerMenuItems ? 25 : 27);

            QMenuItem        *mi(data.menuItem());
            const QPopupMenu *popupmenu(static_cast<const QPopupMenu *>(widget));
            int              maxpmw(data.maxIconWidth()),
                             w(contentsSize.width()), h(contentsSize.height());

            if (mi->custom())
            {
                w = mi->custom()->sizeHint().width();
                h = mi->custom()->sizeHint().height();

                if (!mi->custom()->fullSpan() && h < constMinH)
                    h = constMinH;
            }
            else if(mi->widget())
                ;
            else if (mi->isSeparator())
            {
                w = 10;
                h = opts.thinnerMenuItems ? 6 : 8;
            }
            else
            {
                // check is at least 16x16
                if (h < 16)
                    h = 16;
                if (mi->pixmap())
                    h = QMAX(h, mi->pixmap()->height());
                else if (!mi->text().isNull())
                    h = QMAX(h, popupmenu->fontMetrics().height() + 2);
                if (mi->iconSet()!= 0)
                    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal).height());
                h+=(opts.thinnerMenuItems ? 2 : 4);
            }

            // check | 4 pixels | item | 8 pixels | accel | 4 pixels | check

            // check is at least 16x16
            maxpmw=QMAX(maxpmw, 16);
            w += (maxpmw * 2) + 8;

            if (! mi->text().isNull() && mi->text().find('\t') >= 0)
                w += 8;

            return QSize(w, h);
        }
        case CT_SpinBox:
        {
            QSize size(KStyle::sizeFromContents(contents, widget, contentsSize, data));

            if(!(size.height()%2))
                size.setHeight(size.height()+1);

//             if(!isFormWidget(widget))
//                 size.setHeight(size.height()+2);

            return size;
        }
        case CT_ToolButton:
            if(widget->parent() && ::qt_cast<QToolBar *>(widget->parent()))
                return QSize(contentsSize.width()+8, contentsSize.height()+8);
        default:
            break;  // Remove compiler warnings...
    }

    return KStyle::sizeFromContents(contents, widget, contentsSize, data);
}

int QtCurveStyle::styleHint(StyleHint stylehint, const QWidget *widget, const QStyleOption &option,
                            QStyleHintReturn *returnData) const
{
    switch(stylehint)
    {
        case SH_ScrollView_FrameOnlyAroundContents:
            return opts.gtkScrollViews;
        case SH_EtchDisabledText:
        case SH_Slider_SnapToValue:
        case SH_PrintDialog_RightAlignButtons:
        case SH_FontDialog_SelectAssociatedText:
        case SH_MenuBar_AltKeyNavigation:
        case SH_PopupMenu_MouseTracking:
        case SH_PopupMenu_SpaceActivatesItem:
        case SH_ComboBox_ListMouseTracking:
        case SH_ScrollBar_MiddleClickAbsolutePosition:
            return 1;
        case SH_LineEdit_PasswordCharacter:
            if(opts.passwordChar)
            {
                int                chars[4]={opts.passwordChar, 0x25CF, 0x2022, 0};
                const QFontMetrics &fm(widget ? widget->fontMetrics() : QFontMetrics(QFont()));

                for(int i=0; chars[i]; ++i)
                    if (fm.inFont(QChar(chars[i])))
                        return chars[i];
                return '*';
            }
            else
                return '\0';
        case SH_MainWindow_SpaceBelowMenuBar:
            return 0;
        case SH_PopupMenu_AllowActiveAndDisabled:
            return 0;
        case SH_MenuBar_MouseTracking:
            return opts.menubarMouseOver ? 1 : 0;
        default:
            return KStyle::styleHint(stylehint, widget, option, returnData);
    }
}

void QtCurveStyle::drawMenuItem(QPainter *p, const QRect &r, const QColorGroup &cg,
                                bool mbi, int round, const QColor &bgnd, const QColor *cols) const
{
    if(opts.borderMenuitems)
    {
        int  flags(Style_Raised);
        bool stdColor(!mbi || SHADE_BLEND_SELECTED!=opts.shadeMenubars);

        flags|=Style_Horizontal;

        if(stdColor)
            drawLightBevel(bgnd, p, r, cg, flags, round, cols[ORIGINAL_SHADE],
                           cols, stdColor, !(mbi && IS_GLASS(opts.menubarAppearance)), WIDGET_MENU_ITEM);
        else
        {
            QRect fr(r);

            fr.addCoords(1, 1, -1, -1);

            if(fr.width()>0 && fr.height()>0)
                drawBevelGradient(cols[ORIGINAL_SHADE], true, p, fr, true,
                                  getWidgetShade(WIDGET_MENU_ITEM, true, false, opts.menuitemAppearance),
                                  getWidgetShade(WIDGET_MENU_ITEM, false, false, opts.menuitemAppearance),
                                  false, opts.menuitemAppearance, WIDGET_MENU_ITEM);
            drawBorder(bgnd, p, r, cg, flags, round, cols, WIDGET_OTHER, false, BORDER_FLAT, false, 0);
        }
    }
    else
        drawBevelGradient(cols[ORIGINAL_SHADE], true, p, r, true,
                          getWidgetShade(WIDGET_MENU_ITEM, true, false, opts.menuitemAppearance),
                          getWidgetShade(WIDGET_MENU_ITEM, false, false, opts.menuitemAppearance),
                          false, opts.menuitemAppearance, WIDGET_MENU_ITEM);
}

void QtCurveStyle::drawProgress(QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags,
                                int round, const QWidget *widget) const
{
    if(r.width()<1)
        return;

    int     minWidth(3);
    bool    drawFull(r.width()>minWidth),
            drawStripe(r.width()>(minWidth*1.5));
    QRegion outer(r);

    if(drawStripe)
    {
        int animShift=-PROGRESS_CHUNK_WIDTH;

        if (opts.animatedProgress)
        {
            // find the animation Offset for the current Widget
            QWidget                          *nonConstWidget(const_cast<QWidget*>(widget));
            QMapConstIterator<QWidget*, int> it(itsProgAnimWidgets.find(nonConstWidget));

            if (it!=itsProgAnimWidgets.end())
                animShift += it.data();
        }

        if(opts.stripedProgress)
            for(int offset=0; offset<(r.width()+PROGRESS_CHUNK_WIDTH);
                offset+=(PROGRESS_CHUNK_WIDTH*2))
            {
                QRect   r2(r.x()+offset+animShift, r.y(), PROGRESS_CHUNK_WIDTH, r.height());
                QRegion inner(r2);

                outer=outer.eor(inner);
            }
    }

    if(drawFull)
    {
        flags|=Style_Raised|Style_Horizontal;

        drawLightBevel(cg.background(), p, r, cg, flags, round, itsMenuitemCols[ORIGINAL_SHADE],
                       itsMenuitemCols, true, true, WIDGET_PROGRESSBAR);

        if(drawStripe && opts.stripedProgress)
        {
            p->setClipRegion(outer);
            drawLightBevel(cg.background(), p, r, cg, flags, round, itsMenuitemCols[1],
                           itsMenuitemCols, true, true, WIDGET_PROGRESSBAR);
            p->setClipping(false);
        }
    }
    else
    {
        p->setPen(itsMenuitemCols[5]);
        p->setBrush(itsMenuitemCols[ORIGINAL_SHADE]);
        p->drawRect(r);
    }
    if(QTC_ROUNDED && r.width()>2 && ROUNDED_ALL!=round)
    {
        p->setPen(midColor(cg.background(), itsMenuitemCols[5]));
        if(!(round&CORNER_TL) || !drawFull)
            p->drawPoint(r.x(), r.y());
        if(!(round&CORNER_BL) || !drawFull)
            p->drawPoint(r.x(), r.y()+r.height()-1);
        if(!(round&CORNER_TR) || !drawFull)
            p->drawPoint(r.x()+r.width()-1, r.y());
        if(!(round&CORNER_BR) || !drawFull)
            p->drawPoint(r.x()+r.width()-1, r.y()+r.height()-1);
    }
}

void QtCurveStyle::drawBevelGradient(const QColor &base, bool increase, QPainter *p,
                                     const QRect &origRect, bool horiz, double shadeTop,
                                     double shadeBot, bool sel, EAppearance bevApp, EWidget w) const
{
    if(IS_FLAT(bevApp))
        p->fillRect(origRect, base);
    else
    {
        EAppearance app(APPEARANCE_BEVELLED!=bevApp || WIDGET_BUTTON(w) || WIDGET_LISTVIEW_HEADER==w
                            ? bevApp
                            : APPEARANCE_GRADIENT);

        QRect   r(0, 0, horiz ? QTC_PIXMAP_DIMENSION : origRect.width(),
                        horiz ? origRect.height() : QTC_PIXMAP_DIMENSION);
        QString key(createKey(horiz ? r.height() : r.width(), base.rgb(), horiz, increase,
                              app2App(app, sel), w, shadeTop, shadeBot));
        QPixmap *pix(itsPixmapCache.find(key));

        if(!pix)
        {
            pix=new QPixmap(r.width(), r.height());

            QPainter pixPainter(pix);

            if(!sel && IS_GLASS(app))
            {
                if(WIDGET_TAB_BOT==w)
                {
                    double t(shadeTop);
                    shadeTop=shadeBot;
                    shadeBot=t;
                }

                double shadeTopA(WIDGET_TAB_BOT==w ? 1.0 : shadeTop*SHADE_GLASS_TOP_A(app, w)),
                       shadeTopB(WIDGET_TAB_BOT==w ? 1.0 : shadeTop*SHADE_GLASS_TOP_B(app, w)),
                       shadeBotA(WIDGET_TAB_TOP==w ? 1.0 : shadeBot*SHADE_GLASS_BOT_A(app)),
                       shadeBotB(WIDGET_TAB_TOP==w ? 1.0 : shadeBot*SHADE_GLASS_BOT_B(app));

                QColor topA, topB, botA, botB;
                QRect  r1(r), r2(r), r3(r);

                shade(base, &topA, shadeTopA);
                shade(base, &topB, shadeTopB);
                shade(base, &botA, shadeBotA);
                shade(base, &botB, shadeBotB);

                if(horiz)
                {
                    r1.setHeight(r1.height()/2);
                    r2.setY(r2.y()+r1.height());
                }
                else
                {
                    r1.setWidth(r1.width()/2);
                    r2.setX(r2.x()+r1.width());
                }
                drawGradient(topA, topB, increase, &pixPainter, r1, horiz);
                drawGradient(botA, botB, increase, &pixPainter, r2, horiz);
            }
            else if(!sel && APPEARANCE_BEVELLED==app &&
                    ((horiz ? r.height()
                            : r.width()) > (((WIDGET_BUTTON(w) ? 2 : 1)*BEVEL_BORDER(w))+4)))
            {
                if(WIDGET_LISTVIEW_HEADER==w)
                {
                    QColor bot;
                    QRect  r1(r), r2(r);

                    if(horiz)
                    {
                        r2.setHeight(BEVEL_BORDER(w));
                        r1.setHeight(r.height()-r2.height());
                        r2.moveTop(r.y()+r1.height());
                    }
                    else
                    {
                        r2.setWidth(BEVEL_BORDER(w));
                        r1.setWidth(r.width()-r2.width());
                        r2.moveLeft(r.x()+r1.width());
                    }
                    shade(base, &bot, SHADE_BEVEL_BOT(w));
                    pixPainter.fillRect(r1, base);
                    drawGradient(base, bot, true, &pixPainter, r2, horiz);
                }
                else
                {
                    QColor bot, midTop, midBot, top;
                    QRect  r1(r), r2(r), r3(r);

                    if(horiz)
                    {
                        r1.setHeight(BEVEL_BORDER(w));
                        r3.setHeight(BEVEL_BORDER(w));
                        r2.setHeight(r.height()-(r1.height()+r3.height()));
                        r2.moveTop(r.y()+r1.height());
                        r3.moveTop(r.y()+r1.height()+r2.height());
                    }
                    else
                    {
                        r1.setWidth(BEVEL_BORDER(w));
                        r3.setWidth(BEVEL_BORDER(w));
                        r2.setWidth(r.width()-(r1.width()+r3.width()));
                        r2.moveLeft(r.x()+r1.width());
                        r3.moveLeft(r.x()+r1.width()+r2.width());
                    }

                    shade(base, &top, SHADE_BEVEL_TOP);
                    shade(base, &midTop, SHADE_BEVEL_MID_TOP);
                    shade(base, &midBot, SHADE_BEVEL_MID_BOT);
                    shade(base, &bot, SHADE_BEVEL_BOT(w));
                    drawGradient(top, midTop, true, &pixPainter, r1, horiz);
                    drawGradient(midTop, midBot, true, &pixPainter, r2, horiz);
                    drawGradient(midBot, bot, true, &pixPainter, r3, horiz);
                }
            }
            else
            {
                QColor top, bot;

                if(equal(1.0, shadeTop))
                    top=base;
                else
                    shade(base, &top, shadeTop);
                if(equal(1.0, shadeBot))
                    bot=base;
                else
                    shade(base, &bot, shadeBot);

                drawGradient(top, bot, sel || APPEARANCE_INVERTED!=app ? increase : !increase,
                             &pixPainter, r, horiz);
            }
            pixPainter.end();
            itsPixmapCache.insert(key, pix, pix->width()*pix->height()*(pix->depth()/8));
        }
        p->drawTiledPixmap(origRect, *pix);
    }
}

void QtCurveStyle::drawGradient(const QColor &top, const QColor &bot, bool increase,
                                QPainter *p, QRect const &r, bool horiz) const
{
    if(r.width()>0 && r.height()>0)
    {
        if(top==bot)
            p->fillRect(r, top);
        else
        {
            int rh(r.height()), rw(r.width()),
                rTop(top.red()), gTop(top.green()), bTop(top.blue()),
                rx, ry, rx2, ry2,
                size(horiz ? rh : rw);

            r.coords(&rx, &ry, &rx2, &ry2);

            register int rl(rTop << 16);
            register int gl(gTop << 16);
            register int bl(bTop << 16);
            register int i;

            int dr(((1<<16) * (bot.red() - rTop)) / size),
                dg(((1<<16) * (bot.green() - gTop)) / size),
                db(((1<<16) * (bot.blue() - bTop)) / size);

            if(increase)
                if(horiz)
                {
                    for (i=0; i < size; i++)
                    {
                        p->setPen(QColor(rl>>16, gl>>16, bl>>16));
                        p->drawLine(rx, ry+i, rx2, ry+i);
                        rl += dr;
                        gl += dg;
                        bl += db;
                    }
                }
                else
                    for(i=0; i < size; i++)
                    {
                        p->setPen(QColor(rl>>16, gl>>16, bl>>16));
                        p->drawLine(rx+i, ry, rx+i, ry2);
                        rl += dr;
                        gl += dg;
                        bl += db;
                    }
            else
                if(horiz)
                {
                    for(i=size-1; i>=0; i--)
                    {
                        p->setPen(QColor(rl>>16, gl>>16, bl>>16));
                        p->drawLine(rx, ry+i, rx2, ry+i);
                        rl += dr;
                        gl += dg;
                        bl += db;
                    }
                }
                else
                    for(i=size-1; i>=0; i--)
                    {
                        p->setPen(QColor(rl>>16, gl>>16, bl>>16));
                        p->drawLine(rx+i, ry, rx+i, ry2);
                        rl += dr;
                        gl += dg;
                        bl += db;
                    }
        }
    }
}

void QtCurveStyle::drawSbSliderHandle(QPainter *p, const QRect &orig, const QColorGroup &cg,
                                      SFlags flags) const
{
    int          min(MIN_SLIDER_SIZE(opts.sliderThumbs));
    const QColor *use(sliderColors(/*cg, */flags));
    QRect        r(orig);
//     EShade       shade(opts.shadeSliders);

    if(flags&(Style_Sunken|Style_Down))
        flags|=Style_MouseOver;
    flags&=~Style_Down;
    if(r.width()>r.height())
        flags|=Style_Horizontal;
    flags|=Style_Raised;

    drawLightBevel(p, r, cg, flags,
#ifndef QTC_SIMPLE_SCROLLBARS
                        SCROLLBAR_NONE==opts.scrollbarType ? ROUNDED_ALL :
#endif
                        ROUNDED_NONE,
                   getFill(flags, use), use, true, false, WIDGET_SB_SLIDER);

    const QColor *markers(/*opts.coloredMouseOver && flags&Style_MouseOver
                              ? SHADE_NONE==shade ? itsMouseOverCols : itsBackgroundCols
                              : */use);

    if(LINE_NONE!=opts.sliderThumbs && ((flags & Style_Horizontal && r.width()>=min)|| r.height()>=min))
        switch(opts.sliderThumbs)
        {
            case LINE_SUNKEN:
                drawLines(p, r, !(flags & Style_Horizontal), 4, 3, markers, 0, 3);
                break;
            case LINE_DOTS:
            default:
                drawDots(p, r, !(flags & Style_Horizontal), 5, 2, markers, 0, 5);
        }
}

void QtCurveStyle::drawSliderHandle(QPainter *p, const QRect &r, const QColorGroup &cg,
                                    SFlags flags, bool tb) const
{
    bool horiz(r.width()>r.height());

    if(ROUND_FULL==opts.round)
    {
        const QColor *use(sliderColors(/*cg, */flags));
        const QColor &fill(getFill(flags, use));
        int          x(r.x()),
                     y(r.y()),
                     xo(horiz ? 8 : 0),
                     yo(horiz ? 0 : 8);

        if(horiz)
            y++;
        else
            x++;

        QPointArray clipRegion;

        clipRegion.setPoints(8, x,       y+8+yo,  x,       y+4,     x+4,    y,        x+8+xo, y,
                                x+12+xo, y+4,     x+12+xo, y+8+yo,  x+8+xo, y+12+yo,  x+4,    y+12+yo);

        if(!tb)
            p->fillRect(QRect(x, y, r.width()-(horiz ? 0 : 2), r.height()-(horiz ? 2 : 0)), cg.background());
        p->setClipRegion(QRegion(clipRegion)); // , QPainter::CoordPainter);
        if(IS_FLAT(opts.appearance))
        {
            p->fillRect(r, fill);
            if(MO_PLASTIK==opts.coloredMouseOver && flags&Style_MouseOver)
            {
                int col(QTC_SLIDER_MO_SHADE),
                    len(QTC_SLIDER_MO_LEN);

                if(horiz)
                {
                    p->fillRect(QRect(x+1, y+1, len, r.height()-2), itsMouseOverCols[col]);
                    p->fillRect(QRect(x+r.width()-(1+len), y+1, len, r.height()-2), itsMouseOverCols[col]);
                }
                else
                {
                    p->fillRect(QRect(x+1, y+1, r.width()-2, len), itsMouseOverCols[col]);
                    p->fillRect(QRect(x+1, y+r.height()-(1+len), r.width()-2, len), itsMouseOverCols[col]);
                }
            }
        }
        else
        {
            drawBevelGradient(fill, true, p, QRect(x+1, y+1, r.width()-2, r.height()-2),
                              horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK,
                              false, opts.sliderAppearance);

            if(MO_PLASTIK==opts.coloredMouseOver && flags&Style_MouseOver)
            {
                int col(QTC_SLIDER_MO_SHADE),
                    len(QTC_SLIDER_MO_LEN);

                if(horiz)
                {
                    drawBevelGradient(itsMouseOverCols[col], true, p, QRect(x+1, y+1, len, r.height()-2),
                                      horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                    drawBevelGradient(itsMouseOverCols[col], true, p, QRect(x+r.width()-(1+len), y+1, len, r.height()-2),
                                      horiz,SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                }
                else
                {
                    drawBevelGradient(itsMouseOverCols[col], true, p, QRect(x+1, y+1, r.width()-2, len),
                                      horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                    drawBevelGradient(itsMouseOverCols[col], true, p, QRect(x+1, y+r.height()-(1+len), r.width()-2, len),
                                      horiz, SHADE_BEVEL_GRAD_LIGHT, SHADE_BEVEL_GRAD_DARK, false, opts.sliderAppearance);
                }
            }
        }

        p->setClipping(false);

        p->drawPixmap(x, y,
                    *getPixmap(use[opts.coloredMouseOver && flags&Style_MouseOver ? 4 : QT_BORDER(flags&Style_Enabled)],
                                horiz ? PIX_SLIDER : PIX_SLIDER_V, 0.8));
        if(MO_PLASTIK!=opts.coloredMouseOver || !(flags&Style_MouseOver))
            p->drawPixmap(x, y, *getPixmap(use[0], horiz ? PIX_SLIDER_LIGHT : PIX_SLIDER_LIGHT_V));
    }
    else
    {
        QRect sr(r);

        if(horiz)
            sr.addCoords(0, 1, 0, 0);
        else
            sr.addCoords(1, 0, 0, 0);

        drawSbSliderHandle(p, sr, cg, flags|(horiz ? Style_Horizontal : 0));
    }
}

void QtCurveStyle::drawSliderGroove(QPainter *p, const QRect &r, const QColorGroup &cg,
                                    SFlags flags, const QWidget *widget) const
{
    const QSlider *sliderWidget((const QSlider *)widget);
    QRect         groove(r);
    bool          horiz(Qt::Horizontal==sliderWidget->orientation()),
                  reverse(QApplication::reverseLayout());
    const QColor  &usedCol=itsSliderCols
                            ? itsSliderCols[ORIGINAL_SHADE]
                            : itsMouseOverCols
                                ? itsMouseOverCols[ORIGINAL_SHADE]
                                : itsMenuitemCols[1];

    if(horiz)
    {
        int dh=(groove.height()-5)>>1;
        groove.addCoords(0, dh, 0, -dh);
        flags|=Style_Horizontal;
    }
    else
    {
        int dw=(groove.width()-5)>>1;
        groove.addCoords(dw, 0, -dw, 0);
    }

    drawLightBevel(p, groove, cg, flags|Style_Down, ROUNDED_ALL, itsBackgroundCols[flags&Style_Enabled ? 2 : ORIGINAL_SHADE],
                   itsBackgroundCols, true, true, WIDGET_SLIDER_TROUGH);

    if(opts.fillSlider && sliderWidget->maxValue()!=sliderWidget->minValue() && flags&Style_Enabled)
    {
        QRect used(groove);
        int   pos((int)(((double)(horiz ? groove.width() : groove.height()) /
                                     (sliderWidget->maxValue()-sliderWidget->minValue()))  *
                                 (sliderWidget->value() - sliderWidget->minValue())));

        if(horiz)
        {
            pos+=(groove.width()>10 && pos<(groove.width()/2)) ? 3 : 0;
            if(reverse)
                used.addCoords(groove.width()-pos, 0, 0, 0);
            else
                used.addCoords(0, 0, -(groove.width()-pos), 0);
        }
        else
        {
            pos+=(groove.height()>10 && pos<(groove.height()/2)) ? 3 : 0;
            used.addCoords(0, pos, 0, 0);
        }
        if(used.height()>0 && used.width()>0)
            drawLightBevel(p, used, cg, flags|Style_Down, ROUNDED_ALL, usedCol, NULL,
                           true, true, WIDGET_SLIDER_TROUGH);
    }
}

void QtCurveStyle::drawMenuOrToolBarBackground(QPainter *p, const QRect &r, const QColorGroup &cg,
                                               bool menu, bool horiz) const
{
    EAppearance app(menu ? opts.menubarAppearance : opts.toolbarAppearance);
    QColor      color(menu && itsActive ? itsMenubarCols[ORIGINAL_SHADE] : cg.background());
    double      from(0.0), to(0.0);

    switch(app)
    {
        default:
        case APPEARANCE_GRADIENT:
            from=SHADE_MENU_LIGHT;
            to=SHADE_MENU_DARK;
            break;
        case APPEARANCE_FLAT:
        case APPEARANCE_RAISED:
            break;
        case APPEARANCE_SHINY_GLASS:
        case APPEARANCE_DULL_GLASS:
            from=SHADE_BEVEL_GRAD_LIGHT;
            to=SHADE_BEVEL_GRAD_DARK;
    }

    drawBevelGradient(color, true, p, r, horiz, from, to, false, app);
}

void QtCurveStyle::drawHandleMarkers(QPainter *p, const QRect &r, SFlags flags, bool tb,
                                     ELine handles) const
{
    if(r.width()<2 || r.height()<2)
        return;

//     if(!(flags&Style_MouseOver) && p->device()==itsHoverWidget)
//         flags|=Style_MouseOver;
    flags&=~Style_MouseOver; // Dont mouse-over handles - we dont do this for KDE4...

    const QColor *border(borderColors(flags, itsBackgroundCols));

    switch(handles)
    {
        case LINE_DOTS:
            drawDots(p, r, !(flags & Style_Horizontal), 2,
                     APP_KICKER==itsThemedApp ? 1 : tb ? 5 : 3, border,
                     APP_KICKER==itsThemedApp ? 1 : tb ? -2 : 0, 5);
            break;
        case LINE_DASHES:
            if(flags&Style_Horizontal)
            {
                QRect r1(r.x()+(tb ? 2 : (r.width()-6)/2), r.y(), 5, r.height());

                drawLines(p, r1, true, (r.height()-8)/3,
                          tb ? 0 : (r.width()-5)/2, border, 0, 5);
            }
            else
            {
                QRect r1(r.x(), r.y()+(tb ? 2 : (r.height()-6)/2), r.width(), 5);

                drawLines(p, r1, false, (r.width()-8)/3,
                          tb ? 0 : (r.height()-5)/2, border, 0, 5);
            }
            break;
        default:
            drawLines(p, r, !(flags & Style_Horizontal), 2,
                      APP_KICKER==itsThemedApp ? 1 : tb ? 4 : 2, border,
                      APP_KICKER==itsThemedApp ? 1 : tb ? -2 : 0, 3);
    }
}

void QtCurveStyle::shadeColors(const QColor &base, QColor *vals) const
{
    QTC_SHADES

    for(int i=0; i<NUM_STD_SHADES; ++i)
        shade(base, &vals[i], QTC_SHADE(opts.contrast, i));
    shade(base, &vals[SHADE_ORIG_HIGHLIGHT], opts.highlightFactor);
    shade(vals[4], &vals[SHADE_4_HIGHLIGHT], opts.highlightFactor);
    shade(vals[2], &vals[SHADE_2_HIGHLIGHT], opts.highlightFactor);
    vals[ORIGINAL_SHADE]=base;
}

const QColor * QtCurveStyle::buttonColors(const QColorGroup &cg) const
{
    if(cg.button()!=itsButtonCols[ORIGINAL_SHADE])
    {
        shadeColors(cg.button(), itsColoredButtonCols);
        return itsColoredButtonCols;
    }

    return itsButtonCols;
}

const QColor * QtCurveStyle::sliderColors(/*const QColorGroup &cg, */ SFlags flags) const
{
    return (flags&Style_Enabled)
                ? SHADE_NONE!=opts.shadeSliders //&& cg.button()==itsButtonCols[ORIGINAL_SHADE]
                        ? itsSliderCols
                        : itsButtonCols // buttonColors(cg)
                : itsBackgroundCols;
}

const QColor * QtCurveStyle::backgroundColors(const QColor &c) const
{
    if(c!=itsBackgroundCols[ORIGINAL_SHADE])
    {
        shadeColors(c, itsColoredBackgroundCols);
        return itsColoredBackgroundCols;
    }

    return itsBackgroundCols;
}

const QColor * QtCurveStyle::borderColors(SFlags flags, const QColor *use) const
{
    return itsMouseOverCols && opts.coloredMouseOver && flags&Style_MouseOver
               ? itsMouseOverCols : use;
}

const QColor * QtCurveStyle::getSidebarButtons() const
{
    if(!itsSidebarButtonsCols)
    {
        if(SHADE_BLEND_SELECTED==opts.shadeSliders)
            itsSidebarButtonsCols=itsSliderCols;
        else if(IND_COLORED==opts.defBtnIndicator)
            itsSidebarButtonsCols=itsDefBtnCols;
        else
        {
            itsSidebarButtonsCols=new QColor [TOTAL_SHADES+1];
            shadeColors(midColor(itsMenuitemCols[ORIGINAL_SHADE], itsButtonCols[ORIGINAL_SHADE]),
                        itsSidebarButtonsCols);
        }
    }

    return itsSidebarButtonsCols;
}

void QtCurveStyle::setMenuColors(const QColorGroup &cg)
{
    switch(opts.shadeMenubars)
    {
        case SHADE_NONE:
            memcpy(itsMenubarCols, itsBackgroundCols, sizeof(QColor)*(TOTAL_SHADES+1));
            break;
        case SHADE_BLEND_SELECTED: // For menubars we dont actually blend...
            shadeColors(IS_GLASS(opts.appearance)
                            ? shade(itsMenuitemCols[ORIGINAL_SHADE], MENUBAR_GLASS_SELECTED_DARK_FACTOR)
                            : itsMenuitemCols[ORIGINAL_SHADE],
                        itsMenubarCols);
            break;
        case SHADE_CUSTOM:
            shadeColors(opts.customMenubarsColor, itsMenubarCols);
            break;
        case SHADE_DARKEN:
            shadeColors(shade(cg.background(), MENUBAR_DARK_FACTOR), itsMenubarCols);
    }
}

bool QtCurveStyle::redrawHoverWidget(const QPoint &pos)
{
    if(!itsHoverWidget || !itsHoverWidget->isShown() || !itsHoverWidget->isVisible())
        return false;

#if QT_VERSION >= 0x030200
    //
    // Qt>=3.2 sets the sensitive part of a check/radio to the image + label -> anything else
    // is not sensitive. But, the widget can ocupy a larger area - and this whole area will
    // react to mouse over. This needs to be counteracted so that it looks as if only the
    // sensitive area mouse-overs...
    QRadioButton *rb(::qt_cast<QRadioButton *>(itsHoverWidget));

    if(rb)
    {
        QRect rect(0, 0,
                   visualRect(subRect(SR_RadioButtonFocusRect, rb), rb).width()+
                   pixelMetric(PM_ExclusiveIndicatorWidth)+4, itsHoverWidget->height());

        itsHover=rect.contains(pos) ? HOVER_RADIO : HOVER_NONE;
        return (HOVER_NONE!=itsHover && !rect.contains(itsOldPos)) ||
               (HOVER_NONE==itsHover && rect.contains(itsOldPos));
    }
    else
    {
        QCheckBox *cb(::qt_cast<QCheckBox *>(itsHoverWidget));

        if(cb)
        {
            QRect rect(0, 0,
                       visualRect(subRect(SR_CheckBoxFocusRect, cb), cb).width()+
                       pixelMetric(PM_IndicatorWidth)+4, itsHoverWidget->height());

            itsHover=rect.contains(pos) ? HOVER_CHECK : HOVER_NONE;
            return (HOVER_NONE!=itsHover && !rect.contains(itsOldPos)) || (HOVER_NONE==itsHover && rect.contains(itsOldPos));
        }
        else
        {
#endif
            QScrollBar *sb(::qt_cast<QScrollBar *>(itsHoverWidget));

            if(sb)  // So, are we over add button, sub button, slider, or none?
            {
                bool  useThreeButtonScrollBar(SCROLLBAR_KDE==opts.scrollbarType);
                QRect subline(querySubControlMetrics(CC_ScrollBar, itsHoverWidget,
                                                     SC_ScrollBarSubLine)),
                      addline(querySubControlMetrics(CC_ScrollBar, itsHoverWidget,
                                                     SC_ScrollBarAddLine)),
                      slider(querySubControlMetrics(CC_ScrollBar, itsHoverWidget,
                                                    SC_ScrollBarSlider)),
                      subline2(addline);

                if (useThreeButtonScrollBar)
                    if (Qt::Horizontal==sb->orientation())
                    subline2.moveBy(-addline.width(), 0);
                else
                    subline2.moveBy(0, -addline.height());

                if(slider.contains(pos))
                    itsHover=HOVER_SB_SLIDER;
                else if(subline.contains(pos))
                    itsHover=HOVER_SB_SUB;
                else if(addline.contains(pos))
                    itsHover=HOVER_SB_ADD;
                else if(subline2.contains(pos))
                    itsHover=HOVER_SB_SUB2;
                else
                    itsHover=HOVER_NONE;

                return (HOVER_SB_SLIDER==itsHover && !slider.contains(itsOldPos)) ||
                       (HOVER_SB_SLIDER!=itsHover && slider.contains(itsOldPos)) ||
                       (HOVER_SB_SUB==itsHover && !subline.contains(itsOldPos)) ||
                       (HOVER_SB_SUB!=itsHover && subline.contains(itsOldPos)) ||

                       (useThreeButtonScrollBar &&
                            (HOVER_SB_SUB2==itsHover && !subline2.contains(itsOldPos)) ||
                            (HOVER_SB_SUB2!=itsHover && subline2.contains(itsOldPos)))  ||

                       (HOVER_SB_ADD==itsHover && !addline.contains(itsOldPos)) ||
                       (HOVER_SB_ADD!=itsHover && addline.contains(itsOldPos));
            }
            else
            {
#if KDE_VERSION >= 0x30400 && KDE_VERSION < 0x30500
                QToolButton *tb(::qt_cast<QToolButton *>(itsHoverWidget));

                if(tb)
                {
                    itsHover=APP_KICKER==itsThemedApp ? HOVER_KICKER : HOVER_NONE;
                    return HOVER_KICKER==itsHover;
                }
                else
                {
#endif
                    QHeader *hd(::qt_cast<QHeader *>(itsHoverWidget));

                    if(hd)
                    {
                        // Hmm... this ones tricky, as there's only 1 widget - but it has different
                        // sections...
                        // and the ones that aren't clickable should not highlight on mouse over!

                        QRect rect(0, 0, itsHoverWidget->width(), itsHoverWidget->height());
                        int  s(0);
                        bool redraw(false);

                        itsHover=rect.contains(pos) ? HOVER_HEADER : HOVER_NONE;
                        itsHoverSect=QTC_NO_SECT;

                        for(s=0; s<hd->count() && (QTC_NO_SECT==itsHoverSect || !redraw); ++s)
                        {
                            QRect r(hd->sectionRect(s));
                            bool  hasNew(r.contains(pos));

                            if(hasNew)
                                itsHoverSect=s;

                            if(!redraw)
                            {
                                bool hasOld(r.contains(itsOldPos));

                                if((hasNew && !hasOld) || (!hasNew && hasOld))
                                    redraw=true;
                            }
                        }
                        return redraw;
                    }
                    else
                    {
                        QSpinWidget *sw(::qt_cast<QSpinWidget *>(itsHoverWidget));

                        if(sw)  // So, are we over up or down?
                        {
                            QRect up(querySubControlMetrics(CC_SpinWidget, itsHoverWidget,
                                                            SC_SpinWidgetUp)),
                                  down(querySubControlMetrics(CC_SpinWidget, itsHoverWidget,
                                                              SC_SpinWidgetDown));

                            if(up.contains(pos))
                                itsHover=HOVER_SW_UP;
                            else if(down.contains(pos))
                                itsHover=HOVER_SW_DOWN;
                            else
                                itsHover=HOVER_NONE;

                            return (HOVER_SW_UP==itsHover && !up.contains(itsOldPos)) ||
                                   (HOVER_SW_UP!=itsHover && up.contains(itsOldPos)) ||
                                   (HOVER_SW_DOWN==itsHover && !down.contains(itsOldPos)) ||
                                   (HOVER_SW_DOWN!=itsHover && down.contains(itsOldPos));
                        }
                        else
                        {
                            QTabBar *tabbar(::qt_cast<QTabBar *>(itsHoverWidget));

                            if(tabbar)
                            {
                                bool redraw(false);
                                QTab *tab(tabbar->selectTab(pos));
                                int  tabIndex(tab ? tabbar->indexOf(tab->identifier()) : -1),
                                     selectedTab(tabbar->currentTab());

                                redraw=tab!=itsHoverTab && tabIndex!=selectedTab;
                                itsHoverTab=tab;

                                return redraw;
                            }
                            else
                            {
                                QComboBox *cb(::qt_cast<QComboBox *>(itsHoverWidget));

                                if(cb)
                                {
                                    QRect arrow(cb->rect());

                                    if(!cb->editable())
                                        itsHover=HOVER_CB_ARROW;
                                    else
                                    {
                                        arrow=(querySubControlMetrics(CC_ComboBox, itsHoverWidget,
                                                                      SC_ComboBoxArrow));

                                        if(arrow.contains(pos))
                                            itsHover=HOVER_CB_ARROW;
                                        else
                                            itsHover=HOVER_NONE;
                                    }

                                   return (HOVER_CB_ARROW==itsHover && !arrow.contains(itsOldPos)) ||
                                          (HOVER_CB_ARROW!=itsHover && arrow.contains(itsOldPos));
                                }
                                else
                                    return itsOldPos==QPoint(-1, -1);

                            }
                        }
#if KDE_VERSION >= 0x30400 && KDE_VERSION < 0x30500
                    }
#endif
#if KDE_VERSION >= 0x30400
                }
#endif
            }
#if QT_VERSION >= 0x030200
        }
    }
#endif

    return false;
}

const QColor & QtCurveStyle::getFill(SFlags flags, const QColor *use) const
{
    return !(flags&Style_Enabled)
               ? use[ORIGINAL_SHADE]
               : flags&Style_Down
                   ? use[4]
                   : flags&Style_MouseOver
                         ? flags&(Style_On | Style_Sunken)
                               ? use[SHADE_4_HIGHLIGHT]
                               : use[SHADE_ORIG_HIGHLIGHT]
                         : flags&(Style_On | Style_Sunken)
                               ? use[4]
                               : use[ORIGINAL_SHADE];
}

const QColor & QtCurveStyle::getTabFill(bool current, bool highlight, const QColor *use) const
{
    return current
            ? use[ORIGINAL_SHADE]
            : highlight
                ? use[SHADE_2_HIGHLIGHT]
                : use[2];
}

QPixmap * QtCurveStyle::getPixelPixmap(const QColor col) const
{
    QRgb    rgb(col.rgb());
    QString key(createKey(rgb));

    QPixmap *pix=itsPixmapCache.find(key);

    if(!pix)
    {
        static const int constAlpha=110;

        QImage img(1, 1, 32);

        img.setAlphaBuffer(true);
        img.setPixel(0, 0, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), constAlpha));
        pix=new QPixmap(img);
        itsPixmapCache.insert(key, pix, pix->depth()/8);
    }

    return pix;
}

QPixmap * QtCurveStyle::getPixmap(const QColor col, EPixmap p, double shade) const
{
    QRgb    rgb(col.rgb());
    QString key(createKey(rgb, p));
    QPixmap *pix=itsPixmapCache.find(key);

    if(!pix)
    {
        pix=new QPixmap();

        QImage img;

        switch(p)
        {
            case PIX_RADIO_BORDER:
                img.loadFromData(qembed_findData("radio_frame.png"));
                break;
            case PIX_RADIO_LIGHT:
                img.loadFromData(qembed_findData("radio_light.png"));
                break;
            case PIX_RADIO_ON:
                img.loadFromData(qembed_findData("radio_on.png"));
                break;
            case PIX_CHECK:
                img.loadFromData(qembed_findData("check_on.png"));
                break;
            case PIX_SLIDER:
                img.loadFromData(qembed_findData("slider.png"));
                break;
            case PIX_SLIDER_LIGHT:
                img.loadFromData(qembed_findData("slider_light.png"));
                break;
            case PIX_SLIDER_V:
                img.loadFromData(qembed_findData("slider_v.png"));
                break;
            case PIX_SLIDER_LIGHT_V:
                img.loadFromData(qembed_findData("slider_light_v.png"));
                break;
        }

        if (img.depth()<32)
            img=img.convertDepth(32);

        adjustPix(img.bits(), 4, img.width(), img.height(), img.bytesPerLine(), col.red(),
                  col.green(), col.blue(), shade);
        pix->convertFromImage(img);
        itsPixmapCache.insert(key, pix, pix->depth()/8);
    }

    return pix;
}

void QtCurveStyle::setSbType()
{
    switch(opts.scrollbarType)
    {
        case SCROLLBAR_KDE:
            this->setScrollBarType(KStyle::ThreeButtonScrollBar);
            break;
        default:
        case SCROLLBAR_WINDOWS:
            this->setScrollBarType(KStyle::WindowsStyleScrollBar);
            break;
        case SCROLLBAR_PLATINUM:
            this->setScrollBarType(KStyle::PlatinumStyleScrollBar);
            break;
        case SCROLLBAR_NEXT:
            this->setScrollBarType(KStyle::NextStyleScrollBar);
            break;
    }
}

void QtCurveStyle::updateProgressPos()
{
    // Taken from lipstik!
    QMap<QWidget*, int>::iterator it(itsProgAnimWidgets.begin()),
                                  end(itsProgAnimWidgets.end());
    bool                          visible(false);
    for (; it!=end; ++it)
    {
        if (!::qt_cast<QProgressBar*>(it.key()))
            continue;

        QProgressBar *pb(::qt_cast<QProgressBar*>(it.key()));

        if (it.key() -> isEnabled() && pb -> progress()!=pb->totalSteps())
        {
            // update animation Offset of the current Widget
            it.data() = (it.data() + (QApplication::reverseLayout() ? -1 : 1))
                        % (PROGRESS_CHUNK_WIDTH*2);
            it.key()->update();
        }
        if (it.key()->isVisible())
            visible = true;
    }
    if (!visible)
        itsAnimationTimer->stop();
}

void QtCurveStyle::progressBarDestroyed(QObject *bar)
{
    itsProgAnimWidgets.remove(static_cast<QWidget*>(bar));
}

void QtCurveStyle::sliderThumbMoved(int)
{
    QSlider *slider(::qt_cast<QSlider*>(sender()));

    if(slider)
        slider->update();
}

void QtCurveStyle::khtmlWidgetDestroyed(QObject *o)
{
    itsKhtmlWidgets.remove(static_cast<const QWidget *>(o));
}

#include "qtcurve.moc"
