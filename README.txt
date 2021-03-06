
This directory is the distribution of version 4.2 of the link grammar
parsing system for windows.  This version was created in April 2001.
Please see our web page http://www.link.cs.cmu.edu/link for more
information, including documentation.

Changes over version 4.1:

   1. Compiles under RedHat linux 7.0.
   2. Release of windows version

Changes over version 4.0:

   1. The program api-example.c has been updated.
   2. An example from the documentation is included: constituent-example.c
   3. The gnu header file stdarg.h is not included.


CONTENTS of this directory:

   src/*.c            The program.  (Written in ANSI-C)
   include/*.h

   link42.exe         Win32 executable (with no arguments it looks for
                      the data directory in data.  See instructions below.)

   link42.dsw         Microsoft Visual C++ 6.0 workspace file

   data/4.0.dict      The file containing the dictionary definitions.

   data/4.0.knowledge The post-processing knowledge file.

   data/4.0.constituents    The constituent knowledge file.

   data/4.0.affix     The affix file.

   data/tiny.dict     A small sample dictionary.

   data/words/*       A directory full of word lists.

   data/4.0.batch     This is a batch file of sentences (both grammatical
                      and ungrammatical ones) that are handled correctly
                      by this release of the system.  Feed this into the
                      parser with "./parse 4.0.dict < 4.0.batch"

CREATING the system:

   This was created with Microsoft Visual C++ 6.0.

RUNNING the program:

   To run the program simply double click on the link42.exe file.
   This starts the program.  Help is available there with "!help".  A
   number of user-settable variables control what happens.  "!var" shows
   these variables and their current values.

   We've used a special batch mode for testing the system on a large
   number of sentences.  The following command runs the parser on
   3.0.batch

       link42.exe data\4.0.dict < data\4.0.batch

   The line "!batch" near the top of 4.0.batch turns on batch mode.  In
   this mode sentences labeled with an initial "*" should be rejected
   and those not starting with a "*" should be accepted.  Any deviation
   from this behavior is reported as an error.

USING the parser in your own applications:

   There is a API (application program interface) to the parser.  This
   makes it easy to incorporate it into your own applications.  This is
   documented on our web site.

COMMERCIAL use:

   We feel that our system (or a derivative of it) could be useful in a
   number of commercial applications.  We would be pleased to discuss
   terms with those wanting to use our system in this way.  Without an
   explicit agreement with the authors (listed below) such use is
   forbidden.  By "commercial use" we mean selling a product or service.
   Of course everybody is free to take the system for purposes of
   experiment and evaluation.  Of course all academic products or papers
   must reference our work.

ADDRESSES

   If you have any questions, or find any bugs, please feel free to send
   a note to:

     John Lafferty                     lafferty@cs.cmu.edu
     Computer Science Department       412-268-6791
     Carnegie Mellon University        www.cs.cmu.edu/~lafferty
     Pittsburgh, PA 15213              

     Daniel Sleator                    sleator@cs.cmu.edu  
     Computer Science Department       412-268-7563
     Carnegie Mellon University        www.cs.cmu.edu/~sleator
     Pittsburgh, PA 15213              

     Davy Temperley                    dtemp@theory.esm.rochester.edu
     Eastman School of Music	       716-274-1557
     26 Gibbs St.	               www.link.cs.cmu.edu/temperley
     Rochester, NY 14604
