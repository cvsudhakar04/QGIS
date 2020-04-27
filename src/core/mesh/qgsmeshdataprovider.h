/***************************************************************************
                         qgsmeshdataprovider.h
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHDATAPROVIDER_H
#define QGSMESHDATAPROVIDER_H

#include <QVector>
#include <QString>
#include <QMap>
#include <limits>

#include "qgis_core.h"
#include "qgspoint.h"
#include "qgsdataprovider.h"

class QgsRectangle;

/**
 * \ingroup core
 *
 * QgsMeshDatasetIndex is index that identifies the dataset group (e.g. wind speed)
 * and a dataset in this group (e.g. magnitude of wind speed in particular time)
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshDatasetIndex
{
  public:
    //! Creates an index. -1 represents invalid group/dataset
    QgsMeshDatasetIndex( int group = -1, int dataset = -1 );
    //! Returns a group index
    int group() const;
    //! Returns a dataset index within group()
    int dataset() const;
    //! Returns whether index is valid, ie at least groups is set
    bool isValid() const;
    //! Equality operator
    bool operator == ( QgsMeshDatasetIndex other ) const;
    //! Inequality operator
    bool operator != ( QgsMeshDatasetIndex other ) const;
  private:
    int mGroupIndex = -1;
    int mDatasetIndex = -1;
};

//! xyz coords of vertex
typedef QgsPoint QgsMeshVertex;

//! List of vertex indexes
typedef QVector<int> QgsMeshFace;

/**
 * \ingroup core
 *
 *  Mesh - vertices and faces
 *
 * \since QGIS 3.6
 */
struct CORE_EXPORT QgsMesh
{
  //! Returns number of vertices
  int vertexCount() const;
  //! Returns number of faces
  int faceCount() const;

  //! Returns a vertex at the index
  QgsMeshVertex vertex( int index ) const;
  //! Returns a face at the index
  QgsMeshFace face( int index ) const;

  //! vertices
  QVector<QgsMeshVertex> vertices SIP_SKIP;
  //! faces
  QVector<QgsMeshFace> faces SIP_SKIP;
};

/**
 * \ingroup core
 *
 * QgsMeshDatasetValue represents single dataset value
 *
 * could be scalar or vector. Nodata values are represented by NaNs.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetValue
{
  public:
    //! Constructor for vector value
    QgsMeshDatasetValue( double x,
                         double y );

    //! Constructor for scalar value
    QgsMeshDatasetValue( double scalar );

    //! Default Ctor, initialize to NaN
    QgsMeshDatasetValue() = default;

    //! Dtor
    ~QgsMeshDatasetValue() = default;

    //! Sets scalar value
    void set( double scalar );

    //! Sets X value
    void setX( double x );

    //! Sets Y value
    void setY( double y ) ;

    //! Returns magnitude of vector for vector data or scalar value for scalar data
    double scalar() const;

    //! Returns x value
    double x() const;

    //! Returns y value
    double y() const;

    bool operator==( QgsMeshDatasetValue other ) const;

  private:
    double mX = std::numeric_limits<double>::quiet_NaN();
    double mY = std::numeric_limits<double>::quiet_NaN();
};

/**
 * \ingroup core
 *
 * QgsMeshDataBlock is a block of integers/doubles that can be used
 * to retrieve:
 * active flags (e.g. face's active integer flag)
 * scalars (e.g. scalar dataset double values)
 * vectors (e.g. vector dataset doubles x,y values)
 *
 * data are implicitly shared, so the class can be quickly copied
 * std::numeric_limits<double>::quiet_NaN() represents NODATA value
 *
 * Data can be accessed all at once with values() (faster) or
 * value by value (slower) with active() or value()
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsMeshDataBlock
{
  public:
    //! Type of data stored in the block
    enum DataType
    {
      ActiveFlagInteger, //!< Integer boolean flag whether face is active
      ScalarDouble, //!< Scalar double values
      Vector2DDouble, //!< Vector double pairs (x1, y1, x2, y2, ... )
    };

    //! Constructs an invalid block
    QgsMeshDataBlock();

    //! Constructs a new block
    QgsMeshDataBlock( DataType type, int count );

    //! Type of data stored in the block
    DataType type() const;

    //! Number of items stored in the block
    int count() const;

    //! Whether the block is valid
    bool isValid() const;

    /**
     * Returns a value represented by the index
     * For active flag the behavior is undefined
     */
    QgsMeshDatasetValue value( int index ) const;

    /**
     * Returns a value for active flag by the index
     * For scalar and vector 2d the behavior is undefined
     */
    bool active( int index ) const;

    /**
     * Sets active flag values.
     *
     * If the data provider/datasets does not have active
     * flag capability (== all values are valid), just
     * set block validity by setValid( true )
     *
     * \param vals value vector with size count()
     *
     * For scalar and vector 2d the behavior is undefined
     *
     * \since QGIS 3.12
     */
    void setActive( const QVector<int> &vals );

    /**
     * Returns active flag array
     *
     * Even for active flag valid dataset, the returned array could be empty.
     * This means that the data provider/dataset does not support active flag
     * capability, so all faces are active by default.
     *
     * For scalar and vector 2d the behavior is undefined
     *
     * \since QGIS 3.12
     */
    QVector<int> active() const;

    /**
     * Returns buffer to the array with values
     * For vector it is pairs (x1, y1, x2, y2, ... )
     *
     * \since QGIS 3.12
     */
    QVector<double> values() const;

    /**
     * Sets values
     *
     * For scalar datasets, it must have size count()
     * For vector datasets, it must have size 2 * count()
     * For active flag the behavior is undefined
     *
     * \since QGIS 3.12
     */
    void setValues( const QVector<double> &vals );

    //! Sets block validity
    void setValid( bool valid );

  private:
    QVector<double> mDoubleBuffer;
    QVector<int> mIntegerBuffer;
    DataType mType;
    int mSize = 0;
    bool mIsValid = false;
};

/**
 * \ingroup core
 *
 * QgsMesh3dDataBlock is a block of 3d stacked mesh data related N
 * faces defined on base mesh frame.
 *
 * data are implicitly shared, so the class can be quickly copied
 * std::numeric_limits<double>::quiet_NaN() represents NODATA value
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMesh3dDataBlock
{
  public:
    //! Constructs an invalid block
    QgsMesh3dDataBlock();

    //! Dtor
    ~QgsMesh3dDataBlock();

    //! Constructs a new block for count faces
    QgsMesh3dDataBlock( int count, bool isVector );

    //! Sets block validity
    void setValid( bool valid );

    //! Whether the block is valid
    bool isValid() const;

    //! Whether we store vector values
    bool isVector() const;

    //! Number of 2d faces for which the volume data is stored in the block
    int count() const;

    //! Index of the first volume stored in the buffer (absolute)
    int firstVolumeIndex() const;

    //! Index of the last volume stored in the buffer (absolute)
    int lastVolumeIndex() const;

    //! Returns number of volumes stored in the buffer
    int volumesCount() const;

    /**
     * Returns number of vertical level above 2d faces
     */
    QVector<int> verticalLevelsCount() const;

    /**
     * Sets the vertical level counts
     */
    void setVerticalLevelsCount( const QVector<int> &verticalLevelsCount );

    /**
     * Returns the vertical levels height
     */
    QVector<double> verticalLevels() const;

    /**
     * Sets the vertical levels height
     */
    void setVerticalLevels( const QVector<double> &verticalLevels );

    /**
     * Returns the indexing between faces and volumes
     */
    QVector<int> faceToVolumeIndex() const;

    /**
     * Sets the indexing between faces and volumes
     */
    void setFaceToVolumeIndex( const QVector<int> &faceToVolumeIndex );

    /**
     * Returns the values at volume centers
     *
     * For vector datasets the number of values is doubled (x1, y1, x2, y2, ... )
     */
    QVector<double> values() const;

    /**
     * Returns the value at volume centers
     *
     * \param volumeIndex volume index relative to firstVolumeIndex()
     * \returns value (scalar or vector)
     */
    QgsMeshDatasetValue value( int volumeIndex ) const;

    /**
     * Sets the values at volume centers
     *
     * For vector datasets the number of values is doubled (x1, y1, x2, y2, ... )
     */
    void setValues( const QVector<double> &doubleBuffer );

  private:
    int mSize = 0;
    bool mIsValid = false;
    bool mIsVector = false;
    QVector<int> mVerticalLevelsCount;
    QVector<double> mVerticalLevels;
    QVector<int> mFaceToVolumeIndex;
    QVector<double> mDoubleBuffer; // for scalar/vector values
};

/**
 * \ingroup core
 *
 * QgsMeshDatasetGroupMetadata is a collection of dataset group metadata
 * such as whether the data is vector or scalar, name
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshDatasetGroupMetadata
{
  public:

    //! Location of where data is specified for datasets in the dataset group
    enum DataType
    {
      DataOnFaces = 0, //!< Data is defined on faces
      DataOnVertices,  //!< Data is defined on vertices
      DataOnVolumes    //!< Data is defined on volumes \since QGIS 3.12
    };

    //! Constructs an empty metadata object
    QgsMeshDatasetGroupMetadata() = default;

    /**
     * Constructs a valid metadata object
     *
     * \param name name of the dataset group
     * \param isScalar dataset contains scalar data, specifically the y-value of QgsMeshDatasetValue is NaN
     * \param dataType where the data are defined on (vertices, faces or volumes)
     * \param minimum minimum value (magnitude for vectors) present among all group's dataset values
     * \param maximum maximum value (magnitude for vectors) present among all group's dataset values
     * \param maximumVerticalLevels maximum number of vertical levels for 3d stacked meshes, 0 for 2d meshes
     * \param referenceTime reference time of the dataset group
     * \param extraOptions dataset's extra options stored by the provider. Usually contains the name, time value, time units, data file vendor, ...
     */
    QgsMeshDatasetGroupMetadata( const QString &name,
                                 bool isScalar,
                                 DataType dataType,
                                 double minimum,
                                 double maximum,
                                 int maximumVerticalLevels,
                                 const QDateTime &referenceTime,
                                 const QMap<QString, QString> &extraOptions );

    /**
     * Returns name of the dataset group
     */
    QString name() const;

    /**
     * Returns extra metadata options, for example description
     */
    QMap<QString, QString> extraOptions() const;

    /**
     * \brief Returns whether dataset group has vector data
     */
    bool isVector() const;

    /**
     * \brief Returns whether dataset group has scalar data
     */
    bool isScalar() const;

    /**
     * Returns whether dataset group data is defined on vertices or faces or volumes
     *
     * \since QGIS 3.12
     */
    DataType dataType() const;

    /**
     * \brief Returns minimum scalar value/vector magnitude present for whole dataset group
     */
    double minimum() const;

    /**
     * \brief Returns maximum scalar value/vector magnitude present for whole dataset group
     */
    double maximum() const;

    /**
     * Returns maximum number of vertical levels for 3d stacked meshes
     *
     * \since QGIS 3.12
     */
    int maximumVerticalLevelsCount() const;

    /**
     * Returns the reference time
     *
     * \since QGIS 3.12
     */
    QDateTime referenceTime() const;

  private:
    QString mName;
    bool mIsScalar = false;
    DataType mDataType = DataType::DataOnFaces;
    double mMinimumValue = std::numeric_limits<double>::quiet_NaN();
    double mMaximumValue = std::numeric_limits<double>::quiet_NaN();
    QMap<QString, QString> mExtraOptions;
    int mMaximumVerticalLevelsCount = 0; // for 3d stacked meshes
    QDateTime mReferenceTime;
};

/**
 * \ingroup core
 *
 * QgsMeshDatasetMetadata is a collection of mesh dataset metadata such
 * as whether the data is valid or associated time for the dataset
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetMetadata
{
  public:
    //! Constructs an empty metadata object
    QgsMeshDatasetMetadata() = default;

    /**
     * Constructs a valid metadata object
     *
     * \param time a time which this dataset represents in the dataset group
     * \param isValid dataset is loadad and valid for fetching the data
     * \param minimum minimum value (magnitude for vectors) present among dataset values
     * \param maximum maximum value (magnitude for vectors) present among dataset values
     * \param maximumVerticalLevels maximum number of vertical levels for 3d stacked meshes, 0 for 2d meshes
     */
    QgsMeshDatasetMetadata( double time,
                            bool isValid,
                            double minimum,
                            double maximum,
                            int maximumVerticalLevels
                          );

    /**
     * Returns the time value for this dataset
     */
    double time() const;

    /**
     * Returns whether dataset is valid
     */
    bool isValid() const;

    /**
     * Returns minimum scalar value/vector magnitude present for the dataset
     */
    double minimum() const;

    /**
     * Returns maximum scalar value/vector magnitude present for the dataset
     */
    double maximum() const;

    /**
     * Returns maximum number of vertical levels for 3d stacked meshes
     *
     * \since QGIS 3.12
     */
    int maximumVerticalLevelsCount() const;

  private:
    double mTime = std::numeric_limits<double>::quiet_NaN();
    bool mIsValid = false;
    double mMinimumValue = std::numeric_limits<double>::quiet_NaN();
    double mMaximumValue = std::numeric_limits<double>::quiet_NaN();
    int mMaximumVerticalLevelsCount = 0; // for 3d stacked meshes
};

/**
 * \ingroup core
 *
 * Interface for mesh data sources
 *
 * Mesh is a collection of vertices and faces in 2D or 3D space
 *  - vertex - XY(Z) point (in the mesh's coordinate reference system)
 *  - faces - sets of vertices forming a closed shape - typically triangles or quadrilaterals
 *
 * Base on the underlying data provider/format, whole mesh is either stored in memory or
 * read on demand
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDataSourceInterface SIP_ABSTRACT
{
  public:
    //! Dtor
    virtual ~QgsMeshDataSourceInterface() = default;

    /**
     * \brief Returns number of vertices in the native mesh
     * \returns Number of vertices in the mesh
     */
    virtual int vertexCount() const = 0;

    /**
     * \brief Returns number of faces in the native mesh
     * \returns Number of faces in the mesh
     */
    virtual int faceCount() const = 0;

    /**
     * Populates the mesh vertices and faces
     * \since QGIS 3.6
     */
    virtual void populateMesh( QgsMesh *mesh ) const = 0;
};

/**
 * \ingroup core
 * Interface for mesh datasets and dataset groups
 *
 *  Dataset is a  collection of vector or scalar values on vertices or faces of the mesh.
 *  Based on the underlying data provider/format, whole dataset is either stored in memory
 *  or read on demand
 *
 *  Datasets are grouped in the dataset groups. A dataset group represents a measured quantity
 *  (e.g. depth or wind speed), dataset represents values of the quantity in a particular time.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetSourceInterface SIP_ABSTRACT
{
  public:
    //! Dtor
    virtual ~QgsMeshDatasetSourceInterface() = default;

    /**
     * \brief Associate dataset with the mesh
     *
     * emits dataChanged when successful
     */
    virtual bool addDataset( const QString &uri ) = 0;

    /**
     * Returns list of additional dataset file URIs added using addDataset() calls.
     */
    virtual QStringList extraDatasets() const = 0;

    /**
     * \brief Returns number of datasets groups loaded
     */
    virtual int datasetGroupCount( ) const = 0;

    /**
     * \brief Returns number of datasets loaded in the group
     */
    virtual int datasetCount( int groupIndex ) const = 0;

    /**
     * \brief Returns number of datasets loaded in the group
     */
    int datasetCount( QgsMeshDatasetIndex index ) const;

    /**
     * \brief Returns dataset group metadata
     */
    virtual QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const = 0;

    /**
     * \brief Returns dataset group metadata
     */
    QgsMeshDatasetGroupMetadata datasetGroupMetadata( QgsMeshDatasetIndex index ) const;

    /**
     * \brief Returns dataset metadata
     */
    virtual QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const = 0;

    /**
     * \brief Returns vector/scalar value associated with the index from the dataset
     * To read multiple continuous values, use datasetValues()
     *
     * See QgsMeshDatasetMetadata::isVector() or QgsMeshDataBlock::type()
     * to check if the returned value is vector or scalar
     *
     * Returns invalid value for DataOnVolumes
     *
     * \see datasetValues
     */
    virtual QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const = 0;

    /**
     * \brief Returns N vector/scalar values from the index from the dataset
     *
     * See QgsMeshDatasetMetadata::isVector() or QgsMeshDataBlock::type()
     * to check if the returned value is vector or scalar
     *
     * Returns invalid block for DataOnVolumes. Use QgsMeshLayerUtils::datasetValues() if you
     * need block for any type of data type
     *
     * \since QGIS 3.6
     */
    virtual QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const = 0;

    /**
     * \brief Returns N vector/scalar values from the face index from the dataset for 3d stacked meshes
     *
     * See QgsMeshDatasetMetadata::isVector() to check if the returned value is vector or scalar
     *
     * returns invalid block for DataOnFaces and DataOnVertices.
     *
     * \see datasetValues
     *
     * \since QGIS 3.12
     */
    virtual QgsMesh3dDataBlock dataset3dValues( QgsMeshDatasetIndex index, int faceIndex, int count ) const = 0;

    /**
     * \brief Returns whether the face is active for particular dataset
     *
     * For example to represent the situation when F1 and F3 are flooded, but F2 is dry,
     * some solvers store water depth on vertices V1-V8 (all non-zero values) and
     * set active flag for F2 to FALSE.
     *  V1 ---- V2 ---- V5-----V7
     *  |   F1  |   F2   | F3  |
     *  V3 ---- V4 ---- V6-----V8
     */
    virtual bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const = 0;

    /**
     * \brief Returns whether the faces are active for particular dataset
     *
     * \since QGIS 3.6
     */
    virtual QgsMeshDataBlock areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const = 0;

    /**
     * Creates a new dataset group from a data and
     * persists it into a destination path
     *
     * On success, the mesh's dataset group count is changed
     *
     * \param path destination path of the stored file in form DRIVER_NAME:path
     * \param meta new group's metadata
     * \param datasetValues scalar/vector values for all datasets and all faces/vertices in the group
     * \param datasetActive active flag values for all datasets in the group. Empty array represents can be used
     *                      when all faces are active
     * \param times times in hours for all datasets in the group
     * \returns TRUE on failure, FALSE on success
     *
    * \note Doesn't work if there is ":" in the path (e.g. Windows system)
     *
     * \since QGIS 3.6
     * \deprecated QGIS 3.12.3
     */
    Q_DECL_DEPRECATED virtual bool persistDatasetGroup( const QString &path,
        const QgsMeshDatasetGroupMetadata &meta,
        const QVector<QgsMeshDataBlock> &datasetValues,
        const QVector<QgsMeshDataBlock> &datasetActive,
        const QVector<double> &times
                                                      ) SIP_DEPRECATED;

    /**
     * Creates a new dataset group from a data and
     * persists it into a destination path
     *
     * On success, the mesh's dataset group count is changed
     *
     * \param outputFilePath destination path of the stored file
     * \param outputDriver output driver name
     * \param meta new group's metadata
     * \param datasetValues scalar/vector values for all datasets and all faces/vertices in the group
     * \param datasetActive active flag values for all datasets in the group. Empty array represents can be used
     *                      when all faces are active
     * \param times times in hours for all datasets in the group
     * \returns TRUE on failure, FALSE on success
     *
     * \since QGIS 3.12.3
     */
    virtual bool persistDatasetGroup( const QString &outputFilePath,
                                      const QString &outputDriver,
                                      const QgsMeshDatasetGroupMetadata &meta,
                                      const QVector<QgsMeshDataBlock> &datasetValues,
                                      const QVector<QgsMeshDataBlock> &datasetActive,
                                      const QVector<double> &times
                                    ) = 0;
};


/**
 * \ingroup core
 * Base class for providing data for QgsMeshLayer
 *
 * Responsible for reading native mesh data
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDataProvider: public QgsDataProvider, public QgsMeshDataSourceInterface, public QgsMeshDatasetSourceInterface
{
    Q_OBJECT

  public:
    //! Ctor
    QgsMeshDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );

  signals:
    //! Emitted when some new dataset groups have been added
    void datasetGroupsAdded( int count );
};

#endif // QGSMESHDATAPROVIDER_H
