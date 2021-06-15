#include "MultiMap.h"
#include "MultiMapTest.h"
#include "POTrace.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(POTraceTester)
{
	POTrace poTrace;
	int errCode = poTrace.Test();
	BOOST_CHECK(errCode == 0 );
}