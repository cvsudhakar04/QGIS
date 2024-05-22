/***************************************************************************
    qgsvectorlayertoolscontext.h
    ------------------------
    begin                : May 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERTOOLSCONTEXT_H
#define QGSVECTORLAYERTOOLSCONTEXT_H

#include "qgsexpressioncontext.h"
#include "qgis_core.h"

#include <memory>

/**
 * \ingroup core
 * \class QgsVectorLayerToolsContext
 * \brief Contains settings which reflect the context in which vector layer tool operations should
 * consider.
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsVectorLayerToolsContext
{
  public:

    /**
     * Constructor for QgsVectorLayerToolsContext.
     */
    QgsVectorLayerToolsContext() = default;

    /**
     * Copy constructor.
     * \param other source QgsVectorLayerToolsContext
     */
    QgsVectorLayerToolsContext( const QgsVectorLayerToolsContext &other );

    QgsVectorLayerToolsContext &operator=( const QgsVectorLayerToolsContext &other );

    /**
     * Sets the optional expression context used by the vector layer tools.
     * \param context expression context pointer. Ownership is not transferred.
     * \see expressionContext()
     * \see setAdditionalExpressionContextScope()
     */
    void setExpressionContext( QgsExpressionContext *context );

    /**
     * Returns the optional expression context used by the vector layer tools.
     * \see setExpressionContext()
     * \see additionalExpressionContextScope()
     */
    QgsExpressionContext *expressionContext() const;

    /**
     * Sets an additional expression context scope to be made available when calculating expressions.
     * \param scope additional scope
     * \see additionalExpressionContextScope()
     */
    void setAdditionalExpressionContextScope( QgsExpressionContextScope *scope );

    /**
     * Returns an additional expression context scope to be made available when calculating expressions.
     * \see setAdditionalExpressionContextScope()
     */
    QgsExpressionContextScope *additionalExpressionContextScope();

  private:

    std::unique_ptr< QgsExpressionContext > mExpressionContext;
    std::unique_ptr< QgsExpressionContextScope > mAdditionalExpressionContextScope;

};

#endif // QGSVECTORLAYERTOOLSCONTEXT_H
