/* Copyright (C) 2007 - 2008 Jan Kundrát <jkt@gentoo.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QAuthenticator>
#include <QDockWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QTreeView>

#include "Window.h"
#include "ComposeWidget.h"
#include "MessageView.h"
#include "SettingsDialog.h"
#include "SettingsNames.h"
#include "Imap/Model/Model.h"
#include "Imap/Model/MailboxModel.h"
#include "Imap/Model/MailboxTree.h"
#include "Imap/Model/ModelWatcher.h"
#include "Imap/Model/MsgListModel.h"
#include "Imap/Model/MemoryCache.h"
#include "Streams/SocketFactory.h"

#include "Imap/Model/ModelTest/modeltest.h"

namespace Gui {

MainWindow::MainWindow(): QMainWindow()
{
    createWidgets();
    setupModels();
    createActions();
    createMenus();
}

void MainWindow::createActions()
{
    reloadMboxList = new QAction( tr("Rescan Child Mailboxes"), this );
    connect( reloadMboxList, SIGNAL( triggered() ), this, SLOT( slotReloadMboxList() ) );

    resyncMboxList = new QAction( tr("Re-synchronize Mailbox with Server"), this );
    connect( resyncMboxList, SIGNAL(triggered()), this, SLOT(slotResyncMboxList()) );

    reloadAllMailboxes = new QAction( tr("Reload All Mailboxes"), this );
    connect( reloadAllMailboxes, SIGNAL( triggered() ), model, SLOT( reloadMailboxList() ) );

    exitAction = new QAction( tr("E&xit"), this );
    exitAction->setShortcut( tr("Ctrl+Q") );
    exitAction->setStatusTip( tr("Exit the application") );
    connect( exitAction, SIGNAL( triggered() ), this, SLOT( close() ) );

    QActionGroup* netPolicyGroup = new QActionGroup( this );
    netPolicyGroup->setExclusive( true );
    netOffline = new QAction( tr("Offline"), netPolicyGroup );
    netOffline->setCheckable( true );
    connect( netOffline, SIGNAL( triggered() ), model, SLOT( setNetworkOffline() ) );
    netExpensive = new QAction( tr("Expensive Connection"), netPolicyGroup );
    netExpensive->setCheckable( true );
    connect( netExpensive, SIGNAL( triggered() ), model, SLOT( setNetworkExpensive() ) );
    netOnline = new QAction( tr("Free Access"), netPolicyGroup );
    netOnline->setCheckable( true );
    connect( netOnline, SIGNAL( triggered() ), model, SLOT( setNetworkOnline() ) );

    showFullView = new QAction( tr("Show Full Tree Window"), this );
    showFullView->setCheckable( true );
    connect( showFullView, SIGNAL( triggered(bool) ), this, SLOT( fullViewToggled(bool) ) );
    connect( allDock, SIGNAL( visibilityChanged(bool) ), showFullView, SLOT( setChecked(bool) ) );

    configSettings = new QAction( tr("Settings..."), this );
    connect( configSettings, SIGNAL( triggered() ), this, SLOT( slotShowSettings() ) );

    composeMail = new QAction( tr("Compose Mail..."), this );
    connect( composeMail, SIGNAL(triggered()), this, SLOT(slotComposeMail()) );

    expunge = new QAction( tr("Expunge Mailbox"), this );
    connect( expunge, SIGNAL(triggered()), this, SLOT(slotExpunge()) );

    markAsRead = new QAction( tr("Mark as Read"), this );
    markAsRead->setCheckable( true );
    connect( markAsRead, SIGNAL(triggered(bool)), this, SLOT(handleMarkAsRead(bool)) );

    markAsDeleted = new QAction( tr("Mark as Deleted"), this );
    markAsDeleted->setCheckable( true );
    connect( markAsDeleted, SIGNAL(triggered(bool)), this, SLOT(handleMarkAsDeleted(bool)) );
}

void MainWindow::createMenus()
{
    QMenu* imapMenu = menuBar()->addMenu( tr("IMAP") );
    imapMenu->addAction( composeMail );
    imapMenu->addAction( expunge );
    imapMenu->addSeparator()->setText( tr("Network Access") );
    imapMenu->addAction( netOffline );
    imapMenu->addAction( netExpensive );
    imapMenu->addAction( netOnline );
    imapMenu->addSeparator();
    imapMenu->addAction( showFullView );
    imapMenu->addSeparator();
    imapMenu->addAction( configSettings );
    imapMenu->addSeparator();
    imapMenu->addAction( exitAction );

    QMenu* mailboxMenu = menuBar()->addMenu( tr("Mailbox") );
    mailboxMenu->addAction( reloadAllMailboxes );
}

void MainWindow::createWidgets()
{
    mboxTree = new QTreeView();
    mboxTree->setUniformRowHeights( true );
    mboxTree->setContextMenuPolicy(Qt::CustomContextMenu);
    mboxTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
    mboxTree->setAllColumnsShowFocus( true );
    connect( mboxTree, SIGNAL( customContextMenuRequested( const QPoint & ) ),
            this, SLOT( showContextMenuMboxTree( const QPoint& ) ) );

    msgListTree = new QTreeView();
    msgListTree->setUniformRowHeights( true );
    msgListTree->setContextMenuPolicy(Qt::CustomContextMenu);
    msgListTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
    msgListTree->setAllColumnsShowFocus( true );
    msgListTree->setAlternatingRowColors( true );
    connect( msgListTree, SIGNAL( customContextMenuRequested( const QPoint & ) ),
            this, SLOT( showContextMenuMsgListTree( const QPoint& ) ) );

    msgView = new MessageView();

    QSplitter* hSplitter = new QSplitter();
    QSplitter* vSplitter = new QSplitter();
    vSplitter->setOrientation( Qt::Vertical );
    vSplitter->addWidget( msgListTree );
    vSplitter->addWidget( msgView );
    hSplitter->addWidget( mboxTree );
    hSplitter->addWidget( vSplitter );
    setCentralWidget( hSplitter );

    allDock = new QDockWidget( "Everything", this );
    allTree = new QTreeView( allDock );
    allDock->hide();
    allTree->setUniformRowHeights( true );
    allTree->setHeaderHidden( true );
    allDock->setWidget( allTree );
}

void MainWindow::setupModels()
{
    Imap::Mailbox::SocketFactoryPtr factory;
    QSettings s;
    if ( s.value( SettingsNames::imapMethodKey ).toString() == SettingsNames::methodTCP ) {
        factory.reset( new Imap::Mailbox::TlsAbleSocketFactory(
                s.value( SettingsNames::imapHostKey ).toString(),
                s.value( SettingsNames::imapPortKey ).toUInt() ) );
        factory->setStartTlsRequired( s.value( SettingsNames::imapStartTlsKey, true ).toBool() );
    } else if ( s.value( SettingsNames::imapMethodKey ).toString() == SettingsNames::methodSSL ) {
        factory.reset( new Imap::Mailbox::SslSocketFactory(
                s.value( SettingsNames::imapHostKey ).toString(),
                s.value( SettingsNames::imapPortKey ).toUInt() ) );
    } else {
        QStringList args = s.value( SettingsNames::imapProcessKey ).toString().split( QLatin1Char(' ') );
        Q_ASSERT( ! args.isEmpty() ); // FIXME
        QString appName = args.takeFirst();
#ifdef CUSTOM_UNIX_SOCKET
        factory.reset( new Imap::Mailbox::UnixProcessSocketFactory( appName, args ) );
#else
        factory.reset( new Imap::Mailbox::ProcessSocketFactory( appName, args ) );
#endif
    }

    cache.reset( new Imap::Mailbox::MemoryCache() );
    model = new Imap::Mailbox::Model( this, cache, factory );
    model->setObjectName( QLatin1String("model") );
    mboxModel = new Imap::Mailbox::MailboxModel( this, model );
    mboxModel->setObjectName( QLatin1String("mboxModel") );
    msgListModel = new Imap::Mailbox::MsgListModel( this, model );
    msgListModel->setObjectName( QLatin1String("msgListModel") );

    QObject::connect( mboxTree, SIGNAL( clicked(const QModelIndex&) ), msgListModel, SLOT( setMailbox(const QModelIndex&) ) );
    QObject::connect( mboxTree, SIGNAL( activated(const QModelIndex&) ), msgListModel, SLOT( setMailbox(const QModelIndex&) ) );
    QObject::connect( mboxTree, SIGNAL( clicked(const QModelIndex&) ), model, SLOT( switchToMailbox( const QModelIndex& ) ) );
    QObject::connect( mboxTree, SIGNAL( activated(const QModelIndex&) ), model, SLOT( switchToMailbox( const QModelIndex& ) ) );
    QObject::connect( msgListModel, SIGNAL( mailboxChanged() ), this, SLOT( slotResizeMsgListColumns() ) );

    QObject::connect( msgListTree, SIGNAL( clicked(const QModelIndex&) ), msgView, SLOT( setMessage(const QModelIndex&) ) );
    QObject::connect( msgListTree, SIGNAL( activated(const QModelIndex&) ), msgView, SLOT( setMessage(const QModelIndex&) ) );
    QObject::connect( msgListModel, SIGNAL( modelAboutToBeReset() ), msgView, SLOT( setEmpty() ) );
    QObject::connect( msgListModel, SIGNAL( messageRemoved( void* ) ), msgView, SLOT( handleMessageRemoved( void* ) ) );

    connect( model, SIGNAL( alertReceived( const QString& ) ), this, SLOT( alertReceived( const QString& ) ) );
    connect( model, SIGNAL( connectionError( const QString& ) ), this, SLOT( connectionError( const QString& ) ) );
    connect( model, SIGNAL( authRequested( QAuthenticator* ) ), this, SLOT( authenticationRequested( QAuthenticator* ) ) );

    connect( model, SIGNAL( networkPolicyOffline() ), this, SLOT( networkPolicyOffline() ) );
    connect( model, SIGNAL( networkPolicyExpensive() ), this, SLOT( networkPolicyExpensive() ) );
    connect( model, SIGNAL( networkPolicyOnline() ), this, SLOT( networkPolicyOnline() ) );

    //Imap::Mailbox::ModelWatcher* w = new Imap::Mailbox::ModelWatcher( this );
    //w->setModel( model );

    //ModelTest* tester = new ModelTest( mboxModel, this ); // when testing, test just one model at time

    mboxTree->setModel( mboxModel );
    msgListTree->setModel( msgListModel );
}

void MainWindow::slotResizeMsgListColumns()
{
    // it's broken and forces loading of all envelopes and budy structures
    /*for ( int i = 0; i < msgListTree->header()->count(); ++i )
        msgListTree->resizeColumnToContents( i );*/
}

void MainWindow::showContextMenuMboxTree( const QPoint& position )
{
    QList<QAction*> actionList;
    if ( mboxTree->indexAt( position ).isValid() ) {
        actionList.append( resyncMboxList );
        actionList.append( reloadMboxList );
    }
    actionList.append( reloadAllMailboxes );
    QMenu::exec( actionList, mboxTree->mapToGlobal( position ) );
}

void MainWindow::showContextMenuMsgListTree( const QPoint& position )
{
    QList<QAction*> actionList;
    QModelIndex index = msgListTree->indexAt( position );
    if ( index.isValid() ) {
        markAsRead->setChecked( msgListModel->data( index, Imap::Mailbox::MsgListModel::RoleIsMarkedAsRead ).toBool() );
        actionList.append( markAsRead );
        markAsDeleted->setChecked( msgListModel->data( index, Imap::Mailbox::MsgListModel::RoleIsMarkedAsDeleted ).toBool() );
        actionList.append( markAsDeleted );
    }
    if ( ! actionList.isEmpty() )
        QMenu::exec( actionList, msgListTree->mapToGlobal( position ) );
}

void MainWindow::slotReloadMboxList()
{
    QModelIndexList indices = mboxTree->selectionModel()->selectedIndexes();
    for ( QModelIndexList::const_iterator it = indices.begin(); it != indices.end(); ++it ) {
        Q_ASSERT( it->isValid() );
        Q_ASSERT( it->model() == mboxModel );
        if ( it->column() != 0 )
            continue;
        Imap::Mailbox::TreeItemMailbox* mbox = dynamic_cast<Imap::Mailbox::TreeItemMailbox*>(
                static_cast<Imap::Mailbox::TreeItem*>(
                    mboxModel->mapToSource( *it ).internalPointer()
                    )
                );
        Q_ASSERT( mbox );
        mbox->rescanForChildMailboxes( model );
    }
}

void MainWindow::slotResyncMboxList()
{
    QModelIndexList indices = mboxTree->selectionModel()->selectedIndexes();
    for ( QModelIndexList::const_iterator it = indices.begin(); it != indices.end(); ++it ) {
        Q_ASSERT( it->isValid() );
        Q_ASSERT( it->model() == mboxModel );
        if ( it->column() != 0 )
            continue;
        Imap::Mailbox::TreeItemMailbox* mbox = dynamic_cast<Imap::Mailbox::TreeItemMailbox*>(
                static_cast<Imap::Mailbox::TreeItem*>(
                    mboxModel->mapToSource( *it ).internalPointer()
                    )
                );
        Q_ASSERT( mbox );
        model->resyncMailbox( mbox );
    }
}

void MainWindow::alertReceived( const QString& message )
{
    QMessageBox::warning( this, tr("IMAP Alert"), message );
}

void MainWindow::connectionError( const QString& message )
{
    if ( QSettings().contains( SettingsNames::imapMethodKey ) ) {
        QMessageBox::critical( this, tr("Connection Error"), message );
    } else {
        slotShowSettings();
    }
}

void MainWindow::networkPolicyOffline()
{
    netOffline->setChecked( true );
    netExpensive->setChecked( false );
    netOnline->setChecked( false );
}

void MainWindow::networkPolicyExpensive()
{
    netOffline->setChecked( false );
    netExpensive->setChecked( true );
    netOnline->setChecked( false );
}

void MainWindow::networkPolicyOnline()
{
    netOffline->setChecked( false );
    netExpensive->setChecked( false );
    netOnline->setChecked( true );
}

void MainWindow::fullViewToggled( bool showIt )
{
    if ( showIt ) {
        allDock->show();
        allTree->setModel( model );
        addDockWidget(Qt::LeftDockWidgetArea, allDock);
    } else {
        removeDockWidget( allDock );
    }
}

void MainWindow::slotShowSettings()
{
    SettingsDialog* dialog = new SettingsDialog();
    if ( dialog->exec() == QDialog::Accepted ) {
        // FIXME: wipe cache in case we're moving between servers
        nukeModels();
        setupModels();
    }
}

void MainWindow::authenticationRequested( QAuthenticator* auth )
{
    QSettings s;
    QString user = s.value( SettingsNames::imapUserKey ).toString();
    QString pass = s.value( SettingsNames::imapPassKey ).toString();
    if ( pass.isEmpty() ) {
        bool ok;
        pass = QInputDialog::getText( this, tr("Password"),
                                      tr("Please provide password for %1").arg( user ),
                                      QLineEdit::Password, QString::null, &ok );
        if ( ok ) {
            auth->setUser( user );
            auth->setPassword( pass );
        }
    } else {
        auth->setUser( user );
        auth->setPassword( pass );
    }
}

void MainWindow::nukeModels()
{
    msgView->setEmpty();
    mboxTree->setModel( 0 );
    msgListTree->setModel( 0 );
    allTree->setModel( 0 );
    msgListModel->deleteLater();
    msgListModel = 0;
    mboxModel->deleteLater();
    mboxModel = 0;
    model->deleteLater();
    model = 0;
    cache.reset();
}

void MainWindow::slotComposeMail()
{
    QSettings s;
    ComposeWidget* w = new ComposeWidget( this,
        QString::fromAscii("%1 <%2>").arg(
                s.value( SettingsNames::realNameKey ).toString(),
                s.value( SettingsNames::addressKey ).toString() ),
        QList<QPair<QString,QString> >(),
        QString() );
    w->show();
}

void MainWindow::handleMarkAsRead( bool value )
{
    QModelIndexList indices = msgListTree->selectionModel()->selectedIndexes();
    for ( QModelIndexList::const_iterator it = indices.begin(); it != indices.end(); ++it ) {
        Q_ASSERT( it->isValid() );
        Q_ASSERT( it->model() == msgListModel );
        if ( it->column() != 0 )
            continue;
        Imap::Mailbox::TreeItemMessage* message = dynamic_cast<Imap::Mailbox::TreeItemMessage*>(
                static_cast<Imap::Mailbox::TreeItem*>(
                    msgListModel->mapToSource( *it ).internalPointer()
                    )
                );
        Q_ASSERT( message );
        model->markMessageRead( message, value );
    }
}

void MainWindow::handleMarkAsDeleted( bool value )
{
    QModelIndexList indices = msgListTree->selectionModel()->selectedIndexes();
    for ( QModelIndexList::const_iterator it = indices.begin(); it != indices.end(); ++it ) {
        Q_ASSERT( it->isValid() );
        Q_ASSERT( it->model() == msgListModel );
        if ( it->column() != 0 )
            continue;
        Imap::Mailbox::TreeItemMessage* message = dynamic_cast<Imap::Mailbox::TreeItemMessage*>(
                static_cast<Imap::Mailbox::TreeItem*>(
                    msgListModel->mapToSource( *it ).internalPointer()
                    )
                );
        Q_ASSERT( message );
        model->markMessageDeleted( message, value );
    }
}

void MainWindow::slotExpunge()
{
    model->expungeMailbox( msgListModel->currentMailbox() );
}

}

#include "Window.moc"
