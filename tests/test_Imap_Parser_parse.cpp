/* Copyright (C) 2007 Jan Kundr�t <jkt@gentoo.org>

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

#include <qtest_kde.h>
#include <QFile>

#include "test_Imap_Parser_parse.h"
#include "test_Imap_Parser_parse.moc"

Q_DECLARE_METATYPE(Imap::Responses::Response)

QTEST_KDEMAIN_CORE( ImapParserParseTest )

void ImapParserParseTest::initTestCase()
{
    array = new QByteArray();
    buf = new QBuffer( array );
    parser = new Imap::Parser( 0, std::auto_ptr<QIODevice>( buf ) );
}

void ImapParserParseTest::cleanupTestCase()
{
    delete parser;
    // buf is deleted by Imap::Parser's destructor
    delete array;
}

void ImapParserParseTest::testParseTagged()
{
    QFETCH( QByteArray, line );
    QFETCH( Imap::Responses::Response, response );

    QCOMPARE( parser->parseTagged( line ), response );
}

void ImapParserParseTest::testParseTagged_data()
{
    using namespace Imap::Responses;

    QTest::addColumn<QByteArray>("line");
    QTest::addColumn<Response>("response");

    QTest::newRow("tagged-ok-simple")
        << QByteArray("y01 OK Everything works, man!\r\n")
        << Response("y01", OK, NONE, QList<QByteArray>(), "Everything works, man!");
    QTest::newRow("tagged-no-simple")
        << QByteArray("12345 NO Nope, something is broken\r\n")
        << Response("12345", NO, NONE, QList<QByteArray>(), "Nope, something is broken");
    QTest::newRow("tagged-bad-simple") 
        << QByteArray("ahoj BaD WTF?\r\n") 
        << Response("ahoj", BAD, NONE, QList<QByteArray>(), "WTF?");

    QTest::newRow("tagged-ok-alert") 
        << QByteArray("y01 oK [ALERT] Server on fire\r\n") 
        << Response("y01", OK, ALERT, QList<QByteArray>(), "Server on fire");
    QTest::newRow("tagged-no-alert") 
        << QByteArray("1337 no [ALeRT] Server on fire\r\n")
        << Response("1337", NO, ALERT, QList<QByteArray>(), "Server on fire");
    QTest::newRow("tagged-ok-capability") 
        << QByteArray("y01 OK [CAPaBILITY blurdybloop IMAP4rev1 WTF] Capabilities updated\r\n") 
        << Response("y01", OK, CAPABILITY, QList<QByteArray>() << "blurdybloop" << "IMAP4rev1" << "WTF", "Capabilities updated");
    QTest::newRow("tagged-ok-parse") 
        << QByteArray("y01 OK [PArSE] Parse error. What do you feed me with?\r\n") 
        << Response("y01", OK, PARSE, QList<QByteArray>(), "Parse error. What do you feed me with?");
    QTest::newRow("tagged-ok-permanentflags-empty") 
        << QByteArray("y01 OK [PERMANENTfLAGS] Behold, the flags!\r\n") 
        << Response("y01", OK, PERMANENTFLAGS, QList<QByteArray>(), "Behold, the flags!");
    QTest::newRow("tagged-ok-permanentflags-flags") 
        << QByteArray("y01 OK [PErMANENTFLAGS \\Foo \\Bar SmrT] Behold, the flags!\r\n") 
        << Response("y01", OK, PERMANENTFLAGS, QList<QByteArray>() << "\\Foo" << "\\Bar" << "SmrT", "Behold, the flags!");
    QTest::newRow("tagged-ok-readonly") 
        << QByteArray("y01 OK [ReAD-ONLY] No writing for you\r\n") 
        << Response("y01", OK, READ_ONLY, QList<QByteArray>(), "No writing for you");
    QTest::newRow("tagged-ok-readwrite") 
        << QByteArray("y01 OK [REaD-WRITE] Write!!!\r\n") 
        << Response("y01", OK, READ_WRITE, QList<QByteArray>(), "Write!!!");
    QTest::newRow("tagged-ok-trycreate") 
        << QByteArray("y01 OK [TryCreate] ...would be better :)\r\n") 
        << Response("y01", OK, TRYCREATE, QList<QByteArray>(), "...would be better :)");
    QTest::newRow("tagged-ok-uidnext") 
        << QByteArray("y01 OK [uidNext 5] Next UID\r\n") 
        << Response("y01", OK, UIDNEXT, QList<QByteArray>() << "5", "Next UID");
    QTest::newRow("tagged-ok-uidvalidity") 
        << QByteArray("y01 OK [UIDVALIDITY 17] UIDs valid\r\n") 
        << Response("y01", OK, UIDVALIDITY, QList<QByteArray>() << "17", "UIDs valid");
    QTest::newRow("tagged-ok-unseen") 
        << QByteArray("y01 OK [unSeen 666] I need my glasses\r\n") 
        << Response("y01", OK, UNSEEN, QList<QByteArray>() << "666", "I need my glasses");

}

#include "Imap/Parser.h"


