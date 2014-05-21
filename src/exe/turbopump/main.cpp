/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "main/TurboPumpApp.h"
#include "programmable/TurboApi.h"

#include "optionparser/ezOptionParser.hpp"
#include <memory>
#include <string>
#include <signal.h>
using namespace ez;
using std::string;

namespace {
	std::unique_ptr<TurboPumpApp> _app;

	void onShutdown(int sig)
	{
		_app->shutdown();
	}
}


int main(int argc, const char** argv)
{
	ezOptionParser opt;
	opt.overview = "A high performance distributed key value store. 'high performance' is a relative term.";
	opt.syntax = "./turbopump --dothething";
	opt.footer = "turbopump 0.01";

	opt.add("", false, 0, 0, "Display usage instructions.", "-h", "--help");
	opt.add("/tmp/turbopump", false, 1, 0, "domain socket path", "-d", "--dataChannel");
	opt.add("9001", false, 1, 0, "udp port", "-p", "--port");
	opt.add("", false, 0, 0, "run cluster in clone mode", "-c", "--clone");
	opt.add("", false, 0, 0, "TEST-ONLY: disable data forwarding", "--no-write-chaining");
	opt.add("", false, 0, 0, "TEST-ONLY: disable query-response data sync between peers", "--no-active-sync");
	opt.add("", false, 0, 0, "peer-to-peer communications use UDP instead of congestion-aware, reliable UDT", "--udp");

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
	if (opt.isSet("--clone"))
		api.options.partition_keys = false;
	if (opt.isSet("--no-write-chaining"))
		api.options.write_chaining = false;
	if (opt.isSet("--no-active-sync"))
		api.options.active_sync = false;
	if (opt.isSet("--udp"))
		api.options.udt = false;

	_app.reset( new TurboPumpApp(api, turbopath, port) );

	::signal(SIGINT, &onShutdown);
	::signal(SIGPIPE, SIG_IGN); // may use SO_NOSIGPIPE and/or MSG_NOSIGNAL instead...
	_app->run();
	return 0;
}
