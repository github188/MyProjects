// Uuid.cpp

#include "tools/framework_test/Common.h"

#include <framework/string/Uuid.h>
using namespace framework::string;
using namespace framework::configure;

static void test_uuid(Config & conf)
{
    Uuid guid;
    guid.generate();

    std::cout << guid.to_string() << std::endl;
}

static TestRegister test("uuid", test_uuid);
