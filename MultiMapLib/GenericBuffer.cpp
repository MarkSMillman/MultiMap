#include "GenericBuffer.h"

STATIC const int GenericBuffer::DeltaX[8] = { 0, 0, -1, 1, 1, -1, -1, 1 };
STATIC const int GenericBuffer::DeltaY[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };

GenericBuffer::~GenericBuffer(void) {
}
GenericBuffer::GenericBuffer(void){
	verbose = false;
	debug = false;
	changed = 0;
	maxStack = 50;
}

MULTIMAP_API void GenericBuffer::setProjectionRef(char* _szProjectionRef) {
	strcpy(szProjectionRef, _szProjectionRef);
}
MULTIMAP_API const char* GenericBuffer::getProjectionRef(void) {
	return (const char*)szProjectionRef;
}
MULTIMAP_API void GenericBuffer::setGeotransform(double xMin, double yMin, double resolution) {
	double xMax = xMin + (static_cast<double>(xSize)* resolution);
	double yMax = yMin + (static_cast<double>(ySize)* resolution);
	geotransform[0] = xMin - (resolution / 2.0);
	geotransform[1] = resolution;
	geotransform[2] = 0.0;
	geotransform[3] = yMax + (resolution / 2.0);
	geotransform[4] = 0.0;
	geotransform[5] = -resolution;
}
MULTIMAP_API void GenericBuffer::setGeotransform(double _geotransform[6]) {
	memcpy(geotransform, _geotransform, sizeof(geotransform));
}
MULTIMAP_API const double* GenericBuffer::getGeotransform(void) {
	return (const double*)geotransform;
}
MULTIMAP_API void GenericBuffer::setCompress(bool compress) {
	this->compress = compress;
}
MULTIMAP_API bool GenericBuffer::getCompress() {
	return compress;
}
MULTIMAP_API void GenericBuffer::setMinPasses(unsigned int passes) {
	this->minPasses = passes;
}
MULTIMAP_API unsigned int GenericBuffer::getMinPasses() {
	return minPasses;
}
MULTIMAP_API void GenericBuffer::setMaxPasses(unsigned int passes) {
	this->maxPasses = passes;
}
MULTIMAP_API unsigned int GenericBuffer::getMaxPasses() {
	return maxPasses;
}
MULTIMAP_API int GenericBuffer::getErrorCode() {
	return errCode;
}
MULTIMAP_API void GenericBuffer::setVerbose(bool verbose) {
	this->verbose = verbose;
}
MULTIMAP_API bool GenericBuffer::getVerbose() {
	return verbose;
}
MULTIMAP_API void GenericBuffer::setDebug(bool debug) {
	this->debug = debug;
}
MULTIMAP_API bool GenericBuffer::getDebug() {
	return debug;
}
MULTIMAP_API bool GenericBuffer::InGrid(size_t x, size_t y) {
	bool status = true;
	if (x >= xSize || y >= ySize) {
		status = false;
	}
	return status;
}
bool GenericBuffer::pop(int &x, int &y) {
	if (stack.size() > 0) {
		size_t p = stack.back();
		stack.pop_back();
		x = (int)(p / ySize);
		y = (int)(p % ySize);
		return true;
	}
	else {
		return false;
	}
}

bool GenericBuffer::push(int x, int y) {
	bool roomLeft = false;
	if (stack.size() < maxStack) {
		size_t offset = y*xSize + x;
		stack.push_back(offset);
		roomLeft = true;
	}
	return roomLeft;

}
MULTIMAP_API bool GenericBuffer::GetNeighbor(size_t direction, size_t x, size_t y, size_t & ix, size_t & iy) {
	bool status = InGrid(x, y);
	if (status) {
		direction %= 8;

		int iix = static_cast<int>(x)+DeltaX[direction];
		if (iix < 0) {
			status = false;
		}
		else {
			int iiy = static_cast<int>(y)+DeltaY[direction];
			if (iiy < 0) {
				status = false;
			}
			else {
				ix = iix;
				iy = iiy;
				status = InGrid(ix, iy);
			}
		}
	}
	return status;
}
MULTIMAP_API bool GenericBuffer::OnEdge(size_t x, size_t y) {
	bool status = true;
	if (x <= 0 || x >= xSize || y <= 0 || y >= ySize) {
		status = true;
	}
	else {
		size_t startX = x;
		while (--startX >= 0 && IsNoData(startX, y));
		if (startX <= 0) {
			status = true;
		}
		else {
			startX = x;
			while (++startX < xSize && IsNoData(startX, y));
			if (startX >= xSize) {
				status = true;
			}
			else {
				size_t startY = y;
				while (--startY > 0 && IsNoData(x, startY));
				if (startY <= 0) {
					status = true;
				}
				else {
					while (++startY < ySize && IsNoData(x, startY));
					if (startY >= ySize) {
						status = true;
					}
				}
			}
		}
	}
	return status;
}
