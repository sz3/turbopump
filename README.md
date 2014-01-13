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

### What works

* static cluster membership
* RAM data store
* local storage and retrieval of small keys via unix domain sockets
* immutable writes
* cross-worker sync -- for "clone" use case
	* sync/healing based on merkle
	* merkle-like summary data structure
* basic function callbacks on write completion
* inter-box UDP communication for small packets. No reliability, nor congestion control. YMMV.
* inter-box UDT communication. No ability to reuse bound socket for outgoing connection -> :(
	* may cause NAT issues going forward
* basic load/latency testing 
	* concurrent writes
	* large writes

### What's planned

* partitioning! DHT! mode to distribute keys to only their rightful place, rather than "everywhere".
* encryption of socket layer via libsodium
* API for function callbacks on key execution. Hook for arbitrary code execution via system()?

### What about...? (TODO)

* paxos and write versioning.
* key deletes!
* windows support...

### Q&A

Q. I am a possible wizard who somehow got this built and running. How do I store and retrieve files?

A. The current method is to send primitive commands over domain sockets. For example:

* echo 'write|name=foo|haha I am a great value' | nc -U /tmp/turbopump
* echo 'read|name=foo|' | nc -U /tmp/turbopump
* echo 'local_list||' | nc -U /tmp/turbopump

