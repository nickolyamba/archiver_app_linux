# archiver_app_linux

1. To compile the app, write "make all" in command line

2. To test the app, write "make tests" in command line

3. myar -x ar12345.ar and ar xo ar12345.ar extract the same files
with the same time and permissions.
If no member is named on the command line when extracting files,
all files are extracted from the archive.

4. -d works: tested with multiple files and with one. 
Archive checked with ar -tv

5. -A works: determine regular files in the dir and archives them
Archive checked with ar -tv
