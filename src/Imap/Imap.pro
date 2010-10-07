QT += network \
    sql
CONFIG += staticlib
DEFINES -= QT3_SUPPORT
INCLUDEPATH += ../ \
    ../.. \
    Model \
    Tasks
DEPENDPATH += ../ \
    ../.. \
    Model \
    Tasks
TARGET = Imap
TEMPLATE = lib
SOURCES += Parser/Parser.cpp \
    Parser/Command.cpp \
    Parser/Response.cpp \
    Parser/LowLevelParser.cpp \
    Parser/Data.cpp \
    Parser/Message.cpp \
    Parser/3rdparty/rfccodecs.cpp \
    Parser/3rdparty/kcodecs.cpp \
    Parser/3rdparty/qmailcodec.cpp \
    Encoders.cpp \
    Exceptions.cpp \
    Model/Model.cpp \
    Model/MailboxMetadata.cpp \
    Model/ModelUpdaters.cpp \
    Model/MailboxModel.cpp \
    Model/PrettyMailboxModel.cpp \
    Model/MsgListModel.cpp \
    Model/MailboxTree.cpp \
    Model/MemoryCache.cpp \
    Model/SQLCache.cpp \
    Model/DiskPartCache.cpp \
    Model/CombinedCache.cpp \
    Model/Utils.cpp \
    Model/IdleLauncher.cpp \
    Model/TaskFactory.cpp \
    Tasks/ImapTask.cpp \
    Tasks/FetchMsgPartTask.cpp \
    Tasks/FetchMsgMetadataTask.cpp \
    Tasks/UpdateFlagsTask.cpp \
    Tasks/ListChildMailboxesTask.cpp \
    Tasks/NumberOfMessagesTask.cpp \
    Tasks/ExpungeMailboxTask.cpp \
    Tasks/CreateMailboxTask.cpp \
    Tasks/DeleteMailboxTask.cpp \
    Tasks/CopyMoveMessagesTask.cpp \
    Tasks/ObtainSynchronizedMailboxTask.cpp \
    Tasks/KeepMailboxOpenTask.cpp \
    Tasks/OpenConnectionTask.cpp \
    Tasks/GetAnyConnectionTask.cpp \
    Tasks/Fake_ListChildMailboxesTask.cpp \
    Tasks/Fake_OpenConnectionTask.cpp \
    Model/ModelWatcher.cpp \
    Network/MsgPartNetworkReply.cpp \
    Network/ForbiddenReply.cpp \
    Network/MsgPartNetAccessManager.cpp
HEADERS += Parser/Parser.h \
    Parser/Command.h \
    Parser/Response.h \
    Parser/LowLevelParser.h \
    Parser/Data.h \
    Parser/Message.h \
    Parser/3rdparty/rfccodecs.h \
    Parser/3rdparty/kcodecs.h \
    Parser/3rdparty/qmailcodec.h \
    Encoders.h \
    Exceptions.h \
    ConnectionState.h \
    Model/Model.h \
    Model/MailboxMetadata.h \
    Model/ModelUpdaters.h \
    Model/MailboxModel.h \
    Model/PrettyMailboxModel.h \
    Model/MsgListModel.h \
    Model/MailboxTree.h \
    Model/MemoryCache.h \
    Model/SQLCache.h \
    Model/DiskPartCache.h \
    Model/CombinedCache.h \
    Model/Cache.h \
    Model/Utils.h \
    Model/IdleLauncher.h \
    Model/TaskFactory.h \
    Model/CopyMoveOperation.h \
    Model/ItemRoles.h \
    Tasks/ImapTask.h \
    Tasks/FetchMsgPartTask.h \
    Tasks/FetchMsgMetadataTask.h \
    Tasks/UpdateFlagsTask.h \
    Tasks/ListChildMailboxesTask.h \
    Tasks/NumberOfMessagesTask.h \
    Tasks/ExpungeMailboxTask.h \
    Tasks/CreateMailboxTask.h \
    Tasks/DeleteMailboxTask.h \
    Tasks/CopyMoveMessagesTask.h \
    Tasks/ObtainSynchronizedMailboxTask.h \
    Tasks/KeepMailboxOpenTask.h \
    Tasks/OpenConnectionTask.h \
    Tasks/GetAnyConnectionTask.h \
    Tasks/Fake_ListChildMailboxesTask.h \
    Tasks/Fake_OpenConnectionTask.h \
    Model/ModelWatcher.h \
    Network/ForbiddenReply.h \
    Network/MsgPartNetAccessManager.h \
    Network/MsgPartNetworkReply.h \
