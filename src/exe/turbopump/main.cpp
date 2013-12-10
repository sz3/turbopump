#include "main/TurboPumpApp.h"
#include "programmable/TurboApi.h"

#include "optionparser/ezOptionParser.hpp"
#include <string>
using namespace ez;
using std::string;

int main(int argc, const char** argv)
{
	ezOptionParser opt;
	opt.overview = "A high performance distributed key value store. 'high performance' is a relative term.";
	opt.syntax = "./turbopump --dothething";
	opt.footer = "turbopump 0.01";

	opt.add("", false, 0, 0, "Display usage instructions.", "-h", "--help");
	opt.add("/tmp/turbopump", false, 1, 0, "domain socket path", "-d", "--dataChannel");
	opt.add("9001", false, 1, 0, "udp port", "-p", "--port");

	opt.parse(argc, argv);

	if (opt.isSet("-h"))
	{
		std::string usage;
		opt.getUsage(usage);
		std::cout << usage << std::endl;
		return 1;
	}
	OptionGroup* og;

	string turbopath;
	if ((og = opt.get("--dataChannel")) != NULL)
		og->getString(turbopath);

	short port;
	if ((og = opt.get("--port")) != NULL)
	{
		unsigned long temp;
		og->getULong(temp);
		port = temp;
	}

	std::cout << turbopath << ":" << port << std::endl;
	TurboApi api;
	TurboPumpApp app(api, turbopath, port);
	app.run();
	return 0;
}
