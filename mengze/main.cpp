#include "core/application.h"

std::unique_ptr<mengze::Application> mengze::create_application(int argc, char** argv)
{
	return std::make_unique<mengze::Application>();
}


int main(int argc, char** argv)
{
	auto app = mengze::create_application(argc, argv);
	app->run();
}