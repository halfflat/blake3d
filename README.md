# Distributed BLAKE3

Nothing material yet!

Goals:

* A library that implements BLAKE3 that can operate thread-parallel and/or distributed via MPI.
* The library should still be speedy in a single process, single-threaded context; explicitly, should be no worse than four times slower than the optimized reference implementation.
* The library abstracts over source data acquisition, so that it might be used (for example) to hash one or more objects accessible via S3 over multiple nodes.
* A command-line utility that at minimum supports hashing a file which is read via mmap.

Later:

* Library and utilty support for streaming input (single node only, and maybe only single-threaded).
* A utility specifically for hashing objects in an S3-accessible store.
* Speed parity(-ish) with optimized reference implementation on a single node.






