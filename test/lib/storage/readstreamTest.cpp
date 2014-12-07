/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "readstream.h"

#include "FileReader.h"
#include "common/KeyMetadata.h"

TEST_CASE( "readstreamTest/testMetadata", "[unit]" )
{
	KeyMetadata md;
	md.version.increment("foo");
	md.totalCopies = 2;

	readstream reader(new FileReader("myfile"), md);
	//assertEquals( 0, reader.size() );
	//assertEquals( 1234, reader.digest() );
	assertEquals( md.version.toString(), reader.version() );
	assertEquals( 2, reader.mirrors() );
	assertFalse( reader.good() );
}
