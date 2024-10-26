#ifndef GIS_CONVERTER_H
#define GIS_CONVERTER_H

#include <QString>
#include <QSize>
#include <QPoint>
#include <QPolygon>
#include <QVector>

#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <ogr_api.h>
#include <ogr_feature.h>

struct SImageInfo
{
    int id = 0;
    QString pathToImage = "";
    QSize size = QSize(256, 256);
};

struct SBox
{
    QPoint topLeft;
    QSize size;
};

struct SAnnotation
{
    int id = 0;
    int imageId = 0;
    int categoryId = 0;
    QVector<QPointF> vecPoint;
    SBox box;
};

class GisConverter
{
public:

    static GisConverter & getInstance();

    void cutIntoTiles(
        const char * pathToInputFile,
        const char * pathToVectorFile,
        const QSize & tileSize,
        const char * pathToOutput
    );

private:
    GisConverter();
    GisConverter(const GisConverter & root) = delete;
    GisConverter & operator=(const GisConverter &) = delete;
    ~GisConverter();

    QString getNameTile(const QString & pathToInputFile, const QString& pathToOutput, int i, int j) const;

    QString getPathToVector(const QString & pathToTile) const;

    void createVectorIntersect(
        const QString & pathToVector,
        const QVector<OGRGeometry*> & vecOGRGeometry
    );

    void getTargetGeoTransform(
        double * targetGeoTransform,
        double * geoTransform,
        const QSize& tileSize,
        int i,
        int j
    );

    char * getStrWktTilePolygon(double * targetGeoTransform, const QSize & tileSize);

    void createCocoAnnotations(
        const QString & pathToAnnotations,
        const QVector<SImageInfo> & vecImageInfo,
        const QVector<SAnnotation> & vecAnnotation
    );

    void drawPolygons(const QString & pathToImg, const QVector<QPolygonF> & vecPolygon);
    int randomBetween(int low, int high, int seed);

    QVector<OGRGeometry *> getGeometryRegionInterest(
        GDALDataset * pDatasetVector,
        OGRCoordinateTransformation * pCTRasterToVector,
        double * geoTransform,
        const QSize& tileSize
    );

    qreal calculateArea(const QVector<QPointF> &vecPoint);

    QVector<QVector<QPointF>> getVecPolygons(
        const QString & strWkt,
        double * targetGeoTransform,
        const QSize & tileSize,
        bool & isMultiPolygon
    );
    QVector<QPointF> getVecPoints(
        const QString & strWkt,
        double * targetGeoTransform,
        const QSize & tileSize
    );
};

#endif // GIS_CONVERTER_H
