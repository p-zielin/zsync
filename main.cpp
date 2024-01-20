#include <iostream>
#include <stdexcept>
#include <Poco/Exception.h>

#include "CService.h"

using namespace Poco;

int main(int argc, char* argv[])
{
	try
	{
		CService service;
		return service.run(argc, argv);
	}
	catch (const Poco::Exception& ex)
    {
        std::clog << ex.displayText() << std::endl << ex.className();
	}
	catch (const std::exception& ex)
	{
		std::clog << ex.what() << std::endl;
	}

	return 1;
}
