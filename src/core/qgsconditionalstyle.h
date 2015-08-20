#ifndef QGSCONDITIONALSTYLE_H
#define QGSCONDITIONALSTYLE_H

#include <QFont>
#include <QColor>
#include <QPixmap>
#include <QDomNode>
#include <QDomDocument>

#include "qgsfeature.h"
#include "qgssymbolv2.h"

/** \class QgsFieldFormat
 * Conditional styling for a rule.
 */
class CORE_EXPORT QgsConditionalStyle
{
  public:
    QgsConditionalStyle();
    QgsConditionalStyle( const QgsConditionalStyle& other );
    QgsConditionalStyle( QString rule );
    ~QgsConditionalStyle();

    QgsConditionalStyle& operator=( const QgsConditionalStyle& other );

    /**
     * @brief Check if the rule matches using the given value and feature
     * @param value The current value being checked. \@value is replaced in the rule with this value.
     * @param feature The feature to match the values from.
     * @return True of the rule matches against the given feature
     */
    bool matches( QVariant value, QgsFeature *feature = 0 );

    /**
     * @brief Render a preview icon of the rule.
     * @return QPixmap preview of the style
     */
    QPixmap renderPreview();

    /**
     * @brief Set the rule for the style.  Rules should be of QgsExpression syntax.
     * Special value of \@value is replaced at run time with the check value
     * @param value The QgsExpression style rule to use for this style
     */
    void setRule( QString value ) { mRule = value; mValid = true; }

    /**
     * @brief Set the background color for the style
     * @param value QColor for background color
     */
    void setBackgroundColor( QColor value ) { mBackColor = value; mValid = true; }

    /**
     * @brief Set the text color for the style
     * @param value QColor for text color
     */
    void setTextColor( QColor value ) { mTextColor = value; mValid = true; }

    /**
     * @brief Set the font for the the style
     * @param value QFont to be used for text
     */
    void setFont( QFont value ) { mFont = value; mValid = true; }

    /**
     * @brief Set the icon for the style. Icons are generated from symbols
     * @param value QgsSymbolV2 to be used when generating the icon
     */
    void setSymbol( QgsSymbolV2* value );

    /**
     * @brief The icon set for style generated from the set symbol
     * @return A QPixmap that was set for the icon using the symbol
     */
    QPixmap icon() const { return mIcon; }

    /**
     * @brief The symbol used to generate the icon for the style
     * @return The QgsSymbolV2 used for the icon
     */
    QgsSymbolV2* symbol() const { return mSymbol.data(); }

    /**
     * @brief The text color set for style
     * @return QColor for text color
     */
    QColor textColor() const { return mTextColor; }

    /**
     * @brief The background color for style
     * @return QColor for background color
     */
    QColor backgroundColor() const { return mBackColor; }
    /**
     * @brief The font for the style
     * @return QFont for the style
     */
    QFont font() const { return mFont; }

    /**
     * @brief The condtion rule set for the style. Rule may contain variable \@value
     * to represent the current value
     * @return QString of the current set rule
     */
    QString rule() const { return mRule; }

    /**
     * @brief isValid Check if this rule is valid.  A valid rule has one or more properties
     * set.
     * @return True if the rule is valid.
     */
    bool isValid() const { return mValid; }

    /** Reads vector conditional style specific state from layer Dom node.
     */
    virtual bool readXml( const QDomNode& node );

    /** Write vector conditional style specific state from layer Dom node.
     */
    virtual bool writeXml( QDomNode & node, QDomDocument & doc );

    bool mValid;
    QString mRule;
    QScopedPointer<QgsSymbolV2> mSymbol;
    QFont mFont;
    QColor mBackColor;
    QColor mTextColor;
    QPixmap mIcon;
};

#endif // QGSCONDITIONALSTYLE_H
