#pragma once
#include "gdal_alg.h"
class GDALRasterPolygonEnumerator2

{
private:
    void     MergePolygon( int nSrcId, int nDstId );
    int      NewPolygon( GInt32 nValue );

public:  // these are intended to be readonly.

    GInt32   *panPolyIdMap;
    GInt32   *panPolyValue;

    int      nNextPolygonId;
    int      nPolyAlloc;

    int      nConnectedness;

public:
             GDALRasterPolygonEnumerator2( int nConnectedness=4 );
            ~GDALRasterPolygonEnumerator2();

    void     ProcessLine( GInt32 *panLastLineVal, GInt32 *panThisLineVal,
                          GInt32 *panLastLineId,  GInt32 *panThisLineId, 
                          int nXSize );

    void     CompleteMerges();

    void     Clear();
};