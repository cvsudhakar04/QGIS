#include "qgstest.h"
#include <QObject>

#include <qgspostgresprovider.h>

class TestQgsPostgresProvider: public QObject
{
    Q_OBJECT
  private slots:
    void decodeHstore()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "\"1\"=>\"2\", \"a\"=>\"b, \\\"c'\", \"backslash\"=>\"\\\\\"" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "1" )] = "2";
      expected[QStringLiteral( "a" )] = "b, \"c'";
      expected[QStringLiteral( "backslash" )] = "\\";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }

    void decodeHstoreNoQuote()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "1=>2, a=>b c" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "1" )] = "2";
      expected[QStringLiteral( "a" )] = "b c";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }

    void decodeArray2StringList()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{\"1\",\"2\", \"a\\\\1\" , \"\\\\\",\"b, \\\"c'\"}" ) );
      QCOMPARE( decoded.type(), QVariant::StringList );

      QStringList expected;
      expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a\\1" ) << QStringLiteral( "\\" ) << QStringLiteral( "b, \"c'" );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toStringList(), expected );
    }

    void decodeArray2StringListNoQuote()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{1,2, a ,b, c}" ) );
      QCOMPARE( decoded.type(), QVariant::StringList );

      QStringList expected;
      expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toStringList(), expected );
    }

    void decodeArray2IntList()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{1, 2, 3,-5,10}" ) );
      QCOMPARE( decoded.type(), QVariant::StringList );

      QVariantList expected;
      expected << QVariant( 1 ) << QVariant( 2 ) << QVariant( 3 ) << QVariant( -5 ) << QVariant( 10 );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toList(), expected );
    }
};

QGSTEST_MAIN( TestQgsPostgresProvider )
#include "testqgspostgresprovider.moc"
