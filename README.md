# rtl_map <a href="https://github.com/orhun/rtl_map/releases"><img src="https://img.shields.io/github/release/orhun/rtl_map.svg"/>
</a>
<a href="https://github.com/orhun/rtl_map/issues"><img src="https://img.shields.io/github/issues/orhun/rtl_map.svg"/></a>
<a href="https://github.com/orhun/rtl_map/pulls"><img src="https://img.shields.io/github/issues-pr/orhun/rtl_map.svg"/></a>
<a href="https://github.com/orhun/rtl_map/stargazers"><img src="https://img.shields.io/github/stars/orhun/rtl_map.svg"/></a>
<a href="https://github.com/orhun/rtl_map/network"><img src="https://img.shields.io/github/forks/orhun/rtl_map.svg"/></a>
<a href="https://github.com/orhun/rtl_map/blob/master/LICENSE"><img src="https://img.shields.io/github/license/orhun/rtl_map.svg"/></a>

### FFT-based visualizer for RTL-SDR devices. (RTL2832/DVB-T)

![820t2](https://user-images.githubusercontent.com/24392180/51805531-935c4700-227f-11e9-8249-44b849b8e757.jpg)

DVB-T dongles based on the `Realtek RTL2832U` can be used as a cheap SDR (_Software-defined radio_), since the chip allows transferring the raw I/Q (_In-phase / Quadrature_) samples to the host, which is officially used for DAB/DAB+/FM demodulation. 
([More about history & discovery of rtl-sdr](http://rtlsdr.org/#history_and_discovery_of_rtlsdr))

There is various software for DSP (_Digital signal processing_), spectral analysis and signal intelligence using RTL-SDR such as [SDR#](https://airspy.com/download/), [gqrx](http://gqrx.dk/) and [gnuradio](https://www.gnuradio.org/). 
Apart from these, there is [librtlsdr](https://github.com/steve-m/librtlsdr) which most of the user-level packages rely because of the reason that librtlsdr comes as a part of the rtl-sdr codebase. 
This codebase contains both the library itself and also a number of command line tools such as rtl_test, rtl_sdr, rtl_tcp, and rtl_fm for testing RTL2832 and performing basic data transfer functions.
Therefore, librtlsdr is one of the major tools in RTL-SDR community.

Most of the technically questions RTL-SDR enthusiasts ask is about reading samples from device and processing raw I/Q samples. I wanted to answer that kind of questions and demonstrate a implementation of this with `rtl_map project` which is built on top of librtlsdr. Also, I went a step further and added graph feature that creates amplitude (dB) - frequency (MHz) graph using [gnuplot](http://www.gnuplot.info/) and [FFT](https://en.wikipedia.org/wiki/Fast_Fourier_transform) (_Fast Fourier transform_) algorithm. ([fftw](http://www.fftw.org/) used for this approach) I can list other useful open-source projects as [#1](https://gist.github.com/creaktive/7eeaeb76de26ca39dc3f), [#2](https://github.com/xofc/rtl_fftmax) and [#3](https://github.com/roger-/pyrtlsdr). 

Another purpose of this project is making a testing tool & frequency scanner application for security researches.

[https://www.rtl-sdr.com/rtl_map-a-simple-fft-visualizer-for-rtl-sdr/](https://www.rtl-sdr.com/rtl_map-a-simple-fft-visualizer-for-rtl-sdr/)

## Installation

### Dependencies
* libusb1.0 (development packages must be installed manually, see [libusb.info](https://libusb.info/))
* gnuplot (must be installed manually, see [installation page](http://gausssum.sourceforge.net/DocBook/ch01s03.html))
* librtlsdr (cmake / automatic installation & linking)
* fftw3 (cmake / automatic installation & linking)

### Clone Repository

```
git clone https://github.com/orhun/rtl_map
```

### Building with CMake (recommended)

```
cd rtl_map
mkdir build
cd build
cmake ../
make
sudo make install
sudo ldconfig
```
### Building with GCC

```
gcc rtl_map.c -o rtl_map -lrtlsdr -lfftw3 -lm
```

## Usage
### Command Line Arguments
```
-d, set device index (default: 0)
-s, set sample rate (default: 2048000 Hz)
-f, center frequency (Hz) [mandatory argument]
-g gain (0 for auto) (default: ~1-3)
-n number of reads (default: int_max.)
-r, refresh rate for continuous read (default: 500ms)
-D, don't show gnuplot graph (default: show)
-C, continuously read samples (default: off)
-M, show magnitude graph (default graph: dB)
-O, disable offset tuning (default: on)
-T, turn off terminal log colors (default: on)
-h, show help message and exit
filename (a '-' dumps samples to stdout)
```

### Example: Print samples to file

```
[k3@arch ~]$ rtl_map -f 88000000 -D capture.dat
[01:00:26] INFO Starting rtl_map ~
[01:00:26] INFO Found 1 device(s):
[01:00:26] INFO #0: Generic RTL2832U OEM
Found Rafael Micro R820T tuner
[01:00:27] INFO Using device: #0
[01:00:27] INFO Gain set to 14.
Supported gain values (29): 0.0 0.9 1.4 2.7 3.7 7.7 8.7 12.5 14.4 15.7 16.6 19.7 20.7 22.9 25.4 
128.0 29.7 32.8 33.8 36.4 37.2 38.6 40.2 42.1 43.4 43.9 44.5 48.0 49.6 
[01:00:27] INFO Center frequency set to 88000000 Hz.
[01:00:27] INFO Sampling at 2048000 S/s
[01:00:27] INFO Reading samples...
[01:00:27] INFO Done, exiting...
```

### Example: Print samples to stdout

```
rtl_map -f 88000000 -D -
```

![print samples to stdout](https://user-images.githubusercontent.com/24392180/51807038-b0e6dc00-2292-11e9-9978-e2ddf5852e7d.gif)

### Example: Print 10x512 samples to stdout

```
rtl_map -f 88000000 -D -C -n 10 -
```

### Example: Create FFT graph from samples
```
[k3@arch ~]$ rtl_map -f 88000000
[01:00:28] INFO Starting rtl_map ~
[01:00:28] INFO Found 1 device(s):
[01:00:28] INFO #0: Generic RTL2832U OEM
Found Rafael Micro R820T tuner
[01:00:28] INFO Using device: #0
[01:00:28] INFO Gain set to 14.
Supported gain values (29): 0.0 0.9 1.4 2.7 3.7 7.7 8.7 12.5 14.4 15.7 16.6 19.7 20.7 22.9 25.4 
28.0 29.7 32.8 33.8 36.4 37.2 38.6 40.2 42.1 43.4 43.9 44.5 48.0 49.6 
[01:00:29] INFO Center frequency set to 88000000 Hz.
[01:00:29] INFO Sampling at 2048000 S/s
[01:00:29] INFO Creating FFT graph from samples using gnuplot...
[01:00:29] INFO Done, exiting...
```
![fft graph](https://user-images.githubusercontent.com/24392180/52183411-c3c05a00-2818-11e9-8883-cdddd2c7376e.jpg)


### Example: Continuously read samples and create FFT graph

```
rtl_map -f 88000000 -C -r 100
```

![continuously read](https://user-images.githubusercontent.com/24392180/52239109-bbcaed80-28de-11e9-921e-7c438f42a4c9.gif)

More example (in german) see

http://blog.wenzlaff.de/?p=12826


## DC Offset & I/Q Imbalance

There is a common issue with cheap RTL-SDR receivers which is `center frequency spike` or `central peak` problem related to I/Q imbalance. This problem can be solved with a implementation of some algorithms. (For more details: [#1](https://github.com/roger-/pyrtlsdr/issues/94), [#2](https://wiki.analog.com/resources/eval/user-guides/ad-fmcomms1-ebz/iq_correction))

## TODO(s)
1. Implement I/Q correction
2. Find the maximum value of samples, show it on graph with a different color.  Might be useful for frequency scanner.
3. Frequency scanner feature
4. Check correctness of min/max point calculation.
5. Check correctness of amplitude (dB) calculation.
* 820T2 tuner used for testing. Other RTL-SDR devices must be tested.

## Contribution

You can contribute to this project if you are a RTL-SDR enthusiast or researcher. Fork the repository and start coding.
I hope some people on this planet will consider my TODO(s) and help me build the frequency scanner tool :)

## License

GNU General Public License ([v3](https://www.gnu.org/licenses/gpl.txt))

## Copyright

Copyright (c) 2019-2020, [orhun](https://www.github.com/orhun)
