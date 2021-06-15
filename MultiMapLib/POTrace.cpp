/* Copyright (C) 2001-2014 Icosasoft Software Inc. All rights reserved.
This file is part of Potrace Professional, and may only be used under
license. For information, contact licensing@icosasoft.ca. */

/* A simple and self-contained demo of the potracelib API */
#include "MultiMap.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "potracelib.h"
#include "POTrace.h"

#include "ogr_api.h"
#include "ogr_core.h"
#include "ogr_srs_api.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"

#define WIDTH 250
#define HEIGHT 250

/**
							OGRDataSource* pDataset3 = NULL;
								if ( false ) {
									POTrace poTrace;
									ByteBuffer rasterTools;
									rasterTools.ReadBuffer(rastLayer);
									poTrace.SetBitmap(rasterTools.getBuffer(),rasterTools.getRasterX(),rasterTools.getRasterY());
									poTrace.Trace();
									OGRLayer* pnewLayer;
									poTrace.SaveToOGRLayer(wayLayerShp.c_str(),pDataset,&pnewLayer,geotransform,true);
									GDALClose(pDataset);
								} else {
**/

/* ---------------------------------------------------------------------- */
/* auxiliary bitmap functions */

/* macros for writing individual bitmap pixels */
#define BM_WORDSIZE ((int)sizeof(potrace_word))
#define BM_WORDBITS (8*BM_WORDSIZE)
#define BM_HIBIT (((potrace_word)1)<<(BM_WORDBITS-1))
#define bm_scanline(bm, y) ((bm)->map + (y)*(bm)->dy)
#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x)/BM_WORDBITS])
#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS-1)))
#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)

POTrace::~POTrace(void) {
	FreeBitMap();
	FreeVectors();
	FreeParameters();
}

POTrace::POTrace(void) {
	bm = NULL;
	st = NULL;
	param = potrace_param_default();
}

/* return new un-initialized bitmap. NULL with errno on error */
potrace_bitmap_t* POTrace::AllocateBitmap(int width, int height) {
	potrace_bitmap_t *bm;
	int dy = (width + BM_WORDBITS - 1) / BM_WORDBITS;

	bm = (potrace_bitmap_t *) malloc(sizeof(potrace_bitmap_t));
	if (bm) {
		bm->w = width;
		bm->h = height;
		bm->dy = dy;
		size_t bmSize = dy * height * BM_WORDSIZE;
		bm->map = (potrace_word *) malloc(bmSize);
		if (!bm->map) {
			free(bm);
			bm = NULL;
		}
	}
	return bm;
}

PRIVATE int POTrace::createTestBitmap () {
	/* create a bitmap */
	bm = AllocateBitmap(WIDTH, HEIGHT);
	if (!bm) {
		fprintf(stderr, "Error allocating bitmap: %s\n", strerror(errno)); 
		return 1;
	}

	/* fill the bitmap with some pattern */
	for (int y=0; y<HEIGHT; y++) {
		for (int x=0; x<WIDTH; x++) {
			BM_PUT(bm, x, y, ((x*x + y*y*y) % 10000 < 5000) ? 1 : 0);
		}
	}
	return 0;
}
MULTIMAP_API int POTrace::SetBitmap(unsigned char *bitMap, int width, int height) {
	int errCode = 0;
	if ( bm ) {
		FreeBitMap(&bm);
	}
	bm = AllocateBitmap(width, height);
	if ( bm ) {
		for (int y=0; y<width; y++) {
			for (int x=0; x<height; x++) {
				size_t offset = y*width + x;
				BM_PUT(bm,x,y,bitMap[offset]==0?0:1);
			}
		}
	} else {
		errCode = 1;
	}
	return errCode;
}
MULTIMAP_API void POTrace::FreeBitMap(potrace_bitmap_t **bitMap){
	if ( !bitMap ) {
		bitMap = &bm;
	}
	if (*bitMap) {
		free((*bitMap)->map);
		free((*bitMap));
		*bitMap = NULL;
	}
}
MULTIMAP_API void POTrace::FreeVectors(potrace_state_t **state ) {
	if ( !state ) {
		state = &st;
	}
	if ( *state ) {
		potrace_state_free(*state);
		*state = NULL;
	}
}
MULTIMAP_API void POTrace::FreeParameters(potrace_param_t **parameters ) {
	if (!parameters) {
		parameters = &param;
	}
	if ( *parameters ) {
		potrace_param_free(*parameters);
		*parameters = NULL;
	}
}
MULTIMAP_API void POTrace::SetGeotransform(double geotransform[6]) {
	for ( int i=0; i<6; i++ ) {
		this->geotransform[i] = geotransform[i];
	}
}
MULTIMAP_API int POTrace::Trace(potrace_bitmap_t *bitMap ) {
	if ( !bitMap ) {
		bitMap = bm;
	}
	if ( !param ) {
		param = potrace_param_default();
	}
	param->turdsize = 0;
	st = potrace_trace(param, bm);

	return st!=0?0:1;
}
MULTIMAP_API int POTrace::SaveToOGRLayer(const char* layerName, OGRDataSource* pDatasource, OGRLayer** ppLayer, double geotransform[6], bool polygon, potrace_state_t *state){
	int errCode = 0;
	if ( !state ) {
		state = st;
	}

	if (state) {
		if ( polygon ) {
			*ppLayer = pDatasource->CreateLayer(layerName,NULL,wkbPolygon,NULL);
		} else {
			*ppLayer = pDatasource->CreateLayer(layerName,NULL,wkbLineString,NULL);
		}

		double resolution = geotransform[1];
		double minX = geotransform[0] + (resolution/2.0);
		double maxY = geotransform[3] - (resolution/2.0);
		int xSize = this->bm->w;
		int ySize = this->bm->h;
		double minY = maxY-(ySize*resolution);

		OGRLayer* pLayer = *ppLayer;

		OGRFieldDefn idField("ID", OFTInteger);
		errCode = pLayer->CreateField(&idField);
		if ( errCode == 0 ) {
			OGRFeatureDefn* pFeatureDef = pLayer->GetLayerDefn();
			potrace_dpoint_t (*c)[3];
			potrace_path_t *p = state->plist;
			int id = 1;
			while (p != NULL) {
				int n = p->curve.n;
				int* tag = p->curve.tag;
				OGRFeature* pFeature = OGRFeature::CreateFeature(pFeatureDef);
				pFeature->SetField("ID",id++);
				OGRPolygon poly;
				OGRLinearRing ring;
				c = p->curve.c;
				double x[3];
				double y[3];
				for ( int j=0;j<3;j++) {
					x[j] = c[n-1][j].x * resolution + minX;
					y[j] = c[n-1][j].y * resolution + minY;
				}
				ring.addPoint(x[2],y[2]);
				printf("%12.3f %12.3f\n", x[2],y[2]);//c[n-1][2].x, c[n-1][2].y);
				for (int i=0; i<n; i++) {
					for ( int j=0;j<3;j++) {
						x[j] = c[i][j].x * resolution + minX;
						y[j] = c[i][j].y * resolution + minY;
					}
					switch (tag[i]) {
					case POTRACE_CORNER:
						ring.addPoint(x[1],y[1]);
						ring.addPoint(x[2],y[2]);
						printf("%12.3f %12.3 lineto\n", x[1],y[1]);//c[i][1].x, c[i][1].y);
						printf("%12.3f %12.3f lineto\n", x[2],y[2]);//c[i][2].x, c[i][2].y);
						break;
					case POTRACE_CURVETO:
						ring.addPoint(x[0],y[0]);
						ring.addPoint(x[1],y[1]);
						ring.addPoint(x[2],y[2]);
						printf("%12.3f %12.3f\n",x[0],y[0]);// : %12f %12f : %12f %12f curveto\n", 
						printf("%12.3f %12.3f\n",x[1],y[1]);
						printf("%12.3f %12.3f\n",x[2],y[2]);
							//x[0],y[0],//c[i][0].x, c[i][0].y,
							//x[1],y[1],//c[i][1].x, c[i][1].y,
							//x[2],y[2]);//c[i][2].x, c[i][2].y);
						break;
					}
				}
				/* at the end of a group of a positive path and its negative
				children, fill. */
				if (p->next == NULL || p->next->sign == '+') {
					//fprintf(file,"0 setgray fill\n");
				}
				poly.addRing(&ring);
				pFeature->SetGeometry(&poly);
				pLayer->CreateFeature(pFeature);
				p = p->next;
			}
		} else {
			errCode = 2;
		}
	} else {
		errCode = 1;
	}
	return errCode;
}
MULTIMAP_API int POTrace::SaveToEPS(const char* filePath, potrace_state_t *state) {
	int errCode = 0;

	if ( !state ) {
		state = st;
	}
	if ( state ) {
		/* output vector data, e.g. as a rudimentary EPS file */
		FILE* file = fopen(filePath,"wt");
		errCode = SaveToEPS(file,state);
		fclose(file);
	}
	return errCode;
}
MULTIMAP_API int POTrace::SaveToEPS(FILE* file, potrace_state_t *state) {
	int errCode = 0;

	if ( !state ) {
		state = st;
	}
	if ( state ) {
		if ( file ) {
			fprintf(file,"%%!PS-Adobe-3.0 EPSF-3.0\n");
			fprintf(file,"%%%%BoundingBox: 0 0 %d %d\n", WIDTH, HEIGHT);
			fprintf(file,"gsave\n");

			/* draw each curve */
			potrace_dpoint_t (*c)[3];
			potrace_path_t *p = state->plist;
			while (p != NULL) {
				int n = p->curve.n;
				int* tag = p->curve.tag;
				c = p->curve.c;
				fprintf(file,"%f %f moveto\n", c[n-1][2].x, c[n-1][2].y);
				for (int i=0; i<n; i++) {
					switch (tag[i]) {
					case POTRACE_CORNER:
						fprintf(file,"%f %f lineto\n", c[i][1].x, c[i][1].y);
						fprintf(file,"%f %f lineto\n", c[i][2].x, c[i][2].y);
						break;
					case POTRACE_CURVETO:
						fprintf(file,"%f %f %f %f %f %f curveto\n", 
							c[i][0].x, c[i][0].y,
							c[i][1].x, c[i][1].y,
							c[i][2].x, c[i][2].y);
						break;
					}
				}
				/* at the end of a group of a positive path and its negative
				children, fill. */
				if (p->next == NULL || p->next->sign == '+') {
					fprintf(file,"0 setgray fill\n");
				}
				p = p->next;
			}
			fprintf(file,"grestore\n");
			fprintf(file,"%%EOF\n");
		} else {
			errCode = 2;
		}
	} else {
		errCode = 1;
	}
	return errCode;
}
int POTrace::Test() {
	int errCode = 0;
	createTestBitmap();
	Trace();
	SaveToEPS("C:\\TEMP\\TEST.EPS");
	return errCode;
}
