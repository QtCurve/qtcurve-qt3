#ifndef __QTCURVECONFIG_H__
#define __QTCURVECONFIG_H__

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

#define QTC_COMMON_FUNCTIONS
#define QTC_CONFIG_DIALOG

#include <qtcurveconfigbase.h>
#include <qcombobox.h>
#include <qmap.h>
#include "common.h"

class QPopupMenu;
class QListViewItem;
class KDoubleNumInput;
class CExportThemeDialog;
class CStackItem;
class QtCurveConfig;

class CGradientPreview : public QWidget
{
    Q_OBJECT

    public:

    CGradientPreview(QtCurveConfig *c, QWidget *p);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void paintEvent(QPaintEvent *);
    void setGrad(const GradientStopCont &s);

    public slots:

    void setColor(const QColor &col);

    private:

    QtCurveConfig    *cfg;
    QColor           color;
    GradientStopCont stops;
};

class QtCurveConfig : public QtCurveConfigBase
{
    Q_OBJECT

    public:

    QtCurveConfig(QWidget *parent);
    virtual ~QtCurveConfig();

    EShading currentShading() const { return (EShading)shading->currentItem(); }

    signals:

    void changed(bool);

    private:

    void loadStyles(QPopupMenu *menu);

    public slots:

    void save();
    void defaults();

    private slots:

    void setStyle(int s);
    void updateChanged();
    void focusChanged();
    void roundChanged();
    void importStyle();
    void exportStyle();
    void exportTheme();
    void emboldenToggled();
    void defBtnIndicatorChanged();
    void buttonEffectChanged();
    void coloredMouseOverChanged();
    void shadeSlidersChanged();
    void shadeMenubarsChanged();
    void shadeCheckRadioChanged();
    void customMenuTextColorChanged();
    void menuStripeChanged();
    void stripedProgressChanged();
    void activeTabAppearanceChanged();
    void tabMoChanged();
    void shadingChanged();
    void passwordCharClicked();

    void changeStack();
    void gradChanged(int i);
    void itemChanged(QListViewItem *i, int col);
    void addGradStop();
    void removeGradStop();
    void updateGradStop();
    void stopSelected();

    private:

    void setupStack();
    void setupGradientsTab();
    void setupShadesTab();
    void setupShade(KDoubleNumInput *w, int shade);
    void populateShades(const Options &opts);
    bool diffShades(const Options &opts);
    void setPasswordChar(int ch);
    void loadStyle(const QString &file);
    void setOptions(Options &opts);
    void setWidgetOptions(const Options &opts);
    bool settingsChanged();

    private:

    Options            currentStyle,
                       defaultStyle;
    QMap<int, QString> styles;
    CExportThemeDialog *exportDialog;
    CGradientPreview   *gradPreview;
    GradientCont       customGradient;
    KDoubleNumInput    *shadeVals[NUM_STD_SHADES];
    CStackItem         *lastCategory;
};

#endif
