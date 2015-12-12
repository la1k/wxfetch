wxdecode is based on ATPDEC version 1.7 (http://atpdec.sourceforge.net/) (Thierry Leconte F4DWV (c) 2004-2005). 

The code has been changed in order to be able to run APT decoding on any input stream, not limited to a file read using libsndfile. 

wxdecode is primarily meant to aid the gnuradio module gr-apt in decoding an APT input stream, but can be used 
as a standalone application like ATPDEC, or as a library for other applications. 

For some kind of library API, see `src/apt.h`. Standalone executable is produced as `wxdecode-bin`, and can be run using `./wxdecode-bin soundfile.wav output_filename.png`. Input file must be sampled at 11025 Hz.

Build instructions:

1. mkdir build
2. cd build
3. cmake ..
4. make

Requirements: 
* libsndfile version >= 1.0.25 
* OpenCV version >= 2.4
