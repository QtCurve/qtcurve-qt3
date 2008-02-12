/*
  QtCurve (C) Craig Drummond, 2003 - 2007 Craig.Drummond@lycos.co.uk

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
#include <klocale.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kcharselect.h>
#include <kdialogbase.h>
#include <knuminput.h>
#include <unistd.h>
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

static int toInt(const QString &str)
{
    return str.length()>1 ? str[0].unicode() : 0;
}

static void insertShadeEntries(QComboBox *combo, bool withDarken, bool checkRadio=false)
{
    combo->insertItem(checkRadio ? i18n("Text")
                                 : withDarken ? i18n("Background")
                                              : i18n("Button"));
    combo->insertItem(i18n("Custom:"));

    if(checkRadio) // For check/radio, we dont blend, and dont allow darken
        combo->insertItem(i18n("Selected background"));
    else if(withDarken)
    {
         // For menubars we dont actually blend...
        combo->insertItem(i18n("Selected background"));
        combo->insertItem(i18n("Darken"));
    }
    else
    {
        combo->insertItem(i18n("Blended selected background"));
        combo->insertItem(i18n("Selected background"));
    }
}

static void insertAppearanceEntries(QComboBox *combo, bool split=true, bool bev=true)
{
    combo->insertItem(i18n("Flat"));
    combo->insertItem(i18n("Raised"));
    combo->insertItem(i18n("Dull glass"));
    combo->insertItem(i18n("Shiny glass"));
    combo->insertItem(i18n("Gradient"));
    combo->insertItem(i18n("Inverted gradient"));
    if(split)
    {
        combo->insertItem(i18n("Split gradient"));
        if(bev)
            combo->insertItem(i18n("Bevelled"));
    }
}

static void insertLineEntries(QComboBox *combo, bool none)
{
    combo->insertItem(i18n("Sunken lines"));
    combo->insertItem(i18n("Flat lines"));
    combo->insertItem(i18n("Dots"));
    combo->insertItem(none ? i18n("None") : i18n("Dashes"));
}

static void insertDefBtnEntries(QComboBox *combo)
{
    combo->insertItem(i18n("Corner indicator"));
    combo->insertItem(i18n("Font color thin border"));
    combo->insertItem(i18n("Selected background thick border"));
    combo->insertItem(i18n("None"));
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
}

static void insertMouseOverEntries(QComboBox *combo)
{
    combo->insertItem(i18n("No coloration"));
    combo->insertItem(i18n("Color border"));
    combo->insertItem(i18n("Plastik style"));
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
    combo->insertItem(i18n("Triangular"));
}

QtCurveConfig::QtCurveConfig(QWidget *parent)
             : QtCurveConfigBase(parent),
               exportDialog(NULL)
{
    titleLabel->setText("QtCurve " VERSION " - (C) Craig Drummond, 2003-2007");
    insertShadeEntries(shadeSliders, false);
    insertShadeEntries(shadeMenubars, true);
    insertShadeEntries(shadeCheckRadio, false, true);
    insertAppearanceEntries(appearance);
    insertAppearanceEntries(menubarAppearance);
    insertAppearanceEntries(toolbarAppearance);
    insertAppearanceEntries(lvAppearance);
    insertAppearanceEntries(sliderAppearance);
    insertAppearanceEntries(tabAppearance, false, false);
    insertAppearanceEntries(progressAppearance);
    insertAppearanceEntries(menuitemAppearance);
    insertAppearanceEntries(titlebarAppearance, true, false);
    insertLineEntries(handles, false);
    insertLineEntries(sliderThumbs, true);
    insertLineEntries(toolbarSeparators, true);
    insertLineEntries(splitters, false);
    insertDefBtnEntries(defBtnIndicator);
    insertScrollbarEntries(scrollbarType);
    insertRoundEntries(round);
    insertMouseOverEntries(coloredMouseOver);
    insertToolbarBorderEntries(toolbarBorders);
    insertEffectEntries(buttonEffect);
    insertShadingEntries(shading);
    insertStripeEntries(stripedProgress);
    insertSliderStyleEntries(sliderStyle);

    highlightFactor->setMinValue(MIN_HIGHLIGHT_FACTOR);
    highlightFactor->setMaxValue(MAX_HIGHLIGHT_FACTOR);
    highlightFactor->setValue(((int)(DEFAULT_HIGHLIGHT_FACTOR*100))-100);

    connect(lighterPopupMenuBgnd, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(round, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(toolbarBorders, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderThumbs, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(handles, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(appearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(customMenuTextColor, SIGNAL(toggled(bool)), SLOT(customMenuTextColorChanged()));
    connect(stripedProgress, SIGNAL(activated(int)), SLOT(stripedProgressChanged()));
    connect(animatedProgress, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(embolden, SIGNAL(toggled(bool)), SLOT(emboldenToggled()));
    connect(defBtnIndicator, SIGNAL(activated(int)), SLOT(dbiChanged()));
    connect(highlightTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(menubarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(toolbarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(lvAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(sliderAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(tabAppearance, SIGNAL(activated(int)), SLOT(tabAppearanceChanged()));
    connect(toolbarSeparators, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(splitters, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(fixParentlessDialogs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(fillSlider, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(sliderStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(roundMbTopOnly, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gradientPbGroove, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(darkerBorders, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(vArrows, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(xCheck, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(colorSelTab, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(stdSidebarButtons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(borderMenuitems, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(progressAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(menuitemAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(titlebarAppearance, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(shadeCheckRadio, SIGNAL(activated(int)), SLOT(shadeCheckRadioChanged()));
    connect(customCheckRadioColor, SIGNAL(changed(const QColor &)), SLOT(updateChanged()));

#ifdef QTC_PLAIN_FOCUS_ONLY
    delete stdFocus;
#else
    connect(stdFocus, SIGNAL(toggled(bool)), SLOT(updateChanged()));
#endif
    connect(lvLines, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(drawStatusBarFrames, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(buttonEffect, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(coloredMouseOver, SIGNAL(activated(int)), SLOT(updateChanged()));
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
    connect(shading, SIGNAL(activated(int)), SLOT(updateChanged()));
    connect(gtkScrollViews, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gtkComboMenus, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(gtkButtonOrder, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(mapKdeIcons, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(passwordChar, SIGNAL(clicked()), SLOT(passwordCharClicked()));
    connect(framelessGroupBoxes, SIGNAL(toggled(bool)), SLOT(updateChanged()));
    connect(inactiveHighlight, SIGNAL(toggled(bool)), SLOT(updateChanged()));

    defaultSettings(&defaultStyle);
    if(!readConfig(NULL, &currentStyle, &defaultStyle))
        currentStyle=defaultStyle;

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
        defBtnIndicator->setCurrentItem(IND_COLORED);
    updateChanged();
}

void QtCurveConfig::dbiChanged()
{
    if(IND_NONE==defBtnIndicator->currentItem() && !embolden->isChecked())
        embolden->setChecked(true);
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

void QtCurveConfig::stripedProgressChanged()
{
    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentItem());
    if(animatedProgress->isChecked() && STRIPE_NONE==stripedProgress->currentItem())
        animatedProgress->setChecked(false);
    updateChanged();
}

void QtCurveConfig::tabAppearanceChanged()
{
    if(colorSelTab->isChecked() && APPEARANCE_GRADIENT!=tabAppearance->currentItem())
        colorSelTab->setChecked(false);
    colorSelTab->setEnabled(APPEARANCE_GRADIENT==tabAppearance->currentItem());
    updateChanged();
}

void QtCurveConfig::passwordCharClicked()
{
    int              cur(toInt(passwordChar->text()));
    CharSelectDialog dlg(this, cur);

    if(QDialog::Accepted==dlg.exec() && dlg.currentChar()!=cur)
        setPasswordChar(dlg.currentChar());
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
#ifndef QTC_PLAIN_FOCUS_ONLY
    opts.stdFocus=stdFocus->isChecked();
#endif
    opts.lvLines=lvLines->isChecked();
    opts.drawStatusBarFrames=drawStatusBarFrames->isChecked();
    opts.buttonEffect=(EEffect)buttonEffect->currentItem();
    opts.coloredMouseOver=(EMouseOver)coloredMouseOver->currentItem();
    opts.menubarMouseOver=menubarMouseOver->isChecked();
    opts.shadeMenubarOnlyWhenActive=shadeMenubarOnlyWhenActive->isChecked();
    opts.thinnerMenuItems=thinnerMenuItems->isChecked();
    opts.fixParentlessDialogs=fixParentlessDialogs->isChecked();
    opts.animatedProgress=animatedProgress->isChecked();
    opts.stripedProgress=(EStripe)stripedProgress->currentItem();
    opts.lighterPopupMenuBgnd=lighterPopupMenuBgnd->isChecked();
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
    opts.toolbarSeparators=(ELine)toolbarSeparators->currentItem();
    opts.splitters=(ELine)splitters->currentItem();
    opts.customSlidersColor=customSlidersColor->color();
    opts.customMenubarsColor=customMenubarsColor->color();
    opts.highlightFactor=((double)(highlightFactor->value()+100))/100.0;
    opts.customMenuNormTextColor=customMenuNormTextColor->color();
    opts.customMenuSelTextColor=customMenuSelTextColor->color();
    opts.customMenuTextColor=customMenuTextColor->isChecked();
    opts.fillSlider=fillSlider->isChecked();
    opts.sliderStyle=(ESliderStyle)sliderStyle->currentItem();
    opts.roundMbTopOnly=roundMbTopOnly->isChecked();
    opts.gradientPbGroove=gradientPbGroove->isChecked();
    opts.darkerBorders=darkerBorders->isChecked();
    opts.vArrows=vArrows->isChecked();
    opts.xCheck=xCheck->isChecked();
    opts.colorSelTab=colorSelTab->isChecked();
    opts.stdSidebarButtons=stdSidebarButtons->isChecked();
    opts.borderMenuitems=borderMenuitems->isChecked();
    opts.progressAppearance=(EAppearance)progressAppearance->currentItem();
    opts.menuitemAppearance=(EAppearance)menuitemAppearance->currentItem();
    opts.titlebarAppearance=(EAppearance)titlebarAppearance->currentItem();
    opts.shadeCheckRadio=(EShade)shadeCheckRadio->currentItem();
    opts.customCheckRadioColor=customCheckRadioColor->color();
    opts.shading=(EShading)shading->currentItem();
    opts.gtkScrollViews=gtkScrollViews->isChecked();
    opts.gtkComboMenus=gtkComboMenus->isChecked();
    opts.gtkButtonOrder=gtkButtonOrder->isChecked();
    opts.mapKdeIcons=mapKdeIcons->isChecked();
    opts.passwordChar=toInt(passwordChar->text());
    opts.framelessGroupBoxes=framelessGroupBoxes->isChecked();
    opts.inactiveHighlight=inactiveHighlight->isChecked();
}

void QtCurveConfig::setWidgetOptions(const Options &opts)
{
    round->setCurrentItem(opts.round);
    scrollbarType->setCurrentItem(opts.scrollbarType);
    lighterPopupMenuBgnd->setChecked(opts.lighterPopupMenuBgnd);
    toolbarBorders->setCurrentItem(opts.toolbarBorders);
    sliderThumbs->setCurrentItem(opts.sliderThumbs);
    handles->setCurrentItem(opts.handles);
    appearance->setCurrentItem(opts.appearance);
#ifndef QTC_PLAIN_FOCUS_ONLY
    stdFocus->setChecked(opts.stdFocus);
#endif
    lvLines->setChecked(opts.lvLines);
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
    toolbarSeparators->setCurrentItem(opts.toolbarSeparators);
    splitters->setCurrentItem(opts.splitters);
    shadeSliders->setCurrentItem(opts.shadeSliders);
    shadeMenubars->setCurrentItem(opts.shadeMenubars);
    highlightFactor->setValue((int)(opts.highlightFactor*100)-100);
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

    animatedProgress->setEnabled(STRIPE_NONE!=stripedProgress->currentItem());

    fillSlider->setChecked(opts.fillSlider);
    sliderStyle->setCurrentItem(opts.sliderStyle);
    roundMbTopOnly->setChecked(opts.roundMbTopOnly);
    gradientPbGroove->setChecked(opts.gradientPbGroove);
    darkerBorders->setChecked(opts.darkerBorders);
    vArrows->setChecked(opts.vArrows);
    xCheck->setChecked(opts.xCheck);
    colorSelTab->setChecked(opts.colorSelTab);
    stdSidebarButtons->setChecked(opts.stdSidebarButtons);
    borderMenuitems->setChecked(opts.borderMenuitems);
    progressAppearance->setCurrentItem(opts.progressAppearance);
    menuitemAppearance->setCurrentItem(opts.menuitemAppearance);
    titlebarAppearance->setCurrentItem(opts.titlebarAppearance);
    shadeCheckRadio->setCurrentItem(opts.shadeCheckRadio);
    customCheckRadioColor->setColor(opts.customCheckRadioColor);

    shading->setCurrentItem(opts.shading);
    gtkScrollViews->setChecked(opts.gtkScrollViews);
    gtkComboMenus->setChecked(opts.gtkComboMenus);
    gtkButtonOrder->setChecked(opts.gtkButtonOrder);
    mapKdeIcons->setChecked(opts.mapKdeIcons);
    setPasswordChar(opts.passwordChar);
    framelessGroupBoxes->setChecked(opts.framelessGroupBoxes);
    inactiveHighlight->setChecked(opts.inactiveHighlight);
}

bool QtCurveConfig::settingsChanged()
{
    return round->currentItem()!=currentStyle.round ||
         toolbarBorders->currentItem()!=currentStyle.toolbarBorders ||
         appearance->currentItem()!=(int)currentStyle.appearance ||
#ifndef QTC_PLAIN_FOCUS_ONLY
         stdFocus->isChecked()!=currentStyle.stdFocus ||
#endif
         lvLines->isChecked()!=currentStyle.lvLines ||
         drawStatusBarFrames->isChecked()!=currentStyle.drawStatusBarFrames ||
         buttonEffect->currentItem()!=(int)currentStyle.buttonEffect ||
         coloredMouseOver->currentItem()!=(int)currentStyle.coloredMouseOver ||
         menubarMouseOver->isChecked()!=currentStyle.menubarMouseOver ||
         shadeMenubarOnlyWhenActive->isChecked()!=currentStyle.shadeMenubarOnlyWhenActive ||
         thinnerMenuItems->isChecked()!=currentStyle.thinnerMenuItems ||
         fixParentlessDialogs->isChecked()!=currentStyle.fixParentlessDialogs ||
         animatedProgress->isChecked()!=currentStyle.animatedProgress ||
         stripedProgress->currentItem()!=currentStyle.stripedProgress ||
         lighterPopupMenuBgnd->isChecked()!=currentStyle.lighterPopupMenuBgnd ||
         embolden->isChecked()!=currentStyle.embolden ||
         fillSlider->isChecked()!=currentStyle.fillSlider ||
         sliderStyle->currentItem()!=currentStyle.sliderStyle ||
         roundMbTopOnly->isChecked()!=currentStyle.roundMbTopOnly ||
         gradientPbGroove->isChecked()!=currentStyle.gradientPbGroove ||
         darkerBorders->isChecked()!=currentStyle.darkerBorders ||
         vArrows->isChecked()!=currentStyle.vArrows ||
         xCheck->isChecked()!=currentStyle.xCheck ||
         colorSelTab->isChecked()!=currentStyle.colorSelTab ||
         stdSidebarButtons->isChecked()!=currentStyle.stdSidebarButtons ||
         borderMenuitems->isChecked()!=currentStyle.borderMenuitems ||
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
         progressAppearance->currentItem()!=currentStyle.progressAppearance ||
         menuitemAppearance->currentItem()!=currentStyle.menuitemAppearance ||
         titlebarAppearance->currentItem()!=currentStyle.titlebarAppearance ||
         toolbarSeparators->currentItem()!=currentStyle.toolbarSeparators ||
         splitters->currentItem()!=currentStyle.splitters ||

         shading->currentItem()!=(int)currentStyle.shading ||
         gtkScrollViews->isChecked()!=currentStyle.gtkScrollViews ||
         gtkComboMenus->isChecked()!=currentStyle.gtkComboMenus ||
         gtkButtonOrder->isChecked()!=currentStyle.gtkButtonOrder ||
         mapKdeIcons->isChecked()!=currentStyle.mapKdeIcons ||
         framelessGroupBoxes->isChecked()!=currentStyle.framelessGroupBoxes ||
         inactiveHighlight->isChecked()!=currentStyle.inactiveHighlight ||

         toInt(passwordChar->text())!=currentStyle.passwordChar ||

         (highlightFactor->value()+100)!=(int)(currentStyle.highlightFactor*100) ||
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
               customMenuSelTextColor->color()!=currentStyle.customMenuSelTextColor);
}

#include "qtcurveconfig.moc"
