/* Copyright (C) 2007 - 2010 Jan Kundrát <jkt@flaska.net>

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

#include "TaskFactory.h"
#include "CopyMoveMessagesTask.h"
#include "CreateMailboxTask.h"
#include "DeleteMailboxTask.h"
#include "ExpungeMailboxTask.h"
#include "FetchMsgMetadataTask.h"
#include "FetchMsgPartTask.h"
#include "GetAnyConnectionTask.h"
#include "KeepMailboxOpenTask.h"
#include "Fake_ListChildMailboxesTask.h"
#include "Fake_OpenConnectionTask.h"
#include "NumberOfMessagesTask.h"
#include "ObtainSynchronizedMailboxTask.h"
#include "OpenConnectionTask.h"
#include "UpdateFlagsTask.h"
#include "Imap/Parser/Parser.h"

namespace Imap {
namespace Mailbox {

TaskFactory::~TaskFactory()
{
}

OpenConnectionTask* TaskFactory::createOpenConnectionTask( Model* _model )
{
    return new OpenConnectionTask( _model );
}

CopyMoveMessagesTask* TaskFactory::createCopyMoveMessagesTask( Model* _model, const QModelIndexList& _messages,
                                                               const QString& _targetMailbox, const CopyMoveOperation _op )
{
    return new CopyMoveMessagesTask( _model, _messages, _targetMailbox, _op );
}

CreateMailboxTask* TaskFactory::createCreateMailboxTask( Model* _model, const QString& _mailbox )
{
    return new CreateMailboxTask( _model, _mailbox );
}

GetAnyConnectionTask* TaskFactory::createGetAnyConnectionTask( Model* _model )
{
    return new GetAnyConnectionTask( _model );
}

ListChildMailboxesTask* TaskFactory::createListChildMailboxesTask( Model* _model, const QModelIndex& mailbox )
{
    return new ListChildMailboxesTask( _model, mailbox );
}

DeleteMailboxTask* TaskFactory::createDeleteMailboxTask( Model* _model, const QString& _mailbox )
{
    return new DeleteMailboxTask( _model, _mailbox );
}

ExpungeMailboxTask* TaskFactory::createExpungeMailboxTask( Model* _model, const QModelIndex& mailbox )
{
    return new ExpungeMailboxTask( _model, mailbox );
}

FetchMsgMetadataTask* TaskFactory::createFetchMsgMetadataTask( Model* _model, const QModelIndexList& messages )
{
    return new FetchMsgMetadataTask( _model, messages );
}

FetchMsgPartTask* TaskFactory::createFetchMsgPartTask( Model* _model, TreeItemMailbox* mailbox, TreeItemPart* part )
{
    return new FetchMsgPartTask( _model, mailbox, part );
}

KeepMailboxOpenTask* TaskFactory::createKeepMailboxOpenTask( Model* _model, const QModelIndex& mailbox, Parser* oldParser )
{
    return new KeepMailboxOpenTask( _model, mailbox, oldParser );
}

NumberOfMessagesTask* TaskFactory::createNumberOfMessagesTask( Model* _model, const QModelIndex& mailbox )
{
    return new NumberOfMessagesTask( _model, mailbox );
}

ObtainSynchronizedMailboxTask* TaskFactory::createObtainSynchronizedMailboxTask( Model* _model, const QModelIndex& _mailboxIndex, ImapTask* parentTask )
{
    return new ObtainSynchronizedMailboxTask( _model, _mailboxIndex, parentTask );
}

UpdateFlagsTask* TaskFactory::createUpdateFlagsTask( Model* _model, const QModelIndexList& _messages, const QString& _flagOperation, const QString& _flags )
{
    return new UpdateFlagsTask( _model, _messages, _flagOperation, _flags );
}

UpdateFlagsTask* TaskFactory::createUpdateFlagsTask( Model* _model, CopyMoveMessagesTask* copyTask, const QList<QPersistentModelIndex>& _messages, const QString& _flagOperation, const QString& _flags )
{
    return new UpdateFlagsTask( _model, copyTask, _messages, _flagOperation, _flags );
}



TestingTaskFactory::TestingTaskFactory(): TaskFactory(), fakeOpenConnectionTask(false), fakeListChildMailboxes(false)
{
}

Parser* TestingTaskFactory::newParser( Model* model )
{
    Parser* parser = new Parser( model, model->_socketFactory->create(), ++model->lastParserId );
    Model::ParserState parserState = Model::ParserState( parser, 0, Model::ReadOnly, CONN_STATE_NONE );
    QObject::connect( parser, SIGNAL(responseReceived()), model, SLOT(responseReceived()) );
    QObject::connect( parser, SIGNAL(disconnected(const QString)), model, SLOT(slotParserDisconnected(const QString)) );
    QObject::connect( parser, SIGNAL(connectionStateChanged(Imap::ConnectionState)), model, SLOT(handleSocketStateChanged(Imap::ConnectionState)) );
    QObject::connect( parser, SIGNAL(sendingCommand(QString)), model, SLOT(parserIsSendingCommand(QString)) );
    QObject::connect( parser, SIGNAL(parseError(QString,QString,QByteArray,int)), model, SLOT(slotParseError(QString,QString,QByteArray,int)) );
    QObject::connect( parser, SIGNAL(lineReceived(QByteArray)), model, SLOT(slotParserLineReceived(QByteArray)) );
    QObject::connect( parser, SIGNAL(lineSent(QByteArray)), model, SLOT(slotParserLineSent(QByteArray)) );
    model->_parsers[ parser ] = parserState;
    return parser;
}

OpenConnectionTask* TestingTaskFactory::createOpenConnectionTask( Model *_model )
{
    if ( fakeOpenConnectionTask ) {
        return new Fake_OpenConnectionTask( _model, newParser( _model ) );
    } else {
        return TaskFactory::createOpenConnectionTask( _model );
    }
}

ListChildMailboxesTask* TestingTaskFactory::createListChildMailboxesTask( Model* _model, const QModelIndex& mailbox )
{
    if ( fakeListChildMailboxes ) {
        return new Fake_ListChildMailboxesTask( _model, mailbox );
    } else {
        return TaskFactory::createListChildMailboxesTask( _model, mailbox );
    }
}

}
}