/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "UserCommandContext.h"
#include "mock/MockCommand.h"
#include "mock/MockUserPacketHandler.h"
using std::string;

namespace {
	class TestableUserCommandContext : public UserCommandContext
	{
	public:
		TestableUserCommandContext(IUserPacketHandler& handler)
			: UserCommandContext(handler)
		{}

		using UserCommandContext::onBegin;
		using UserCommandContext::onBody;
		using UserCommandContext::onComplete;
		using UserCommandContext::onUrl;

		using UserCommandContext::_status;
		using UserCommandContext::_command;
		using UserCommandContext::_params;
		using UserCommandContext::_url;
	};
}

TEST_CASE( "UserCommandContextTest/testFeed", "[unit]" )
{
	MockUserPacketHandler handler;
	UserCommandContext context(handler);

	handler._command = new MockCommand;
	string buff = "GET /list-keys?deleted=1&all=1 HTTP/1.1\r\n\r\n";
	assertTrue( context.feed(buff.data(), buff.size()) );

	assertEquals( "command(list-keys,all=1 deleted=1)|sendResponse(200)", handler._history.calls() );
}

TEST_CASE( "UserCommandContextTest/testFeed.Minimal", "[unit]" )
{
	MockUserPacketHandler handler;
	UserCommandContext context(handler);

	handler._command = new MockCommand;
	string buff = "GET /status HTTP/1.1\r\n\r\n";
	assertTrue( context.feed(buff.data(), buff.size()) );

	assertEquals( "command(status,)|sendResponse(200)", handler._history.calls() );
}

TEST_CASE( "UserCommandContextTest/testOnUrl", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	string url = "/hello";
	assertEquals( 0, context.onUrl(url.data(), url.size()) );
	assertEquals( "/hello", context._url );

	url = "/world";
	assertEquals( 0, context.onUrl(url.data(), url.size()) );
	assertEquals( "/hello/world", context._url );

	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserCommandContextTest/testOnUrl.Fails", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);
	context._command.reset(new MockCommand);

	string url = "/hello";
	assertEquals( 1, context.onUrl(url.data(), url.size()) );
	assertEquals( "", context._url );

	url = "/world";
	assertEquals( 1, context.onUrl(url.data(), url.size()) );
	assertEquals( "", context._url );
}

TEST_CASE( "UserCommandContextTest/testOnUrl.TooBig", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	string url;
	url.resize(8191, 'a');
	assertEquals( 0, context.onUrl(url.data(), url.size()) );
	assertEquals( 8191, context._url.size() );

	assertEquals( 0, context.status().integer() );

	// now, for the kaboom
	url = "bb";
	assertEquals( 1, context.onUrl(url.data(), url.size()) );
	assertEquals( 8191, context._url.size() );

	assertEquals( 414, context.status().integer() );
}

TEST_CASE( "UserCommandContextTest/testOnBegin", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	handler._command = new MockCommand;
	context._url = "/foourl";

	HttpParser::Status status(NULL);
	assertEquals( 0, context.onBegin(status) );
	assertEquals( 0, context._status.integer() );
	assertEquals( "command(foourl,)", handler._history.calls() );
	assertNotNull( context._command.get() );
}

TEST_CASE( "UserCommandContextTest/testOnBegin.BadUrl", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	handler._command = new MockCommand;
	context._url = "";

	HttpParser::Status status(NULL);
	assertEquals( 0, context.onBegin(status) );
	assertEquals( 400, context._status.integer() );
	assertEquals( "", handler._history.calls() );
	assertNull( context._command.get() );
}

TEST_CASE( "UserCommandContextTest/testOnBody", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	context._command.reset(new MockCommand);
	string body = "foobar";

	assertEquals( 0, context.onBody(body.data(), body.size()) );
	assertEquals( 0, context._status.integer() );
	assertEquals( "run(foobar)", ((MockCommand*)context._command.get())->_history.calls() );
	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserCommandContextTest/testOnBody.Empty", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	context._command.reset(new MockCommand);

	assertEquals( 0, context.onBody(NULL, 0) );
	assertEquals( 0, context._status.integer() );
	assertEquals( "run()", ((MockCommand*)context._command.get())->_history.calls() );
	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserCommandContextTest/testOnBody.Kaboom", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	context._command.reset(new MockCommand);
	context._command->setStatus(504);

	assertEquals( 0, context.onBody(NULL, 0) );
	assertEquals( 504, context._status.integer() );
	assertEquals( "run()", ((MockCommand*)context._command.get())->_history.calls() );
	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserCommandContextTest/testOnBody.NoCommand", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	string body = "foobar";

	assertEquals( 0, context.onBody(body.data(), body.size()) );
	assertEquals( 0, context._status.integer() );
	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserCommandContextTest/testOnComplete", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	context._url = "foobar";
	context._command.reset(new MockCommand);

	assertEquals( 0, context.onComplete() );

	assertEquals( "sendResponse(200)", handler._history.calls() );
	assertEquals( "", context._url );
	assertNull( context._command.get() );
}

TEST_CASE( "UserCommandContextTest/testOnComplete.WithExistingStatus", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	context._url = "foobar";
	context._status = 123;
	context._command.reset(new MockCommand);

	assertEquals( 0, context.onComplete() );

	assertEquals( "sendResponse(123)", handler._history.calls() );
	assertEquals( "", context._url );
	assertNull( context._command.get() );
}

TEST_CASE( "UserCommandContextTest/testOnComplete.BadCommand", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserCommandContext context(handler);

	context._url = "foobar";
	context._status = 0;

	assertEquals( 0, context.onComplete() );

	assertEquals( "sendResponse(406)", handler._history.calls() );
	assertEquals( "", context._url );
}

