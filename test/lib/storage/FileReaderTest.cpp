/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "FileReader.h"
#include "common/KeyMetadata.h"

#include "file/File.h"
#include "file/UseTempDirectory.h"
#include "socket/StringByteStream.h"


TEST_CASE( "FileReaderTest/testDefault", "[unit]" )
{
	UseTempDirectory temp;
	File::save("myfile", "012345678");

	FileReader reader("myfile");
	assertTrue( reader.good() );
	assertEquals( 9, reader.size() );

	StringByteStream sink;
	assertEquals( 9, reader.stream(sink) );
	assertEquals( "012345678", sink.writeBuffer() );
}

TEST_CASE( "FileReaderTest/testSeek", "[unit]" )
{
	UseTempDirectory temp;
	File::save("myfile", "012345678");

	FileReader reader("myfile", 5);
	assertTrue( reader.good() );
	assertEquals( 9, reader.size() );

	StringByteStream sink;
	assertEquals( 4, reader.stream(sink) );
	assertEquals( "5678", sink.writeBuffer() );
}
