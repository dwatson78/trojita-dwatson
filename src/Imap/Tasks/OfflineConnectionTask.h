/* Copyright (C) 2007 - 2012 Jan Kundrát <jkt@flaska.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAP_OFFLINECONNECTIONTASK_H
#define IMAP_OFFLINECONNECTIONTASK_H

#include "ImapTask.h"
#include "../Model/Model.h"

namespace Imap {
namespace Mailbox {

/** @short Create new "connection" that will fail immediately

The whole point of this class is to create a Parser because the whole stack depends on its existence too much.
*/
class OfflineConnectionTask : public ImapTask
{
Q_OBJECT
public:
    OfflineConnectionTask(Model* _model);
    virtual void perform();
private:
    Socket *m_sock;
};

}
}

#endif // IMAP_OFFLINECONNECTIONTASK_H