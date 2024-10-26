#include "converter.h"

#include <algorithm>

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QDateTime>
#include <QtMath>


#include "cpl_string.h"
#include "vrtdataset.h"

GisConverter &GisConverter::getInstance()
{
    static GisConverter s;
    return s;
}

GisConverter::GisConverter()
{
}

GisConverter::~GisConverter()
{
}

QString GisConverter::getNameTile(const QString& pathToInputFile, const QString& pathToOutput, int i, int j) const
{
    QString suffixFolder = "";
    QString pathToDirTiles =
        QFileInfo(pathToOutput).absolutePath() + "/" + suffixFolder;
    if (!QDir(pathToDirTiles).exists()) {
        bool res = QDir().mkpath(
            pathToDirTiles
        );

        if(res) {
            qDebug() << "mkdir true ";
        } else {
            qDebug() << "mkdir false ";
        }
    }

    QString pathToTargetTile =
        pathToDirTiles + "/" +
        QFileInfo(pathToInputFile).completeBaseName() + "_" +
        QString::number(i) + "_" +
        QString::number(j) + "." +
        QFileInfo(pathToInputFile).suffix();

    qDebug() << "pathToTargetTile: " << pathToTargetTile;
    return pathToTargetTile;
}

QString GisConverter::getPathToVector(const QString &pathToTile) const
{
    QString pathToVector =
        QFileInfo(pathToTile).absolutePath() + "/" +
        QFileInfo(pathToTile).completeBaseName() + ".shp";
    return pathToVector;
}

void GisConverter::createVectorIntersect(
    const QString & pathToVector,
    const QVector<OGRGeometry *> & vecOGRGeometry
)
{
    if (vecOGRGeometry.isEmpty()) {
        return;
    }

    const char * pszDriverName = "ESRI Shapefile";
    GDALDriver * pDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
    if ( pDriver == NULL ) {
        qDebug() << QString("%0 driver not available.\n").arg(pszDriverName);
        return;
    }

    GDALDataset *pDatasetVector;
    qDebug() << "pathToVector: " << pathToVector;
    QByteArray pathToVectorTemp = pathToVector.toLocal8Bit();
    pDatasetVector = pDriver->Create(pathToVectorTemp.data(), 0, 0, 0, GDT_Unknown, NULL);
    if ( pDatasetVector == NULL ) {
        qDebug() << "Creation of output file failed.\n";
        return;
    }

    OGRLayer *pLayer = pDatasetVector->CreateLayer("point_out", NULL, wkbPolygon, NULL);
    if ( pLayer == NULL ) {
        qDebug() << "Layer creation failed.\n";
        return;
    }

    OGRFeature * pFeature = OGRFeature::CreateFeature(pLayer->GetLayerDefn());
    for(int numberField = 0; numberField < vecOGRGeometry.size(); ++numberField) {
        pFeature->SetGeomFieldDirectly(numberField, vecOGRGeometry[numberField]);
    }

    if(pLayer->CreateFeature(pFeature) != OGRERR_NONE) {
        qDebug() << "Failed to create feature.\n";
        return;
    }

    OGRFeature::DestroyFeature( pFeature );
    GDALClose( pDatasetVector );
}

void GisConverter::getTargetGeoTransform(
    double * targetGeoTransform,
    double * geoTransform,
    const QSize & tileSize,
    int i,
    int j
)
{
    targetGeoTransform[0] = geoTransform[0] + (j * tileSize.width()) * geoTransform[1];
    targetGeoTransform[1] = geoTransform[1];
    targetGeoTransform[2] = geoTransform[2];
    targetGeoTransform[3] = geoTransform[3] - (i * tileSize.height()) * geoTransform[1];
    targetGeoTransform[4] = geoTransform[4];
    targetGeoTransform[5] = geoTransform[5];
}

char * GisConverter::getStrWktTilePolygon(double *targetGeoTransform, const QSize& tileSize)
{
    char* pszWKT;

    QString strWKT;
    QByteArray strWKTTemp;
    char f = 'g';
    int prec = 10;

    strWKT =
            "Polygon ((" +

            QString::number(targetGeoTransform[0], f, prec) + " " +
            QString::number(targetGeoTransform[3], f, prec) + ", " +

            QString::number(targetGeoTransform[0] + tileSize.width() * targetGeoTransform[1], f, prec) + " " +
            QString::number(targetGeoTransform[3], f, prec) + ", " +

            QString::number(targetGeoTransform[0] + tileSize.width() * targetGeoTransform[1], f, prec) + " " +
            QString::number(targetGeoTransform[3] - tileSize.height() * targetGeoTransform[1], f, prec) + ", " +

            QString::number(targetGeoTransform[0] , f, prec) + " " +
            QString::number(targetGeoTransform[3] - tileSize.height() * targetGeoTransform[1], f, prec) + ", " +

            QString::number(targetGeoTransform[0], f, prec) + " " +
            QString::number(targetGeoTransform[3], f, prec) + "))";

    qDebug() << "strWKT: " << strWKT;
    strWKTTemp = strWKT.toLocal8Bit();
    pszWKT = strWKTTemp.data();
    return pszWKT;
}

void GisConverter::createCocoAnnotations(
    const QString & pathToAnnotations,
    const QVector<SImageInfo> & vecImageInfo,
    const QVector<SAnnotation> & vecAnnotation
)
{
    QString strAnnotations = "{\"images\": [";

    bool firstImg = true;
    for (const auto imageInfo : vecImageInfo) {
        QString strImageInfo = "";
        if (!firstImg) {
            strImageInfo += ", ";
        }

        strImageInfo += "{\"license\": 0, \"url\": null, ";
        strImageInfo += "\"file_name\": \"" + QFileInfo(imageInfo.pathToImage).fileName() + "\", ";
        strImageInfo += "\"height\": " + QString::number(imageInfo.size.height()) + ", ";
        strImageInfo += "\"width\": " + QString::number(imageInfo.size.width()) + ", ";
        strImageInfo += "\"date_captured\": null, ";
        strImageInfo += "\"id\": " + QString::number(imageInfo.id) + "}";

        strAnnotations += strImageInfo;
        firstImg = false;
    }

    strAnnotations += "]";

    if (!vecAnnotation.isEmpty()) {
        strAnnotations += ", \"type\": \"instances\", \"annotations\": [";

        int categoryId = 1;

        bool firstAnnotation = true;
        for (const auto annotation : vecAnnotation) {
            QString strAnnotation = "";
            if (!firstAnnotation) {
                strAnnotation += ", ";
            }

            strAnnotation += "{\"id\": " + QString::number(annotation.id) + ", ";
            strAnnotation += "\"image_id\": " + QString::number(annotation.imageId) + ", ";
            strAnnotation += "\"category_id\": " + QString::number(categoryId) + ", ";
            strAnnotation += "\"segmentation\": [[";

            bool firstPoint = true;
            for (int numberPoint = 0; numberPoint < annotation.vecPoint.size(); ++numberPoint) {
                QPointF point = annotation.vecPoint[numberPoint];
                if (!firstPoint) {
                    strAnnotation += ", ";
                }

                strAnnotation += QString::number(point.x());
                strAnnotation += ", ";
                strAnnotation += QString::number(point.y());
                firstPoint = false;
            }

            strAnnotation += "]], ";
            strAnnotation += "\"area\": " + QString::number(calculateArea(annotation.vecPoint)) + ", ";
            strAnnotation += "\"bbox\": [" +
                    QString::number(annotation.box.topLeft.x()) + ", " +
                    QString::number(annotation.box.topLeft.y()) + ", " +
                    QString::number(annotation.box.size.width()) + ", " +
                    QString::number(annotation.box.size.height()) + "], ";

            strAnnotation += "\"iscrowd\": 0}";
            strAnnotations += strAnnotation;

            firstAnnotation = false;
        }
        strAnnotations += "]";
    }

    strAnnotations += ", \"categories\": ["
            //                      "{\"supercategory\": null, \"id\": 0, \"name\": \"_background_\"}, "
            "{\"supercategory\": null, \"id\": 1, \"name\": \"water\"}]}";

    QFile file(pathToAnnotations);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << strAnnotations << endl;
    }
}

qreal GisConverter::calculateArea(const QVector<QPointF>& vecPoint)
{
    qreal area = 0;

    for (int numberPoint = 0; numberPoint < vecPoint.size(); ++numberPoint) {
        QPointF p1 = vecPoint[numberPoint];
        QPointF p2 = vecPoint[(numberPoint + 1) % vecPoint.size()];

        qreal d = p1.x() * p2.y() - p2.x() * p1.y();
        area += d;
    }

    return abs(area) / 2;
}

QVector<QVector<QPointF>> GisConverter::getVecPolygons(
    const QString & strWkt,
    double * targetGeoTransform,
    const QSize & tileSize,
    bool & isMultiPolygon
)
{
    isMultiPolygon = false;
    QString curStrWkt = strWkt;
    QVector<QVector<QPointF> > vecPolygons;

    QString titlePolygon = "POLYGON";
    QString titleMultiPolygon = "MULTIPOLYGON";

    if (curStrWkt.contains(titleMultiPolygon)) {
        isMultiPolygon = true;
        return vecPolygons;
        curStrWkt = curStrWkt.remove("MULTIPOLYGON (((");
        curStrWkt = curStrWkt.remove(")))");

        QStringList listPolygons = curStrWkt.split(")),((");

        for (auto strPolygon : listPolygons) {
            qDebug() << "strPolygon: " << strPolygon;
            QVector<QPointF> vecPoints = getVecPoints(curStrWkt, targetGeoTransform, tileSize);
            vecPolygons.push_back(vecPoints);
        }
    } else if(curStrWkt.contains(titlePolygon)) {
        curStrWkt = curStrWkt.remove("POLYGON ((");
        curStrWkt = curStrWkt.remove("))");

        QVector<QPointF> vecPoints = getVecPoints(curStrWkt, targetGeoTransform, tileSize);
        vecPolygons.push_back(vecPoints);
    }

    return vecPolygons;
}

QVector<QPointF> GisConverter::getVecPoints(const QString &strWkt, double* targetGeoTransform, const QSize& tileSize)
{
    QString curStrWkt = strWkt;
    QVector<QPointF> vecPoints;

    QStringList listCoord = curStrWkt.split(",");

    for (auto coord : listCoord) {
        QStringList listPoint = coord.split(" ");

        qreal x = listPoint[0].toDouble();
        x = (x - targetGeoTransform[0])/targetGeoTransform[1];

        qreal y = listPoint[1].toDouble();
        y = (targetGeoTransform[3] - y)/targetGeoTransform[1];

        x = qBound(0.0, x, (qreal)(tileSize.width() - 1));
        y = qBound(0.0, y, (qreal)(tileSize.height() - 1));

        QPointF point(x, y);

        vecPoints.push_back(point);
    }

    return vecPoints;
}

void GisConverter::drawPolygons(const QString &pathToImg, const QVector<QPolygonF> &vecPolygon)
{
    if (!vecPolygon.isEmpty()) {
        QPixmap pix(pathToImg);
        QPainter *painter = new QPainter(&pix);
        painter->setPen(Qt::red);
        painter->setBrush(Qt::green);

        for (const auto polygon : vecPolygon) {
            painter->drawPolygon(polygon);
        }

        painter->end();

        QString suffixFolder = "mark_img";
        QString pathToDirMarkImg = QFileInfo(pathToImg).absolutePath() + "/" + suffixFolder;
        if (!QDir(pathToDirMarkImg).exists()) {
            bool res = QDir(QFileInfo(pathToImg).absolutePath()).mkdir(suffixFolder);

            if (res) {
                qDebug() << "mkdir true ";
            } else {
                qDebug() << "mkdir false ";
            }
        }

        QString pathToMarkPng =
            QFileInfo(pathToImg).absolutePath() + "/" +
            suffixFolder + "/" +
            QFileInfo(pathToImg).completeBaseName() + "_mark.png";
        pix.save(pathToMarkPng);
    }
}

int GisConverter::randomBetween(int low, int high, int seed)
{
    qsrand(seed);
    return (qrand() % ((high + 1) - low) + low);
}

QVector<OGRGeometry *> GisConverter::getGeometryRegionInterest(
    GDALDataset * pDatasetVector,
    OGRCoordinateTransformation * pCTRasterToVector,
    double * geoTransform,
    const QSize& tileSize
)
{
    QVector<OGRGeometry *> vecOGRGeometry;

    OGRGeometry * pGeometry;
    const char * pszWKT = getStrWktTilePolygon(geoTransform, tileSize);
    OGRGeometryFactory::createFromWkt( &pszWKT, NULL, &pGeometry );

    pGeometry->transform(pCTRasterToVector);

    for (int numberLayer = 0; numberLayer < pDatasetVector->GetLayerCount(); ++numberLayer) {
        OGRLayer  *pLayer = pDatasetVector->GetLayer(numberLayer);

        for (GIntBig numberFeature = 0; numberFeature < pLayer->GetFeatureCount(); ++numberFeature) {
            OGRFeature * pFeature = pLayer->GetFeature(numberFeature);
            OGRGeometry *pGeom = pFeature->GetGeometryRef();
            OGRBoolean res = pGeometry->Intersects(pGeom);
            if (res == 1) {
                vecOGRGeometry.push_back(pGeom);
            }
        }
    }
    return vecOGRGeometry;
}

void GisConverter::cutIntoTiles(
    const char * pathToInputFile,
    const char * pathToVectorFile,
    const QSize & tileSize,
    const char * pathToOutput
)
{
    int nRows = 0;
    int nCols = 0;

    GDALAllRegister();

    GDALDataset * pDatasetRaster = (GDALDataset*)GDALOpen(pathToInputFile, GA_ReadOnly);

    GDALDriver * gTIF = GetGDALDriverManager()->GetDriverByName(pDatasetRaster->GetDriverName());
    GDALDriver * gPNG = GetGDALDriverManager()->GetDriverByName("PNG");

    // Открываем вектор.
    GDALDataset * pDatasetVector;
    pDatasetVector = (GDALDataset*) GDALOpenEx(pathToVectorFile, GDAL_OF_VECTOR, NULL, NULL, NULL );
    if ( pDatasetVector == NULL ) {
        qDebug() << "Open vector failed!";
        return;
    } else {
        qDebug() << "LayerCount: " << pDatasetVector->GetLayerCount();
        for (int numberLayer = 0; numberLayer < pDatasetVector->GetLayerCount(); ++numberLayer) {
            OGRLayer  *poLayer = pDatasetVector->GetLayer(numberLayer);
            qDebug() << "Name Layer: " << poLayer->GetName();
        }
    }

    double geoTransform[6];
    pDatasetRaster->GetGeoTransform(geoTransform);

    OGRCoordinateTransformation *pCTRasterToVector;
    OGRCoordinateTransformation *pCTVectorToRaster;
    {
        OGRSpatialReference inSpatialRef ( pDatasetRaster->GetProjectionRef () );

        qDebug() << "inSpatialRef :" << inSpatialRef.GetName();

        OGRLayer  *pLayerVector = pDatasetVector->GetLayer(0);
        OGRSpatialReference* outSpatialRef = pLayerVector->GetLayerDefn()->GetGeomFieldDefn(0)->GetSpatialRef();

        qDebug() << "outSpatialRef :" << outSpatialRef->GetName();

        pCTRasterToVector = OGRCreateCoordinateTransformation( &inSpatialRef, outSpatialRef );

        pCTVectorToRaster = OGRCreateCoordinateTransformation( outSpatialRef, &inSpatialRef );
    }

    QVector<GDALRasterBand*> rasterBands;
    for (int numberBand = 1; numberBand <= pDatasetRaster->GetRasterCount(); ++numberBand) {
        rasterBands.push_back(pDatasetRaster->GetRasterBand(numberBand));
    }

    nCols = rasterBands[0]->GetXSize();
    nRows = rasterBands[0]->GetYSize();
    qDebug() << "nCols = " << nCols << "___" << "nRows = " << nRows << "\n";

    //rasterBand->GetBlockSize(&nBlockXSize, &nBlockYSize);

    int nXBlocks = (nCols + tileSize.width() - 1) / tileSize.width() - 1;
    int nYBlocks = (nRows + tileSize.height() - 1) / tileSize.height() - 1;

    qDebug() << nYBlocks << "___" << nXBlocks << "\n";
    qDebug() << "tileSize.width() = " << tileSize.width() << "___" <<
        "tileSize.height() = " << tileSize.height() << "\n";

    int numberTile = 0;
    int maxCountTile = INT_MAX;

    GByte* rasterBlock = (GByte*)CPLMalloc(tileSize.width()*tileSize.height());

    QPolygon polygon;

    int categoryId = 1;
    int imageId = 0;
    int annotationId = 0;
    QVector<SImageInfo> vecImageInfoTrain;
    QVector<SAnnotation> vecAnnotationTrain;

    QVector<SImageInfo> vecImageInfoVal;
    QVector<SAnnotation> vecAnnotationVal;

    QSize sizeRegion(tileSize.width() * nXBlocks, tileSize.height() * nYBlocks);
    QVector<OGRGeometry *> vecTargetOGRGeometry = getGeometryRegionInterest(
        pDatasetVector,
        pCTRasterToVector,
        geoTransform,
        sizeRegion
    );

    for (int i = 0; i < nYBlocks && numberTile < maxCountTile; ++i) {
        for (int j = 0; j < nXBlocks && numberTile < maxCountTile; ++j) {
            int rVal = randomBetween(1, 100, QDateTime::currentMSecsSinceEpoch());
            bool writeToTrain = true;
            if (rVal >= 90) {
                writeToTrain = false;
            }

            QString pathToTile = getNameTile(QString(pathToInputFile), QString(pathToOutput), i, j);
            QByteArray pathToTileTemp = pathToTile.toLocal8Bit();
            qDebug() << "pathToTile: " << pathToTileTemp.data();
            GDALDataset *tile = gTIF->Create(
                pathToTileTemp.data(),
                tileSize.width(),
                tileSize.height(),
                pDatasetRaster->GetRasterCount(),
                GDT_Byte,
                NULL
            );

            double targetGeoTransform[6];
            getTargetGeoTransform(targetGeoTransform, geoTransform, tileSize, i, j);
            tile->SetGeoTransform(targetGeoTransform);

            // Создаем вектор boundingRect растра.
            QVector<QPolygonF> vecPolygon;
            bool isMultiPolygon = false;
            {
                OGRGeometry *pGeometry;
                const char* pszWKT = getStrWktTilePolygon(targetGeoTransform, tileSize);
                OGRGeometryFactory::createFromWkt( &pszWKT, NULL, &pGeometry );

                pGeometry->transform(pCTRasterToVector);

                // Ищем пересечение полигона границ тайла с целевым вектором.


                int countAnnotationTrain = 0;
                int countAnnotationVal = 0;
                {
                    for (auto pGeom : vecTargetOGRGeometry) {
                        OGRBoolean res = pGeometry->Intersects(pGeom);
                        if (res == 1) {
                            qDebug() << "Intersect!!!";
                            OGRGeometry *pGeomIntersection = pGeometry->Intersection(pGeom);

                            if (!pGeomIntersection->IsValid()) {
                                continue;
                            }
                            pGeomIntersection->transform(pCTVectorToRaster);

                            char* pWKTIntersection;
                            pGeomIntersection->exportToWkt(&pWKTIntersection);

                            QString strWKT(pWKTIntersection);
                            qDebug() << "strWKTIntersection: " << strWKT;
                            QVector<QVector<QPointF>> vecPolygons = getVecPolygons(strWKT, targetGeoTransform, tileSize, isMultiPolygon);

                            if (isMultiPolygon) {
                                break;
                            }
                            for (auto vecPoints : vecPolygons) {
                                QPolygonF polygon = QPolygonF(vecPoints);

                                qDebug() << "Qt polygon: " << polygon;
                                vecPolygon.push_back(polygon);

                                QRectF boundingRect = polygon.boundingRect();

                                if (writeToTrain) {
                                    countAnnotationTrain++;
                                    vecAnnotationTrain.push_back(
                                        {
                                            annotationId,
                                            imageId,
                                            categoryId,
                                            vecPoints,
                                            {
                                                 QPoint(qFloor(boundingRect.topLeft().x()), qFloor(boundingRect.topLeft().y())),
                                                 QSize(qFloor(boundingRect.size().width()), qFloor(boundingRect.size().width()))
                                            }
                                         }
                                    );
                                } else {
                                    countAnnotationVal++;
                                    vecAnnotationVal.push_back(
                                        {
                                            annotationId,
                                            imageId,
                                            categoryId,
                                            vecPoints,
                                            {
                                                QPoint(qFloor(boundingRect.topLeft().x()), qFloor(boundingRect.topLeft().y())),
                                                QSize(qFloor(boundingRect.size().width()), qFloor(boundingRect.size().width()))
                                            }
                                        }
                                    );
                                }

                                qDebug() << "annotationId: " << annotationId;
                                annotationId++;
                            }
                        }
                    }
                }

                if (isMultiPolygon) {
                    for (int i = 0; i < countAnnotationTrain; ++i) {
                        vecAnnotationTrain.pop_back();
                    }
                    for (int i = 0; i < countAnnotationVal; ++i) {
                        vecAnnotationVal.pop_back();
                    }
                    for (int i = 0; i < (countAnnotationTrain + countAnnotationVal); ++i) {
                        vecPolygon.pop_back();
                    }
                    annotationId = annotationId - countAnnotationTrain - countAnnotationVal;
                }

                QVector<OGRGeometry *> vecOGRGeometry;
                vecOGRGeometry.push_back(pGeometry);
                createVectorIntersect(getPathToVector(pathToTile), vecOGRGeometry);
            }

            for (int numberBand = 0; numberBand <  pDatasetRaster->GetRasterCount(); numberBand++) {
                int nXSize = std::min(
                    tileSize.width(),
                    rasterBands[numberBand]->GetXSize() - j * tileSize.width()
                );
                int nYSize = std::min(
                    tileSize.height(),
                    rasterBands[numberBand]->GetYSize() - i * tileSize.height()
                );

                if (rasterBands[numberBand] != nullptr) {
                    rasterBands[numberBand]->RasterIO(
                        GDALRWFlag::GF_Read,
                        j * tileSize.width(),
                        i * tileSize.height(),
                        nXSize,
                        nYSize,
                        rasterBlock,
                        nXSize,
                        nYSize,
                        GDALDataType::GDT_Byte,
                        0,
                        0
                    );
                }
                //Not sure if is working
                tile->GetRasterBand(numberBand + 1)->RasterIO(
                    GDALRWFlag::GF_Write,
                    0,
                    0,
                    tileSize.width(),
                    tileSize.height(),
                    rasterBlock,
                    tileSize.width(),
                    tileSize.height(),
                    GDALDataType::GDT_Byte,
                    0,
                    0
                );
            }
            GDALClose(tile);

            if (isMultiPolygon) {
                continue;
            }

            numberTile++;

            GDALDataset *pSourceDS = (GDALDataset*)GDALOpen(pathToTileTemp.data(), GA_ReadOnly);

            QString suffixFolder = "images";
            QString pathToDirMarkImg = QFileInfo(pathToTile).absolutePath() + "/" + suffixFolder;
            if (!QDir(pathToDirMarkImg).exists()) {
                bool res = QDir(QFileInfo(pathToTile).absolutePath()).mkdir(suffixFolder);

                if (res) {
                    qDebug() << "mkdir true ";
                } else {
                    qDebug() << "mkdir false ";
                }
            }

            QString pathToPng =
                QFileInfo(pathToTile).absolutePath() + "/" +
                suffixFolder + "/" +
                QFileInfo(pathToTile).completeBaseName() + ".png";
            qDebug() << "pathToPng: " << pathToPng;
            QByteArray pathToPngTemp = pathToPng.toLocal8Bit();
            GDALDataset* pPngDS = gPNG->CreateCopy(
                pathToPngTemp.data(),
                pSourceDS,
                FALSE,
                NULL,
                NULL,
                NULL
            );

            GDALClose(pSourceDS);
            GDALClose(pPngDS);

            //gTIF->Delete ( pathToTileTemp.data() );

            {
                QImage imgTile(pathToPng);

                if (writeToTrain) {
                    vecImageInfoTrain.push_back({imageId, pathToPng, imgTile.size()});
                } else {
                    vecImageInfoVal.push_back({imageId, pathToPng, imgTile.size()});
                }
            }

            qDebug() << "imageId: " << imageId;
            imageId++;

            drawPolygons(pathToPng, vecPolygon);
        }
    }

    QString pathToTrainAnnotations = QFileInfo(QString(pathToOutput)).absolutePath() + "/" + "gis_train.json";
    createCocoAnnotations(pathToTrainAnnotations, vecImageInfoTrain, vecAnnotationTrain);

    QString pathToValAnnotations = QFileInfo(QString(pathToOutput)).absolutePath() + "/" + "gis_val.json";
    createCocoAnnotations(pathToValAnnotations, vecImageInfoVal, vecAnnotationVal);

    CPLFree(rasterBlock);
    GDALClose(pDatasetVector);
    GDALClose(pDatasetRaster);
}
