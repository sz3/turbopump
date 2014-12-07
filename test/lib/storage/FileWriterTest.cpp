/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "FileWriter.h"

#include "IReader.h"
#include "file/File.h"
#include "file/UseTempDirectory.h"
#include <memory>

TEST_CASE( "FileWriterTest/testWrite", "[unit]" )
{
	UseTempDirectory temp;

	{
		FileWriter writer("myfile");
		assertTrue( writer.good() );

		assertEquals( 10, writer.write("0123456789", 10) );
		assertEquals( 6, writer.write("abcdef", 6) );
	}

	std::string contents;
	assertTrue( File::load("myfile", contents) );
	assertEquals( "0123456789abcdef", contents );
}

TEST_CASE( "FileWriterTest/testCreateReader", "[unit]" )
{
	UseTempDirectory temp;

	FileWriter writer("myfile");
	assertTrue( writer.good() );

	assertEquals( 10, writer.write("0123456789", 10) );
	assertEquals( 6, writer.write("abcdef", 6) );

	assertTrue( writer.flush() );
	std::unique_ptr<IReader> reader( writer.reader() );
	assertTrue( !!reader );
	assertEquals( 16, reader->size() );
}

