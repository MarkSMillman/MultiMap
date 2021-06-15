#include "MultiMap.h"
#include "ByteBuffer.h"
#include "Parameters.h"
#include "MBRect.h"
#include "Logger.h"

#pragma warning ( disable : 4251 ) // disable std::vector<_Ty> needs dll-interface warning from within gdal_priv.h
#include "gdal_priv.h"

#undef min
#undef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

MULTIMAP_API ByteBuffer::~ByteBuffer(void) {
	if (buffer && !foreignBuffer) {
		delete[] buffer;
	}
	if (neighbors) {
		delete[] neighbors;
	}
}

MULTIMAP_API ByteBuffer::ByteBuffer(void) {

	logger = Logger::Instance();
	logger->logToFile = false;
	logger->logToScreen = true;
	logger->SetLevel(INFO);

	xSize = 0;
	ySize = 0;
	foreignBuffer = false;
	buffer = NULL;
	dataType = GDT_Byte;
	neighbors = NULL;
	minPasses = 50;
	invert = false;

	memset(geotransform, 0, sizeof(geotransform));
	memset(szProjectionRef, 0, sizeof(szProjectionRef));
	memset(szDriverDescr, 0, sizeof(szDriverDescr));
	resolution = 0.0;
	dNoData = static_cast<double>(-32767);
	cNoData = 0xFF;
	errCode = CE_None;
}

MULTIMAP_API ByteBuffer::ByteBuffer(unsigned char* _buffer, unsigned int _xSize, unsigned int _ySize, double _resolution) {
	logger = Logger::Instance();
	logger->logToFile = false;
	logger->logToScreen = true;
	logger->SetLevel(INFO);

	xSize = _xSize;
	ySize = _ySize;
	buffer = _buffer;
	dataType = GDT_Byte;
	foreignBuffer = true;
	neighbors = NULL;
	invert = false;

	memset(geotransform, 0, sizeof(geotransform));
	memset(szProjectionRef, 0, sizeof(szProjectionRef));
	memset(szDriverDescr, 0, sizeof(szDriverDescr));
	resolution = _resolution;
	dNoData = static_cast<double>(-32767);
	errCode = CE_None;
}

MULTIMAP_API unsigned char* ByteBuffer::getBuffer() {
	return buffer;
}
MULTIMAP_API int ByteBuffer::ReadBuffer(GDALDataset* pDataset, MBRect *cropWindow) {
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

		if (gDataType == GDT_Byte) {
			dataType = (int)gDataType;
			buffer = new unsigned char[ySize * xSize];

			for (unsigned int y = 0; y < ySize; y++) {
				unsigned int offset = y*xSize;
				unsigned char* scanLine = &buffer[offset];
				errCode = pBand->RasterIO(GF_Read, 0, y, xSize, 1, scanLine, xSize, 1, GDT_Byte, 0, 0);
				if (errCode != CE_None) {
					break;
				}
			}
		}
	}
	if (invert) {
		InvertBuffer(binary);
	}
	return errCode;
}
MULTIMAP_API bool ByteBuffer::ReadBuffer(const char* inputPath, MBRect *cropWindow) {
	errCode = CE_Failure;
	if (buffer) {
		delete[] buffer;
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
MULTIMAP_API int ByteBuffer::SaveBuffer(GDALDataset* pDataset, MBRect *cropWindow) {
	errCode = CE_Failure;
	if (pDataset) {
		//pDataset->SetGeoTransform(geotransform);
		//if ( strlen(szProjectionRef) > 0 ) {
		//	pDataset->SetProjection(szProjectionRef);
		//}
		GDALRasterBand*	pBand = pDataset->GetRasterBand(1);
		for (unsigned int y = 0; y < ySize; y++) {
			unsigned int offset = y*xSize;
			unsigned char* scanLine = &buffer[offset];
			errCode = pBand->RasterIO(GF_Write, 0, y, xSize, 1, scanLine, xSize, 1, GDT_Byte, 0, 0);
			if (errCode != CE_None) {
				break;
			}
		}
	}
	return errCode;
}
MULTIMAP_API bool ByteBuffer::SaveBuffer(const char* outputPath, MBRect *cropWindow) {
	errCode = CE_Failure;
	GDALRegister_GTiff();
	GDALDriver* pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (pDriver) {
		char **papszOptions = NULL;
		papszOptions = CSLSetNameValue(papszOptions, "BLOCKXSIZE", "512");
		papszOptions = CSLSetNameValue(papszOptions, "BLOCKYSIZE", "512");
		papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
		if (compress) {
			papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
		}
		GDALDataset* pDataset = pDriver->Create(outputPath, xSize, ySize, 1, GDT_Byte, papszOptions);
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
		CSLDestroy(papszOptions);
		GDALDeregisterDriver((GDALDriverH)pDriver);
	}
	return errCode == CE_None ? true : false;
}
MULTIMAP_API void ByteBuffer::InvertBuffer(bool binary) {
	int bufferSize = xSize*ySize;
	for (int i = 0; i < bufferSize; i++) {
		if (binary) {
			if (buffer[i]) {
				buffer[i] = 0x00;
			}
			else {
				buffer[i] = 0xFF;
			}
		}
		else {
			buffer[i] = 0xFF - buffer[i];
		}
	}
}
MULTIMAP_API void ByteBuffer::BinaryBuffer(unsigned char minValue) {
	int bufferSize = xSize*ySize;
	if (minValue > 0x01) {
		for (int i = 0; i < bufferSize; i++) {
			if (buffer[i] >= minValue) {
				buffer[i] = 0xFF;
			}
			else {
				buffer[i] = 0x00;
			}
		}
	}
	else {
		// we may do this millions and millions of times so
		// the default case should be as fast as possible
		for (int i = 0; i < bufferSize; i++) {
			if (buffer[i]) {
				buffer[i] = 0xFF;
			}
		}
	}
}
MULTIMAP_API void ByteBuffer::setInvert(bool invert) {
	this->invert = invert;
}
MULTIMAP_API bool ByteBuffer::getInvert() {
	return invert;
}
MULTIMAP_API void ByteBuffer::setBinary(bool binary) {
	this->binary = binary;
}
MULTIMAP_API bool ByteBuffer::getBinary() {
	return binary;
}

/**
* This method can be called to build a map of the original neighbor count.
* @see DecrementNeighbors
**/
MULTIMAP_API void ByteBuffer::CreateNeighborMap() {
	if (neighbors) {
		delete[] neighbors;
	}
	neighbors = new unsigned char[ySize*xSize];
	for (unsigned int y = 0; y < ySize; y++) {
		for (unsigned int x = 0; x < xSize; x++) {
			int offset = y*xSize + x;
			neighbors[offset] = NeighborCount(x, y);
		}
	}
	return;
}
/**
* UNTESTED
* Updates the neighbors array
**/
MULTIMAP_API void ByteBuffer::DecrementNeighbors(size_t x, size_t y) {
	size_t tx, ty;
	for (int direction = 0; direction < 8; direction++) {
		if (GetNeighbor(direction, x, y, tx, ty)) {
			size_t eIndex = ty * xSize + tx;
			if (neighbors[eIndex] > 0)
				neighbors[eIndex]--;
		}
	}
}
MULTIMAP_API unsigned int ByteBuffer::NeighborCountMatch(size_t x, size_t y, bool use8) {
	unsigned int count = 0;
	if (InGrid(x, y)) {
		int use = use8 ? 8 : 4;
		size_t eIndex = y * xSize + x;
		unsigned char test = buffer[eIndex];
		size_t tx, ty;
		for (int direction = 0; direction < use; direction++) {
			if (GetNeighbor(direction, x, y, tx, ty)) {
				size_t eIndex = ty * xSize + tx;
				unsigned char e = buffer[eIndex];
				if (e == test) {
					count++;
				}
			}
		}
	}
	return count;
}
MULTIMAP_API unsigned int ByteBuffer::NeighborCountDiffer(size_t x, size_t y, bool use8) {
	unsigned int count = 0;
	if (InGrid(x, y)) {
		int use = use8 ? 8 : 4;
		size_t eIndex = y * xSize + x;
		unsigned char test = buffer[eIndex];
		size_t tx, ty;
		for (int direction = 0; direction < use; direction++) {
			if (GetNeighbor(direction, x, y, tx, ty)) {
				size_t eIndex = ty * xSize + tx;
				unsigned char e = buffer[eIndex];
				if (e != test) {
					count++;
				}
			}
		}
	}
	return count;;
}
MULTIMAP_API unsigned int ByteBuffer::NeighborCount(size_t x, size_t y, bool use8) {
	unsigned int count = 0;

	if (InGrid(x, y)) {
		int use = use8 ? 8 : 4;
		size_t tx, ty;
		for (int direction = 0; direction < use; direction++) {
			if (GetNeighbor(direction, x, y, tx, ty)) {
				size_t eIndex = ty * xSize + tx;
				unsigned char e = buffer[eIndex];
				if (invert && e) {
					count++;
				}
				else if (!invert && !e) {
					count++;
				}
			}
		}
	}
	return count;
}
MULTIMAP_API bool ByteBuffer::IsLonely(size_t x, size_t y, size_t pixelDistance) {
	bool lonely = true;
	unsigned int count = -1;
	unsigned int minc = UINT_MAX;
	unsigned int maxc = 0;

	int minX = static_cast<int>(x - pixelDistance);
	int minY = static_cast<int>(y - pixelDistance);
	int maxX = static_cast<int>(x + pixelDistance);
	int maxY = static_cast<int>(y + pixelDistance);

	GenericPoint<size_t> xy(x * 100, y * 100);
	pixelDistance *= 100;

	for (int ny = minY; ny <= maxY; ny++) {
		for (int nx = minX; nx <= maxX; nx++) {
			if (InGrid(nx, ny) && nx != x && ny != y) {
				GenericPoint<size_t> nxy(nx * 100, ny * 100);
				size_t testDistance = xy.Distance2d(nxy);
				if (testDistance <= pixelDistance) {
					size_t eIndex = ny * xSize + nx;
					unsigned char e = buffer[eIndex];
					if (e && e != cNoData) {
						lonely = false;
						break;
					}
				}
			}
		}
	}

	return lonely;
}
/**
* This method returns the number of neighbors a cell has and the minimum and maximum number of neighbors any of they have
* The method is used by Filler
* @param x the x ordinate of the cell to check
* @param y the y ordinate of the cell to check
* @param minCount minimum neighbors any neighbor has
* @param maxCount maximum neighbors any neighbor has
* @return number of neighbors found, -1 if the neighbors array has not been created, 0 if {x,y} not in grid
**/
MULTIMAP_API unsigned int ByteBuffer::NeighborhoodCount(size_t x, size_t y, size_t & minCount, size_t & maxCount) {
	unsigned int count = -1;
	unsigned int minc = UINT_MAX;
	unsigned int maxc = 0;
	minCount = 0;
	maxCount = 0;
	if (neighbors) {
		if (InGrid(x, y)) {
			size_t tx, ty;
			for (int direction = 0; direction < 8; direction++) {
				if (GetNeighbor(direction, x, y, tx, ty)) {
					if (InGrid(tx, ty)) {
						count++;
						unsigned int nc = NeighborCount(tx, ty);
						minc = min(minc, nc);
						maxc = max(maxc, nc);
					}
				}
			}
			if (minc <= 8) {
				minCount = minc;
			}
			if (maxc > 0) {
				maxCount = maxc;
			}
		}
		else {
			count = 0;
		}
	}
	return count;
}

MULTIMAP_API bool ByteBuffer::IsSet(size_t x, size_t y) {
	bool isSet = false;
	char byte = 0;
	if (x < xSize && y < ySize) {
		size_t eIndex = y * xSize + x;
		byte = buffer[eIndex];
		if (byte && byte != cNoData) {
			isSet = true;
		}
	}
	return isSet;
}

MULTIMAP_API bool ByteBuffer::IsNoData(size_t x, size_t y) {
	bool isNoData = false;
	char byte = 0;
	if (x < xSize && y < ySize) {
		size_t eIndex = y * xSize + x;
		byte = buffer[eIndex];
		if (byte == cNoData) {
			isNoData = true;
		}
	}
	return isNoData;
}

MULTIMAP_API bool ByteBuffer::Erode(unsigned int factor) {
	bool status = true;

	if (verbose) {
		logger->Log(INFO, "Erode factor=%u", factor);
	}

	unsigned int totalErosion = 0;
	for (unsigned int f = factor; f >= factor; f--) {
		unsigned int eroded = 0;
		unsigned int passes = 0;
		unsigned int thisFactor = 8 - f;
		do {
			eroded = 0;
			passes++;
			for (unsigned int y = 0; y < ySize; y++) {
				for (unsigned int x = 0; x < xSize; x++) {
					int offset = y*xSize + x;
					if (!buffer[offset]) {
						unsigned int nc = NeighborCountMatch(x, y);
						if (nc <= thisFactor) {
							buffer[offset] = 0xFF;
							eroded++;
						}
					}
				}
			}
			if (debug) {
				logger->Log(INFO, "Factor=%u Pass=%u Eroded=%u", f, passes, eroded);
			}
			if (passes > maxPasses) {
				break;
			}
			totalErosion += eroded;
		} while (f != 8 && eroded > minPasses);

		if (!debug && verbose)
			printf("\n");

		if (debug || verbose) {
			logger->Log(INFO, "Erosion factor %u required %u passes, eroding %u pixels", f, passes, totalErosion);
		}
	}
	return status;
}


/**
* What we likely need and do not yet have is a filler that will fill holes that are narrower than a given number of pixels.
* To do this we need to determine that we are in fact inside a hole, i.e. a clear space completely surrounded by black with
* route to the edge.  And then within this hole we can fill any pixel that is within 'n' pixels of the hole-edge on at least 2 sides.
* Or something like that.  Perhaps we could create a secondary grid that is shrunk by the allowed pixel factor with a fill any
* rule on shrinkage which we could then use to repaint the area or interest.  The trouble with this is that it would have the affect
* of corroding edges.  mmmm - have to think on this for a bit.
*
* @see http://en.wikipedia.org/wiki/Flood_fill
* @see http://www.princeton.edu/~achaney/tmve/wiki100k/docs/Flood_fill.html
**/

int ByteBuffer::floodCount4(int x, int y, unsigned char newColor, unsigned char oldColor, int & maxFillCount) {

	if (x >= 0 && x < (int)xSize && y >= 0 && y < (int)ySize && getBuffer(x, y) == oldColor && getBuffer(x, y) != newColor) {
		if (--maxFillCount > 0) {
			if (maxFillCount > 0) {
				floodCount4(x + 1, y, newColor, oldColor, maxFillCount);
				if (maxFillCount > 0) {
					floodCount4(x - 1, y, newColor, oldColor, maxFillCount);
					if (maxFillCount > 0) {
						floodCount4(x, y + 1, newColor, oldColor, maxFillCount);
						if (maxFillCount > 0) {
							floodCount4(x, y - 1, newColor, oldColor, maxFillCount);
						}
					}
				}
			}
		}
	}
	return (int)maxFillCount;
}
void ByteBuffer::floodFill4(int x, int y, unsigned char newColor, unsigned char oldColor) {
	if (x >= 0 && x < (int)xSize && y >= 0 && y < (int)ySize && getBuffer(x, y) == oldColor && getBuffer(x, y) != newColor) {
		setBuffer(x, y, newColor); //set color before starting recursion

		floodFill4(x + 1, y, newColor, oldColor);
		floodFill4(x - 1, y, newColor, oldColor);
		floodFill4(x, y + 1, newColor, oldColor);
		floodFill4(x, y - 1, newColor, oldColor);
	}
}
void ByteBuffer::floodFillScanlineStack(int x, int y, unsigned char newColor, unsigned char oldColor, size_t maxFillCount) {
	if (oldColor != newColor) {
		stack.clear();
		this->maxStack = maxFillCount;

		int y1;
		bool spanLeft, spanRight;

		if (!push(x, y)) return;

		while (pop(x, y))
		{
			y1 = y;
			while (y1 >= 0 && getBuffer(x, y1) == oldColor) y1--;
			y1++;
			spanLeft = spanRight = 0;
			while (y1 < (int)ySize && getBuffer(x, y1) == oldColor) {
				setBuffer(x, y1, newColor);

				if (!spanLeft && x > 0 && getBuffer(x - 1, y1) == oldColor) {
					if (!push(x - 1, y1)) return;
					spanLeft = 1;
				}
				else if (spanLeft && x > 0 && getBuffer(x - 1, y1) != oldColor) {
					spanLeft = 0;
				}

				if (!spanRight && x < (int)xSize - 1 && getBuffer(x + 1, y1) == oldColor) {
					if (!push(x + 1, y1)) return;
					spanRight = 1;
				}
				else if (spanRight && x < (int)xSize - 1 && getBuffer(x + 1, y1) != oldColor)	{
					spanRight = 0;
				}
				y1++;
			}
		}
	}
}
MULTIMAP_API bool ByteBuffer::Fill(unsigned int factor, unsigned int minFills) {
	bool status = true;
	changed = 0;

	if (factor <= 0) {
		factor = 8;
	}
	size_t testedPixels = 0;
	size_t totalFills = 0;
	unsigned int passes = 0;
	bool floodFill = true;
	bool use8 = !floodFill;
	if (floodFill) {
		maxPasses = 1;
	}
	do {
		passes++;
		for (unsigned int y = 0; y < ySize; y++) {
			for (unsigned int x = 0; x < xSize; x++) {
				int offset = y*xSize + x;
				if (buffer[offset]) {
					testedPixels++;
					unsigned int nc = NeighborCountDiffer(x, y, use8);
					if (floodFill) {
						if (nc >= 1) {
							int maxFill = (int)factor;
							int f = floodCount4(x, y, 0, buffer[offset], maxFill);
							if (f > 0) {
								floodFill4(x, y, 0x00, buffer[offset]);
								changed += (factor - f);
							}
						}
					}
					else {
						if (nc >= factor) {
							buffer[offset] = 0x0;
							changed++;
						}
					}
				}
			}
		}
		totalFills += changed;
		if (passes >= maxPasses) {
			break;
		}
	} while (changed > minFills);

	if (debug || verbose) {
		if (floodFill)
			logger->Log(INFO, "Filler filled %u pixels in holes %u and smaller", totalFills, factor);
		else
			logger->Log(INFO, "Filler factor %u required %u passes, filled %u pixels", factor, passes, totalFills);
	}
	return status;
}