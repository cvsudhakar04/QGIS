/***************************************************************************
                         qgsunittypes.h
                         --------------
    begin                : February 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSUNITTYPES_H
#define QGSUNITTYPES_H

#include "qgis.h"
#include "symbology-ng/qgssymbolv2.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * \class QgsUnitTypes
 * \brief Helper functions for various unit types.
 * \note Added in version 2.14
 */

class CORE_EXPORT QgsUnitTypes
{
  public:

    /** Types of distance units
     */
    enum DistanceUnitType
    {
      Standard = 0, /*!< unit is a standard measurement unit */
      Geographic, /*!< unit is a geographic (eg degree based) unit */
      UnknownType, /*!< unknown unit type */
    };

    /** Returns the type for a distance unit.
    */
    static DistanceUnitType unitType( QGis::UnitType unit );

    /** Encodes a distance unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeDistanceUnit()
    */
    static QString encodeUnit( QGis::UnitType unit );

    /** Decodes a distance unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
    */
    static QGis::UnitType decodeDistanceUnit( const QString& string, bool *ok = 0 );

    /** Returns a translated string representing a distance unit.
     * @param unit unit to convert to string
     * @see fromString()
     */
    static QString toString( QGis::UnitType unit );

    /** Converts a translated string to a distance unit.
     * @param string string representing a distance unit
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @see toString()
     */
    static QGis::UnitType stringToDistanceUnit( const QString& string, bool *ok = 0 );

    /** Returns the conversion factor between the specified distance units.
     * @param fromUnit distance unit to convert from
     * @param toUnit distance unit to convert to
     * @returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit );


    /** Encodes a symbol unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeSymbolUnit()
    */
    static QString encodeUnit( QgsSymbolV2::OutputUnit unit );

    /** Decodes a symbol unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
    */
    static QgsSymbolV2::OutputUnit decodeSymbolUnit( const QString& string, bool *ok = 0 );

};

#endif // QGSUNITTYPES_H
