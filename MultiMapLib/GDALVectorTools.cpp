#include "MultiMap.h"
#include "MFUtils.h"
#include "GDALVectorTools.h"
#include "GDALImageTools.h"
#include "GDALPolygonize.h"

DISABLE_WARNINGS
#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogr_api.h"
#include "ogr_core.h"
#include "ogrsf_frmts.h"
#include "cpl_conv.h"
#include "cpl_string.h"
ENABLE_WARNINGS

MULTIMAP_API GDALVectorTools::~GDALVectorTools() {
}
MULTIMAP_API GDALVectorTools::GDALVectorTools() {
}

MULTIMAP_API OGRLayer* GDALVectorTools::Eliminate(OGRDataSource* pDstDS, std::string layerName, OGRLayer* pSrcLayer, double lengthTol, double areaTol, double apRatio  ) {
	ErrorReset ();
	int errCode = CE_None;
	OGRLayer* pDstLayer = NULL;

	if ( pDstDS && pSrcLayer ) {
		pDstLayer = pDstDS->CreateLayer(layerName.c_str(),pSrcLayer->GetSpatialRef(),pSrcLayer->GetGeomType(),NULL);
		for ( int fid=0; fid<pSrcLayer->GetFeatureCount(); fid++ ){
			OGRFeature* feature = pSrcLayer->GetFeature(fid);
			if ( feature ) {
				OGRGeometry* geom = feature->GetGeometryRef();
				OGRwkbGeometryType wkbGeomType = wkbFlatten(geom->getGeometryType());
				switch (wkbGeomType) {
				case wkbPoint:
					break;
				case wkbPolygon:
					{
						OGRPolygon* poly = (OGRPolygon*)geom;
						if ( poly != NULL ) {
							if ( poly->getExteriorRing() ) {
								int nPoints = poly->getExteriorRing()->getNumPoints();
								if ( nPoints > 3 ) {
									double perimeter = poly->getExteriorRing()->get_Length();
									if ( perimeter > 0.0 ) {
										double area = DBL_MAX;
										double apRatio = DBL_MAX;
										if ( areaTol > 0.0 || apRatio > 0.0 ) {
											area = poly->get_Area();
											apRatio = area/perimeter;
										}
										if (area > areaTol && perimeter > lengthTol && apRatio > apRatio) {
											errCode = feature->SetGeometry(poly);
											if ( errCode == CE_None )
												errCode = pDstLayer->CreateFeature(feature);
										}
									}
								}
							}
						}
					}
					break;
				case wkbLineString:
					{
						OGRLineString* lstring = (OGRLineString*)geom;
						if ( lstring != NULL ) {
							if ( lstring->getNumPoints() > 1 ) {
								if ( lstring->get_Length() > lengthTol ) {
									errCode = feature->SetGeometry(lstring);
									if ( errCode == CE_None )
										errCode = pDstLayer->CreateFeature(feature);
								}
							}
						}
					}
					break;
				}
			}
			if ( errCode != CE_None ) {
				if ( GetLastErrorType() >= breakIfErrorType ) {
					break;
				}
			}
		}
	} else {
		SetError(99,"Eliminate called with missing source layer or destination dataset",false);
	}

	return pDstLayer;
}
MULTIMAP_API size_t GDALVectorTools::PointCount(OGRGeometry* pGeometry) {
	size_t pointCount = 0;
	OGRwkbGeometryType wkbGeomType = wkbFlatten(pGeometry->getGeometryType());
	switch (wkbGeomType) {
	case wkbPoint:
		pointCount = 1;
		break;
	case wkbPolygon:
		{
			OGRPolygon* pPo = (OGRPolygon*) pGeometry;
			OGRLinearRing* per = pPo->getExteriorRing();
			pointCount = per->getNumPoints();
			for ( int i=0; i<pPo->getNumInteriorRings(); i++ ) {
				OGRLinearRing* pir = pPo->getInteriorRing(i);
				pointCount += pir->getNumPoints();
			}
		}
		break;
	case wkbLineString:
		{
			OGRLineString* pLS = (OGRLineString*) pGeometry;
			pointCount = pLS->getNumPoints();
		}
		break;
	}
	return pointCount;
}
MULTIMAP_API OGRLayer* GDALVectorTools::Simplify(OGRDataSource* pDstDS, std::string layerName, OGRLayer* pSrcLayer, double dTol, bool keepTopo, double lengthTol, double areaTol, double apRatio  ) {
	ErrorReset ();
	int errCode = CE_None;
	inPointCount = 0;
	outPointCount = 0;
	OGRLayer* pDstLayer = NULL;
	if (lengthTol <= 0.0 ) {
		lengthTol = dTol;
	}
	if ( areaTol <= 0.0 ) {
		areaTol = dTol * dTol;
	}
	if ( pDstDS && pSrcLayer ) {
		pDstLayer = pDstDS->CreateLayer(layerName.c_str(),pSrcLayer->GetSpatialRef(),pSrcLayer->GetGeomType(),NULL);
		for ( int fid=0; fid<pSrcLayer->GetFeatureCount(); fid++ ){
			OGRFeature* feature = pSrcLayer->GetFeature(fid);
			if ( feature ) {
				OGRGeometry* geom = feature->GetGeometryRef();
				OGRwkbGeometryType wkbGeomType = wkbFlatten(geom->getGeometryType());
				inPointCount += PointCount(geom);
				switch (wkbGeomType) {
				case wkbPoint:
					break;
				case wkbPolygon:
					{
						OGRPolygon* poly = (OGRPolygon*)geom;

						OGRPolygon* newGeom = NULL; 
						if (keepTopo) {
							newGeom =(OGRPolygon*)poly->SimplifyPreserveTopology(dTol);
						} else {
							newGeom =(OGRPolygon*)poly->Simplify(dTol);
						}
						if ( newGeom != NULL ) {
							if ( newGeom->getExteriorRing() ) {
								int nPoints = newGeom->getExteriorRing()->getNumPoints();
								if ( nPoints > 3 ) {
									double perimeter = newGeom->getExteriorRing()->get_Length();
									if ( perimeter > 0.0 ) {
										double area = DBL_MAX;
										double apratio = DBL_MAX;
										if ( areaTol > 0.0 || apRatio > 0.0 ) {
											area = newGeom->get_Area();
											apRatio = area/perimeter;
										}
										if (area > areaTol && perimeter > lengthTol && apratio > apRatio) {
											outPointCount += PointCount(newGeom);
											OGRFeature* newFeature = feature->Clone();
											newFeature->SetGeometry(newGeom);
											if ( errCode == CE_None )
												errCode = pDstLayer->CreateFeature(newFeature);
										}
									}
								}
							}
						} else {
							const char* message;
							this->GetLastError(&message);
							printf("%s",message);
							outPointCount += PointCount(geom);
							OGRFeature* newFeature = feature->Clone();
							newFeature->SetGeometry(geom);
							if ( errCode == CE_None )
								errCode = pDstLayer->CreateFeature(newFeature);
						}
					}
					break;
				case wkbLineString:
					{
						OGRLineString* ls = (OGRLineString*)geom;
						OGRLineString* newGeom = NULL;
						newGeom =(OGRLineString*)ls->Simplify(dTol);
						if ( newGeom != NULL ) {
							if ( newGeom->getNumPoints() > 1 ) {
								if ( newGeom->get_Length() > lengthTol ) {
									OGRFeature* newFeature = feature->Clone();
									newFeature->SetGeometry(newGeom);
									if ( errCode == CE_None )
										outPointCount += PointCount(newGeom);
									errCode = pDstLayer->CreateFeature(newFeature);
								}
							}
						} else {
							outPointCount += PointCount(geom);
							OGRFeature* newFeature = feature->Clone();
							newFeature->SetGeometry(geom);
							if ( errCode == CE_None )
								errCode = pDstLayer->CreateFeature(newFeature);
						}
					}
					break;
				}
			}
			if ( errCode != CE_None ) {
				if ( GetLastErrorType() >= breakIfErrorType ) {
					break;
				}
			}
		}
	} else {
		SetError(99,"Simplify called with missing source layer or destination dataset",false);
	}

	return pDstLayer;
}


/**
* Polygonize creates vector polygons for all connected regions of pixels in the raster sharing a common pixel value. 
* Each polygon is created with an attribute indicating the pixel value of that polygon.
*
* From the GDAL Documentation : Create polygon coverage from raster data.
*
* This function creates vector polygons for all connected regions of pixels in the raster sharing a common pixel value. 
* Optionally each polygon may be labelled with the pixel value in an attribute. Optionally a mask band can be provided 
* to determine which pixels are eligible for processing.  Note that currently the source pixel band values are read into 
* a signed 32bit integer buffer (Int32), so floating point or complex bands will be implicitly truncated before processing.
* Polygon features will be created on the output layer, with polygon geometries representing the polygons. 
* The polygon geometries will be in the georeferenced coordinate system of the image (based on the geotransform of the source dataset).
* It is acceptable for the output layer to already have features. Note that GDALPolygonize() does not set the coordinate system on the output layer.
* Application code should do this when the layer is created, presumably matching the raster coordinate system. The algorithm used attempts to 
* minimize memory use so that very large rasters can be processed. However, if the raster has many polygons or very large/complex polygons, 
* the memory use for holding polygon enumerations and active polygon geometries may grow to be quite large.
* The algorithm will generally produce very dense polygon geometries, with edges that follow exactly on pixel boundaries for all non-interior pixels. 
* For non-thematic raster data (such as satellite images) the result will essentially be one small polygon per pixel, and memory and output layer sizes 
* will be substantial. The algorithm is primarily intended for relatively simple thematic imagery, masks, and classification results.
*
* @see SieveFilter which may be called prior to calling Polygonize for images that are not binary or that may have holes.
**/
MULTIMAP_API OGRLayer* GDALVectorTools::Polygonize(GDALRasterBand* pBand, OGRLayer* pLayer, bool simplify, int itype) {
	ErrorReset ();
	OGRwkbGeometryType gType = (OGRwkbGeometryType) itype;
	OGRLayerH hLayer = (OGRLayerH)pLayer;
	GDALRasterBandH hBand = (GDALRasterBandH)pBand;
	GDALRasterBand* pMaskBand = pBand->GetMaskBand();
	GDALImageTools imageTool;
	imageTool.Invert(pMaskBand,pBand);
	GDALRasterBandH hMaskBand = (GDALRasterBandH )pMaskBand;
	char **papszOptions = NULL;
	//CSLSetNameValue( papszOptions, "8CONNECTED", "8" );
	int errCode = 0;
	if (simplify) {
		GDALPolygonize2 polygonizer;
		errCode = polygonizer.Run(hBand,hMaskBand,hLayer,0,papszOptions, pfnProgress, pProgressArg,simplify);
	} else {
		errCode = GDALPolygonize(hBand,hMaskBand,hLayer,0,papszOptions, pfnProgress, pProgressArg);
	}
	//CSLDestroy(papszOptions);
	return pLayer;
}
/**
* Contour a DEM to a vector layer.  The NoData information will be extracted from the pSrcBand, if any exists.  
* If the source band does not contain a NoData value then the algorithm will assume that all source data is valid elevations.
*
* From the GDAL Documentation : Create vector contours from raster DEM.
* This algorithm will generate contours vectors for the input raster band on the requested set of contour levels. 
* The vector contours are written to the passed in OGR vector layer. Also, a NODATA value may be specified to identify pixels 
* that should not be considered in contour line generation. The gdal/apps/gdal_contour.cpp mainline can be used as an example of 
* how to use this function.
* 
* For contouring purposes raster pixel values are assumed to represent a point value at the center of the corresponding pixel region. 
* For the purpose of contour generation we virtually connect each pixel center to the values to the left, right, top and bottom. 
* We assume that the pixel value is linearly interpolated between the pixel centers along each line, and determine where (if any) 
* contour lines will appear onlong these line segements. Then the contour crossings are connected.
*
* In the "nodata" case we treat the whole nodata pixel as a no-mans land. We extend the corner pixels near the nodata out to half way 
* and then construct extra lines from those points to the center which is assigned an averaged value from the two nearby points (in this case (12+3+5)/3).
* This means that contour lines nodes won't actually be on pixel edges, but rather along vertical and horizontal lines connecting the pixel centers.

**/
MULTIMAP_API OGRLayer* GDALVectorTools::Contour(GDALRasterBand* pSrcBand, OGRLayer* pDstLayer, double interval, double base, int fixedLevelCount, int idField, int elevField) {
	ErrorReset ();
	double* padfFixedLevels = NULL;
	int	bUseNoData = FALSE;
	double dfNoDataValue =  pSrcBand->GetNoDataValue(&bUseNoData);

	GDALContourGenerate	((GDALRasterBandH) pSrcBand,interval,base,fixedLevelCount,padfFixedLevels,bUseNoData,dfNoDataValue,(void*)pDstLayer,idField,elevField,pfnProgress,pProgressArg);		

	return pDstLayer;
}
