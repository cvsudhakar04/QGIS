/***************************************************************************

               ----------------------------------------------------
              date                 : 17.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswelcomepageitemsmodel.h"
#include "qgsmessagelog.h"

#include <QPixmap>
#include <QFile>
#include <QPainter>

QgsWelcomePageItemsModel::QgsWelcomePageItemsModel( QObject* parent )
    : QAbstractListModel( parent )
{

}

void QgsWelcomePageItemsModel::setRecentProjects( const QList<RecentProjectData>& recentProjects )
{
  beginResetModel();
  mRecentProjects = recentProjects;
  endResetModel();
}


int QgsWelcomePageItemsModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent )
  return mRecentProjects.size();
}

QVariant QgsWelcomePageItemsModel::data( const QModelIndex& index, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
      if ( mRecentProjects.at( index.row() ).previewImagePath != "" )
      {
      return QString( "<table style='border:0;' cellpadding='0'><tr>"
                      "<td style='padding:6px;background-color:#dddddd;'><img src='%1'></td>"
                      "<td style='padding:10px 10px 10px 10px;'><span style='font-size:18px;font-weight:bold;'>%2</span><br>%3</td"
                      "</tr></table>" ).arg( mRecentProjects.at( index.row() ).previewImagePath ).arg( mRecentProjects.at( index.row() ).title != mRecentProjects.at( index.row() ).path ? mRecentProjects.at( index.row() ).title : QString( "- untitled -" ) ).arg( mRecentProjects.at( index.row() ).path );
      }
      else
      {
      return QString( "<table style='border:0;' cellpadding='5'><tr>"
                      "<td width='262' height='170' style='padding:6px;background-color:#dddddd;'></td>"
                      "<td style='padding:10px 10px 10px 10px;'><span style='font-size:18px;font-weight:bold;'>%1</span><br>%2</td"
                      "</tr></table>" ).arg( mRecentProjects.at( index.row() ).title != mRecentProjects.at( index.row() ).path ? mRecentProjects.at( index.row() ).title : QString( "- untitled -" ) ).arg( mRecentProjects.at( index.row() ).path );
      }
      break;

    /*case Qt::DecorationRole:
    {
      QImage thumbnail( mRecentProjects.at( index.row() ).previewImagePath );
      if ( thumbnail.isNull() )
        return QVariant();

      //nicely round corners so users don't get paper cuts
      QImage previewImage( thumbnail.size(), QImage::Format_ARGB32 );
      previewImage.fill( Qt::transparent );
      QPainter previewPainter( &previewImage );
      previewPainter.setRenderHint( QPainter::Antialiasing, true );
      previewPainter.setPen( Qt::NoPen );
      previewPainter.setBrush( Qt::black );
      previewPainter.drawRoundedRect( 0, 0, previewImage.width(), previewImage.height(), 8, 8 );
      previewPainter.setCompositionMode( QPainter::CompositionMode_SourceIn );
      previewPainter.drawImage( 0, 0, thumbnail );
      previewPainter.end();

      return QPixmap::fromImage( previewImage );
      break;
    }*/

    case Qt::ToolTipRole:
      return mRecentProjects.at( index.row() ).path;
      break;

    default:
      return QVariant();
  }
}


Qt::ItemFlags QgsWelcomePageItemsModel::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  const RecentProjectData& projectData = mRecentProjects.at( index.row() );

  if ( !QFile::exists(( projectData.path ) ) )
    flags &= ~Qt::ItemIsEnabled;

  return flags;

}
