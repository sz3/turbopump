/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "UserActionContext.h"
#include "mock/MockCommand.h"
#include "mock/MockUserPacketHandler.h"
using std::string;

namespace {
	class TestableUserActionContext : public UserActionContext
	{
	public:
		TestableUserActionContext(IUserPacketHandler& handler)
			: UserActionContext(handler)
		{}

		using UserActionContext::onBegin;
		using UserActionContext::onBody;
		using UserActionContext::onComplete;
		using UserActionContext::onUrl;

		using UserActionContext::_status;
		using UserActionContext::_command;
		using UserActionContext::_params;
		using UserActionContext::_url;
	};
}

TEST_CASE( "UserActionContextTest/testFeed", "[unit]" )
{
	MockUserPacketHandler handler;
	UserActionContext context(handler);

	handler._command = new MockCommand;
	string buff = "GET /list-keys?deleted=1&all=1 HTTP/1.1\r\n\r\n";
	assertTrue( context.feed(buff.data(), buff.size()) );

	assertEquals( "command(list-keys,all=1 deleted=1)|sendResponse(200)", handler._history.calls() );
}

TEST_CASE( "UserActionContextTest/testFeed.Minimal", "[unit]" )
{
	MockUserPacketHandler handler;
	UserActionContext context(handler);

	handler._command = new MockCommand;
	string buff = "GET /status HTTP/1.1\r\n\r\n";
	assertTrue( context.feed(buff.data(), buff.size()) );

	assertEquals( "command(status,)|sendResponse(200)", handler._history.calls() );
}

TEST_CASE( "UserActionContextTest/testOnUrl", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	string url = "/hello";
	assertEquals( 0, context.onUrl(url.data(), url.size()) );
	assertEquals( "/hello", context._url );

	url = "/world";
	assertEquals( 0, context.onUrl(url.data(), url.size()) );
	assertEquals( "/hello/world", context._url );

	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserActionContextTest/testOnUrl.Fails", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);
	context._command.reset(new MockCommand);

	string url = "/hello";
	assertEquals( 1, context.onUrl(url.data(), url.size()) );
	assertEquals( "", context._url );

	url = "/world";
	assertEquals( 1, context.onUrl(url.data(), url.size()) );
	assertEquals( "", context._url );
}

TEST_CASE( "UserActionContextTest/testOnBegin", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	handler._command = new MockCommand;
	context._url = "/foourl";

	HttpParser::Status status(NULL);
	assertEquals( 0, context.onBegin(status) );
	assertEquals( 0, context._status.integer() );
	assertEquals( "command(foourl,)", handler._history.calls() );
	assertNotNull( context._command.get() );
}

TEST_CASE( "UserActionContextTest/testOnBegin.BadUrl", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	handler._command = new MockCommand;
	context._url = "";

	HttpParser::Status status(NULL);
	assertEquals( 0, context.onBegin(status) );
	assertEquals( 400, context._status.integer() );
	assertEquals( "", handler._history.calls() );
	assertNull( context._command.get() );
}

TEST_CASE( "UserActionContextTest/testOnBody", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	context._command.reset(new MockCommand);
	string body = "foobar";

	assertEquals( 0, context.onBody(body.data(), body.size()) );
	assertEquals( 200, context._status.integer() );
	assertEquals( "run(foobar)", ((MockCommand*)context._command.get())->_history.calls() );
	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserActionContextTest/testOnBody.Empty", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	context._command.reset(new MockCommand);

	assertEquals( 0, context.onBody(NULL, 0) );
	assertEquals( 200, context._status.integer() );
	assertEquals( "run()", ((MockCommand*)context._command.get())->_history.calls() );
	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserActionContextTest/testOnBody.NoAction", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	string body = "foobar";

	assertEquals( 0, context.onBody(body.data(), body.size()) );
	assertEquals( 0, context._status.integer() );
	assertEquals( "", handler._history.calls() );
}

TEST_CASE( "UserActionContextTest/testOnComplete", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	context._url = "foobar";
	context._command.reset(new MockCommand);

	assertEquals( 0, context.onComplete() );

	assertEquals( "sendResponse(200)", handler._history.calls() );
	assertEquals( "", context._url );
	assertNull( context._command.get() );
}

TEST_CASE( "UserActionContextTest/testOnComplete.WithExistingStatus", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	context._url = "foobar";
	context._status = 123;
	context._command.reset(new MockCommand);

	assertEquals( 0, context.onComplete() );

	assertEquals( "sendResponse(123)", handler._history.calls() );
	assertEquals( "", context._url );
	assertNull( context._command.get() );
}

TEST_CASE( "UserActionContextTest/testOnComplete.BadAction", "[unit]" )
{
	MockUserPacketHandler handler;
	TestableUserActionContext context(handler);

	context._url = "foobar";
	context._status = 0;

	assertEquals( 0, context.onComplete() );

	assertEquals( "sendResponse(406)", handler._history.calls() );
	assertEquals( "", context._url );
}

