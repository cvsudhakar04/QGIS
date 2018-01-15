#include "qgstest.h"
#include <QObject>

#include <qgspostgresconn.h>

class TestQgsPostgresConn: public QObject
{
    Q_OBJECT
  private slots:
    void quotedValueHstore()
    {
      QVariantMap map;
      map[QStringLiteral( "1" )] = "2";
      map[QStringLiteral( "a" )] = "b \"c' \\x";

      const QString actual = QgsPostgresConn::quotedValue( map );
      QCOMPARE( actual, QString( "E'\"1\"=>\"2\",\"a\"=>\"b \\\\\"c\\' \\\\\\\\x\"'::hstore" ) );
    }

    void quotedValueString()
    {
      QCOMPARE( QgsPostgresConn::quotedValue( "b" ), QString( "'b'" ) );
      QCOMPARE( QgsPostgresConn::quotedValue( "b's" ), QString( "'b''s'" ) );
      QCOMPARE( QgsPostgresConn::quotedValue( "b \"c' \\x" ), QString( "E'b \"c'' \\\\x'" ) );
    }

    void quotedValueStringArray()
    {
      QStringList list;
      list << QStringLiteral( "a" ) << QStringLiteral( "b \"c' \\x" );
      const QString actual = QgsPostgresConn::quotedValue( list );
      QCOMPARE( actual, QString( "E'{\"a\",\"b \\\\\"c\\' \\\\\\\\x\"}'" ) );
    }

    void quotedValueIntArray()
    {
      QVariantList list;
      list << 1 << -5;
      const QString actual = QgsPostgresConn::quotedValue( list );
      QCOMPARE( actual, QString( "E'{\"1\",\"-5\"}'" ) );
    }

};

QGSTEST_MAIN( TestQgsPostgresConn )
#include "testqgspostgresconn.moc"
