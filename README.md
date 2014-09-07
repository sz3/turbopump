# turbopump

An aspiring low-latency, extensible, distributed key value store written in C++11. Currently in beta, i.e. stuff doesn't necessarily work yet.

### Build dependencies

* linux (for now)
* g++ >= 4.8 (for proper c++11)
* cmake

### Library dependencies

* turbolib (see adjacent library)
* UDT (libudt)
* intel's TBB library (libtbb)
* boost::filesystem -- used in integration tests

### What works

* static and dynamic cluster membership. Online addition of peers. (removal is broken-ish)
* RAM data store
* local storage and retrieval of keys via HTTP over unix domain sockets
* auto-versioned writes
* primitive deletes
* cross-peer sync
	* efficient synchronization of changes to data store based on a hierarchical merkle-tree-like summary data structure
* basic function callbacks on write completion
* inter-box UDP communication for small writes.
	* No reliability, nor congestion control. YMMV.
* inter-box UDT communication.
	* No ability to reuse bound socket for outgoing connection -> :(
	* may cause NAT issues going forward
* partitioning! Mode to distribute keys to only their rightful place, rather than "everywhere".
	* also supports a simpler "clone" mode, where every peer duplicates the entire data store.
* basic load/latency testing
	* concurrent writes
	* large writes
	* cross-peer sync

### What's planned

* encryption of socket layer via libsodium.
* a "transient" mode, where keys are only kept until they have been successfully propagated to the destination.
* API for function callbacks on key execution. Hook for arbitrary code execution via system()?
* dynamic membership -- authenticated (signed) peer removal.
* libnice NAT traversal

### What about...? (TODO)

* deleted key cleanup (age-out of deleted metadata)
* append/mutate of given version
* windows support...

### Q&A

Q. I am a possible wizard who somehow got this built and running. How do I store and retrieve files?

A. The API is still in flux, but you can use HTTP over unix domain sockets. For example:

* echo -e -n 'GET /state HTTP/1.1\r\n\r\n' | nc -U /tmp/turbopump
* echo -e -n 'GET /local_list HTTP/1.1\r\n\r\n' | nc -U /tmp/turbopump
* echo -e -n 'POST /write?name=foo HTTP/1.1\r\ncontent-length:5\r\n\r\n012345' | nc -U /tmp/turbopump
* echo -e -n 'GET /read?name=foo HTTP/1.1\r\n\r\n' | nc -U /tmp/turbopump

