#pragma once
#include "QuickHistogram.hpp"
#include "MBR.h"
#include <boost/unordered_map.hpp>

#define QDSM_START 0
#define QDSM_MAX 0
#define QDSM_MIN 1
#define QDSM_MEDIAN 2
#define QDSM_MEAN 3
#define QDSM_MODEA 4
#define QDSM_MODEM 5
#define QDSM_DIFFERENCE 6
#define QDSM_END 6

class Logger;
class QuickDSM {
public:
	MIZARASTER_API ~QuickDSM(void);
	MIZARASTER_API QuickDSM(int mode=QDSM_MAX, float resolution = 1.0f);

	//MIZARASTER_API bool Create(Container* _pContainer, std::string outputGeoTIFFPath, MBR *cropWindow = NULL);
	//MIZARASTER_API bool CreateBuffer(Container* _pContainer, MBR *cropWindow = NULL);
	MIZARASTER_API bool SaveBuffer(std::string outputGeoTIFFPath);
	MIZARASTER_API void FindEdgeData(void);
	MIZARASTER_API void ResetEdgeData(void);
	MIZARASTER_API size_t FillNoData(int minCount, size_t & passes );
	MIZARASTER_API QuickHistogram<float>* Histogram(size_t binCount = 64, std::string name = "");
	MIZARASTER_API float GetResolution(void);
	MIZARASTER_API size_t GetNoDataCount(bool forceRecount = false);
	MIZARASTER_API size_t GetEdgeCount(void);
	MIZARASTER_API DoublePoint GetOrigin(void);

	MIZARASTER_API float NeighborhoodMean(size_t x, size_t y,  size_t *count, size_t *noDataNeighbors = NULL);

	MIZARASTER_API inline size_t GetIndex(size_t x, size_t y);
	MIZARASTER_API float Get(size_t x, size_t y);
	MIZARASTER_API float Get(size_t index);
	MIZARASTER_API float Get(DoublePoint & dPoint, size_t *x = NULL, size_t *y = NULL);
	MIZARASTER_API DoublePoint GetPoint(size_t x, size_t y);
	MIZARASTER_API bool Set(float value, size_t x, size_t y);
	MIZARASTER_API bool GetNeighbor(size_t direction, size_t x, size_t y, size_t & ix, size_t & iy);
	MIZARASTER_API bool GetNeighbor(int direction, int x, int y, int & ix, int & iy);
	MIZARASTER_API bool InGrid(size_t x, size_t y);
	MIZARASTER_API bool InGrid(int x, int y);
	MIZARASTER_API bool InGrid(DoublePoint dPoint);
	MIZARASTER_API bool OnEdge(size_t x, size_t y);
	MIZARASTER_API bool OnEdge(int x, int y);
	MIZARASTER_API bool IsNoData(int x, int y);

	MIZARASTER_API bool GetDataSurround( size_t x, size_t y, size_t & minX, size_t & minY, size_t & maxX, size_t & maxY, size_t buffer=5 );
	MIZARASTER_API bool GetDataSurround( size_t x, size_t y, int & minX, int & minY, int & maxX, int & maxY, size_t buffer=5 );
	MIZARASTER_API bool GetDataSurround( int x, int y, int & minX, int & minY, int & maxX, int & maxY, size_t buffer=5 );

	//Container* pContainer;
	size_t          xSize;
	size_t          ySize;
	size_t       bandRows;

	MBR             range;
	int              espg;
	int              mode;
	bool             fill;
	size_t      maxPasses;
	bool          verbose;

	float*       pfBuffer;
	long long  bufferSize;

private:
	class Z {
	public:
		std::vector<float>       Zs;
		//std::vector<size_t>   index;
		size_t                  key;
		void Z::Clear() {
			Zs.clear();
			Zs.shrink_to_fit();
			//index.clear();
			//index.shrink_to_fit();
		}
		Z::~Z() {Clear();}
		friend class QuickDSM;
	};
	class XY {
	public:
		std::vector<Z> zs;
		void XY::Clear() { 
			for (size_t v=0; v<zs.size(); v++ ) {
				zs[v].Clear();
			}
			zs.clear();
			zs.shrink_to_fit();
		}
		XY::~XY() { Clear(); }
		friend class QuickDSM;
	};

	static const int	DeltaX[8]; // = { 0, 1, 1, 1, 0,-1,-1,-1 };
	static const int	DeltaY[8]; // = { 1, 1, 0,-1,-1,-1, 0, 1 };

	bool QuickDSM::ResetMeasureArrays(void);
	bool QuickDSM::ProcessRow(float* & elevations, size_t & row, unsigned int & rowsWritten, std::vector<DoublePoint> & dPoints);
	//bool QuickDSM::CreateSetup(Container* _pContainer, bool bufferMode = false, MBR *cropWindow = NULL);
	void QuickDSM::Compute(MBR mbr, unsigned int rowsThisStrip, std::vector<DoublePoint> & dPoints);
	void QuickDSM::ComputeMode(MBR mbr, unsigned int rowsThisStrip, std::vector<DoublePoint> & dPoints);
	void QuickDSM::VoxelToElevation(void);

	double QuickDSM::_Get_ValAtPos_BiCubicSpline(double dx, double dy, double z_xy[4][4]) const;

	float      resolution;
	float*       minimums;
	float*       maximums;
	unsigned int*   count;
	XY*             voxel;

	MBR blockMBR;
	unsigned int rowsPerStrip;
	size_t      cellsPerStrip;
	size_t             minRow;
	size_t             maxRow;
	size_t             minCol;
	size_t             maxCol;
	char           noData[16];
	float             fNoData;
	float           fEdgeData;
	size_t        noDataCount;
	size_t          edgeCount;
	bool             edgesSet;
	bool        noDataCounted;
	Logger*            logger;

	QuickHistogram<float>* histogram;
	friend class DSMTest;
};