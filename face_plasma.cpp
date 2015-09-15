#ifdef FACE_PLASMA

#include "math.h"

void spectrumDisplay();

/************  PLASMA definitions **********/
static const int8_t PROGMEM sinetab[256] = {
     0,   2,   5,   8,  11,  15,  18,  21,
    24,  27,  30,  33,  36,  39,  42,  45,
    48,  51,  54,  56,  59,  62,  65,  67,
    70,  72,  75,  77,  80,  82,  85,  87,
    89,  91,  93,  96,  98, 100, 101, 103,
   105, 107, 108, 110, 111, 113, 114, 116,
   117, 118, 119, 120, 121, 122, 123, 123,
   124, 125, 125, 126, 126, 126, 126, 126,
   127, 126, 126, 126, 126, 126, 125, 125,
   124, 123, 123, 122, 121, 120, 119, 118,
   117, 116, 114, 113, 111, 110, 108, 107,
   105, 103, 101, 100,  98,  96,  93,  91,
    89,  87,  85,  82,  80,  77,  75,  72,
    70,  67,  65,  62,  59,  56,  54,  51,
    48,  45,  42,  39,  36,  33,  30,  27,
    24,  21,  18,  15,  11,   8,   5,   2,
     0,  -3,  -6,  -9, -12, -16, -19, -22,
   -25, -28, -31, -34, -37, -40, -43, -46,
   -49, -52, -55, -57, -60, -63, -66, -68,
   -71, -73, -76, -78, -81, -83, -86, -88,
   -90, -92, -94, -97, -99,-101,-102,-104,
  -106,-108,-109,-111,-112,-114,-115,-117,
  -118,-119,-120,-121,-122,-123,-124,-124,
  -125,-126,-126,-127,-127,-127,-127,-127,
  -128,-127,-127,-127,-127,-127,-126,-126,
  -125,-124,-124,-123,-122,-121,-120,-119,
  -118,-117,-115,-114,-112,-111,-109,-108,
  -106,-104,-102,-101, -99, -97, -94, -92,
   -90, -88, -86, -83, -81, -78, -76, -73,
   -71, -68, -66, -63, -60, -57, -55, -52,
   -49, -46, -43, -40, -37, -34, -31, -28,
   -25, -22, -19, -16, -12,  -9,  -6,  -3
};

const float radius1  = 65.2, radius2  = 92.0, radius3  = 163.2, radius4  = 176.8,
            centerx1 = 64.4, centerx2 = 46.4, centerx3 =  93.6, centerx4 =  16.4,
            centery1 = 34.8, centery2 = 26.0, centery3 =  56.0, centery4 = -11.6;
float       angle1   =  0.0, angle2   =  0.0, angle3   =   0.0, angle4   =   0.0;
long        hueShift =  0;
/*******************************************/

void plasma() {
	int           x1, x2, x3, x4, y1, y2, y3, y4, sx1, sx2, sx3, sx4;
	unsigned char x, y;
	long          value;
	unsigned long slowFrameRate = millis();

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

		if (millis() - slowFrameRate >= 150) {

			sx1 = (int)(cos(angle1) * radius1 + centerx1);
			sx2 = (int)(cos(angle2) * radius2 + centerx2);
			sx3 = (int)(cos(angle3) * radius3 + centerx3);
			sx4 = (int)(cos(angle4) * radius4 + centerx4);
			y1  = (int)(sin(angle1) * radius1 + centery1);
			y2  = (int)(sin(angle2) * radius2 + centery2);
			y3  = (int)(sin(angle3) * radius3 + centery3);
			y4  = (int)(sin(angle4) * radius4 + centery4);

			for(y=0; y<(matrix.height()); y++) {
				x1 = sx1; x2 = sx2; x3 = sx3; x4 = sx4;
				for(x=0; x<matrix.width(); x++) {
					value = hueShift
					+ (int8_t)pgm_read_byte(sinetab + (uint8_t)((x1 * x1 + y1 * y1) >> 4))
					+ (int8_t)pgm_read_byte(sinetab + (uint8_t)((x2 * x2 + y2 * y2) >> 4))
					+ (int8_t)pgm_read_byte(sinetab + (uint8_t)((x3 * x3 + y3 * y3) >> 5))
					+ (int8_t)pgm_read_byte(sinetab + (uint8_t)((x4 * x4 + y4 * y4) >> 5));
					matrix.drawPixel(x, y, matrix.ColorHSV(value * 3, 255, 255, true));
					x1--; x2--; x3--; x4--;
				}
				y1--; y2--; y3--; y4--;
			}

			angle1 += 0.03;
			angle2 -= 0.07;
			angle3 += 0.13;
			angle4 -= 0.15;
			hueShift += 2;

			matrix.fillRect(7, 0, 19, 7, 0);

			int mins = Time.minute();
			int hours = Time.hour();
			char buffer[3];

			itoa(hours,buffer,10);
			//fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
			if (hours < 10) {
				buffer[1] = buffer[0];
				buffer[0] = '0';
			}
			vectorNumber(buffer[0]-'0',8,1,matrix.Color333(229,0,0),1,1);
			vectorNumber(buffer[1]-'0',12,1,matrix.Color333(229,0,0),1,1);

			itoa(mins,buffer,10);
			//fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
			if (mins < 10) {
				buffer[1] = buffer[0];
				buffer[0] = '0';
			}
			vectorNumber(buffer[0]-'0',18,1,matrix.Color333(229,0,0),1,1);
			vectorNumber(buffer[1]-'0',22,1,matrix.Color333(229,0,0),1,1);

			matrix.drawPixel(16,2,matrix.Color333(229,0,0));
			matrix.drawPixel(16,4,matrix.Color333(229,0,0));

			matrix.swapBuffers(false);

			slowFrameRate = millis();
			bgProcess();
		}
		bgProcess();	//Give the background process some lovin'
		delay(30);
	}
}
#endif