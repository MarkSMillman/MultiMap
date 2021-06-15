#include "stdafx.h"
#include "QuickDSM.h"
//#include "QuickTIFF.h"
#include "QuickHistogram.hpp"
#include "Logger.h"

#undef min
#undef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

STATIC const int QuickDSM::DeltaX[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };
STATIC const int QuickDSM::DeltaY[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };

MIZARASTER_API QuickDSM::~QuickDSM(void) {
	if ( voxel ) {
		delete[] voxel;
		voxel = NULL;
	}
	if (minimums) {
		delete[] minimums;
		minimums = NULL;
	}
	if (maximums) {
		delete[] maximums;
		maximums = NULL;
	}
	if (count) {
		delete[] count;
		count = NULL;
	}
	if (histogram) {
		delete histogram;
		histogram = NULL;
	}
}
float ValidResolution ( float resolution ) {
	bool status = false;
	if ( resolution < 0.023438f ) {
		return 0.015625f;
	} else if ( resolution >= 0.023438f && resolution < 0.046875f ) {
		return 0.03125f;
	} else if ( resolution >= 0.046875f && resolution < 0.09375f ) {
		return 0.0625f;
	} else if ( resolution >= 0.09375f && resolution < 0.1875f ) {
		return 0.125f;
	} else if ( resolution >= 0.1875f && resolution < 0.375f ) {
		return 0.25f;
	} else if ( resolution >= 0.375f && resolution < 0.75f ) {
		return 0.5f;
	} else if ( resolution >= 0.75f && resolution < 1.5f ) {
		return 1.0f;
	} else if ( resolution >= 1.5f && resolution < 3.0f ) {
		return 2.0f;
	} else if ( resolution >= 3.0f && resolution < 6.0f ) {
		return 4.0f;
	} else if ( resolution >= 6.0f && resolution < 12.0f ) {
		return 8.0f;
	} else if ( resolution >= 12.0f && resolution < 24.0f ) {
		return 16.0f;
	} else if ( resolution >= 24.0f && resolution < 48.0f ) {
		return 32.0f;
	} else if ( resolution >= 48.0f && resolution < 96.0f ) {
		return 64.0f;
	} else if ( resolution >= 96.0f && resolution < 192.0f ) {
		return 128.0f;
	} else if ( resolution >= 192.0f && resolution < 384.0f ) {
		return 512.0f;
	} else if ( resolution >= 384.0f && resolution < 768.0f ) {
		return 1024.0f;
	} else if ( resolution >= 768.0f ) {
		return 2048.0f;
	}
	return status;
}
MIZARASTER_API QuickDSM::QuickDSM(int _mode, float _resolution) {
	logger = Logger::Instance();
	minimums = NULL;
	maximums = NULL;
	count = NULL;
	voxel = NULL;
	if ( _mode >= QDSM_START && _mode <= QDSM_END ) {
		mode = _mode;
	} else {
		mode = QDSM_MAX;
	}
	verbose = false;

	resolution = ValidResolution(_resolution);
	xSize = 0;
	ySize = 0;
	espg = 0;

	cellsPerStrip = 0;
	minRow = 0;
	maxRow = 0;
	minCol = 0;
	maxCol = 0;

	pfBuffer = NULL;
	bufferSize = 0;
	strcpy(noData,"-32767");
	fNoData = static_cast<float>(atof(noData));
	fEdgeData = -fNoData;
	noDataCount = 0;
	edgeCount = 0;
	noDataCounted = false;
	edgesSet = false;

	fill = false;
	maxPasses = 200;

	histogram = NULL;
}

MIZARASTER_API float QuickDSM::GetResolution(void) {
	return resolution;
}

MIZARASTER_API DoublePoint QuickDSM::GetOrigin(void) {
	DoublePoint origin(range.minX,range.minY,range.minZ);
	return origin;
}

MIZARASTER_API size_t QuickDSM::GetNoDataCount(bool forceRecount) {
	if ( !noDataCounted || forceRecount) {
		noDataCount = 0;
		for ( size_t b=0; b<xSize*ySize; b++ ) {
			if ( pfBuffer[b] == fNoData ) {
				noDataCount++;
			}
		}
		noDataCounted = true;
	}
	return noDataCount;
}
MIZARASTER_API size_t QuickDSM::GetEdgeCount(void) {
	return edgeCount;
}
#ifdef _WOOLPERT
PRIVATE bool QuickDSM::CreateSetup(Container* _pContainer, bool bufferMode, MBR *cropWindow) {
	bool status = true;
	pContainer = _pContainer;
	if ( pContainer ) {
		if ( cropWindow ) {
			range = *cropWindow;
		} else {
			range = pContainer->GetHeader().mbr;
		}
		range.Align((unsigned long)pContainer->crateSize);

		// adjust range and resolution to fit container
		DoublePoint lowerLeft(range.minX,range.minY);
		size_t blockIndex = pContainer->CrateIndex(lowerLeft);
		pContainer->CrateIndex(&minCol,&minRow,blockIndex);

		DoublePoint upperRight(range.maxX,range.maxY);
		blockIndex = pContainer->CrateIndex(upperRight);
		pContainer->CrateIndex(&maxCol,&maxRow,blockIndex);

		minCol = max(0,minCol);
		minRow = max(0,minRow);
		maxCol = min(pContainer->cratesX,maxCol);
		maxRow = min(pContainer->cratesY,maxRow);
		float tileSize = static_cast<float>(pContainer->crateSize);

		if ( resolution < 1.0f ) {
			int iresolution = static_cast<int>(tileSize/resolution);
			resolution = tileSize / static_cast<float>(iresolution);
		}

		// set the range to map to the subarea of the file.
		pContainer->GetMBR(&blockMBR,minCol,minRow);
		range.minX = blockMBR.minX;
		range.minY = blockMBR.minY;
		pContainer->GetMBR(&blockMBR,maxCol,maxRow);
		range.maxX = blockMBR.maxX;
		range.maxY = blockMBR.maxY;

		xSize = static_cast<size_t>((range.maxX - range.minX )/resolution);
		ySize = static_cast<size_t>((range.maxY - range.minY )/resolution);

		rowsPerStrip = static_cast<unsigned int>(tileSize / resolution);
		cellsPerStrip = xSize*rowsPerStrip;

		switch (mode) {
		case QDSM_MAX:
			maximums = new float[cellsPerStrip];
			break;
		case QDSM_MIN:
			minimums = new float[cellsPerStrip];
			break;
		case QDSM_DIFFERENCE:
		case QDSM_MEDIAN:
			maximums = new float[cellsPerStrip];
			minimums = new float[cellsPerStrip];
			break;
		case QDSM_MEAN:
			maximums = new float[cellsPerStrip];
			count = new unsigned int[cellsPerStrip];
			break;
		case QDSM_MODEA:
		case QDSM_MODEM:
			voxel = new XY[cellsPerStrip];
			maximums = new float[cellsPerStrip];
			break;
		default:
			status = false;
			break;
		}
	}
	return status;
}
#endif
PRIVATE bool QuickDSM::ResetMeasureArrays(void) {
	bool status = true;
	switch (mode) {
	case QDSM_MAX:
		for ( size_t c=0; c<cellsPerStrip; c++ ) {
			maximums[c] = -FLT_MAX;
		}
		break;
	case QDSM_MIN:
		for ( size_t c=0; c<cellsPerStrip; c++ ) {
			minimums[c] = FLT_MAX;
		}
		break;
	case QDSM_DIFFERENCE:
	case QDSM_MEDIAN:
		for ( size_t c=0; c<cellsPerStrip; c++ ) {
			minimums[c] = FLT_MAX;
			maximums[c] = -FLT_MAX;
		}
		break;
	case QDSM_MEAN:
		for ( size_t c=0; c<cellsPerStrip; c++ ) {
			count[c] = 0;
			maximums[c] = 0.f;
		}
		break;
	case QDSM_MODEA:
	case QDSM_MODEM:
		for ( size_t v=0; v<cellsPerStrip; v++ ) {
			voxel[v].Clear();
			maximums[v] = 0.f;
		}
		break;
	default:
		status = false;
		break;
	}
	return status;
}
#ifdef _WOOLPERT
PRIVATE bool QuickDSM::ProcessRow(float* & elevations, size_t & row, unsigned int & rowsWritten, std::vector<DoublePoint> & dPoints) {
	bool status = ResetMeasureArrays();

	unsigned int rowsThisStrip = rowsPerStrip;
	if ( rowsPerStrip + rowsWritten > ySize ) {
		rowsThisStrip = static_cast<unsigned int>(ySize) - rowsWritten;
	}
	for ( size_t col=minCol; col<maxCol; col++ ) {
		dPoints.clear();
		pContainer->GetPointsXY(dPoints,col,row);
		if ( dPoints.size() > 0 ) {
			pContainer->GetMBR(&blockMBR,col,row);
			if ( mode == QDSM_MODEA || mode == QDSM_MODEM ) {
				ComputeMode(blockMBR,rowsThisStrip,dPoints);
			} else {
				Compute(blockMBR,rowsThisStrip,dPoints);
			}

		}
	}

	size_t undefinedPerStrip = 0;
	switch (mode) {
	case QDSM_MAX:
		elevations = maximums;
		for ( size_t c=0; c<cellsPerStrip; c++ ) {
			if ( elevations[c] == -FLT_MAX ) {
				elevations[c] = fNoData;
				undefinedPerStrip++;
			}
		}
		break;
	case QDSM_MIN:
		elevations = minimums;
		for ( size_t c=0; c<cellsPerStrip; c++ ) {
			if ( elevations[c] == FLT_MAX ) {
				elevations[c] = fNoData;
				undefinedPerStrip++;
			}
		}
		break;
	case QDSM_DIFFERENCE:
		for ( size_t m=0; m<cellsPerStrip; m++ ) {
			if ( maximums[m] == -FLT_MAX || minimums[m] == FLT_MAX ) {
				maximums[m] = fNoData;
				undefinedPerStrip++;
			} else {
				maximums[m] -= minimums[m];
			}
		}
		elevations = maximums;
		break;
	case QDSM_MEDIAN:
		for ( size_t m=0; m<cellsPerStrip; m++ ) {
			if ( maximums[m] == -FLT_MAX || minimums[m] == FLT_MAX ) {
				maximums[m] = fNoData;
				undefinedPerStrip++;
			} else {
				maximums[m] = (maximums[m]/2.0f + minimums[m]/2.0f);
			}
		}
		elevations = maximums;
		break;
	case QDSM_MEAN:
		elevations = maximums;
		for ( size_t c=0; c<cellsPerStrip; c++ ) {
			if ( count[c] == 0 ) {
				elevations[c] = fNoData;
				undefinedPerStrip++;
			}
		}
		break;
	case QDSM_MODEA:
	case QDSM_MODEM:
		VoxelToElevation();
		elevations = maximums;
		break;
	}
	return status;
}
#endif
MIZARASTER_API void QuickDSM::FindEdgeData(void) {
	int x_size = static_cast<int>(xSize);
	int y_size = static_cast<int>(ySize);
	int y=0;
	for ( int x=0; x<x_size; x++ ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else {
			break;
		}
	}

	for ( int x=x_size-1; x>=0; x-- ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else if (pfBuffer[c] != fEdgeData) {
			break;
		}
	}

	y=y_size-1;
	for ( int x=0; x<x_size; x++ ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else if (pfBuffer[c] != fEdgeData) {
			break;
		}
	}

	for ( int x=x_size-1; x>=0; x-- ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else if (pfBuffer[c] != fEdgeData) {
			break;
		}
	}

	int x=0;
	for ( int y=0; y<y_size; y++ ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else if (pfBuffer[c] != fEdgeData) {
			break;
		}
	}

	for ( int y=y_size-1; y>=0; y-- ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else if (pfBuffer[c] != fEdgeData) {
			break;
		}
	}

	x=x_size-1;
	for ( int y=0; y<y_size; y++ ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else if (pfBuffer[c] != fEdgeData) {
			break;
		}
	}

	for ( int y=y_size-1; y>=0; y-- ) {
		size_t c = GetIndex(x,y);
		if ( pfBuffer[c] == fNoData ) {
			pfBuffer[c] = fEdgeData;
			edgeCount++;
		} else if (pfBuffer[c] != fEdgeData) {
			break;
		}
	}	
	edgesSet = true;
}
MIZARASTER_API void QuickDSM::ResetEdgeData(void) {
	for ( size_t d=0; d<xSize*ySize; d++ ) {
		if ( pfBuffer[d] == fEdgeData ) {
			pfBuffer[d] = fNoData;
		}
	}
	edgesSet = false;
}
/**
* @param minCount if positive the minimum number of populated adjacent pixels 
if negative the minimum difference between the populated and NoData adjacent pixels
required to update a given pixel
* @param passes the maximum number of passes through pfBuffer to make (decremented on output by the actual number of passes
* @return the number of filled NoData pixels
**/
MIZARASTER_API size_t QuickDSM::FillNoData(int minCount, size_t & passes) {
	size_t filledHoles = 0;
	bool reset = false;
	if ( !edgesSet ) {
		FindEdgeData();
		reset = true;
	}
	size_t passesOut = passes;
	size_t previousSumNoData = GetNoDataCount(true);
	for ( size_t pass = 0; pass<passes; pass++ ) {
		passesOut--;
		size_t sumNoData = 0;
		for ( size_t y=0; y<ySize; y++ ) {
			for ( size_t x=0; x<xSize; x++ ) {
				size_t nx,ny;
				DoublePoint dPoint = GetPoint(x,y);
				float elevation = Get(dPoint,&nx,&ny);
				if (elevation == fNoData) {
					size_t noDataNeighbors = 0;
					size_t count = 0;
					float average = NeighborhoodMean(x,y, &count, &noDataNeighbors);
					if ( minCount > 0 ) {
						if ( count > minCount ) { //&& count >= noDataNeighbors ) {
							Set(average,x,y);
							filledHoles++;
						} else {
							sumNoData++;
						}
					} else {
						if ( count - noDataNeighbors > -minCount ) {
							Set(average,x,y);
							filledHoles++;
						} else {
							sumNoData++;
						}
					}
				}
				if ( GetNoDataCount() == 0 ) {
					break;
				}
			}
		}
		if ( verbose ) {
			logger->Log(ALWAYS,"Remaining iterations %4lu minimum neighbor count=%d remaining NoData pixels %lu",(passes-pass),minCount,GetNoDataCount());
		}
		if ( sumNoData == previousSumNoData ) {
			break;
		} else {
			previousSumNoData = sumNoData;
		}
		if ( GetNoDataCount() == 0 ) {
			break;
		}
	}
	passes = passesOut;
	if ( reset ) {
		ResetEdgeData();
	}
	return filledHoles;
}
#ifdef _WOOLPERT
MIZARASTER_API bool QuickDSM::CreateBuffer(Container* _pContainer, MBR *cropWindow) {
	bool status = CreateSetup(_pContainer,true,cropWindow);
	if ( status ) {
		bufferSize = xSize * ySize * sizeof(float);
		std::vector<DoublePoint> dPoints;
		unsigned int rowsWritten = 0;
		bufferSize = xSize*ySize*sizeof(float);
		pfBuffer = new float[xSize*ySize];
		float* pBuffer = pfBuffer;
		for ( size_t row=minRow; row<=maxRow; row++ ) {
			float* elevations = NULL;
			QuickDSM::ProcessRow(elevations, row, rowsWritten, dPoints);

			memcpy(pBuffer,elevations,cellsPerStrip*sizeof(float));
			pBuffer += cellsPerStrip;
			rowsWritten++;
		}
		dPoints.clear();
		dPoints.shrink_to_fit();

		FindEdgeData();

		if ( fill ) {
			int count = 8;
			size_t passes = maxPasses;
			while (  GetNoDataCount() > 0 && passes > 0 && count > 0) {
				FillNoData(count,passes);
				count--;
			}
		}
	}
	return status;
}

MIZARASTER_API bool QuickDSM::SaveBuffer(std::string outputGeoTIFFPath) {
	bool status = true;
	MFUtils mfUtils;
	if ( pfBuffer && bufferSize > 0 ) {
		ResetEdgeData();
		if ( outputGeoTIFFPath.length() > 0 ) {
			mfUtils.FolderExists(outputGeoTIFFPath,true);
			QuickTIFF quickTIFF;
			double origin[2] = {range.minX,range.minY};
			TIFF* hTIFF = quickTIFF.AllocateDEM(outputGeoTIFFPath,(unsigned int)xSize,(unsigned int)ySize,origin,resolution);
			if ( hTIFF ) {
				unsigned int rowOffset = 0;
				unsigned int cellOffset = 0;
				for ( size_t y=0; y<ySize; y++ ) {
					if (quickTIFF.WriteDEM(hTIFF,(unsigned int)xSize,rowOffset,1,&pfBuffer[cellOffset]) ) {
						rowOffset++;
						cellOffset += (unsigned int)xSize;
					} else {
						status = false;
						break;
					}
				}
				quickTIFF.CloseDEM(hTIFF);
				quickTIFF.Close();
			}
		} else {
			status = false;
		}
	} else {
		status = false;
	}
	return status;
}

MIZARASTER_API bool QuickDSM::Create(Container* _pContainer, std::string outputGeoTIFFPath, MBR *cropWindow) {
	bool status = true;
	MFUtils mfUtils;
	pContainer = _pContainer;
	if ( outputGeoTIFFPath.length() > 0 && CreateSetup(_pContainer,false,cropWindow) ) {
		mfUtils.FolderExists(outputGeoTIFFPath,true);
		QuickTIFF quickTIFF;
		double origin[2] = {range.minX,range.minY};
		TIFF* hTIFF = quickTIFF.AllocateDEM(outputGeoTIFFPath,(unsigned int)xSize,(unsigned int)ySize,origin,resolution);

		if ( hTIFF ) {
			std::vector<DoublePoint> dPoints;
			unsigned int rowsWritten = 0;
			unsigned int offset = 0;
			for ( size_t row=minRow; row<=maxRow; row++ ) {
				unsigned int rowsThisStrip = rowsPerStrip;
				float* elevations = NULL;

				QuickDSM::ProcessRow(elevations, row, rowsWritten, dPoints);

				if (quickTIFF.WriteDEM(hTIFF,(unsigned int)xSize,offset,rowsThisStrip,elevations) ) {
					offset += rowsThisStrip;
				} else {
					status = false;
					break;
				}
			}

			dPoints.clear();
			dPoints.shrink_to_fit();
			quickTIFF.CloseDEM(hTIFF);
			quickTIFF.Close();
		} else {
			logger->Log(ERR,"Could not allocate %s",outputGeoTIFFPath.c_str());
			status = false;
		}
	} else {
		Logger::Instance()->Log(FATAL,"%s called with NULL pContainer or empty outputPath");
		status = false;
	}
	return status;
}
#endif
MIZARASTER_API float QuickDSM::NeighborhoodMean(size_t x, size_t y, size_t *count, size_t *noDataNeighbors){
	float elevation = fNoData;
	int alpha=0;
	int beta = 0;
	if ( InGrid(x,y) ) {
		if (noDataNeighbors) {
			*noDataNeighbors = 0;
		}
		double sum = 0.0;
		*count = 0;
		int ix = static_cast<int>(x);
		int iy = static_cast<int>(y);
		int tx,ty;
		for ( int direction=0; direction<8; direction++ ) {
			if ( GetNeighbor(direction,ix,iy,tx,ty) ) {
				size_t eIndex = ty * xSize + tx;
				float e = pfBuffer[eIndex];
				if ( e != fNoData && e != fEdgeData ) {
					sum += static_cast<double>(e);
					(*count)++;
				} else if (noDataNeighbors) {
					(*noDataNeighbors)++;
				}
			}
		}
		if ( (*count) > 0 ) {
			elevation = static_cast<float>( sum / *count );
		}
	}
	return elevation;
}
MIZARASTER_API inline size_t QuickDSM::GetIndex(size_t x, size_t y) {
	size_t eIndex = y * xSize + x;
	return eIndex;
}
MIZARASTER_API float QuickDSM::Get(size_t x, size_t y) {
	float elevation = fNoData;
	if ( x < xSize && y < ySize ) {
		size_t eIndex = y * xSize + x;
		elevation = pfBuffer[eIndex];
	}
	return elevation;
}

MIZARASTER_API float QuickDSM::Get(DoublePoint & dPoint, size_t *x, size_t *y) {
	float elevation = fNoData;
	double dx = dPoint.X - range.minX;
	double dy = dPoint.Y - range.minY;
	unsigned int rx = static_cast<unsigned int>(dx / resolution);
	if ( rx >= 0 && rx < xSize ) {
		unsigned int ry = static_cast<unsigned int>(dy / resolution);
		if ( ry >= 0 && ry < ySize ) {
			size_t eIndex = ry * xSize + rx;
			elevation = pfBuffer[eIndex];
			if ( x ) {
				*x = static_cast<size_t>(rx);
			}
			if ( y ) {
				*y = static_cast<size_t>(ry);
			}
		}
	}
	return elevation;
}

MIZARASTER_API DoublePoint QuickDSM::GetPoint(size_t x, size_t y) {
	DoublePoint dPoint;
	if ( x < xSize && y < ySize ) {
		dPoint.X = static_cast<double>(x) * resolution + range.minX + resolution/2.0;
		dPoint.Y = static_cast<double>(y) * resolution + range.minY + resolution/2.0;
		size_t eIndex = y * xSize + x;
		dPoint.Z = pfBuffer[eIndex];
	}
	return dPoint;
}

MIZARASTER_API bool QuickDSM::Set(float value, size_t x, size_t y) {
	bool status = false;
	if ( x < xSize && y < ySize ) {
		size_t eIndex = y * xSize + x;
		if ( pfBuffer[eIndex] == fNoData && value != fNoData) {
			noDataCount--;
		}
		if ( pfBuffer[eIndex] != fNoData && value == fNoData) {
			noDataCount++;
		}
		if ( edgesSet ) {
			if (pfBuffer[eIndex] == fEdgeData && value != fEdgeData) {
				edgeCount--;
			}
			if ( pfBuffer[eIndex] != fEdgeData && value == fEdgeData) {
				edgeCount++;
			}
		}
		pfBuffer[eIndex] = value;
		status = true;
	}
	return status;
}

MIZARASTER_API bool QuickDSM::GetNeighbor(size_t direction, size_t x, size_t y, size_t & ix, size_t & iy) {
	bool status = InGrid(x,y);
	if ( status ) {
		direction %= 8;

		int iix = static_cast<int>(x) + DeltaX[direction];
		if ( iix < 0 ) {
			status = false;
		} else {
			int iiy = static_cast<int>(y) + DeltaY[direction];
			if ( iiy < 0 ) {
				status = false;
			} else {
				ix = iix;
				iy = iiy;
				status = InGrid(ix,iy);
			}
		}
	}
	return status;
}
MIZARASTER_API bool QuickDSM::GetNeighbor(int direction, int x, int y, int & ix, int & iy) {
	bool status = InGrid(x,y);
	if ( status ) {
		direction %= 8;
		if ( direction < 0 ) {
			direction += 8;
		}
		ix = x + DeltaX[direction];
		iy = y + DeltaY[direction];

		status = InGrid(ix,iy);
	}
	return status;
}
MIZARASTER_API bool QuickDSM::InGrid(DoublePoint dPoint) {
	bool status = false;
	if ( range.Covers(dPoint) ) {
		status = true;
	}
	return status;
}
MIZARASTER_API bool QuickDSM::InGrid(size_t x, size_t y) {
	bool status = true;
	if ( x >= xSize || y >= ySize ) {
		status = false;
	}
	return status;
}
MIZARASTER_API bool QuickDSM::InGrid(int x, int y) {
	bool status = true;
	if ( x < 0 || y < 0 || x >= xSize || y >= ySize ) {
		status = false;
	}
	return status;
}

MIZARASTER_API bool QuickDSM::OnEdge(size_t x, size_t y) {
	return OnEdge( (int) x, (int) y );
}

MIZARASTER_API bool QuickDSM::OnEdge(int x, int y) {
	bool status = true;
	if ( x <= 0 || x >= (int)xSize || y <= 0 || y >= (int)ySize ) {
		status = true;
	} else {
		int startX = x;
		while ( --startX >= 0 && Get(startX,y) == fNoData );
		if ( startX <= 0 ) {
			status = true;
		} else {
			startX = x;
			while ( ++startX < (int)xSize && Get(startX,y) == fNoData );
			if ( startX >= (int)xSize ) {
				status = true;
			} else {
				int startY = y;
				while ( --startY > 0 && Get(x,startY) == fNoData);
				if ( startY <= 0 ) {
					status = true;
				} else {
					while ( ++startY < (int)ySize && Get(x,startY) == fNoData );
					if ( startY >= (int)ySize ) {
						status = true;
					}
				}
			}
		}
	}
	return status;
}

void QuickDSM::Compute(MBR mbr, unsigned int rowsThisStrip, std::vector<DoublePoint> & dPoints) {

	for ( size_t p=0; p<dPoints.size(); p++ ) {
		double x = dPoints[p].X - range.minX;
		double y = dPoints[p].Y - mbr.minY;
		unsigned int rx = static_cast<unsigned int>(x / resolution);
		unsigned int ry = static_cast<unsigned int>(y / resolution);
		//unsigned int ry = rowsThisStrip - static_cast<unsigned int>(y / resolution) - 1;
		if ( rx < 0 ) 
			rx = 0;
		if ( rx >= xSize ) 
			rx = (unsigned int)xSize - 1;
		if ( ry < 0 ) 
			ry = 0;
		if ( ry >= rowsThisStrip ) {
			ry = rowsThisStrip;
		}
		size_t eIndex = ry * xSize + rx;
		switch (mode) {
		case QDSM_MAX:
			maximums[eIndex] = max(maximums[eIndex],static_cast<float>(dPoints[p].Z));
			break;
		case QDSM_MIN:
			minimums[eIndex] = min(minimums[eIndex],static_cast<float>(dPoints[p].Z));
			break;
		case QDSM_DIFFERENCE:
		case QDSM_MEDIAN:
			maximums[eIndex] = max(maximums[eIndex],static_cast<float>(dPoints[p].Z)); 
			minimums[eIndex] = min(minimums[eIndex],static_cast<float>(dPoints[p].Z));
			break;
		case QDSM_MEAN:
			double sum = static_cast<double>(maximums[eIndex]) * count[eIndex] + static_cast<double>(dPoints[p].Z);
			count[eIndex]++;
			maximums[eIndex] = static_cast<float>(sum / static_cast<double>(count[eIndex]));
			break;
		}
	}
}

void QuickDSM::ComputeMode(MBR mbr, unsigned int rowsThisStrip, std::vector<DoublePoint> & dPoints) {
	mbr.UpdateZ(dPoints);
	for ( size_t p=0; p<dPoints.size(); p++ ) {
		double x = dPoints[p].X - range.minX;
		double y = dPoints[p].Y - mbr.minY;
		unsigned int rx = static_cast<unsigned int>(x / resolution);
		unsigned int ry = static_cast<unsigned int>(y / resolution);
		//unsigned int ry = rowsThisStrip - static_cast<unsigned int>(y / resolution) - 1;

		if ( rx < 0 ) 
			rx = 0;
		if ( rx >= xSize ) 
			rx = (unsigned int)xSize - 1;
		if ( ry < 0 ) 
			ry = 0;
		if ( ry >= rowsThisStrip ) {
			ry = rowsThisStrip-1;
		}
		// compute a Z key
		size_t Z =  static_cast<size_t>((dPoints[p].Z - mbr.minZ) / resolution);
		size_t I = ry * xSize + rx;
		bool found = false;
		for ( size_t z=0; z<voxel[I].zs.size(); z++ ) {
			if ( voxel[I].zs[z].key == Z ) {
				found = true;
				voxel[I].zs[z].Zs.push_back(static_cast<float>(dPoints[p].Z));
				break;
			}
		}
		if ( !found ) {
			QuickDSM::Z z;
			z.key = Z;
			z.Zs.push_back(static_cast<float>(dPoints[p].Z));
			voxel[I].zs.push_back(z);
		}
	}
}

void QuickDSM::VoxelToElevation(void) {
	for ( size_t xy=0; xy<cellsPerStrip; xy++ ) {
		size_t size = 0;
		size_t modeIndex = ULLONG_MAX;
		if ( voxel[xy].zs.size() > 0 ) {
			// find the modal Z-cell of the voxel element
			for ( size_t v=0; v<voxel[xy].zs.size(); v++ ) {
				if ( voxel[xy].zs[v].Zs.size() > size ) {
					modeIndex = v;
					size = voxel[xy].zs[v].Zs.size();
				}
			}
			if ( modeIndex < voxel[xy].zs.size() ) {
				if ( mode == QDSM_MODEA ) {
					// use the average of the Z values in the voxel cell
					maximums[xy] = 0.f;
					size_t count=0;
					for ( size_t p=0; p<voxel[xy].zs[modeIndex].Zs.size(); p++ ) {
						double sum = static_cast<double>(maximums[xy]) * count + static_cast<double>(voxel[xy].zs[modeIndex].Zs[p]);
						count++;
						maximums[xy] = static_cast<float>(sum / static_cast<double>(count));
					}
				} else {
					// use the median of the Z values in the voxel cell
					float minZ = FLT_MAX;
					float maxZ = -FLT_MAX;
					for ( size_t p=0; p<voxel[xy].zs[modeIndex].Zs.size(); p++ ) {
						minZ = min(minZ,voxel[xy].zs[modeIndex].Zs[p]);
						maxZ = max(maxZ,voxel[xy].zs[modeIndex].Zs[p]);
					}
					maximums[xy] = minZ/2.0f + maxZ/2.0f;
				}
			}
		} else {
			maximums[xy] = fNoData;
		}
	}
}
QuickHistogram<float>* QuickDSM::Histogram(size_t binCount, std::string name ) {
	if ( histogram ) {
		delete histogram;
		histogram = NULL;
	}
	histogram = new QuickHistogram<float>(binCount,name);
	histogram->noData = atoi(noData);
	if ( pfBuffer && bufferSize > 0 ) {
		histogram->Array(pfBuffer,xSize,ySize);
	}

	return histogram;
}

bool QuickDSM::GetDataSurround( size_t x, size_t y, size_t & minX, size_t & minY, size_t & maxX, size_t & maxY, size_t buffer ) {
	int iminX, imaxX, iminY, imaxY;
	bool status = GetDataSurround((int)x, (int)y, iminX, iminY, imaxX, imaxY, buffer);
	minX = static_cast<size_t>(iminX);
	maxX = static_cast<size_t>(imaxX);
	minY = static_cast<size_t>(iminY);
	maxY = static_cast<size_t>(imaxY);
	return status;
}

bool QuickDSM::GetDataSurround( size_t x, size_t y, int & minX, int & minY, int & maxX, int & maxY, size_t buffer ) {
	return GetDataSurround((int)x, int(y), minX, minY, maxX, maxY, buffer);
}

bool QuickDSM::GetDataSurround( int x, int y, int & minX, int & minY, int & maxX, int & maxY, size_t buffer ) {
	bool status = false;
	if ( InGrid(x,y) ) {
		if ( Get(x,y) == fNoData ) {
			int ix = static_cast<int>(x);
			int iy = static_cast<int>(y);

			minX = maxX = x;
			minY = maxY = y;

			int startX = x;
			int stopX = x;
			while ( startX > 0  ) {
				float e = Get(startX,y);
				if ( e == fNoData ) {
					startX--;
				} else {
					break;
				}
			}
			while ( stopX < xSize ) {
				float e = Get(stopX,y);
				if ( e == fNoData ) {
					stopX++;
				} else {
					break;
				}
			}

			float a,b;
			for ( int xx=startX; xx<=stopX; xx++ ) {
				int startY = y;
				int stopY = y;
				while ( startY > 0  ) {
					float e = Get(xx,startY);
					if ( e == fNoData ) {
						startY--;
					} else {
						break;
					}
				}
				while ( stopY < xSize ) {
					float e = Get(xx,stopY);
					if ( e == fNoData ) {
						stopY++;
					} else {
						break;
					}
				}
				minY = min(minY,startY);
				maxY = max(maxY,stopY);

				a = Get(xx, startY);
				b = Get(xx, stopY);
			}

			for ( int yy=minY; yy<=maxY; yy++ ) {
				startX = x;
				stopX = x;
				while ( startX > 0  ) {
					float e = Get(startX,y);
					if ( e == fNoData ) {
						startX--;
					} else {
						break;
					}
				}
				while ( stopX < xSize ) {
					float e = Get(stopX,y);
					if ( e == fNoData ) {
						stopX++;
					} else {
						break;
					}
				}
				minX = min(minX,startX);
				maxX = max(maxX,stopX);
			}
			minX = max(0,minX-(int)buffer);
			maxX = min((int)xSize-1,maxX+(int)buffer);
			minY = max(0,minY-(int)buffer);
			maxY = min((int)ySize-1,maxY+(int)buffer);
			if ( (maxX-minX) - (maxY-minY) > 2 ) {
				status = true;
			}
		}
	}
	return status;
}

double QuickDSM::_Get_ValAtPos_BiCubicSpline(double dx, double dy, double z_xy[4][4]) const {
	double	a0, a2, a3, b1, b2, b3, c[4];

	for (int i=0; i<4; i++) 	{
		a0		= z_xy[0][i] - z_xy[1][i];
		a2		= z_xy[2][i] - z_xy[1][i];
		a3		= z_xy[3][i] - z_xy[1][i];

		b1		= -a0 / 3.0 + a2       - a3 / 6.0;
		b2		=  a0 / 2.0 + a2 / 2.0;
		b3		= -a0 / 6.0 - a2 / 2.0 + a3 / 6.0;

		c[i]	= z_xy[1][i] + b1 * dx + b2 * dx*dx + b3 * dx*dx*dx;
	}

	a0		= c[0] - c[1];
	a2		= c[2] - c[1];
	a3		= c[3] - c[1];

	b1		= -a0 / 3.0 + a2       - a3 / 6.0;
	b2		=  a0 / 2.0 + a2 / 2.0;
	b3		= -a0 / 6.0 - a2 / 2.0 + a3 / 6.0;

	return( c[1] + b1 * dy + b2 * dy*dy + b3 * dy*dy*dy );
}