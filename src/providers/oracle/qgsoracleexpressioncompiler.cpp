/***************************************************************************
    qgsoracleexpressioncompiler.cpp
    ----------------------------------------------------
    date                 : December 2015
    copyright            : (C) 2015 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracleexpressioncompiler.h"
#include "qgssqlexpressioncompiler.h"
#include "qgsexpressionnodeimpl.h"

QgsOracleExpressionCompiler::QgsOracleExpressionCompiler( QgsOracleFeatureSource *source )
  : QgsSqlExpressionCompiler( source->mFields )
{
}

QgsSqlExpressionCompiler::Result QgsOracleExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
  {
    const QgsExpressionNodeBinaryOperator *bin( static_cast<const QgsExpressionNodeBinaryOperator *>( node ) );

    switch ( bin->op() )
    {
      case QgsExpressionNodeBinaryOperator::boConcat:
        // oracle's handling of || WRT null is not standards compliant
        return Fail;

      case QgsExpressionNodeBinaryOperator::boPow:
      case QgsExpressionNodeBinaryOperator::boRegexp:
      case QgsExpressionNodeBinaryOperator::boILike:
      case QgsExpressionNodeBinaryOperator::boNotILike:
      case QgsExpressionNodeBinaryOperator::boMod:
      {
        QString op1, op2;

        if ( compileNode( bin->opLeft(), op1 ) != Complete ||
             compileNode( bin->opRight(), op2 ) != Complete )
          return Fail;

        switch ( bin->op() )
        {
          case QgsExpressionNodeBinaryOperator::boPow:
            result = QString( "power(%1,%2)" ).arg( op1, op2 );
            return Complete;

          case QgsExpressionNodeBinaryOperator::boRegexp:
            result = QString( "regexp_like(%1,%2)" ).arg( op1, op2 );
            return Complete;

          case QgsExpressionNodeBinaryOperator::boILike:
            result = QString( "lower(%1) LIKE lower(%2)" ).arg( op1, op2 );
            return Complete;

          case QgsExpressionNodeBinaryOperator::boNotILike:
            result = QString( "NOT lower(%1) LIKE lower(%2)" ).arg( op1, op2 );
            return Complete;

          case QgsExpressionNodeBinaryOperator::boMod  :
            result = QString( "MOD(%1,%2)" ).arg( op1, op2 );
            return Complete;

          default:
            break;
        }
      }

      default:
        break;
    }
  }

  //fallback to default handling
  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsOracleExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  return QgsOracleConn::quotedIdentifier( identifier );
}

QString QgsOracleExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;

  switch ( value.type() )
  {
    case QVariant::Bool:
      //no boolean literal support in Oracle, so fake it
      return value.toBool() ? "(1=1)" : "(1=0)";

    default:
      return QgsOracleConn::quotedValue( value );
  }
}
