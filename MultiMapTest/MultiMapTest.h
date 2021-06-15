#pragma once
#include <string>

class MultiMapTest {
public:
	static std::string MultiMapTest::TestFolder(std::string subfolder = "");
	static void MultiMapTest::Message(const char* format, ...);
	static void MultiMapTest::Printf(const char* format, ...);
	static void MultiMapTest::ReportRunTime(std::string message, double seconds );

	static bool verbose;

private:
	MultiMapTest::~MultiMapTest();
	MultiMapTest::MultiMapTest();
};