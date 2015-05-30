/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "app.h"
#include "api/Options.h"

#include "optionparser/ezOptionParser.hpp"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include <memory>
#include <string>
#include <vector>

#include <signal.h>
using namespace ez;
using namespace turbo;
using std::string;

namespace {
	std::unique_ptr<TurboApp> _app;

	void onShutdown(int sig)
	{
		_app->shutdown();
	}

	std::string examples()
	{
		std::vector<string> ex = {
			"./turbopumpd -l /tmp/turbopump -p 9001 --clone",
			"./turbopumpd -l :1592"
		};
		return str::join(ex, '\n');
	}
}

int main(int argc, const char** argv)
{
	ezOptionParser opt;
	opt.overview = "A high performance distributed key value store. 'high performance' is a relative term.";
	opt.syntax = "./turbopumpd --dothething";
	opt.example = examples();
	opt.footer = "\n\n***version 0.01";

	opt.add("", false, 0, 0, "Display usage instructions.", "-h", "--help");
	opt.add("localhost:1592", false, 1, 0, "local server. File path (domain socket), or tcp bind address.", "-l", "--local-addr");
	opt.add("1593", false, 1, 0, "udp port", "-p", "--port");
	opt.add("", false, 0, 0, "run cluster in clone mode", "-c", "--clone");

	opt.add("2000", false, 1, 0, "sync interval (ms)", "--sync-interval");

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

	socket_address localAddr;
	if ((og = opt.get("--local-addr")) != NULL)
	{
		string turbopath;
		og->getString(turbopath);
		std::vector<string> comps = str::split(turbopath, ':');
		if (comps.size() == 1)
			localAddr = socket_address(comps[0]);
		else if (comps.size() == 2)
			localAddr = socket_address(comps[0], std::stoi(comps[1]));
	}

	Turbopump::Options options;
	if ((og = opt.get("--port")) != NULL)
	{
		unsigned long temp;
		og->getULong(temp);
		options.internal_port = temp;
	}

	if ((og = opt.get("--sync-interval")) != NULL)
	{
		unsigned long temp;
		og->getULong(temp);
		options.sync_interval_ms = temp;
	}

	if (opt.isSet("--clone"))
		options.partition_keys = false;
	if (opt.isSet("--no-write-chaining"))
		options.write_chaining = false;
	if (opt.isSet("--no-active-sync"))
		options.active_sync = false;
	if (opt.isSet("--udp"))
		options.udt = false;

	std::cout << localAddr.toString() << ":" << options.internal_port << std::endl;
	_app.reset( new TurboApp(options, localAddr) );

	::signal(SIGINT, &onShutdown);
	::signal(SIGTERM, &onShutdown);
	::signal(SIGPIPE, SIG_IGN); // may use SO_NOSIGPIPE and/or MSG_NOSIGNAL instead...
	_app->run();
	return 0;
}
