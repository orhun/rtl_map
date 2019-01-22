// gcc rtl_fft.c -o rtl_fft -lrtlsdr -lfftw3 -lm & ./rtl_fft

#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <complex.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <fftw3.h>
#include <rtl-sdr.h>

#define NUM_READ 512
#define log_info(...) print_log(INFO, __VA_ARGS__)
#define log_error(...) print_log(ERROR, __VA_ARGS__)
#define log_fatal(...) print_log(FATAL, __VA_ARGS__)

static rtlsdr_dev_t *dev;
static fftw_plan fftwp;
static fftw_complex *in, *out;
static FILE *gnuplotPipe, *file;
static struct sigaction sig_act;
static const int n_read = NUM_READ;
static int n, out_r, out_i,
	_center_freq,
	_dev_id = 0, 
	_samp_rate = n_read * 4000,
	_gain = 14,
	_refresh_rate = 500,
	_use_gnuplot = 1,
	_cont_read = 0,
	_mag_graph = 0,
	_offset_tuning = 1,
	_log_colors = 1,
	_write_file = 0;
static float amp, db;
static char t_buf[16],
	*_filename,
	*log_levels[] = {
		"INFO", "ERROR", "FATAL"
	},
	*level_colors[] = {
		"\x1b[92m", "\x1b[91m", "\x1b[33m"
	},
	*bold_attr = "\x1b[1m",
	*all_attr_off = "\x1b[0m";
static va_list vargs;
static time_t raw_time;
enum log_level {INFO, ERROR, FATAL};
typedef struct SampleBin {
	float val;
    int id;
} Bin;
static Bin sample_bin[NUM_READ];

void create_fft(int sample_c, unsigned char *buf);
static void print_log(int level, char *format, ...){
	raw_time = time(NULL);
	struct tm *local_time = localtime(&raw_time);
	t_buf[strftime(t_buf, sizeof(t_buf), 
			"%H:%M:%S", local_time)] = '\0';
	if (_log_colors)
		fprintf(stderr, "%s[%s] %s%s%s ", 
		bold_attr, 
		t_buf, 
		level_colors[level], 
		log_levels[level], 
		all_attr_off);
	else
		fprintf(stderr, "[%s] %s ", 
		t_buf, 
		log_levels[level]);
  	va_start(vargs, format);
  	vfprintf(stderr, format, vargs);
  	va_end(vargs);
}
static void do_exit(){
	rtlsdr_cancel_async(dev);
	if(_use_gnuplot)
		pclose(gnuplotPipe);
	if(_filename != NULL && strcmp(_filename, "-"))
		fclose(file);
	exit(0);
}
static void sig_handler(int signum){
    log_info("Signal caught, exiting...\n");
	do_exit();
}
static void register_signals(){
	sig_act.sa_handler = sig_handler;
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;
    sigaction(SIGINT, &sig_act, NULL);
    sigaction(SIGTERM, &sig_act, NULL);
    sigaction(SIGQUIT, &sig_act, NULL);
    sigaction(SIGPIPE, &sig_act, NULL);
}
static void gnuplot_exec(char *format, ...){
	va_start(vargs, format);
  	vfprintf(gnuplotPipe, format, vargs);
  	va_end(vargs);
}
void configure_gnuplot(){
	if (!_use_gnuplot)
		return;
	gnuplotPipe = popen("gnuplot -persistent", "w");
	if (!gnuplotPipe) {
		log_error("Failed to open gnuplot pipe.");
		exit(1);
	}
	gnuplot_exec("set title 'RTL-FFTX' enhanced\n");
	gnuplot_exec("set xlabel 'Frequency (kHz)'\n");
	gnuplot_exec("set ylabel 'Amplitude (dB)'\n");
	float center_mhz = _center_freq / pow(10, 6);
	float step_size = (n_read * pow(10, 3))  / pow(10, 6);
	gnuplot_exec("set xtics ('%.1f' 1, '%.1f' 256, '%.1f' 512)\n", 
		center_mhz-step_size, 
		center_mhz, 
		center_mhz+step_size);
}
void configure_rtlsdr(){

	int device_count = rtlsdr_get_device_count();
	if (!device_count) {
		log_error("No supported devices found.\n");
		exit(1);
	}
	log_info("Found %d device(s):\n", device_count);
	for(int n = 0; n < device_count; n++){
		if(_log_colors)
			log_info("#%d: %s%s%s\n", n, bold_attr, rtlsdr_get_device_name(n), all_attr_off);
		else
			log_info("#%d: %s\n", n, rtlsdr_get_device_name(n));
	}

	int dev_open = rtlsdr_open(&dev, _dev_id);
	if (dev_open < 0) {
		log_fatal("Failed to open RTL-SDR device #%d\n", _dev_id);
		exit(1);
	}else{
		log_info("Using device: #%d\n", dev_open);
	}
	if(!_gain){
		rtlsdr_set_tuner_gain_mode(dev, _gain);
		log_info("Gain mode set to auto.\n");
	}else{
		rtlsdr_set_tuner_gain_mode(dev, 1);
		rtlsdr_set_tuner_gain(dev, _gain);
		int gain_count = rtlsdr_get_tuner_gains(dev, NULL);
		log_info("Gain set to %d.\nSupported gain values (%d): ", _gain, gain_count);
		int gains[gain_count], supported_gains = rtlsdr_get_tuner_gains(dev, gains);
		for (int i = 0; i < supported_gains; i++)
			fprintf(stderr, "%.1f ", gains[i] / 10.0);
		fprintf(stderr, "\n");
	}

	rtlsdr_set_offset_tuning(dev, _offset_tuning);
	rtlsdr_set_center_freq(dev, _center_freq);
	rtlsdr_set_sample_rate(dev, _samp_rate);

	log_info("Center frequency set to %d Hz.\n", _center_freq);
	log_info("Sampling at %d S/s\n", _samp_rate);

	int r = rtlsdr_reset_buffer(dev);
	if (r < 0)
		log_fatal("Failed to reset buffers.\n");
}
void open_file(){
	if (_filename != NULL){
		_write_file = 1;
		if(!strcmp(_filename, "-")) {
        	file = stdout;
		} else {
			file = fopen(_filename, "w+");
			if (!file) {
				log_error("Failed to open %s\n", _filename);
				exit(1);
			}
   		}
	}
}
int cmp_sample(const void * a, const void * b){
  float fa = *(const float*) a;
  float fb = *(const float*) b;
  return (fa > fb) - (fa < fb);
}
static void async_read_callback(unsigned char *n_buf, uint32_t len, void *ctx){
	create_fft(n_read, n_buf);
	if (_cont_read){
		usleep(1000*_refresh_rate);
	}else{
		log_info("Done, exiting...\n");
		do_exit();
	}
}

void create_fft(int sample_c, unsigned char *buf){
	//Configure FFTW to convert the samples in time domain to frequency domain
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*sample_c);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*sample_c);
	fftwp = fftw_plan_dft_1d(sample_c, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	//convert buffer from IQ to complex ready for FFTW, seems that rtlsdr outputs IQ data as IQIQIQIQIQIQ so ......
	n = 0;
	for (int i=0; i<sample_c; i+=2){
		//sample is 127 for zero signal,so 127 +/-127
		in[i] = (buf[n]-127) + (buf[n+1]-127) * I;
		n++;
	}
	//Convert the complex samples to complex frequency domain
	fftw_execute(fftwp);

	//compute magnitude from complex = sqrt(real^2 + imaginary^2)
	//magnitude [dB] = 10 * Log(sqr(Re^2 + Im^2))
	//print the 512 bin spectrum as numbers
	if(!_cont_read)
		log_info("Creating FFT graph from samples using gnuplot...\n");
	if(_use_gnuplot)
		gnuplot_exec("plot '-' smooth frequency with linespoints lt -1 notitle\n");
	for (int i=0; i < sample_c; i++){
		out_r = pow(creal(out[i]), 2);
		out_i =  pow(cimag(out[i]), 2);
		amp = sqrt(out_r + out_i);
		if (!_mag_graph)
			db = 10 * log10(amp);
		else
			db = amp;
		if(_write_file)
			fprintf(file, "%d	%f\n", i+1, db);
		if(_use_gnuplot)
			gnuplot_exec("%d	%f\n", db, i+1);
		sample_bin[i].id = i;
		sample_bin[i].val = db;
	}
	//qsort(sample_bin, n_read, sizeof(Bin), cmp_sample);
	if(_use_gnuplot){
		gnuplot_exec("e\n");
		fflush(gnuplotPipe);
	}
	fftw_destroy_plan(fftwp);
	fftw_free(in); 
	fftw_free(out);
}
void print_usage(){
	char *usage =  "Usage:\t[-d device index (default: 0)]\n"
                   "\t[-s samplerate (default: 2048000 Hz)]\n"
				   "\t[-f center frequency (Hz)] *\n"
				   "\t[-g gain (0 for auto) (default: 1.4)]\n"
				   "\t[-r refresh rate for -C read (default: 500ms)]\n"
				   "\t[-D don't show gnuplot graph (default: show)]\n"
				   "\t[-C continuously read samples (default: off)]\n"
				   "\t[-M show magnitude graph (default graph: dB)]\n"
				   "\t[-O disable offset tuning (default: on)]\n"
				   "\t[-T turn off log colors (default: on)]\n"
				   "\t[-h show this help message and exit]\n"
                   "\t[filename (a '-' dumps samples to stdout)]\n\n";
    fprintf(stderr, "%s", usage);
    exit(0);
}
void parse_args(int argc, char **argv){
	int opt;
	while ((opt = getopt(argc, argv, "d:s:f:g:r:DCMOTh")) != -1) {
        switch (opt) {
            case 'd':
                _dev_id = atoi(optarg);
                break;
			case 's':
                _samp_rate = atoi(optarg);
            	break;
            case 'f':
                _center_freq = atoi(optarg);
                break;
            case 'g':
                _gain = (int)(atof(optarg) * 10); /* tenths of a dB */
                break;
			case 'r':
                _refresh_rate = atoi(optarg);
                break;
			case 'D':
                _use_gnuplot = 0;
                break;		
			case 'C':
                _cont_read = 1;
                break;
			case 'M':
                _mag_graph = 1;
                break;
			case 'O':
                _offset_tuning = 0;
                break;
			case 'T':
                _log_colors = 0;
                break;
            case 'h':
                print_usage();
                break;
            default:
                print_usage();
                break;
        }
    }
	if (!_center_freq)
		print_usage();
	_filename = argv[optind];
}
int main(int argc, char **argv){
	parse_args(argc, argv);
	register_signals();
	configure_gnuplot();
	configure_rtlsdr();
	open_file();
	rtlsdr_read_async(dev, async_read_callback, NULL, 0, pow(n_read, 2));
}