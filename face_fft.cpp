#ifdef FACE_FFT

/************  FFT definitions **********/
// Define MIC input pin
#define MIC A5			// A7 for Core, A5 for Photon

int8_t im[128];
int8_t fftdata[128];
int8_t spectrum[32];

byte
peak[32],		// Peak level of each column; used for falling dots
dotCount = 0,	// Frame counter for delaying dot-falling speed
colCount = 0;	// Frame counter for storing past column data

int8_t
col[32][10],	// Column levels for the prior 10 frames
minLvlAvg[32],	// For dynamic adjustment of low & high ends of graph,
maxLvlAvg[32];	// pseudo rolling averages for the prior few frames.
/***************************************/


//Spectrum Analyser stuff
void spectrumDisplay() {
	uint8_t static i = 0;
	//static unsigned long tt = 0;
	int16_t val;

	uint8_t  c;
	uint16_t x,minLvl, maxLvl;
	int      level, y, off;

	DEBUGpln("in Spectrum");

	off = 0;

	cls();
	//for (int show = 0; show < SHOWCLOCK ; show++) {
	int showTime = Time.now();

	while((Time.now() - showTime) < showClock) {
		if (mode_changed == 1)
		return;

		if(mode_quick){
			mode_quick = false;
			display_date();

#ifdef FACE_WEATHER
			quickWeather();
#endif

			spectrumDisplay();
			return;
		}

		if (i < 128){
			val = map(analogRead(MIC),0,4095,0,1023);
			fftdata[i] = (val / 4) - 128;
			im[i] = 0;
			i++;
		}
		else {
			//this could be done with the fix_fftr function without the im array.
			fix_fft(fftdata,im,7,0);
			//fix_fftr(fftdata,7,0);

			// I am only interessted in the absolute value of the transformation
			for (i=0; i< 64;i++){
				fftdata[i] = sqrt(fftdata[i] * fftdata[i] + im[i] * im[i]);
				//ftdata[i] = sqrt(fftdata[i] * fftdata[i] + fftdata[i+64] * fftdata[i+64]);
			}

			for (i=0; i< 32;i++){
				spectrum[i] = fftdata[i*2] + fftdata[i*2 + 1];   // average together
			}

			for(int l=0; l<16;l++){
				int col = matrix.Color444(16-l,0,l);
				matrix.drawLine(0,l,31,l,col);
			}

			// Downsample spectrum output to 32 columns:
			for(x=0; x<32; x++) {
				col[x][colCount] = spectrum[x];

				minLvl = maxLvl = col[x][0];
				int colsum=col[x][0];
				for(i=1; i<10; i++) { // Get range of prior 10 frames
					if(i<10)colsum = colsum + col[x][i];
					if(col[x][i] < minLvl)      minLvl = col[x][i];
					else if(col[x][i] > maxLvl) maxLvl = col[x][i];
				}
				// minLvl and maxLvl indicate the extents of the FFT output, used
				// for vertically scaling the output graph (so it looks interesting
				// regardless of volume level).  If they're too close together though
				// (e.g. at very low volume levels) the graph becomes super coarse
				// and 'jumpy'...so keep some minimum distance between them (this
				// also lets the graph go to zero when no sound is playing):
				if((maxLvl - minLvl) < 16) maxLvl = minLvl + 8;
				minLvlAvg[x] = (minLvlAvg[x] * 7 + minLvl) >> 3; // Dampen min/max levels
				maxLvlAvg[x] = (maxLvlAvg[x] * 7 + maxLvl) >> 3; // (fake rolling average)

				level = col[x][colCount];
				// Clip output and convert to byte:
				if(level < 0L)      c = 0;
				else if(level > 18) c = 18; // Allow dot to go a couple pixels off top
				else                c = (uint8_t)level;

				if(c > peak[x]) peak[x] = c; // Keep dot on top

				if(peak[x] <= 0) { // Empty column?
					matrix.drawLine(x, 0, x, 15, off);
					continue;
				}
				else if(c < 15) { // Partial column?
					matrix.drawLine(x, 0, x, 15 - c, off);
				}

				// The 'peak' dot color varies, but doesn't necessarily match
				// the three screen regions...yellow has a little extra influence.
				y = 16 - peak[x];
				matrix.drawPixel(x,y,matrix.Color444(peak[x],0,16-peak[x]));
			}
			i=0;
		}

		int mins = Time.minute();
		int hours = Time.hour();

		char buffer[3];

		itoa(hours,buffer,10);
		//fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
		if (hours < 10) {
			buffer[1] = buffer[0];
			buffer[0] = '0';
		}
		vectorNumber(buffer[0]-'0',8,1,matrix.Color333(0,1,0),1,1);
		vectorNumber(buffer[1]-'0',12,1,matrix.Color333(0,1,0),1,1);

		itoa(mins,buffer,10);
		//fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
		if (mins < 10) {
			buffer[1] = buffer[0];
			buffer[0] = '0';
		}
		vectorNumber(buffer[0]-'0',18,1,matrix.Color333(0,1,0),1,1);
		vectorNumber(buffer[1]-'0',22,1,matrix.Color333(0,1,0),1,1);

		matrix.drawPixel(16,2,matrix.Color333(0,1,0));
		matrix.drawPixel(16,4,matrix.Color333(0,1,0));

		matrix.swapBuffers(true);
		//delay(10);


		// Every third frame, make the peak pixels drop by 1:
		if(++dotCount >= 3) {
			dotCount = 0;
			for(x=0; x<32; x++) {
				if(peak[x] > 0) peak[x]--;
			}
		}

		if(++colCount >= 10) colCount = 0;

		bgProcess();	//Give the background process some lovin'
	}
}

#endif