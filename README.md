# rtl_map 
### FFT-based visualizer for RTL-SDR devices. (RTL2832/DVB-T)

![820t2](https://user-images.githubusercontent.com/24392180/51805531-935c4700-227f-11e9-8249-44b849b8e757.jpg)

DVB-T dongles based on the `Realtek RTL2832U` can be used as a cheap SDR (_Software-defined radio_), since the chip allows transferring the raw I/Q (_In-phase / Quadrature_) samples to the host, which is officially used for DAB/DAB+/FM demodulation. 
([More about History & Discovery of rtl-sdr](http://rtlsdr.org/#history_and_discovery_of_rtlsdr))

There is various software for DSP (_Digital signal processing_), spectral analysis and signal intelligence using RTL-SDR such as [SDR#](https://airspy.com/download/), [gqrx](http://gqrx.dk/) and [gnuradio](https://www.gnuradio.org/). 
Apart from these, there is [librtlsdr](https://github.com/steve-m/librtlsdr) which most of the user-level packages rely because of the reason that librtlsdr comes as a part of the rtl-sdr codebase. 
This codebase contains both the library itself and also a number of command line tools such as rtl_test, rtl_sdr, rtl_tcp, and rtl_fm for testing RTL2832 and performing basic data transfer functions.
Therefore, librtlsdr is one of the major tools in RTL-SDR community.

Most of the technically questions RTL-SDR enthusiasts ask is about reading samples from device and processing raw I/Q samples. I wanted to answer that kind of questions and demonstrate a implementation of this with `rtl_map project` which is built on top of librtlsdr. Also, I went a step further and added graph feature that creates amplitude (dB) - frequency (MHz) graph using [gnuplot](http://www.gnuplot.info/) and [FFT](https://en.wikipedia.org/wiki/Fast_Fourier_transform) (_Fast Fourier transform_) algorithm. ([fftw](http://www.fftw.org/) used for this approach) I can list other useful open-source projects as [#1](https://gist.github.com/creaktive/7eeaeb76de26ca39dc3f), [#2](https://github.com/xofc/rtl_fftmax) and [#3](https://github.com/roger-/pyrtlsdr). 

Another purpose of this project is making a testing tool & frequency scanner application for security researches.




