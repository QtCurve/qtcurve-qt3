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

#include "exportthemedialog.h"
#include <klocale.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#define CONFIG_WRITE
#include "config_file.c"

CExportThemeDialog::CExportThemeDialog(QWidget *parent)
                  : KDialogBase(parent, "ExportDialog", true, i18n("Export Theme"),
                                Ok|Cancel)
{
    QWidget     *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page, 3, 2, 0, spacingHint());

    layout->addWidget(new QLabel(i18n("Name:"), page), 0, 0);
    layout->addWidget(new QLabel(i18n("Comment:"), page), 1, 0);
    layout->addWidget(new QLabel(i18n("Destination folder:"), page), 2, 0);
    layout->addWidget(themeName=new QLineEdit(page), 0, 1);
    layout->addWidget(themeComment=new QLineEdit(i18n("QtCurve based theme"), page), 1, 1);
    layout->addWidget(themeUrl=new KURLRequester(page), 2, 1);

    themeUrl->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
    themeUrl->lineEdit()->setReadOnly(true);
    themeUrl->setURL(QDir::homeDirPath());
    setMainWidget(page);
}

void CExportThemeDialog::run(const Options &o)
{
    opts=o;
    exec();
}

void CExportThemeDialog::slotOk()
{
    QString name(themeName->text().stripWhiteSpace().lower());

    if(name.isEmpty())
        KMessageBox::error(this, i18n("Name is empty!"));
    else
    {
        QString fileName(themeUrl->url()+"/qtc_"+name+".themerc");

        KConfig cfg(fileName, false, false);
        bool    rv(!cfg.isReadOnly());

        if(rv)
        {
            cfg.setGroup("Misc");
            cfg.writeEntry("Name", themeName->text().stripWhiteSpace());
            cfg.writeEntry("Comment", themeComment->text());
            cfg.setGroup("KDE");
            cfg.writeEntry("WidgetStyle", "qtc_"+name);

            rv=writeConfig(&cfg, opts, opts, true);
        }

        if(rv)
        {
            QDialog::accept();
            KMessageBox::information(this, i18n("Succesfully created:\n%1").arg(fileName));
        }
        else
            KMessageBox::error(this, i18n("Failed to create file: %1").arg(fileName));
    }
}

#include "exportthemedialog.moc"

