#include "MultiMap.h"
#include "LASProcessor.h"
#include "Parameters.h"
#include "NRC_Laplace.h"
#include "GenericPoint.hpp"
#include "ByteBuffer.h"
#include "FloatBuffer.h"

#define SKIP_CONSTANTS
#include "IOReaderLAS.h"
#include "IOWriterLAS.h"
#include "LidarHeader.h"

DISABLE_WARNINGS
#include "ogr_srs_api.h"
#include "ogr_spatialref.h"
ENABLE_WARNINGS

using namespace FLIDAR;

MULTIMAP_API LASProcessor::~LASProcessor(void) {
	if (m_buffer) {
		delete[] m_buffer;
		m_buffer = NULL;
	}
	if (m_mask) {
		delete[] m_mask;
		m_mask = NULL;
	}
}
MULTIMAP_API LASProcessor::LASProcessor(LASParams* params) {
	m_logger = Logger::Instance();
	m_logger->logToFile = false;
	m_logger->logToScreen = true;
	m_logger->SetLevel(INFO);

	if (params) {
		m_inputFiles = params->inputFiles;
		m_inputFile = params->inputPath;
		m_outputFile = params->outputPath;
		m_resolution = params->resolution;
		m_maxPitDepth = params->maxPitDepth;
		m_smoothingPixels = params->smoothingPixels;
		m_keepOriginalPts = params->keepOriginalPts;
		m_minDistanceFilter = params->minDistanceFilter;
		m_fillClass = params->fillClass;
		m_includeClasses = params->includeClasses;
		m_excludeClasses = params->excludeClasses;
		m_verbose = params->verbose;
		m_debug = params->debug;
	}
	else {
		m_resolution = 1;
		m_smoothingPixels = 4;
		m_keepOriginalPts = true;
		m_maxPitDepth = 0.8f;
		m_minDistanceFilter = -1;
		m_fillClass = 0;
		m_verbose = false;
		m_debug = false;
	}
	m_buffer = NULL;
	m_mask = NULL;
}

MULTIMAP_API std::string LASProcessor::getProjectionWKT(int epsg) {
	std::string projectionWKT;
	if (epsg <= 0) {
		if (m_inputFile.length() > 0) {
			LidarHeader lidarHeader;
			IOReaderLAS::GetHeader(lidarHeader, m_inputFile);
			epsg = lidarHeader.m_epsgCode;
		}
	}
	if (epsg > 0) {
		OGRSpatialReference srs;
		int rtc = srs.importFromEPSG(epsg);
		if (rtc == 0) {
			char* wkt = NULL;
			rtc = srs.exportToWkt(&wkt);
			if (rtc == 0) {
				projectionWKT = std::string(wkt);
			}
			if (wkt) {
				OGRFree(wkt);
			}
		}
	}
	return projectionWKT;
}
PRIVATE size_t LASProcessor::countPoints(IOReaderLAS* reader) {
	size_t usedPoints = 0;
	LidarPoint lidarPoint;
	reader->Rewind();
	while (reader->NextRow(lidarPoint, true)) {
		bool use = false;
		if (m_includeClasses.size() == 0) {
			use = true;
		}
		else if (m_includeClasses.size() > 0){
			for (size_t c = 0; c < m_includeClasses.size(); c++){
				if (lidarPoint.classification == m_includeClasses[c]) {
					use = true;
					break;
				}
			}
		}
		if (m_excludeClasses.size() > 0) {
			for (size_t c = 0; c < m_excludeClasses.size(); c++){
				if (lidarPoint.classification == m_excludeClasses[c]) {
					use = false;
					break;
				}
			}
		}
		if (use && m_range.Valid()) {
			GenericPoint<double> genericPoint(lidarPoint.X, lidarPoint.Y);
			if (!m_range.Covers(genericPoint)) {
				use = false;
			}
		}
		if (use) {
			usedPoints++;
		}
	}
	reader->Rewind();
	return usedPoints;
}
MULTIMAP_API int LASProcessor::fill(void) {
	int status = 0;

	if (m_debug) {
		m_logger->SetLevel(DBG);
	}

	MFUtils mfUtils;
	bool outputLAS = false;
	bool outputTIF = false;

	IOReaderLAS* reader = new IOReaderLAS();
	reader->Open(m_inputFile);
	if (!m_range.Valid()) {
		m_range.minX = reader->m_header.m_range.minX;
		m_range.minY = reader->m_header.m_range.minY;
		m_range.maxX = reader->m_header.m_range.maxX;
		m_range.maxY = reader->m_header.m_range.maxY;
	}

	int epsgCode = reader->m_header.m_epsgCode;

	m_bufferXSize = static_cast<size_t>((m_range.maxX - m_range.minX) / m_resolution) + 1;
	m_bufferYSize = static_cast<size_t>((m_range.maxY - m_range.minY) / m_resolution) + 1;

	size_t bufferSize = m_bufferXSize * m_bufferYSize;
	m_buffer = new float[bufferSize];
	for (size_t b = 0; b < bufferSize; b++){
		m_buffer[b] = NODATA;
	}

	FloatBuffer demBuffer(m_buffer, (unsigned int)m_bufferXSize, (unsigned int)m_bufferYSize, NODATA);
	demBuffer.setVerbose(m_verbose);
	demBuffer.setDebug(m_debug);

	m_mask = new unsigned char[bufferSize];
	memset(m_mask, MASK_UNDEFINED, bufferSize);

	std::string outputPathTIF;
	std::string outputPathLAS;
	std::string outputType = mfUtils.GetExtension(m_outputFile, true);
	if (outputType.length() == 0) {
		outputLAS = true;
		outputTIF = true;
		outputPathLAS = m_outputFile + ".las";
		outputPathTIF = m_outputFile + ".tif";
	}
	else if (mfUtils.Compare(outputType, "las", true)) {
		outputLAS = true;
		outputPathLAS = m_outputFile;
	}
	else if (mfUtils.BeginsWith(outputType, "tif", true)) {
		outputTIF = true;
		outputPathTIF = m_outputFile;
	}

	IOWriterLAS* writer = NULL;
	size_t usedPointCount = 0;
	size_t allocateRows = 0;
	if (outputLAS) {
		usedPointCount = countPoints(reader);
		allocateRows = usedPointCount + bufferSize;
		writer = new IOWriterLAS();
	}
	if (!writer || writer->Create(outputPathLAS, allocateRows, 6)) {
		if (writer) {
			writer->m_header.m_offset[0] = reader->m_header.m_offset[0];
			writer->m_header.m_offset[1] = reader->m_header.m_offset[1];
			writer->m_header.m_offset[2] = reader->m_header.m_offset[2];
		}
		LidarPoint lidarPoint;

		while (reader->NextRow(lidarPoint, true)) {
			bool use = false;
			if (m_includeClasses.size() == 0) {
				use = true;
			}
			else if (m_includeClasses.size() > 0){
				for (size_t c = 0; c < m_includeClasses.size(); c++){
					if (lidarPoint.classification == m_includeClasses[c]) {
						use = true;
						break;
					}
				}
			}
			if (m_excludeClasses.size() > 0) {
				for (size_t c = 0; c < m_excludeClasses.size(); c++){
					if (lidarPoint.classification == m_excludeClasses[c]) {
						use = false;
						break;
					}
				}
			}
			if (use && m_range.Valid()) {
				GenericPoint<double> genericPoint(lidarPoint.X, lidarPoint.Y);
				if (!m_range.Covers(genericPoint)) {
					use = false;
				}
			}
			if (use) {
				double x = lidarPoint.X - m_range.minX;
				double y = lidarPoint.Y - m_range.minY;
				unsigned int rx = static_cast<unsigned int>(x / m_resolution);
				unsigned int ry = static_cast<unsigned int>(y / m_resolution);
				size_t eIndex = ry * m_bufferXSize + rx;
				float Zf = static_cast<float>(lidarPoint.Z);
				m_buffer[eIndex] = std::max(m_buffer[eIndex], Zf);
				m_mask[eIndex] = MASK_ORIGINAL;

				if (writer && m_minDistanceFilter == 0 && m_keepOriginalPts) {
					writer->MapRow(lidarPoint); // other wise we'll do this after computing lonely grid cells
				}
			}
		}

		ByteBuffer mask(m_mask, (unsigned int)m_bufferXSize, (unsigned int)m_bufferYSize, m_resolution);

		if (m_minDistanceFilter > 0) {
			size_t minDistancePixels = static_cast<size_t>(m_minDistanceFilter / m_resolution);
			for (size_t ny = 0; ny < m_bufferYSize; ny++) {
				for (size_t nx = 0; nx < m_bufferXSize; nx++) {
					size_t eIndex = ny*m_bufferXSize + nx;
					if (m_mask[eIndex] == MASK_ORIGINAL) {
						size_t minCount = 0;
						size_t maxCount = 0;
						if (mask.IsLonely(nx, ny, minDistancePixels)) {
							m_mask[eIndex] = MASK_OMIT;
							m_buffer[eIndex] = NODATA;
						}
					}
				}
			}

			// Now reread the input file and copy over all points that are not lonely
			if (writer && m_keepOriginalPts) {
				reader->Rewind();
				while (reader->NextRow(lidarPoint, true)) {
					bool use = false;
					if (m_includeClasses.size() == 0) {
						use = true;
					}
					else if (m_includeClasses.size() > 0){
						for (size_t c = 0; c < m_includeClasses.size(); c++){
							if (lidarPoint.classification == m_includeClasses[c]) {
								use = true;
								break;
							}
						}
					}
					if (m_excludeClasses.size() > 0) {
						for (size_t c = 0; c < m_excludeClasses.size(); c++){
							if (lidarPoint.classification == m_excludeClasses[c]) {
								use = false;
								break;
							}
						}
					}
					if (use && m_range.Valid()) {
						GenericPoint<double> genericPoint(lidarPoint.X, lidarPoint.Y);
						if (!m_range.Covers(genericPoint)) {
							use = false;
						}
					}
					if (use) {
						double x = lidarPoint.X - m_range.minX;
						double y = lidarPoint.Y - m_range.minY;
						unsigned int rx = static_cast<unsigned int>(x / m_resolution);
						unsigned int ry = static_cast<unsigned int>(y / m_resolution);
						size_t eIndex = ry * m_bufferXSize + rx;
						if (m_mask[eIndex] == MASK_ORIGINAL) {
							writer->MapRow(lidarPoint);
						}
					}
				}
			}
		}

		demBuffer.Laplace();
		demBuffer.NoDataPits(m_maxPitDepth, static_cast<int>(m_smoothingPixels));
		demBuffer.Laplace();
		//demBuffer.FillPits(m_maxPitDepth, static_cast<int>(m_smoothingPixels));

		if (writer) {
			for (size_t ny = 0; ny < m_bufferYSize; ny++) {
				for (size_t nx = 0; nx < m_bufferXSize; nx++) {
					size_t index = ny*m_bufferXSize + nx;
					if (m_buffer[index] != NODATA) {
						if (!m_keepOriginalPts || m_mask[index] != MASK_ORIGINAL) {
							double x = (static_cast<double>(nx)*m_resolution) + m_range.minX;
							double y = (static_cast<double>(ny)*m_resolution) + m_range.minY;
							LidarPoint lidarPoint(x, y, static_cast<double>(m_buffer[index]));
							lidarPoint.classification = m_fillClass;
							writer->MapRow(lidarPoint);
						}
					}
				}
			}
			if (writer) {
				writer->Close();
				delete writer;
				writer = NULL;
			}
		}

		if (outputTIF) {
			std::string projWKT = getProjectionWKT(epsgCode);
			if (projWKT.length() > 0) {
				demBuffer.setProjectionRef((char*)projWKT.c_str());
			}
			demBuffer.setGeotransform(m_range.minX,m_range.minY, m_resolution);
			demBuffer.SaveBuffer(outputPathTIF.c_str());
		}
	}
	else {
		m_logger->Log(FATAL, "Failed to create %s with %llu points", m_outputFile.c_str(), allocateRows);
	}
	reader->Close();
	delete reader;
	reader = NULL;

	return status;
}
MULTIMAP_API int LASProcessor::createDSM(void) {
	m_logger->Log(INFO, "%s called", __FUNCTION__);
	return 0;
}

MULTIMAP_API void LASProcessor::setInput(std::string filePath) {
	m_inputFile = filePath;
}
MULTIMAP_API std::string LASProcessor::getInput(void) {
	return m_inputFile;
}
MULTIMAP_API void LASProcessor::setOutput(std::string filePath) {
	m_outputFile = filePath;
}
MULTIMAP_API std::string LASProcessor::getOutput(void) {
	return m_outputFile;
}
MULTIMAP_API void LASProcessor::setResolution(double resolution) {
	m_resolution = resolution;
}
MULTIMAP_API double LASProcessor::getResolution(void) {
	return m_resolution;
}
MULTIMAP_API void LASProcessor::setSmoothingPixels(size_t pixels) {
	m_smoothingPixels = pixels;
}
MULTIMAP_API size_t LASProcessor::getSmoothingPixels(void) {
	return m_smoothingPixels;
}
MULTIMAP_API void LASProcessor::setKeepOriginalPoints(bool keepOriginalPoints) {
	m_keepOriginalPts = keepOriginalPoints;
}
MULTIMAP_API bool LASProcessor::getKeepOriginalPoints(void) {
	return m_keepOriginalPts;
}
MULTIMAP_API void LASProcessor::setFillClass(int fillClass) {
	m_fillClass = fillClass;
}
MULTIMAP_API int LASProcessor::getFillClass(void) {
	return m_fillClass;
}
MULTIMAP_API void LASProcessor::setIncludeClasses(std::vector<int> includeClasses) {
	m_includeClasses = includeClasses;
}
MULTIMAP_API void LASProcessor::setIncludeClass(int includeClass) {
	m_includeClasses.push_back(includeClass);
}
MULTIMAP_API std::vector<int> LASProcessor::getIncludeClasses(void) {
	return m_includeClasses;
}
MULTIMAP_API void LASProcessor::setExcludeClasses(std::vector<int> excludeClasses) {
	m_excludeClasses = excludeClasses;
}
MULTIMAP_API void LASProcessor::setExcludeClass(int excludeClass) {
	m_excludeClasses.push_back(excludeClass);
}
MULTIMAP_API std::vector<int> LASProcessor::getExcludeClasses(void) {
	return m_excludeClasses;
}
MULTIMAP_API void LASProcessor::setRange(MBRect range) {
	m_range = range;
}
MULTIMAP_API void LASProcessor::setRange(double minX, double minY, double maxX, double maxY) {
	m_range.minX = minX;
	m_range.minY = minY;
	m_range.maxX = maxX;
	m_range.maxY = maxY;
}
MULTIMAP_API void LASProcessor::setRange(double minX, double minY, double minZ, double maxX, double maxY, double maxZ) {
	m_range.minX = minX;
	m_range.minY = minY;
	m_range.minZ = minZ;
	m_range.maxX = maxX;
	m_range.maxY = maxY;
	m_range.maxZ = maxZ;
}
MULTIMAP_API MBRect LASProcessor::getRange(void) {
	return m_range;
}
MULTIMAP_API void LASProcessor::setBufferSize(size_t xSize, size_t ySize) {
	m_bufferXSize = xSize;
	m_bufferYSize = ySize;
}
MULTIMAP_API size_t LASProcessor::getBufferXSize(void) {
	return m_bufferXSize;
}
MULTIMAP_API size_t LASProcessor::getBufferYSize(void) {
	return m_bufferYSize;
}
MULTIMAP_API void LASProcessor::setBufferOrigin(GenericPoint<double> origin) {
	m_bufferOrigin = origin;
}
MULTIMAP_API void LASProcessor::setBufferOrigin(double x, double y) {
	m_bufferOrigin.X = x;
	m_bufferOrigin.Y = y;
}
MULTIMAP_API GenericPoint<double> LASProcessor::getBufferOrigin(void) {
	return m_bufferOrigin;
}