/***************************************************************************
                         qgscesiumutils.cpp
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscesiumutils.h"
#include "nlohmann/json.hpp"
#include "qgsjsonutils.h"
#include "qgssphere.h"
#include "qgsorientedbox3d.h"

QgsBox3D QgsCesiumUtils::parseRegion( const json &region )
{
  try
  {
    // The latitude and longitude values are given in radians!
    // TODO -- is this ALWAYS the case? What if there's a region root bounding volume, but a transform object present? What if there's crs metadata specifying a different crs?

    const double west = region[0].get<double>() * 180 / M_PI;
    const double south = region[1].get<double>() * 180 / M_PI;
    const double east = region[2].get<double>() * 180 / M_PI;
    const double north = region[3].get<double>() * 180 / M_PI;
    double minHeight = region[4].get<double>();
    double maxHeight = region[5].get<double>();

    return QgsBox3D( west, south, minHeight, east, north, maxHeight );
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsBox3D();
  }
}

QgsBox3D QgsCesiumUtils::parseRegion( const QVariantList &region )
{
  if ( region.size() != 6 )
    return QgsBox3D();

  return parseRegion( QgsJsonUtils::jsonFromVariant( region ) );
}

QgsOrientedBox3D QgsCesiumUtils::parseBox( const json &box )
{
  if ( box.size() != 12 )
    return QgsOrientedBox3D();

  try
  {
    QgsOrientedBox3D res;
    for ( int i = 0; i < 3; ++i )
    {
      res.mCenter[i] = box[i].get<double>();
    }
    for ( int i = 0; i < 9; ++i )
    {
      res.mHalfAxes[i] = box[i + 3].get<double>();
    }
    return res;
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsOrientedBox3D();
  }
}

QgsOrientedBox3D QgsCesiumUtils::parseBox( const QVariantList &box )
{
  if ( box.size() != 12 )
    return QgsOrientedBox3D();

  return parseBox( QgsJsonUtils::jsonFromVariant( box ) );
}

QgsSphere QgsCesiumUtils::parseSphere( const json &sphere )
{
  if ( sphere.size() != 4 )
    return QgsSphere();

  try
  {
    const double centerX = sphere[0].get<double>();
    const double centerY = sphere[1].get<double>();
    const double centerZ = sphere[2].get<double>();
    const double radius = sphere[3].get<double>();
    return QgsSphere( centerX, centerY, centerZ, radius );
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsSphere();
  }
}

QgsSphere QgsCesiumUtils::parseSphere( const QVariantList &sphere )
{
  if ( sphere.size() != 4 )
    return QgsSphere();

  return parseSphere( QgsJsonUtils::jsonFromVariant( sphere ) );
}
