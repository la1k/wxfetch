# wxfetch

Fetch APT images from satellites automatically as they pass over the horizon. Assumes a QFH antenna and an RTL-SDR dongle. 

Preliminary functionality has been implemented, but development is currently stalled until libpredict has become more stable and feature-complete. 

List of required libraries, in probable installation order:

* librtlsdr (http://sdr.osmocom.org/trac/wiki/rtl-sdr)
* gnuradio (https://github.com/gnuradio/gnuradio)
* osmosdr (http://sdr.osmocom.org/trac/)
* libpredict (https://github.com/la1k/libpredict)

Build instructions: 

1. mkdir build
2. cd build
3. cmake ..
4. make
