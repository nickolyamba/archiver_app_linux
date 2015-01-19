Archiver app for Linux

1. To compile the app, type "make all" in command line

2. To test the app, type "make tests" in command line

3. "myar -x ar12345.ar" and Linux's "ar xo ar12345.ar" extract files
with the same timestamps and permissions.
If no member is named on the command line when extracting files,
all files are extracted from the archive.

4. -d works: tested with multiple files and with single file. 
Archive is checked using build-in Linux archiver with "ar -tv" command

5. -A works: determine regular files in the directory and archives them
Archive is checked using build-in Linux archiver with "ar -tv" command
