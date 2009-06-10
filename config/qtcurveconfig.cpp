/*
  QtCurve (C) Craig Drummond, 2003 - 2009 craig_p_drummond@yahoo.co.uk

  ----

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
*/

#include "qtcurveconfig.h"
#include "exportthemedialog.h"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtabwidget.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <qlistview.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qwidgetstack.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kcharselect.h>
#include <kdialogbase.h>
#include <knuminput.h>
#include <kguiitem.h>
#include <kinputdialog.h>
#include <knuminput.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"
#define CONFIG_READ
#define CONFIG_WRITE
#include "config_file.c"

#define QTC_EXTENSION ".qtcurve"

extern "C"
{
    QWidget * allocate_kstyle_config(QWidget *parent)
    {
        KGlobal::locale()->insertCatalogue("kstyle_qtcurve_config");
        return new QtCurveConfig(parent);
    }
}

static void drawGradient(const QColor &top, const QColor &bot, bool increase,
                         QPainter *p, QRect const &r, bool horiz)
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

class CharSelectDialog : public KDialogBase
{
    public:

    CharSelectDialog(QWidget *parent, int v)
        : KDialogBase(Plain, i18n("Select Password Character"), Ok|Cancel, Cancel, parent)
    {
        QFrame *page=plainPage();

        QVBoxLayout *layout=new QVBoxLayout(page, 0, KDialog::spacingHint());

        itsSelector=new KCharSelect(page, 0L);
        itsSelector->setChar(QChar(v));
        layout->addWidget(itsSelector);
    }

    int currentChar() const { return itsSelector->chr().unicode(); }

    private:

    KCharSelect *itsSelector;
};

class CStackItem : public QListViewItem
{
    public:

    CStackItem(QListView *p, const QString &text, int s)
        : QListViewItem(p, text),
          stackId(s)
    {
    }

    int compare(QListViewItem *i, int, bool) const
    {
        int b=((CStackItem *)i)->stackId;

        return stackId==b
                ? 0
                : stackId<b
                    ? -1
                    : 1;
    }

    int stack() { return stackId; }

    private:

    int stackId;
};

//
// QString.toDouble returns ok=true for "xx" ???
static double toDouble(const QString &str, bool *ok)
{
    if(ok)
    {
        QString stripped(str.stripWhiteSpace());
        int     size(stripped.length());

        for(int i=0; i<size; ++i)
            if(!stripped[i].isNumber() && stripped[i]!='.')
            {
                *ok=false;
                return 0.0;
            }
    }

    return str.toDouble(ok);
}

class CGradItem : public QListViewItem
{
    public:

    CGradItem(QListView *p, const QString &a, const QString &b)
        : QListViewItem(p, a, b)
    {
        setRenameEnabled(0, true);
        setRenameEnabled(1, true);
    }

    virtual ~CGradItem() { }

    void okRename(int col)
    {
        QString prevStr(text(col));

        prev=prevStr.toDouble();
        QListViewItem::okRename(col);

        bool    ok(false);
        double  val=toDouble(text(col), &ok)/100.0;

        if(!ok || (0==col && (val<0.0 || val>1.0)) || (1==col && (val<0.0 || val>2.0)))
        {
            setText(col, prevStr);
            startRename(col);
        }
    }

    int compare(QListViewItem *i, int col, bool) const
    {
        double a(text(col).toDouble()),
               b(i->text(col).toDouble());

        return equal(a, b)
                ? 0
                : a<b
                    ? -1
                    : 1;
    }

    double prevVal() const { return prev; }

    private:

    double prev;
};

CGradientPreview::CGradientPreview(QtCurveConfig *c, QWidget *p)
                : QWidget(p),
                  cfg(c)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
}

QSize CGradientPreview::sizeHint() const
{
    return QSize(64, 64);
}

QSize CGradientPreview::minimumSizeHint() const
{
    return sizeHint();
}

void CGradientPreview::paintEvent(QPaintEvent *)
{
    QRect    r(rect());
    QPainter p(this);

    if(stops.size())
    {
        GradientStopCont                 st(stops.fix());
        GradientStopCont::const_iterator it(st.begin()),
                                         end(st.end());
        QColor                           bot;
        bool                             horiz(true);
        int                              lastPos(horiz ? r.y() : r.x()),
                                         size(horiz ? r.height() : r.width());
        Options                          opts;
        
        opts.shading=cfg->currentShading();
            
        for(int i=0; it!=end; ++it, ++i)
        {
            if(0==i)
            {
                lastPos=(int)(((*it).pos*size)+0.5);
                shade(&opts, color, &bot, (*it).val);
            }
            else
            {
                QColor top(bot);
                int    pos((int)(((*it).pos*size)+0.5));

                shade(&opts, color, &bot, (*it).val);
                drawGradient(top, bot, true, &p,
                             horiz
                                ? QRect(r.x(), lastPos, r.width(), pos-lastPos)
                                : QRect(lastPos, r.y(), pos-lastPos, r.height()),
                             horiz);
                lastPos=pos;
            }
        }
    }
    else
        p.fillRect(r, color);
    p.end();
}

void CGradientPreview::setGrad(const GradientStopCont &s)
{
    stops=s;
    repaint();
}

void CGradientPreview::setColor(const QColor &col)
{
    if(col!=color)
    {
        color=col;
        repaint();
    }
}

static int toInt(const QString &str)
{
    return str.length()>1 ? str[0].unicode() : 0;
}

enum ShadeWidget
{
    SW_MENUBAR,
    SW_SLIDER,
    SW_CHECK_RADIO,
    SW_MENU_STRIPE,
    SW_COMBO
};

static void insertShadeEntries(QComboBox *combo, ShadeWidget sw)
{
    switch(sw)
    {
        case SW_MENUBAR:
            combo->insertItem(i18n("Background"));
            break;
        case SW_COMBO:
        case SW_SLIDER:
            combo->insertItem(i18n("Button"));
            break;
        case SW_CHECK_RADIO:
            combo->insertItem(i18n("Text"));
            break;
        case SW_MENU_STRIPE:
            combo->insertItem(i18n("None"));
            break;
    }

    combo->insertItem(i18n("Custom:"));
    combo->insertItem(i18n("Selected background"));
    if(SW_CHECK_RADIO!=sw) // For check/radio, we dont blend, and dont allow darken
    {
        combo->insertItem(i18n("Blended selected background"));
        combo->insertItem(i18n("Darken"));
    }
}

static void insertAppearanceEntries(QComboBox *combo, bool split=true, bool bev=true, bool fade=false)
{
    for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
        combo->insertItem(i18n("Custom gradient %1").arg((i-APPEARANCE_CUSTOM1)+1));

    combo->insertItem(i18n("Flat"));
    combo->insertItem(i18n("Raised"));
    combo->insertItem(i18n("Dull glass"));
    combo->insertItem(i18n("Shiny glass"));
    combo->insertItem(i18n("Soft gradient"));
    combo->insertItem(i18n("Standard gradient"));
    combo->insertItem(i18n("Harsh gradient"));
    combo->insertItem(i18n("Inverted gradient"));
    if(split)
    {
        combo->insertItem(i18n("Split gradient"));
        if(bev)
        {
            combo->insertItem(i18n("Bevelled"));
            if(fade)
                combo->insertItem(i18n("Fade out (popup menuitems)"));
        }
    }
}

static void insertLineEntries(QComboBox *combo, bool dashes)
{
    combo->insertItem(i18n("None"));
    combo->insertItem(i18n("Sunken lines"));
    combo->insertItem(i18n("Flat lines"));
    combo->insertItem(i18n("Dots"));
    if(dashes)
        combo->insertItem(i18n("Dashes"));
}

static void insertDefBtnEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Corner indicator"));
    combo->insertItem(i18n("Font color thin border"));
    combo->insertItem(i18n("Selected background thick border"));
    combo->insertItem(i18n("Selected background tinting"));
    combo->insertItem(i18n("A slight glow"));
    combo->insertItem(i18n("Darken"));
    combo->insertItem(i18n("No indicator"));
}

static void insertScrollbarEntries(QComboBox *combo)
{
    combo->insertItem(i18n("KDE"));
    combo->insertItem(i18n("Windows"));
    combo->insertItem(i18n("Platinum"));
    combo->insertItem(i18n("Next"));
    combo->insertItem(i18n("No buttons"));
}

static void insertRoundEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Square"));
    combo->insertItem(i18n("Slightly rounded"));
    combo->insertItem(i18n("Fully rounded"));
    combo->insertItem(i18n("Extra rounded (KDE4 & Gtk2)"));
    combo->insertItem(i18n("Max rounded (KDE4 & Gtk2)"));
}

static void insertMouseOverEntries(QComboBox *combo)
{
    combo->insertItem(i18n("No coloration"));
    combo->insertItem(i18n("Color border"));
    combo->insertItem(i18n("Thick color border"));
    combo->insertItem(i18n("Plastik style"));
    combo->insertItem(i18n("Glow"));
}

static void insertToolbarBorderEntries(QComboBox *combo)
{
    combo->insertItem(i18n("None"));
    combo->insertItem(i18n("Light"));
    combo->insertItem(i18n("Dark"));
    combo->insertItem(i18n("Light (all sides)"));
    combo->insertItem(i18n("Dark (all sides)"));
}

static void insertEffectEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Plain"));
    combo->insertItem(i18n("Etched"));
    combo->insertItem(i18n("Shadowed"));
}

static void insertShadingEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Simple"));
    combo->insertItem(i18n("Use HSL color space"));
    combo->insertItem(i18n("Use HSV color space"));
    combo->insertItem(i18n("Use HCY color space"));
}

static void insertStripeEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Plain"));
    combo->insertItem(i18n("Striped"));
    combo->insertItem(i18n("Diagonal stripes"));
}

static void insertSliderStyleEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Plain"));
    combo->insertItem(i18n("Round"));
    combo->insertItem(i18n("Plain - rotated"));
    combo->insertItem(i18n("Round - rotated"));
    combo->insertItem(i18n("Triangular"));
}

static void insertEColorEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Base color"));
    combo->insertItem(i18n("Background color"));
    combo->insertItem(i18n("Darkened background color"));
}

static void insertFocusEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Standard (dotted)"));
    combo->insertItem(i18n("Highlight color"));
    combo->insertItem(i18n("Highlight color (full size)"));
    combo->insertItem(i18n("Highlight color, full, and fill (Gtk2 & KDE4 only)"));
    combo->insertItem(i18n("Line drawn with highlight color"));
}

static void insertGradBorderEntries(QComboBox *combo)
{
    combo->insertItem(i18n("No border"));
    combo->insertItem(i18n("Light border"));
    combo->insertItem(i18n("3D border (light only)"));
    combo->insertItem(i18n("3D border (dark and light)"));
}

static void insertAlignEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Left"));
    combo->insertItem(i18n("Center (between controls)"));
    combo->insertItem(i18n("Center (full width)"));
    combo->insertItem(i18n("Right"));
}

static void insertTabMoEntriess(QComboBox *combo)
{
    combo->insertItem(i18n("Highlight on top"));
    combo->insertItem(i18n("Highlight on bottom"));
    combo->insertItem(i18n("Add a slight glow"));
}

QtCurveConfig::QtCurveConfig(QWidget *parent)
             : QtCurveConfigBase(parent),
               exportDialog(NULL),
               gradPreview(NULL),
               lastCategory(NULL)
{
    titleLabel->setText("QtCurve " VERSION " - (C) Craig Drummond, 2003-2009");
    insertShadeEntries(shadeSliders, SW_SLIDER);
    insertShadeEntries(shadeMenubars, SW_MENUBAR);
    insertShadeEntries(shadeCheckRadio, SW_CHECK_RADIO);
    insertShadeEntries(menuStripe, SW_MENU_STRIPE);
    insertShadeEntries(comboBtn, SW_COMBO);
    insertAppearanceEntries(appearance);
    insertAppearanceEntries(menubarAppearance);
    insertAppearanceEntries(toolbarAppearance);
    insertAppearanceEntries(lvAppearance);
    insertAppearanceEntries(sliderAppearance);
    insertAppearanceEntries(tabAppearance, false, false);
    insertAppearanceEntries(activeTabAppearance, false, false);
    insertAppearanceEntries(progressAppearance);
    insertAppearanceEntries(progressGrooveAppearance);
    insertAppearanceEntries(grooveAppearance);
    insertAppearanceEntries(sunkenAppearance);
    insertAppearanceEntries(menuitemAppearance, true, true, true);
    insertAppearanceEntries(titlebarAppearance, true, false);
    insertAppearanceEntries(inactiveTitlebarAppearance, true, false);
    insertAppearanceEntries(titlebarButtonAppearance);
    insertAppearanceEntries(selectionAppearance, true, false);
    insertAppearanceEntries(menuStripeAppearance, true, false);
    insertAppearanceEntries(sbarBgndAppearance);
    insertAppearanceEntries(sliderFill);
    insertLineEntries(handles, true);
    insertLineEntries(sliderThumbs, false);
    insertLineEntries(toolbarSeparators, false);
    insertLineEntries(splitters, true);
    insertDefBtnEntries(defBtnIndicator);
    insertScrollbarEntries(scrollbarType);
    insertRoundEntries(round);
    insertMouseOverEntries(coloredMouseOver);
    insertToolbarBorderEntries(toolbarBorders);
    insertEffectEntries(buttonEffect);
    insertShadingEntries(shading);
    insertStripeEntries(stripedProgress);
    insertSliderStyleEntries(sliderStyle);
    insertEColorEntries(progressGrooveColor);
    insertFocusEntries(focus);
    insertGradBorderEntries(gradBorder);
    insertAlignEntries(titlebarAlignment);
    insertTabMoEntriess(tabMouseOver);

    highlightFactor->setMinValue(MIN_HIGHLIGHT_FACTOR);
    highlightFactor->setMaxValue(MAX_HIGHLIGHT_FACTOR);
    highlightFactor->setValue(DEFAULT_HIGHLIGHT_FACTOR);

    lighterPopupMenuBgnd->setMinValue(MIN_LIGHTER_POPUP_MENU);
    lighterPopupMenuBgnd->setMaxValue(MAX_LIGHTER_POPUP_MENU);
    lighterPopupMenuBgnd->setValue(DEF_POPUPMENU_LIGHT_FACTOR);

    menuDelay->setMinValue(MIN_MENU_DELAY);
    menuDelay->setMaxValue(MAX_MENU_DELAY);
    menuDelay->setValue(DEFAULT_MENU_DELAY);
    
    connect(lighterPopupMenuBgnd, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(menuDelay, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(menuStripe, SIGNAL(activated(int)), SLOT(menuStripeChanged()));
    connect(customMenuStripeColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(menuStripeAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(round, SIGNAL(activated(int)), SLOT(roundChanged()));
    connect(toolbarBorders, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderThumbs, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(handles, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(appearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(customMenuTextColor, SIGNAL(toggled(bool)), SLOT(customMenuTextColorChanged()));
    connect(stripedProgress, SIGNAL(activated(int)), SLOT(stripedProgressChanged()));
    connect(animatedProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(embolden, SIGNAL(toggled(bool)), SLOT(emboldenToggled()));
    connect(defBtnIndicator, SIGNAL(activated(int)), SLOT(defBtnIndicatorChanged()));
    connect(highlightTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(menubarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(toolbarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(lvAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(tabAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(activeTabAppearance, SIGNAL(activated(int)), SLOT(activeTabAppearanceChanged()));
    connect(toolbarSeparators, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(splitters, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(fixParentlessDialogs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(fillSlider, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(sliderStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(roundMbTopOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(fillProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(darkerBorders, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(comboSplitter, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(comboBtn, SIGNAL(activated(int)), SLOT(comboBtnChanged()));
    connect(customComboBtnColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(unifySpinBtns, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(vArrows, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(xCheck, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(crHighlight, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(crButton, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorSelTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(roundAllTabs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(tabMouseOver, SIGNAL(activated(int)), SLOT(tabMoChanged()));
    connect(stdSidebarButtons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(borderMenuitems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(popupBorder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(progressAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(progressGrooveAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(grooveAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sunkenAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(progressGrooveColor, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(menuitemAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(titlebarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(inactiveTitlebarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(titlebarButtonAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(colorTitlebarOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(selectionAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(shadeCheckRadio, SIGNAL(activated(int)), SLOT(shadeCheckRadioChanged()));
    connect(customCheckRadioColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(focus, SIGNAL(activated(int)), SLOT(focusChanged()));
    connect(lvLines, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(lvButton, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(drawStatusBarFrames, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(buttonEffect, SIGNAL(activated(int)), SLOT(buttonEffectChanged()));
    connect(coloredMouseOver, SIGNAL(activated(int)), SLOT(coloredMouseOverChanged()));
    connect(menubarMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(shadeMenubarOnlyWhenActive, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(thinnerMenuItems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(customSlidersColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenubarsColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenuSelTextColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(customMenuNormTextColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));
    connect(shadeSliders, SIGNAL(activated(int)), SLOT(shadeSlidersChanged()));
    connect(shadeMenubars, SIGNAL(activated(int)), SLOT(shadeMenubarsChanged()));
    connect(highlightFactor, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(scrollbarType, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(shading, SIGNAL(activated(int)), SLOT(shadingChanged()));
    connect(gtkScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(squareScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(highlightScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(sunkenScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(flatSbarButtons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(thinSbarGroove, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorSliderMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarBorder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(sbarBgndAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderFill, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(gtkComboMenus, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gtkButtonOrder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(mapKdeIcons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(passwordChar, SIGNAL(clicked()), SLOT(passwordCharClicked()));
    connect(framelessGroupBoxes, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorMenubarMouseOver, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(useHighlightForMenu, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(groupBoxLine, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(fadeLines, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(titlebarAlignment, SIGNAL(activated(int)), SLOT(updateChanged()));

    defaultSettings(&defaultStyle);
    if(!readConfig(NULL, &currentStyle, &defaultStyle))
        currentStyle=defaultStyle;

    setupShadesTab();
    setWidgetOptions(currentStyle);

    QPopupMenu *menu=new QPopupMenu(this),
               *subMenu=new QPopupMenu(this);

    optionBtn->setPopup(menu);

    menu->insertItem(i18n("Predefined Style"), subMenu);
    menu->insertSeparator();
    menu->insertItem(i18n("Import..."), this, SLOT(importStyle()));
    menu->insertItem(i18n("Export..."), this, SLOT(exportStyle()));
    menu->insertSeparator();
    menu->insertItem(i18n("Export Theme..."), this, SLOT(exportTheme()));

    loadStyles(subMenu);
    setupGradientsTab();
    setupStack();
}

QtCurveConfig::~QtCurveConfig()
{
}

void QtCurveConfig::loadStyles(QPopupMenu *menu)
{
    QStringList  files(KGlobal::dirs()->findAllResources("data", "QtCurve/*"QTC_EXTENSION, false, true));

    files.sort();

    QStringList::Iterator it(files.begin()),
                          end(files.end());
    Options               opts;

    for(; it!=end; ++it)
        if(readConfig(*it, &opts, &defaultStyle))
            styles[menu->insertItem(QFileInfo(*it).fileName().remove(QTC_EXTENSION).replace('_', ' '),
                                    this, SLOT(setStyle(int)))]=*it;
}

void QtCurveConfig::save()
{
    Options opts=currentStyle;

    setOptions(opts);
    writeConfig(NULL, opts, defaultStyle);

    KSharedConfig *cfg=KGlobal::sharedConfig();
    QString       grp(cfg->group());
    bool          useGlobals(cfg->forceGlobal());

    cfg->setForceGlobal(true);
    cfg->setGroup("KDE");

    if(opts.gtkButtonOrder)
        cfg->writeEntry("ButtonLayout", 2);
    else
        cfg->deleteEntry("ButtonLayout");
    cfg->setGroup(grp);
    cfg->sync();
    cfg->setForceGlobal(useGlobals);
}

void QtCurveConfig::defaults()
{
    setWidgetOptions(defaultStyle);
    if (settingsChanged())
        emit changed(true);
}

void QtCurveConfig::setStyle(int s)
{
    loadStyle(styles[s]);
}

void QtCurveConfig::emboldenToggled()
{
    if(!embolden->isChecked() && IND_NONE==defBtnIndicator->currentItem())
        defBtnIndicator->setCurrentItem(IND_TINT);
    updateChanged();
}

void QtCurveConfig::defBtnIndicatorChanged()
{
    if(IND_NONE==defBtnIndicator->currentItem() && !embolden->isChecked())
        embolden->setChecked(true);
    else if(IND_GLOW==defBtnIndicator->currentItem() && EFFECT_NONE==buttonEffect->currentItem())
        buttonEffect->setCurrentItem(EFFECT_SHADOW);

    if(IND_COLORED==defBtnIndicator->currentItem() && round->currentItem()>ROUND_FULL)
        round->setCurrentItem(ROUND_FULL);

    updateChanged();
}

void QtCurveConfig::buttonEffectChanged()
{
    if(EFFECT_NONE==buttonEffect->currentItem())
    {
        if(IND_GLOW==defBtnIndicator->currentItem())
            defBtnIndicator->setCurrentItem(IND_TINT);
        if(MO_GLOW==coloredMouseOver->currentItem())
            coloredMouseOver->setCurrentItem(MO_PLASTIK);
    }

    updateChanged();
}

void QtCurveConfig::coloredMouseOverChanged()
{
    if(MO_GLOW==coloredMouseOver->currentItem() &&
       EFFECT_NONE==buttonEffect->currentItem())
        buttonEffect->setCurrentItem(EFFECT_SHADOW);

    updateChanged();
}

void QtCurveConfig::shadeSlidersChanged()
{
    customSlidersColor->setEnabled(SHADE_CUSTOM==shadeSliders->currentItem());
    updateChanged();
}

void QtCurveConfig::shadeMenubarsChanged()
{
    customMenubarsColor->setEnabled(SHADE_CUSTOM==shadeMenubars->currentItem());
    updateChanged();
}

void QtCurveConfig::shadeCheckRadioChanged()
{
    customCheckRadioColor->setEnabled(SHADE_CUSTOM==shadeCheckRadio->currentItem());
    updateChanged();
}

void QtCurveConfig::customMenuTextColorChanged()
{
    customMenuNormTextColor->setEnabled(customMenuTextColor->isChecked());
    customMenuSelTextColor->setEnabled(customMenuTextColor->isChecked());
    updateChanged();
}

void QtCurveConfig::menuStripeChanged()
{
    customMenuStripeColor->setEnabled(SHADE_CUSTOM==menuStripe->currentItem());
    menuStripeAppearance->setEnabled(SHADE_NONE!=menuStripe->currentItem());
    updateChanged();
}

void QtCurveConfig::comboBtnChanged()
{
    customComboBtnColor->setEnabled(SHADE_CUSTOM==comboBtn->currentItem());
    updateChanged();
}

void QtCurveConfig::stripedProgressChanged()
{
    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentItem());
    if(animatedProgress->isChecked() && STRIPE_NONE==stripedProgress->currentItem())
        animatedProgress->setChecked(false);
    updateChanged();
}

void QtCurveConfig::activeTabAppearanceChanged()
{
    int  current(activeTabAppearance->currentItem());
    bool disableCol(APPEARANCE_FLAT==current && APPEARANCE_RAISED==current);

    if(colorSelTab->isChecked() && disableCol)
        colorSelTab->setChecked(false);
    colorSelTab->setEnabled(!disableCol);
    updateChanged();
}

void QtCurveConfig::tabMoChanged()
{
    if(TAB_MO_GLOW==tabMouseOver->currentItem())
        roundAllTabs->setChecked(true);
    roundAllTabs->setEnabled(TAB_MO_GLOW!=tabMouseOver->currentItem());
    updateChanged();
}

void QtCurveConfig::shadingChanged()
{
    updateChanged();
    if(gradPreview)
        gradPreview->repaint();
}

void QtCurveConfig::passwordCharClicked()
{
    int              cur(toInt(passwordChar->text()));
    CharSelectDialog dlg(this, cur);

    if(QDialog::Accepted==dlg.exec() && dlg.currentChar()!=cur)
    {
        setPasswordChar(dlg.currentChar());
        updateChanged();
    }
}

void QtCurveConfig::setupStack()
{
    int i=0;
    lastCategory=new CStackItem(stackList, i18n("General"), i++);
    new CStackItem(stackList, i18n("Combos"), i++);
    new CStackItem(stackList, i18n("Splitters"), i++);
    new CStackItem(stackList, i18n("Sliders and Scrollbars"), i++);
    new CStackItem(stackList, i18n("Progressbars"), i++);
    new CStackItem(stackList, i18n("Default Button"),i++);
    new CStackItem(stackList, i18n("Mouse-over"), i++);
    new CStackItem(stackList, i18n("Listviews"), i++);
    new CStackItem(stackList, i18n("Scrollviews"), i++);
    new CStackItem(stackList, i18n("Tabs"), i++);
    new CStackItem(stackList, i18n("Checks and Radios"), i++);
    new CStackItem(stackList, i18n("Windows"), i++);
    new CStackItem(stackList, i18n("Menus and Toolbars"), i++);
    new CStackItem(stackList, i18n("Advanced Settings"), i++);
    new CStackItem(stackList, i18n("Custom Gradients"), i++);
    new CStackItem(stackList, i18n("Custom Shades"), i++);

    stackList->setSelected(lastCategory, true);
    stackList->setCurrentItem(lastCategory);
    stackList->setResizeMode(QListView::LastColumn);
    connect(stackList, SIGNAL(selectionChanged()), SLOT(changeStack()));
}

void QtCurveConfig::changeStack()
{
    CStackItem *item=(CStackItem *)(stackList->selectedItem());

    if(item)
        lastCategory=item;
    else
    {
        item=lastCategory;
        if(item)
        {
            stackList->setSelected(item, true);
            stackList->setCurrentItem(item);
        }
    }

    if(item)
        stack->raiseWidget(item->stack());
}

void QtCurveConfig::gradChanged(int i)
{
    GradientCont::const_iterator it(customGradient.find((EAppearance)i));

    gradStops->clear();

    if(it!=customGradient.end())
    {
        gradPreview->setGrad((*it).second.stops);
        gradBorder->setCurrentItem((*it).second.border);

        GradientStopCont::const_iterator git((*it).second.stops.begin()),
                                         gend((*it).second.stops.end());

        for(; git!=gend; ++git)
            new CGradItem(gradStops, QString().setNum((*git).pos*100.0),
                                     QString().setNum((*git).val*100.0));
    }
    else
    {
        gradPreview->setGrad(GradientStopCont());
        gradBorder->setCurrentItem(GB_3D);
    }

    gradBorder->setEnabled(QTC_NUM_CUSTOM_GRAD!=i);
}

void QtCurveConfig::itemChanged(QListViewItem *i, int col)
{
    GradientCont::iterator it=customGradient.find((EAppearance)gradCombo->currentItem());

    if(it!=customGradient.end())
    {
        bool   ok;
        double newVal=toDouble(i->text(col), &ok)/100.0;

        if(ok && ((0==col && (newVal>=0.0 && newVal<=1.0)) ||
                  (1==col && newVal>=0.0 && newVal<=2.0)) )
        {
            double pos=0==(col ? newVal : i->text(0).toDouble())/100.0,
                   val=1==(col ? newVal : i->text(1).toDouble())/100.0,
                   prev=((CGradItem *)i)->prevVal();

            (*it).second.stops.erase(GradientStop(col ? pos : prev, col ? prev : val));
            (*it).second.stops.insert(GradientStop(pos, val));
            gradPreview->setGrad((*it).second.stops);
            i->setText(col, QString().setNum(val));
            emit changed(true);
        }
    }
}

void QtCurveConfig::addGradStop()
{
    GradientCont::iterator cg=customGradient.find((EAppearance)gradCombo->currentItem());

    if(cg==customGradient.end())
    {
        Gradient cust;

        cust.border=(EGradientBorder)gradBorder->currentItem();
        cust.stops.insert(GradientStop(stopPosition->value()/100.0, stopValue->value()/100.0));
        customGradient[(EAppearance)gradCombo->currentItem()]=cust;
        gradChanged(gradCombo->currentItem());
        emit changed(true);
    }
    else
    {
        GradientStopCont::const_iterator it((*cg).second.stops.begin()),
                                         end((*cg).second.stops.end());
        double                           pos(stopPosition->value()/100.0),
                                         val(stopValue->value()/100.0);

        for(; it!=end; ++it)
            if(equal(pos, (*it).pos))
                if(equal(val, (*it).val))
                    return;
                else
                {
                    (*cg).second.stops.erase(it);
                    break;
                }

        unsigned int b4=(*cg).second.stops.size();
        (*cg).second.stops.insert(GradientStop(pos, val));

        if((*cg).second.stops.size()!=b4)
        {
            gradPreview->setGrad((*cg).second.stops);

            QListViewItem *prev=gradStops->selectedItem();

            if(prev)
                gradStops->setSelected(prev, false);

            CGradItem *i=new CGradItem(gradStops, QString().setNum(pos*100.0),
                                                  QString().setNum(val*100.0));

            gradStops->setSelected(i, true);
            emit changed(true);
        }
    }
}

void QtCurveConfig::removeGradStop()
{
    QListViewItem *cur=gradStops->selectedItem();

    if(cur)
    {
        QListViewItem *next=cur->itemBelow();

        if(!next)
            next=cur->itemAbove();

        GradientCont::iterator it=customGradient.find((EAppearance)gradCombo->currentItem());

        if(it!=customGradient.end())
        {
            double pos=cur->text(0).toDouble()/100.0,
                   val=cur->text(1).toDouble()/100.0;

            (*it).second.stops.erase(GradientStop(pos, val));
            gradPreview->setGrad((*it).second.stops);
            emit changed(true);

            delete cur;
            if(next)
                gradStops->setCurrentItem(next);
        }
    }
}

void QtCurveConfig::updateGradStop()
{
    QListViewItem *i=gradStops->selectedItem();

    GradientCont::iterator cg=customGradient.find((EAppearance)gradCombo->currentItem());

    if(i)
    {
        double curPos=i->text(0).toDouble()/100.0,
               curVal=i->text(1).toDouble()/100.0,
               newPos(stopPosition->value()/100.0),
               newVal(stopValue->value()/100.0);

        if(!equal(newPos, curPos) || !equal(newVal, curVal))
        {
            (*cg).second.stops.erase(GradientStop(curPos, curVal));
            (*cg).second.stops.insert(GradientStop(newPos, newVal));

            i->setText(0, QString().setNum(stopPosition->value()));
            i->setText(1, QString().setNum(stopValue->value()));
            gradPreview->setGrad((*cg).second.stops);
            emit changed(true);
        }
    }
    else
        addGradStop();
}

void QtCurveConfig::stopSelected()
{
    QListViewItem *i=gradStops->selectedItem();

    removeButton->setEnabled(i);
    updateButton->setEnabled(i);

    if(i)
    {
        stopPosition->setValue(i->text(0).toInt());
        stopValue->setValue(i->text(1).toInt());
    }
    else
    {
        stopPosition->setValue(0);
        stopValue->setValue(0);
    }
}

void QtCurveConfig::setupGradientsTab()
{
    for(int i=APPEARANCE_CUSTOM1; i<(APPEARANCE_CUSTOM1+QTC_NUM_CUSTOM_GRAD); ++i)
        gradCombo->insertItem(i18n("Custom gradient %1").arg((i-APPEARANCE_CUSTOM1)+1));

    gradCombo->setCurrentItem(APPEARANCE_CUSTOM1);

    gradPreview=new CGradientPreview(this, previewWidgetContainer);
    QVBoxLayout *layout=new QVBoxLayout(previewWidgetContainer);
    layout->addWidget(gradPreview);
    layout->setMargin(0);
    layout->setSpacing(0);
    QColor col(palette().color(QPalette::Active, QColorGroup::Button));
    previewColor->setColor(col);
    gradPreview->setColor(col);
    gradChanged(APPEARANCE_CUSTOM1);
    addButton->setGuiItem(KGuiItem(i18n("Add"), "add"));
    removeButton->setGuiItem(KGuiItem(i18n("Remove"), "remove"));
    updateButton->setGuiItem(KGuiItem(i18n("Update"), "button_ok"));

    gradStops->setDefaultRenameAction(QListView::Reject);
    gradStops->setAllColumnsShowFocus(true);
    gradStops->setSortColumn(0);
    stopPosition->setRange(0, 100, 5);
    stopValue->setRange(0, 200, 5);
    removeButton->setEnabled(false);
    updateButton->setEnabled(false);
    gradStops->setResizeMode(QListView::AllColumns);
    connect(gradCombo, SIGNAL(activated(int)), SLOT(gradChanged(int)));
    connect(previewColor, SIGNAL(changed(const QColor &)), gradPreview, SLOT(setColor(const QColor &)));
    connect(gradStops, SIGNAL(itemRenamed(QListViewItem *, int)), SLOT(itemChanged(QListViewItem *, int)));
    connect(addButton, SIGNAL(clicked()), SLOT(addGradStop()));
    connect(removeButton, SIGNAL(clicked()), SLOT(removeGradStop()));
    connect(updateButton, SIGNAL(clicked()), SLOT(updateGradStop()));
    connect(gradStops, SIGNAL(selectionChanged()), SLOT(stopSelected()));
}

void QtCurveConfig::setupShadesTab()
{
    int shade(0);

    setupShade(shade0, shade++);
    setupShade(shade1, shade++);
    setupShade(shade2, shade++);
    setupShade(shade3, shade++);
    setupShade(shade4, shade++);
    setupShade(shade5, shade++);
    connect(customShading, SIGNAL(toggled(bool)), SLOT(updateChanged()));
}

void QtCurveConfig::setupShade(KDoubleNumInput *w, int shade)
{
    w->setRange(0.0, 2.0, 0.05, false);
    connect(w, SIGNAL(valueChanged(double)), SLOT(updateChanged()));
    shadeVals[shade]=w;
}

void QtCurveConfig::populateShades(const Options &opts)
{
    QTC_SHADES
    int contrast=QSettings().readNumEntry("/Qt/KDE/contrast", 7);

    if(contrast<0 || contrast>10)
        contrast=7;

    customShading->setChecked(QTC_USE_CUSTOM_SHADES(opts));

    for(int i=0; i<NUM_STD_SHADES; ++i)
        shadeVals[i]->setValue(QTC_USE_CUSTOM_SHADES(opts)
                                  ? opts.customShades[i]
                                  : shades[SHADING_SIMPLE==shading->currentItem()
                                            ? 1 : 0]
                                          [contrast]
                                          [i]);
}

bool QtCurveConfig::diffShades(const Options &opts)
{
    if( (!QTC_USE_CUSTOM_SHADES(opts) && customShading->isChecked()) ||
        (QTC_USE_CUSTOM_SHADES(opts) && !customShading->isChecked()) )
        return true;

    if(customShading->isChecked())
    {
        for(int i=0; i<NUM_STD_SHADES; ++i)
            if(!equal(shadeVals[i]->value(), opts.customShades[i]))
                return true;
    }

    return false;
}

void QtCurveConfig::setPasswordChar(int ch)
{
    QString      str;
    QTextOStream s(&str);

    s.setf(QTextStream::hex);
    s << QChar(ch) << " (" << ch << ')';
    passwordChar->setText(str);
}

void QtCurveConfig::updateChanged()
{
    if (settingsChanged())
        emit changed(true);
}

void QtCurveConfig::focusChanged()
{
    if(ROUND_MAX==round->currentItem() && FOCUS_LINE!=focus->currentItem())
        round->setCurrentItem(ROUND_EXTRA);
    updateChanged();
}

void QtCurveConfig::roundChanged()
{
    if(ROUND_MAX==round->currentItem() && FOCUS_LINE!=focus->currentItem())
        focus->setCurrentItem(FOCUS_LINE);

    if(round->currentItem()>ROUND_FULL && IND_COLORED==defBtnIndicator->currentItem())
        defBtnIndicator->setCurrentItem(IND_TINT);
    updateChanged();
}

void QtCurveConfig::importStyle()
{
    QString file(KFileDialog::getOpenFileName(QString::null,
                                              i18n("*"QTC_EXTENSION"|QtCurve Settings Files\n"
                                                   QTC_THEME_PREFIX"*"QTC_THEME_SUFFIX"|QtCurve KDE Theme Files"),
                                              this));

    if(!file.isEmpty())
        loadStyle(file);
}

void QtCurveConfig::exportStyle()
{
    QString file(KFileDialog::getSaveFileName(QString::null, i18n("*"QTC_EXTENSION"|QtCurve Settings Files"), this));

    if(!file.isEmpty())
    {
        KConfig cfg(file, false, false);
        bool    rv(!cfg.isReadOnly());

        if(rv)
        {
            Options opts;

            setOptions(opts);
            rv=writeConfig(&cfg, opts, defaultStyle, true);
        }

        if(!rv)
            KMessageBox::error(this, i18n("Could not write to file:\n%1").arg(file));
    }
}

void QtCurveConfig::exportTheme()
{
    if(!exportDialog)
        exportDialog=new CExportThemeDialog(this);

    Options opts;

    setOptions(opts);
    exportDialog->run(opts);
}

void QtCurveConfig::loadStyle(const QString &file)
{
    Options opts;

    if(readConfig(file, &opts, &defaultStyle))
    {
        setWidgetOptions(opts);
        if (settingsChanged())
            emit changed(true);
    }
}

void QtCurveConfig::setOptions(Options &opts)
{
    opts.round=(ERound)round->currentItem();
    opts.toolbarBorders=(ETBarBorder)toolbarBorders->currentItem();
    opts.appearance=(EAppearance)appearance->currentItem();
    opts.focus=(EFocus)focus->currentItem();
    opts.lvLines=lvLines->isChecked();
    opts.lvButton=lvButton->isChecked();
    opts.drawStatusBarFrames=drawStatusBarFrames->isChecked();
    opts.buttonEffect=(EEffect)buttonEffect->currentItem();
    opts.coloredMouseOver=(EMouseOver)coloredMouseOver->currentItem();
    opts.menubarMouseOver=menubarMouseOver->isChecked();
    opts.shadeMenubarOnlyWhenActive=shadeMenubarOnlyWhenActive->isChecked();
    opts.thinnerMenuItems=thinnerMenuItems->isChecked();
    opts.fixParentlessDialogs=fixParentlessDialogs->isChecked();
    opts.animatedProgress=animatedProgress->isChecked();
    opts.stripedProgress=(EStripe)stripedProgress->currentItem();
    opts.lighterPopupMenuBgnd=lighterPopupMenuBgnd->value();
    opts.menuDelay=menuDelay->value();
    opts.menuStripe=(EShade)menuStripe->currentItem();
    opts.customMenuStripeColor=customMenuStripeColor->color();
    opts.menuStripeAppearance=(EAppearance)menuStripeAppearance->currentItem();
    opts.embolden=embolden->isChecked();
    opts.scrollbarType=(EScrollbar)scrollbarType->currentItem();
    opts.defBtnIndicator=(EDefBtnIndicator)defBtnIndicator->currentItem();
    opts.sliderThumbs=(ELine)sliderThumbs->currentItem();
    opts.handles=(ELine)handles->currentItem();
    opts.highlightTab=highlightTab->isChecked();
    opts.shadeSliders=(EShade)shadeSliders->currentItem();
    opts.shadeMenubars=(EShade)shadeMenubars->currentItem();
    opts.menubarAppearance=(EAppearance)menubarAppearance->currentItem();
    opts.toolbarAppearance=(EAppearance)toolbarAppearance->currentItem();
    opts.lvAppearance=(EAppearance)lvAppearance->currentItem();
    opts.sliderAppearance=(EAppearance)sliderAppearance->currentItem();
    opts.tabAppearance=(EAppearance)tabAppearance->currentItem();
    opts.activeTabAppearance=(EAppearance)activeTabAppearance->currentItem();
    opts.toolbarSeparators=(ELine)toolbarSeparators->currentItem();
    opts.splitters=(ELine)splitters->currentItem();
    opts.customSlidersColor=customSlidersColor->color();
    opts.customMenubarsColor=customMenubarsColor->color();
    opts.highlightFactor=highlightFactor->value();
    opts.customMenuNormTextColor=customMenuNormTextColor->color();
    opts.customMenuSelTextColor=customMenuSelTextColor->color();
    opts.customMenuTextColor=customMenuTextColor->isChecked();
    opts.fillSlider=fillSlider->isChecked();
    opts.sliderStyle=(ESliderStyle)sliderStyle->currentItem();
    opts.roundMbTopOnly=roundMbTopOnly->isChecked();
    opts.fillProgress=fillProgress->isChecked();
    opts.darkerBorders=darkerBorders->isChecked();
    opts.comboSplitter=comboSplitter->isChecked();
    opts.comboBtn=(EShade)comboBtn->currentItem();
    opts.customComboBtnColor=customComboBtnColor->color();
    opts.unifySpinBtns=unifySpinBtns->isChecked();
    opts.vArrows=vArrows->isChecked();
    opts.xCheck=xCheck->isChecked();
    opts.crHighlight=crHighlight->isChecked();
    opts.crButton=crButton->isChecked();
    opts.colorSelTab=colorSelTab->isChecked();
    opts.roundAllTabs=roundAllTabs->isChecked();
    opts.tabMouseOver=(ETabMo)tabMouseOver->currentItem();
    opts.stdSidebarButtons=stdSidebarButtons->isChecked();
    opts.borderMenuitems=borderMenuitems->isChecked();
    opts.popupBorder=popupBorder->isChecked();
    opts.progressAppearance=(EAppearance)progressAppearance->currentItem();
    opts.progressGrooveAppearance=(EAppearance)progressGrooveAppearance->currentItem();
    opts.grooveAppearance=(EAppearance)grooveAppearance->currentItem();
    opts.sunkenAppearance=(EAppearance)sunkenAppearance->currentItem();
    opts.progressGrooveColor=(EColor)progressGrooveColor->currentItem();
    opts.menuitemAppearance=(EAppearance)menuitemAppearance->currentItem();
    opts.titlebarAppearance=(EAppearance)titlebarAppearance->currentItem();
    opts.inactiveTitlebarAppearance=(EAppearance)inactiveTitlebarAppearance->currentItem();
    opts.titlebarButtonAppearance=(EAppearance)titlebarButtonAppearance->currentItem();
    opts.colorTitlebarOnly=colorTitlebarOnly->isChecked();
    opts.selectionAppearance=(EAppearance)selectionAppearance->currentItem();
    opts.shadeCheckRadio=(EShade)shadeCheckRadio->currentItem();
    opts.customCheckRadioColor=customCheckRadioColor->color();
    opts.shading=(EShading)shading->currentItem();
    opts.gtkScrollViews=gtkScrollViews->isChecked();
    opts.highlightScrollViews=highlightScrollViews->isChecked();
    opts.squareScrollViews=squareScrollViews->isChecked();
    opts.sunkenScrollViews=sunkenScrollViews->isChecked();
    opts.flatSbarButtons=flatSbarButtons->isChecked();
    opts.thinSbarGroove=thinSbarGroove->isChecked();
    opts.colorSliderMouseOver=colorSliderMouseOver->isChecked();
    opts.titlebarBorder=titlebarBorder->isChecked();
    opts.sbarBgndAppearance=(EAppearance)sbarBgndAppearance->currentItem();
    opts.sliderFill=(EAppearance)sliderFill->currentItem();
    opts.gtkComboMenus=gtkComboMenus->isChecked();
    opts.gtkButtonOrder=gtkButtonOrder->isChecked();
    opts.mapKdeIcons=mapKdeIcons->isChecked();
    opts.passwordChar=toInt(passwordChar->text());
    opts.framelessGroupBoxes=framelessGroupBoxes->isChecked();
    opts.customGradient=customGradient;
    opts.colorMenubarMouseOver=colorMenubarMouseOver->isChecked();
    opts.useHighlightForMenu=useHighlightForMenu->isChecked();
    opts.groupBoxLine=groupBoxLine->isChecked();
    opts.fadeLines=fadeLines->isChecked();
    opts.titlebarAlignment=(EAlign)titlebarAlignment->currentItem();

    if(customShading->isChecked())
    {
        for(int i=0; i<NUM_STD_SHADES; ++i)
            opts.customShades[i]=shadeVals[i]->value();
    }
    else
        opts.customShades[0]=0;
}

void QtCurveConfig::setWidgetOptions(const Options &opts)
{
    round->setCurrentItem(opts.round);
    scrollbarType->setCurrentItem(opts.scrollbarType);
    lighterPopupMenuBgnd->setValue(opts.lighterPopupMenuBgnd);
    menuDelay->setValue(opts.menuDelay);
    menuStripe->setCurrentItem(opts.menuStripe);
    customMenuStripeColor->setColor(opts.customMenuStripeColor);
    menuStripeAppearance->setCurrentItem(opts.menuStripeAppearance);
    toolbarBorders->setCurrentItem(opts.toolbarBorders);
    sliderThumbs->setCurrentItem(opts.sliderThumbs);
    handles->setCurrentItem(opts.handles);
    appearance->setCurrentItem(opts.appearance);
    focus->setCurrentItem(opts.focus);
    lvLines->setChecked(opts.lvLines);
    lvButton->setChecked(opts.lvButton);
    drawStatusBarFrames->setChecked(opts.drawStatusBarFrames);
    buttonEffect->setCurrentItem(opts.buttonEffect);
    coloredMouseOver->setCurrentItem(opts.coloredMouseOver);
    menubarMouseOver->setChecked(opts.menubarMouseOver);
    shadeMenubarOnlyWhenActive->setChecked(opts.shadeMenubarOnlyWhenActive);
    thinnerMenuItems->setChecked(opts.thinnerMenuItems);
    fixParentlessDialogs->setChecked(opts.fixParentlessDialogs);
    animatedProgress->setChecked(opts.animatedProgress);
    stripedProgress->setCurrentItem(opts.stripedProgress);
    embolden->setChecked(opts.embolden);
    defBtnIndicator->setCurrentItem(opts.defBtnIndicator);
    highlightTab->setChecked(opts.highlightTab);
    menubarAppearance->setCurrentItem(opts.menubarAppearance);
    toolbarAppearance->setCurrentItem(opts.toolbarAppearance);
    lvAppearance->setCurrentItem(opts.lvAppearance);
    sliderAppearance->setCurrentItem(opts.sliderAppearance);
    tabAppearance->setCurrentItem(opts.tabAppearance);
    activeTabAppearance->setCurrentItem(opts.activeTabAppearance);
    toolbarSeparators->setCurrentItem(opts.toolbarSeparators);
    splitters->setCurrentItem(opts.splitters);
    shadeSliders->setCurrentItem(opts.shadeSliders);
    shadeMenubars->setCurrentItem(opts.shadeMenubars);
    highlightFactor->setValue(opts.highlightFactor);
    customSlidersColor->setColor(opts.customSlidersColor);
    customMenubarsColor->setColor(opts.customMenubarsColor);
    customMenuNormTextColor->setColor(opts.customMenuNormTextColor);
    customMenuSelTextColor->setColor(opts.customMenuSelTextColor);
    customMenuTextColor->setChecked(opts.customMenuTextColor);

    customSlidersColor->setEnabled(SHADE_CUSTOM==opts.shadeSliders);
    customMenubarsColor->setEnabled(SHADE_CUSTOM==opts.shadeMenubars);
    customMenuNormTextColor->setEnabled(customMenuTextColor->isChecked());
    customMenuSelTextColor->setEnabled(customMenuTextColor->isChecked());
    customCheckRadioColor->setEnabled(SHADE_CUSTOM==opts.shadeCheckRadio);
    customMenuStripeColor->setEnabled(SHADE_CUSTOM==opts.menuStripe);
    menuStripeAppearance->setEnabled(SHADE_NONE!=opts.menuStripe);

    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentItem());

    fillSlider->setChecked(opts.fillSlider);
    sliderStyle->setCurrentItem(opts.sliderStyle);
    roundMbTopOnly->setChecked(opts.roundMbTopOnly);
    fillProgress->setChecked(opts.fillProgress);
    darkerBorders->setChecked(opts.darkerBorders);
    comboSplitter->setChecked(opts.comboSplitter);
    comboBtn->setCurrentItem(opts.comboBtn);
    customComboBtnColor->setColor(opts.customComboBtnColor);
    unifySpinBtns->setChecked(opts.unifySpinBtns);
    vArrows->setChecked(opts.vArrows);
    xCheck->setChecked(opts.xCheck);
    crHighlight->setChecked(opts.crHighlight);
    crButton->setChecked(opts.crButton);
    colorSelTab->setChecked(opts.colorSelTab);
    roundAllTabs->setChecked(opts.roundAllTabs);
    tabMouseOver->setCurrentItem(opts.tabMouseOver);
    stdSidebarButtons->setChecked(opts.stdSidebarButtons);
    borderMenuitems->setChecked(opts.borderMenuitems);
    popupBorder->setChecked(opts.popupBorder);
    progressAppearance->setCurrentItem(opts.progressAppearance);
    progressGrooveAppearance->setCurrentItem(opts.progressGrooveAppearance);
    grooveAppearance->setCurrentItem(opts.grooveAppearance);
    sunkenAppearance->setCurrentItem(opts.sunkenAppearance);
    progressGrooveColor->setCurrentItem(opts.progressGrooveColor);
    menuitemAppearance->setCurrentItem(opts.menuitemAppearance);
    titlebarAppearance->setCurrentItem(opts.titlebarAppearance);
    inactiveTitlebarAppearance->setCurrentItem(opts.inactiveTitlebarAppearance);
    titlebarButtonAppearance->setCurrentItem(opts.titlebarButtonAppearance);
    colorTitlebarOnly->setChecked(opts.colorTitlebarOnly);
    selectionAppearance->setCurrentItem(opts.selectionAppearance);
    shadeCheckRadio->setCurrentItem(opts.shadeCheckRadio);
    customCheckRadioColor->setColor(opts.customCheckRadioColor);
    colorMenubarMouseOver->setChecked(opts.colorMenubarMouseOver);
    useHighlightForMenu->setChecked(opts.useHighlightForMenu);
    groupBoxLine->setChecked(opts.groupBoxLine);
    fadeLines->setChecked(opts.fadeLines);
    titlebarAlignment->setCurrentItem(opts.titlebarAlignment);

    shading->setCurrentItem(opts.shading);
    gtkScrollViews->setChecked(opts.gtkScrollViews);
    highlightScrollViews->setChecked(opts.highlightScrollViews);
    squareScrollViews->setChecked(opts.squareScrollViews);
    sunkenScrollViews->setChecked(opts.sunkenScrollViews);
    flatSbarButtons->setChecked(opts.flatSbarButtons);
    thinSbarGroove->setChecked(opts.thinSbarGroove);
    colorSliderMouseOver->setChecked(opts.colorSliderMouseOver);
    titlebarBorder->setChecked(opts.titlebarBorder);
    sbarBgndAppearance->setCurrentItem(opts.sbarBgndAppearance);
    sliderFill->setCurrentItem(opts.sliderFill);
    gtkComboMenus->setChecked(opts.gtkComboMenus);
    gtkButtonOrder->setChecked(opts.gtkButtonOrder);
    mapKdeIcons->setChecked(opts.mapKdeIcons);
    setPasswordChar(opts.passwordChar);
    framelessGroupBoxes->setChecked(opts.framelessGroupBoxes);
    customGradient=opts.customGradient;
    gradCombo->setCurrentItem(APPEARANCE_CUSTOM1);

    populateShades(opts);
}

bool QtCurveConfig::settingsChanged()
{
    return round->currentItem()!=currentStyle.round ||
         toolbarBorders->currentItem()!=currentStyle.toolbarBorders ||
         appearance->currentItem()!=(int)currentStyle.appearance ||
         focus->currentItem()!=(int)currentStyle.focus ||
         lvLines->isChecked()!=currentStyle.lvLines ||
         lvButton->isChecked()!=currentStyle.lvButton ||
         drawStatusBarFrames->isChecked()!=currentStyle.drawStatusBarFrames ||
         buttonEffect->currentItem()!=(int)currentStyle.buttonEffect ||
         coloredMouseOver->currentItem()!=(int)currentStyle.coloredMouseOver ||
         menubarMouseOver->isChecked()!=currentStyle.menubarMouseOver ||
         shadeMenubarOnlyWhenActive->isChecked()!=currentStyle.shadeMenubarOnlyWhenActive ||
         thinnerMenuItems->isChecked()!=currentStyle.thinnerMenuItems ||
         fixParentlessDialogs->isChecked()!=currentStyle.fixParentlessDialogs ||
         animatedProgress->isChecked()!=currentStyle.animatedProgress ||
         stripedProgress->currentItem()!=currentStyle.stripedProgress ||
         lighterPopupMenuBgnd->value()!=currentStyle.lighterPopupMenuBgnd ||
         menuDelay->value()!=currentStyle.menuDelay ||
         menuStripe->currentItem()!=currentStyle.menuStripe ||
         menuStripeAppearance->currentItem()!=currentStyle.menuStripeAppearance ||
         embolden->isChecked()!=currentStyle.embolden ||
         fillSlider->isChecked()!=currentStyle.fillSlider ||
         sliderStyle->currentItem()!=currentStyle.sliderStyle ||
         roundMbTopOnly->isChecked()!=currentStyle.roundMbTopOnly ||
         fillProgress->isChecked()!=currentStyle.fillProgress ||
         darkerBorders->isChecked()!=currentStyle.darkerBorders ||
         comboSplitter->isChecked()!=currentStyle.comboSplitter ||
         comboBtn->currentItem()!=(int)currentStyle.comboBtn ||
         unifySpinBtns->isChecked()!=currentStyle.unifySpinBtns ||
         vArrows->isChecked()!=currentStyle.vArrows ||
         xCheck->isChecked()!=currentStyle.xCheck ||
         crHighlight->isChecked()!=currentStyle.crHighlight ||
         crButton->isChecked()!=currentStyle.crButton ||
         colorSelTab->isChecked()!=currentStyle.colorSelTab ||
         roundAllTabs->isChecked()!=currentStyle.roundAllTabs ||
         tabMouseOver->currentItem()!=currentStyle.tabMouseOver ||
         stdSidebarButtons->isChecked()!=currentStyle.stdSidebarButtons ||
         borderMenuitems->isChecked()!=currentStyle.borderMenuitems ||
         popupBorder->isChecked()!=currentStyle.popupBorder ||
         defBtnIndicator->currentItem()!=(int)currentStyle.defBtnIndicator ||
         sliderThumbs->currentItem()!=(int)currentStyle.sliderThumbs ||
         handles->currentItem()!=(int)currentStyle.handles ||
         scrollbarType->currentItem()!=(int)currentStyle.scrollbarType ||
         highlightTab->isChecked()!=currentStyle.highlightTab ||
         shadeSliders->currentItem()!=(int)currentStyle.shadeSliders ||
         shadeMenubars->currentItem()!=(int)currentStyle.shadeMenubars ||
         shadeCheckRadio->currentItem()!=(int)currentStyle.shadeCheckRadio ||
         menubarAppearance->currentItem()!=currentStyle.menubarAppearance ||
         toolbarAppearance->currentItem()!=currentStyle.toolbarAppearance ||
         lvAppearance->currentItem()!=currentStyle.lvAppearance ||
         sliderAppearance->currentItem()!=currentStyle.sliderAppearance ||
         tabAppearance->currentItem()!=currentStyle.tabAppearance ||
         activeTabAppearance->currentItem()!=currentStyle.activeTabAppearance ||
         progressAppearance->currentItem()!=currentStyle.progressAppearance ||
         progressGrooveAppearance->currentItem()!=currentStyle.progressGrooveAppearance ||
         grooveAppearance->currentItem()!=currentStyle.grooveAppearance ||
         sunkenAppearance->currentItem()!=currentStyle.sunkenAppearance ||
         progressGrooveColor->currentItem()!=currentStyle.progressGrooveColor ||
         menuitemAppearance->currentItem()!=currentStyle.menuitemAppearance ||
         titlebarAppearance->currentItem()!=currentStyle.titlebarAppearance ||
         inactiveTitlebarAppearance->currentItem()!=currentStyle.inactiveTitlebarAppearance ||
         titlebarButtonAppearance->currentItem()!=currentStyle.titlebarButtonAppearance ||
         colorTitlebarOnly->isChecked()!=currentStyle.colorTitlebarOnly ||
         selectionAppearance->currentItem()!=currentStyle.selectionAppearance ||
         toolbarSeparators->currentItem()!=currentStyle.toolbarSeparators ||
         splitters->currentItem()!=currentStyle.splitters ||
         colorMenubarMouseOver->isChecked()!=currentStyle.colorMenubarMouseOver ||
         useHighlightForMenu->isChecked()!=currentStyle.useHighlightForMenu ||
         groupBoxLine->isChecked()!=currentStyle.groupBoxLine ||
         fadeLines->isChecked()!=currentStyle.fadeLines ||
         titlebarAlignment->currentItem()!=currentStyle.titlebarAlignment ||

         shading->currentItem()!=(int)currentStyle.shading ||
         gtkScrollViews->isChecked()!=currentStyle.gtkScrollViews ||
         highlightScrollViews->isChecked()!=currentStyle.highlightScrollViews ||
         squareScrollViews->isChecked()!=currentStyle.squareScrollViews ||
         sunkenScrollViews->isChecked()!=currentStyle.sunkenScrollViews ||
         flatSbarButtons->isChecked()!=currentStyle.flatSbarButtons ||
         thinSbarGroove->isChecked()!=currentStyle.thinSbarGroove ||
         colorSliderMouseOver->isChecked()!=currentStyle.colorSliderMouseOver ||
         titlebarBorder->isChecked()!=currentStyle.titlebarBorder ||
         sbarBgndAppearance->currentItem()!=currentStyle.sbarBgndAppearance ||
         sliderFill->currentItem()!=currentStyle.sliderFill ||
         gtkComboMenus->isChecked()!=currentStyle.gtkComboMenus ||
         gtkButtonOrder->isChecked()!=currentStyle.gtkButtonOrder ||
         mapKdeIcons->isChecked()!=currentStyle.mapKdeIcons ||
         framelessGroupBoxes->isChecked()!=currentStyle.framelessGroupBoxes ||

         toInt(passwordChar->text())!=currentStyle.passwordChar ||

         highlightFactor->value()!=currentStyle.highlightFactor ||
         customMenuTextColor->isChecked()!=currentStyle.customMenuTextColor ||
         (SHADE_CUSTOM==currentStyle.shadeSliders &&
               customSlidersColor->color()!=currentStyle.customSlidersColor) ||
         (SHADE_CUSTOM==currentStyle.shadeMenubars &&
               customMenubarsColor->color()!=currentStyle.customMenubarsColor) ||
         (SHADE_CUSTOM==currentStyle.shadeCheckRadio &&
               customCheckRadioColor->color()!=currentStyle.customCheckRadioColor) ||
         (customMenuTextColor->isChecked() &&
               customMenuNormTextColor->color()!=currentStyle.customMenuNormTextColor) ||
         (customMenuTextColor->isChecked() &&
               customMenuSelTextColor->color()!=currentStyle.customMenuSelTextColor) ||
         (SHADE_CUSTOM==currentStyle.menuStripe &&
               customMenuStripeColor->color()!=currentStyle.customMenuStripeColor) ||
         (SHADE_CUSTOM==currentStyle.comboBtn &&
               customComboBtnColor->color()!=currentStyle.customComboBtnColor) ||

         customGradient!=currentStyle.customGradient;

         diffShades(currentStyle);
}

#include "qtcurveconfig.moc"
