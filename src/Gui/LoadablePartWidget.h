/* Copyright (C) 2006 - 2013 Jan Kundrát <jkt@flaska.net>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LOADABLEPARTWIDGET_H
#define LOADABLEPARTWIDGET_H

#include <QPersistentModelIndex>
#include <QStackedWidget>

#include "AbstractPartWidget.h"
#include "SimplePartWidget.h"

class QPushButton;

namespace Gui
{

class MessageView;

/** @short Widget which implements delayed loading of message parts

  This class supports two modes of loading, either a "click-through" one for loading message parts
  on demand after the user clicks a button, or an "automated" mode where the data are loaded after
  this widget becomes visible.

*/
class LoadablePartWidget : public QStackedWidget, public AbstractPartWidget
{
    Q_OBJECT
public:
    /** @short Load when the widget becomes visible, or wait until the user clicks a button? */
    typedef enum {
        LOAD_ON_SHOW, /**< @short Load as soon as the widget becomes visible */
        LOAD_ON_CLICK /**< @short Load onlt after the user has clicked a button */
    } LoadingTriggerMode;
    LoadablePartWidget(QWidget *parent, Imap::Network::MsgPartNetAccessManager *manager, const QModelIndex &part,
                       MessageView *messageView, const LoadingTriggerMode mode);
    QString quoteMe() const;
    virtual void reloadContents() {}
protected:
    virtual void showEvent(QShowEvent *event);
private slots:
    void loadClicked();
private:
    Imap::Network::MsgPartNetAccessManager *manager;
    QPersistentModelIndex partIndex;
    MessageView *m_messageView;
    SimplePartWidget *realPart;
    QPushButton *loadButton;
    bool m_loadOnShow;

    LoadablePartWidget(const LoadablePartWidget &); // don't implement
    LoadablePartWidget &operator=(const LoadablePartWidget &); // don't implement
};

}

#endif // LOADABLEPARTWIDGET_H
