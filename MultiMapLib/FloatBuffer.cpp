#include "MultiMap.h"
#include "NRC_Laplace.h"
#include "FloatBuffer.h"
#include "Parameters.h"
#include "MBRect.h"
#include "Logger.h"

#pragma warning ( disable : 4251 ) // disable std::vector<_Ty> needs dll-interface warning from within gdal_priv.h
#include "gdal_priv.h"

#undef min
#undef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

MULTIMAP_API FloatBuffer::~FloatBuffer(void) {
	if (!foreignBuffer && buffer) {
		delete[] buffer;
		buffer = NULL;
	}
}

MULTIMAP_API FloatBuffer::FloatBuffer(void) {

	logger = Logger::Instance();
	logger->logToFile = false;
	logger->logToScreen = true;
	logger->SetLevel(INFO);

	xSize = 0;
	ySize = 0;
	buffer = NULL;
	dataType = GDT_Float32;
	foreignBuffer = false;

	memset(geotransform, 0, sizeof(geotransform));
	memset(szProjectionRef, 0, sizeof(szProjectionRef));
	memset(szDriverDescr, 0, sizeof(szDriverDescr));
	resolution = 0.0;
	errCode = CE_None;
}
MULTIMAP_API FloatBuffer::FloatBuffer(float* _buffer, unsigned int _xSize, unsigned int _ySize, double _resolution) {
	logger = Logger::Instance();
	logger->logToFile = false;
	logger->logToScreen = true;
	logger->SetLevel(INFO);

	buffer = _buffer;
	dataType = GDT_Float32;
	xSize = _xSize;
	ySize = _ySize;
	dNoData = static_cast<double>(-32767);
	foreignBuffer = true;

	memset(geotransform, 0, sizeof(geotransform));
	memset(szProjectionRef, 0, sizeof(szProjectionRef));
	memset(szDriverDescr, 0, sizeof(szDriverDescr));
	resolution = _resolution;
	errCode = CE_None;
}

MULTIMAP_API void FloatBuffer::setBuffer(float* _buffer, unsigned int _xSize, unsigned int _ySize) {
	if (!foreignBuffer && buffer) {
		delete[] buffer;
	}
	buffer = _buffer;
	xSize = _xSize;
	ySize = _ySize;
	foreignBuffer = true;
}
MULTIMAP_API float* FloatBuffer::getBuffer(unsigned int* _xSize, unsigned int* _ySize) {
	if (_xSize)
		*_xSize = xSize;
	if (_ySize)
		*_ySize = ySize;

	return buffer;
}

MULTIMAP_API int FloatBuffer::ReadBuffer(GDALDataset* pDataset, MBRect *cropWindow) {
	GDALDriver*  pDriver = pDataset->GetDriver();
	const char* pDDesc = pDriver->GetDescription();
	strcpy(szDriverDescr, pDDesc);
	const char* pProj = pDataset->GetProjectionRef();
	strcpy(szProjectionRef, pProj);
	xSize = pDataset->GetRasterXSize();
	ySize = pDataset->GetRasterYSize();
	pDataset->GetGeoTransform(geotransform);
	resolution = geotransform[1];
	GDALRasterBand*	pBand = pDataset->GetRasterBand(1);
	if (pBand) {
		int bSuccess = FALSE;
		dNoData = pBand->GetNoDataValue(&bSuccess);
		GDALDataType gDataType = pBand->GetRasterDataType();

		if (gDataType == GDT_Float32) {
			dataType = (int)gDataType;
			buffer = new float[ySize * xSize];

			for (unsigned int y = 0; y < ySize; y++) {
				unsigned int offset = y*xSize;
				float* scanLine = &buffer[offset];
				errCode = pBand->RasterIO(GF_Read, 0, y, xSize, 1, scanLine, xSize, 1, GDT_Float32, 0, 0);
				if (errCode != CE_None) {
					break;
				}
			}
		}
	}
	return errCode;
}
MULTIMAP_API bool FloatBuffer::ReadBuffer(const char* inputPath, MBRect *cropWindow) {
	errCode = CE_Failure;
	if (buffer) {
		delete[] buffer;
		buffer = NULL;
	}
	xSize = 0;
	ySize = 0;

	GDALRegister_PNG();
	GDALRegister_GTiff();
	GDALDatasetH datasetH = GDALOpen(inputPath, GA_ReadOnly);
	if (datasetH) {
		GDALDataset* pDataset = (GDALDataset*)datasetH;
		errCode = ReadBuffer(pDataset, cropWindow);
	}
	else {
		errCode = CE_Failure;
	}
	return errCode == CE_None ? true : false;
}
MULTIMAP_API int FloatBuffer::SaveBuffer(GDALDataset* pDataset, MBRect *cropWindow) {
	errCode = CE_Failure;
	if (pDataset) {
		GDALRasterBand*	pBand = pDataset->GetRasterBand(1);
		unsigned int row = 0;
		for (unsigned int y = 0; y < ySize; y++) {
			unsigned int offset = ((ySize - 1) - y)*xSize;
			float* scanLine = &buffer[offset];
			errCode = pBand->RasterIO(GF_Write, 0, y, xSize, 1, scanLine, xSize, 1, GDT_Float32, 0, 0);
			if (errCode != CE_None) {
				break;
			}
		}
	}
	return errCode;
}
MULTIMAP_API bool FloatBuffer::SaveBuffer(const char* outputPath, MBRect *cropWindow) {
	errCode = CE_Failure;
	GDALRegister_GTiff();
	GDALDriver* pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (pDriver) {
		char **papszOptions = NULL;

		GDALDataset* pDataset = pDriver->Create(outputPath, (int)xSize, (int)ySize, 1, GDT_Float32, papszOptions);
		if (pDataset) {
			if (strlen(szProjectionRef) > 0) {
				pDataset->SetProjection(szProjectionRef);
			}

			if (geotransform[0] != 0.0 && geotransform[1] != 0.0) {
				pDataset->SetGeoTransform(geotransform);
			}
			errCode = SaveBuffer(pDataset);
			GDALClose((GDALDatasetH)pDataset);
		}
		//CSLDestroy(papszOptions);
		GDALDeregisterDriver((GDALDriverH)pDriver);
	}
	return errCode == CE_None ? true : false;
}

PROTECTED unsigned int FloatBuffer::NeighborCount(size_t x, size_t y, bool use8) {
	unsigned int count = 0;

	if (InGrid(x, y)) {
		int use = use8 ? 8 : 4;
		size_t tx, ty;
		for (int direction = 0; direction < use; direction++) {
			if (GetNeighbor(direction, x, y, tx, ty)) {
				size_t eIndex = ty * xSize + tx;
				float e = buffer[eIndex];
				if (e != dNoData) {
					count++;
				}
			}
		}
	}
	return count;
}

PROTECTED bool FloatBuffer::IsSet(size_t x, size_t y) {
	bool isSet = false;
	float byte = static_cast<float>(dNoData);
	if (x < xSize && y < ySize) {
		size_t eIndex = y * xSize + x;
		byte = buffer[eIndex];
		if (byte != dNoData) {
			isSet = true;
		}
	}
	return isSet;
}
PROTECTED bool FloatBuffer::IsNoData(size_t x, size_t y) {
	bool isNoData = false;
	float byte = static_cast<float>(dNoData);
	if (x < xSize && y < ySize) {
		size_t eIndex = y * xSize + x;
		byte = buffer[eIndex];
		if (byte == dNoData) {
			isNoData = true;
		}
	}
	return isNoData;
}

MULTIMAP_API size_t FloatBuffer::CountNoData(void) {
	size_t noData = 0;
	for (size_t ny = 0; ny < ySize; ny++) {
		for (size_t nx = 0; nx < xSize; nx++) {
			size_t index = ny*xSize + nx;
			if (buffer[index] == dNoData) {
				noData++;
			}
		}
	}
	return noData;
}
MULTIMAP_API double FloatBuffer::NoDataRatio(void) {
	size_t noData = CountNoData();
	size_t total = ySize * xSize;
	return static_cast<double>(noData) / static_cast<double>(total);
}
MULTIMAP_API void FloatBuffer::Laplace(void) {

	MatDoub_IO *matrix = new MatDoub_IO(ySize, xSize, 1.e99);
	if (matrix) {
		for (size_t ny = 0; ny < ySize; ny++) {
			for (size_t nx = 0; nx < xSize; nx++) {
				size_t index = ny*xSize + nx;
				if (buffer[index] == dNoData) {
					(*matrix)[(int)ny][(int)nx] = 1.e99;
				}
				else {
					(*matrix)[(int)ny][(int)nx] = static_cast<Doub>(buffer[index]);
				}
			}
		}

		if (debug)
			logger->Log(INFO, "Start Laplace interpolation");

		NRC_Laplace laplace((*matrix));
		laplace.solve();

		if (debug)
			logger->Log(INFO, "Complete Laplace interpolation");

		for (size_t ny = 0; ny < ySize; ny++) {
			for (size_t nx = 0; nx < xSize; nx++) {
				size_t index = ny*xSize + nx;
				if ((*matrix)[(int)ny][(int)nx] != 1.e99) {
					if (buffer[index] == dNoData) {
						buffer[index] = static_cast<float>((*matrix)[(int)ny][(int)nx]);
					}
				}
			}
		}
		delete matrix;
	}
}
/**
 * Computes average and optionally standard deviation of neighboring cells (excludes cell itself)
 **/
MULTIMAP_API float FloatBuffer::NeighborAverage(size_t x, size_t y, FloatNeighborStats & stats, int pixelDistance) {
	float dfNoData = static_cast<float>(dNoData);
	unsigned int minc = UINT_MAX;
	unsigned int maxc = 0;

	int minX = static_cast<int>(x - pixelDistance);
	int minY = static_cast<int>(y - pixelDistance);
	int maxX = static_cast<int>(x + pixelDistance);
	int maxY = static_cast<int>(y + pixelDistance);

	// scale up because pixelDistance will usually be small and we want to avoid rounding issues.
	GenericPoint<size_t> xy(x * 100, y * 100);
	pixelDistance *= 100;

	double sum = 0.0;
	double rimSum = 0.0;
	std::vector<float> values;
	std::vector<float> rimValues;
	for (int ny = minY; ny <= maxY; ny++) {
		for (int nx = minX; nx <= maxX; nx++) {
			if (InGrid(nx, ny) && nx != x && ny != y) {
				GenericPoint<size_t> nxy(nx * 100, ny * 100);
				int testDistance = static_cast<int>(xy.Distance2d(nxy));
				if (testDistance <= pixelDistance) {
					size_t eIndex = ny * xSize + nx;
					float e = buffer[eIndex];
					if (e != dfNoData) {
						stats.count++;
						sum += static_cast<double>(e);
						values.push_back(e);
						if (abs(testDistance - pixelDistance) < 100) {
							stats.rimCount++;
							rimSum += static_cast<double>(e);
						}
					}
				}
			}
		}
	}

	stats.average = static_cast<float>(sum / static_cast<double>(stats.count));
	double sumDiffSquared = 0.f;
	for (size_t i = 0; i < values.size(); i++) {
		double diff = values[i] - static_cast<double>(stats.average);
		sumDiffSquared += diff*diff;
	}
	stats.stddev = static_cast<float>(sqrt(sumDiffSquared / static_cast<double>(stats.count)));

	stats.rimAverage = static_cast<float>(rimSum / static_cast<double>(stats.rimCount));
	sumDiffSquared = 0.f;
	for (size_t i = 0; i < rimValues.size(); i++) {
		double diff = rimValues[i] - static_cast<double>(stats.rimAverage);
		sumDiffSquared += diff*diff;
	}
	stats.rimStddev = static_cast<float>(sqrt(sumDiffSquared / static_cast<double>(stats.rimCount)));

	values.clear();
	values.resize(0);
	rimValues.clear();
	rimValues.resize(0);

	return stats.average;
}
MULTIMAP_API void FloatBuffer::setNeighbors(size_t x, size_t y, float value, int pixelDistance) {

	int minX = static_cast<int>(x - pixelDistance);
	int minY = static_cast<int>(y - pixelDistance);
	int maxX = static_cast<int>(x + pixelDistance);
	int maxY = static_cast<int>(y + pixelDistance);

	// scale up because pixelDistance will usually be small and we want to avoid rounding issues.
	GenericPoint<size_t> xy(x * 100, y * 100);
	pixelDistance *= 100;

	for (int ny = minY; ny <= maxY; ny++) {
		for (int nx = minX; nx <= maxX; nx++) {
			if (InGrid(nx, ny)) {
				GenericPoint<size_t> nxy(nx * 100, ny * 100);
				int testDistance = static_cast<int>(xy.Distance2d(nxy));
				if (testDistance <= pixelDistance) {
					size_t eIndex = ny * xSize + nx;
					buffer[eIndex] = value;
				}
			}
		}
	}
}
/**
 * Finds single cell pits (cells with values <maxDepth> below the surrounding average)
 * and replaces the value with the average.
 **/
MULTIMAP_API size_t FloatBuffer::FillPits(float maxDepth, int pixelDistance) {
	size_t fillCount = 0;

	size_t minX = static_cast<size_t>(pixelDistance);
	size_t minY = static_cast<size_t>(pixelDistance);
	size_t maxX = static_cast<size_t>(xSize - pixelDistance);
	size_t maxY = static_cast<size_t>(ySize - pixelDistance);


	for (size_t ny = minY; ny < maxY; ny++) {
		for (size_t nx = minY; nx < maxX; nx++) {
			size_t eIndex = ny*xSize + nx;
			float elevation = buffer[eIndex];
			if (elevation != dNoData) {
				FloatNeighborStats stats;
				float average = NeighborAverage(nx, ny, stats, pixelDistance);
				float depth = average - elevation;
				if (depth >= maxDepth) {
					float rough = depth / 2.0f;
					float rimRough = depth / 10.0f;
					if (stats.stddev < rough) {
						if (stats.rimStddev < rimRough) {
							//logger->Log(INFO, "Correcting %5u:%5u rimStddev=%.3f", nx, ny, stats.rimStddev);
							setNeighbors(nx, ny, average, pixelDistance - 1);
						}
						else {
							logger->Log(INFO, "Correcting %5u:%5u from %.3f to %.3f: depth=%.3f rough=%.3f STD=%.3f",
								nx, ny, buffer[eIndex], average, depth, rough, stats.stddev);
							buffer[eIndex] = average;
						}
					}
				}
			}
		}
	}

	return fillCount;
}
/**
* Finds single cell pits (cells with values <maxDepth> below the surrounding average)
* and replaces the value with NODATA
**/
MULTIMAP_API size_t FloatBuffer::NoDataPits(float maxDepth, int pixelDistance) {
	size_t fillCount = 0;

	size_t minX = static_cast<size_t>(pixelDistance);
	size_t minY = static_cast<size_t>(pixelDistance);
	size_t maxX = static_cast<size_t>(xSize - pixelDistance);
	size_t maxY = static_cast<size_t>(ySize - pixelDistance);

	float dfNoData = static_cast<float>(dNoData);

	for (size_t ny = minY; ny < maxY; ny++) {
		for (size_t nx = minY; nx < maxX; nx++) {
			size_t eIndex = ny*xSize + nx;
			float elevation = buffer[eIndex];
			if (elevation != dNoData) {
				FloatNeighborStats stats;
				float average = NeighborAverage(nx, ny, stats, pixelDistance);
				float depth = average - elevation;
				if (depth >= maxDepth) {
					float rough = depth / 2.0f;
					float rimRough = depth / 10.0f;
					if (stats.stddev < rough) {
						if (stats.rimStddev < rimRough) {
							//logger->Log(INFO, "Correcting %5u:%5u rimStddev=%.3f", nx, ny, stats.rimStddev);
							setNeighbors(nx, ny, dfNoData, pixelDistance - 1);
						}
						else {
							//logger->Log(INFO, "Correcting %5u:%5u from %.3f to %.3f: depth=%.3f rough=%.3f STD=%.3f",
							//	nx, ny, buffer[eIndex], average, depth, rough, stats.stddev);
							buffer[eIndex] = dfNoData;
						}
					}
				}
			}
		}
	}

	return fillCount;
}
MULTIMAP_API size_t FloatBuffer::SmoothInvDist(float maxDepth, int pixelDistance) {
	size_t smoothCount = 0;

	return smoothCount;
}