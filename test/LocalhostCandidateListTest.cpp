#include "LocalhostCandidateListTest.h"
#include "LocalhostCandidateList.h"

void LocalhostCandidateListTest::testSelf()
{
	LocalhostCandidateList list(1234);
	assert_equal("127.0.0.1:1234", list.getSelf());
}
