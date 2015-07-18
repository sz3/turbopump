Turbopump - An aspiring low-latency, extensible, distributed key value store

Overview
===============================================================================
Distributed key value stores are tremendously flexible tools for building a 
wide variety of network applications. Mesh networks, file synchronization, 
email, distributed computing, web servers, etc -- anything that requires a 
predictable delegation of data distribution and / or processing seems well 
suited for the eventually consistent, hash-partitioned topology that underlies 
technologies like Amazon's Dynamo, or Riak.

However, specific implementations of distributed key value stores usually 
restrict themselves to one particular use case. This project is an experiment.
Its purpose is to see if a single, small code base can provide high 
performance building blocks for many, seemingly unrelated kinds of distributed 
applications.

Written in C++11. Currently in beta: some stuff isn't implemented, and the rest
probably isn't trustworthy yet.


How to build
===============================================================================
1. System dependencies

 - Linux
 - clang >= 3.4 or gcc >= 4.9 (need std::regex, among other C++11 features)
 - cmake

2. Library dependencies [apt-get install ...]

 - libattr
 - libboostfilesystem
 - libmsgpack
 - libudt

3. git clone https://github.com/sz3/turbolib

 - [turbolib](https://github.com/sz3/turbolib) should be located adjacent to the
   turbopump code in the directory tree. That is:
    * /home/user/code/turbopump
    * /home/user/code/turbolib

   ... obviously, you could also modify CMakeLists.txt to do something else.

4. Build commands

From within the "turbopump" directory, run:
```
cmake .
make
make install
```

To run tests:
```
ctest
```


How to run
===============================================================================
```
./turbopumpd
```

```
./turbopumpd -h
```
...for some help.


Now what?
===============================================================================
The built "turbopumpd" executable has a few toggles for modes of operation:

* UDP or UDT for transit. The default is _UDT_.
  - UDP mode does no flow control, cannot guarantee packet ordering, etc.
      In short, it is vanilla UDP. You can use it to saturate your local 
      network. Or for testing purposes. If your key name + contents that will 
      always be smaller than a single UDP frame (1472 bytes), and you can keep 
      a burst of packets from melting down your switches / routers, this may
      be semi-viable.
  - UDT is slightly slower, but does flow control, packet ordering, etc.
      UDT is the default. It is a user-space (D)ata (T)ransfer protocol
      implemented atop UDP. In turbopump, it enables the transfer of large
      contents in a way that is reliable and non-destructive to the network.

* Partition vs Clone
  - _"Partition" mode._
    The default mode of operation is to partition nodes via consistent hashing.
    That is, each write in the system sets a value `mirrors` for a key, and
    only `mirrors` machines will receive a copy of the data.
  - _"Clone" mode._
    There is a special value of `mirrors` that tells the system to distribute
    a key to all machines: 0. "Clone" mode is a special flag that says to treat
    *all* values of `mirrors` like this. Each member of the turbopump cluster
    will mirror all key / value pairs in this mode.

* File vs RAM vs ...
  - _File._
    The current default is to store values as files on the filesystem.
    However, turbopump does not currently deal well with high latency disk
    drives -- its thread scheduling is naive -- so this is best considered a
    beta feature. Right now, prefered operation is to use RAM, by way of ramfs.
    e.g. on debian-based Linux distros, you might set your data directory to
    `/run/shm/turbopump`. SSD performance is best described as "survivable".
  - _RAM._
    A work in progress.
  - ...
    The storage interface in turbopump is meant to be generic. That is, whether
    *sophia* or *sqlite*, or another local database solution, the desire is
    that turbopump should only need a thin wrapper to use a local database for
    its local storage.
    There are two notable requirements for these wrapper implementations:
      1. We expect to store multiple versions of a key+value simultaneously.
      2. We expect to store some metadata for each key+value.

* Membership
  - Cluster membership is currently initialized through a flat file. The first
    line in the file is the local turbopump's identity. Each successive line
    corresponds to a peer. Membership is symmetric -- for a machine to join the
    cluster, it needs to add a member of the cluster to its membership, *and*
    the cluster member in question needs to add the new machine to its own
    membership.
  - While that is admittedly a bit clunky, the rest will happen auto-magically.
    Members will readily share their member lists with other members, so the new
    recruit will quickly be recognized by the entire group.
 

Okay, great. How do I use it?
===============================================================================
You can use HTTP(1) over TCP or unix domain sockets. By default, turbopump runs
a TCP server on port 1592. It is not exactly a super robust server, so please
don't put it on the public internet (by default it binds to INADDR_LOOPBACK).
Also, the API is not very mature and guaranteed to change. :|

With all that said!
```
> curl localhost:1592/status
> echo "bytes" | curl -d @- localhost:1592/write?name=foo
> curl localhost:1592/read?name=foo
> curl localhost:1592/list-keys
> curl localhost:1592/membership
```

To try out the domain socket functionality (`./turbopump -l /tmp/turbopump`),
you can use netcat:
```
> echo -e -n 'GET /status HTTP/1.1\r\n\r\n' | nc -U /tmp/turbopump
```

TODO (╯°□°）╯
===============================================================================
* reliability...
* get RAM storage operational again.
* HTTP/2
* encryption of socket layer via libsodium.
* signed file writes. Only accept data changes if write is authenticated.
  This is too complicated to describe in a todo. It will be cool though.
* a "transient" mode, where keys are only kept until they have been
  successfully propagated to the destination.
* API for function callbacks on key modification. Hook for arbitrary code
  execution via fork/popen()?
* dynamic membership -- authenticated (signed) peer removal.
* libnice NAT traversal. Maybe. UDT is in the way.
* deleted key cleanup (age-out of deleted metadata)
* directory indexing. Somewhat intertwined with deleted key cleanup, along with
  a concept I call "collections".
* windows support...?


TODONE ᕕ(ᐛ)ᕗ
===============================================================================
* versioned (vector clock'd) writes
* key distribution/partitioning
* cross-peer key sync
* basic function callbacks on write completion
* UDP, UDT internal communication
* simple domain socket or TCP (HTTP) command server
* basic load/latency testing
	* concurrent writes
	* large writes
	* cross-peer sync


These are some of the things distributed key-value stores do.

