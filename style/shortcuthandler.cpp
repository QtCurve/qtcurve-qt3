/*
  QtCurve (C) Craig Drummond, 2007 - 2010 craig.p.drummond@gmail.com

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

#include "shortcuthandler.h"
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qobjectlist.h>
#include <qmainwindow.h>
#include <qdialog.h>
#include <qstyle.h>

ShortcutHandler::ShortcutHandler(QObject *parent)
               : QObject(parent)
{
}

ShortcutHandler::~ShortcutHandler()
{
}

bool ShortcutHandler::hasSeenAlt(const QWidget *widget) const
{
    if(::qt_cast<const QPopupMenu *>(widget))
    {
        const QWidget *w=widget;
        
        while(w)
        {
            if(itsSeenAlt.contains((QWidget *)w))
                return true;
            w=w->parentWidget();
        }
    }

    widget = widget->topLevelWidget();
    return itsSeenAlt.contains((QWidget *)widget);
}

bool ShortcutHandler::showShortcut(const QWidget *widget) const
{
    return itsAltDown && hasSeenAlt(widget);
}

void ShortcutHandler::widgetDestroyed(QObject *o)
{
    itsUpdated.remove(static_cast<QWidget *>(o));
}

void ShortcutHandler::updateWidget(QWidget *w)
{
    if(!itsUpdated.contains(w))
    {
        connect(w, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
        itsUpdated.append(w);
        w->repaint(TRUE);
    }
}

void ShortcutHandler::setSeenAlt(QWidget *w)
{
    if(!itsSeenAlt.contains(w))
        itsSeenAlt.append(w);
}

bool ShortcutHandler::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    QWidget *widget = ::qt_cast<QWidget*>(o);
    switch(e->type()) 
    {
        case QEvent::KeyPress:
            if (((QKeyEvent*)e)->key() == Key_Alt)
            {
                itsAltDown = true;

                if(::qt_cast<QPopupMenu *>(widget))
                {
                    QWidget *w=widget;

                    while(w)
                    {
                        setSeenAlt(w);
                        updateWidget(w);
                        w=w->parentWidget();
                    }
                    if(!widget->parentWidget())
                    {
                        setSeenAlt(qApp->activeWindow());
                        updateWidget(qApp->activeWindow());
                    }
                }
                
                widget = widget->topLevelWidget();
                setSeenAlt(widget);

                // Alt has been pressed - find all widgets that care
                QObjectList *l = widget->queryList("QWidget");
                QObjectListIt it( *l );
                QWidget *w;
                while ((w = (QWidget *)it.current()) != 0)
                {
                    ++it;
                    if (!(w->isTopLevel() || !w->isVisible())) // || w->style().styleHint(QStyle::SH_UnderlineAccelerator, w)))
                        updateWidget(w);
                }
                delete l;
            }
            break;
        case QEvent::KeyRelease:
            if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Alt)
            {
                itsAltDown = false;
                QValueList<QWidget *>::const_iterator it(itsUpdated.begin()),
                                                      end(itsUpdated.end());
                                           
                for (; it!=end; ++it)
                    (*it)->update();
                itsSeenAlt.clear();
                itsUpdated.clear();
                // TODO: If menu is popuped up, it doesn't clear underlines...
            }
            break;
        case QEvent::WindowDeactivate:
        case QEvent::Close:
            // Reset widget when closing
            itsSeenAlt.remove(widget);
            itsUpdated.remove(widget);
            itsSeenAlt.remove(widget->topLevelWidget());
            break;
        default:
            break;
    }
    return QObject::eventFilter(o, e);
}

#include "shortcuthandler.moc"
