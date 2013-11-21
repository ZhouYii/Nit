Nit
===

Git clone with networking.

Week 1: Basic features are working : Add files, Delete files from version control, Commit and revert.
Week 2: File Writes to server.
	EOFs were not being read.
	Defined constants for server opcodes to minimize frequence of file ASCII overlap.
	Buffer errors.
	Flush errors.
Week 3: File Reads from server. 
	Introduced lots of redundant code. (Good because server and client need to
    do handshaking without missing any bytes).
    New tests!
    New comments!
    New feature : Push-to-server.
    New feature : Pull-from-server.

