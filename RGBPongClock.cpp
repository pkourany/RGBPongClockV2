/*  RGB Pong Clock - Andrew Holmes @pongclock
**  Inspired by, and shamelessly derived from
**      Nick's LED Projects
**  https://123led.wordpress.com/about/
**
**  Videos of the clock in action:
**  https://vine.co/v/hwML6OJrBPw
**  https://vine.co/v/hgKWh1KzEU0
**  https://vine.co/v/hgKz5V0jrFn
**  I run this on a Mega 2560, your milage on other chips may vary,
**  Can definately free up some memory if the bitmaps are shrunk down to size.
**  Uses an Adafruit 16x32 RGB matrix availble from here:
**  http://www.phenoptix.com/collections/leds/products/16x32-rgb-led-matrix-panel-by-adafruit
**  This microphone:
**  http://www.phenoptix.com/collections/adafruit/products/electret-microphone-amplifier-max4466-with-adjustable-gain-by-adafruit-1063
**  a DS1307 RTC chip (not sure where I got that from - was a spare)
**  and an Ethernet Shield
**  http://hobbycomponents.com/index.php/dvbd/dvbd-ardu/ardu-shields/2012-ethernet-w5100-network-shield-for-arduino-uno-mega-2560-1280-328.html
**
*/

#define	RGBPCversion	"V1.03g"

#include "Adafruit_GFX.h"   // Core graphics library
#include "RGBmatrixPanel.h" // Hardware-specific library
#include "font3x5.h"
#include "font5x5.h"
#include "face_class.h"

#define WEATHER_CITY		"{\"mycity\": \"Chattanooga,TN\" }"


// // allow us to use itoa() in this scope
extern char* itoa(int a, char* buffer, unsigned char radix);


/** Define RGB matrix panel GPIO pins **/
#if defined (STM32F10X_MD)	//Core
	#define CLK D6
	#define OE  D7
	#define LAT A4
	#define A   A0
	#define B   A1
	#define C   A2
	#define D   A3
#endif

#if defined (STM32F2XX)	//Photon
	#define CLK D6
	#define OE  D7
	#define LAT A4
	#define A   A0
	#define B   A1
	#define C   A2
	#define D   A3
#endif

/****************************************/
#define SHOWCLOCK	10000
#define MAX_CLOCK_MODE	9	// Number of clock modes

#define X_MAX 31		// Matrix X max LED coordinate (for 2 displays placed next to each other)
#define Y_MAX 15


/********** RGB565 Color definitions **********/
// #define Black           0x0000
// #define Navy            0x000F
// #define DarkGreen       0x03E0
// #define DarkCyan        0x03EF
// #define Maroon          0x7800
// #define Purple          0x780F
// #define Olive           0x7BE0
// #define LightGrey       0xC618
// #define DarkGrey        0x7BEF
// #define Blue            0x001F
// #define Green           0x07E0
// #define Cyan            0x07FF
// #define Red             0xF800
// #define Magenta         0xF81F
// #define Yellow          0xFFE0
// #define White           0xFFFF
// #define Orange          0xFD20
// #define GreenYellow     0xAFE5
// #define Pink			0xF81F
/**********************************************/


/***** Create RGBmatrix Panel instance *****
 Last parameter = 'true' enables double-buffering, for flicker-free,
 buttery smooth animation.  Note that NOTHING WILL SHOW ON THE DISPLAY
 until the first call to swapBuffers().  This is normal. */
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);
//RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, true);
/*******************************************/


int mode_changed = 0;			// Flag if mode changed.
bool mode_quick = false;		// Quick weather display
int clock_mode = 0;			// Default clock mode (1 = pong)
uint16_t showClock = 300;		// Default time to show a clock face
unsigned long modeSwitch;
unsigned long updateCTime;		// 24hr timer for resyncing cloud time
uint8_t hour_last = 0;
uint8_t minute_last = 0;
uint8_t second_last = 0;
bool has_shown = 0;

#define MAX_FACES		16
#define MAX_INTERLUDES	8
FaceClass * faces[MAX_FACES];
FaceClass * interludes[MAX_INTERLUDES];

/************ PROTOTYPES **************/
int setMode(String command);
void cls();
void vectorNumber(int n, int x, int y, int color, float scale_x, float scale_y);
void drawString(int x, int y, char* c,uint8_t font_size, uint16_t color);
void drawChar(int x, int y, char c, uint8_t font_size, uint16_t color);
int calc_font_displacement(uint8_t font_size);
void flashing_cursor(byte xpos, byte ypos, byte cursor_width, byte cursor_height, byte repeats);
void bgProcess();
void update_last();
/*************************************/


// #define INTERLUDE_WEATHER
// #define INTERLUDE_PACMAN
// #define INTERLUDE_DATE

// #define FACE_FFT
// #define FACE_WORDCLOCK
// #define FACE_MARQUEE
// #define FACE_PONG
// #define FACE_PLASMA
// #define FACE_JUMBLE
#define FACE_NORMAL

#include "interlude_weather.cpp"

#ifdef FACE_NORMAL
	#include "face_normal.h"
	face_normal FaceNormal();
#endif

#ifdef FACE_PACMAN
	#include "interlude_pacman.cpp"
#endif

#ifdef FACE_DATE
	#include "interlude_date.cpp"
#endif

#ifdef FACE_WORDCLOCK
	#include "face_wordclock.cpp"
#endif

#ifdef FACE_MARQUEE
	#include "face_marquee.cpp"
#endif

#ifdef FACE_PONG
	#include "face_pong.cpp"
#endif

#ifdef FACE_PLASMA
	#define FACE_FFT
	#include "face_plasma.cpp"
#endif

#ifdef FACE_FFT
	#include "fix_fft.h"
	#include "face_fft.cpp"
#endif

#ifdef FACE_JUMBLE
	#include "face_jumble.cpp"
#endif




// A sort of catch-all "background" process that can be called outside of loop()
// if needed
void bgProcess() {
	// Try reconnecting to the cloud
	if(!Particle.connected())
		Particle.connect();

	Particle.process();
}


// Semi-automatic mode to allow the clock to start running immediately
SYSTEM_MODE(SEMI_AUTOMATIC);

// Keep track of OTA update status
bool is_ota_updating = false;


// Make pixel 0,0 reflect the onboard RGB LED state when not connected to the
// cloud or an OTA update is occurring
void ledChangeHandler(uint8_t r, uint8_t g, uint8_t b) {
	bool do_it = false;

	if(!Particle.connected())
		do_it = true;

	if(is_ota_updating)
		do_it = true;

	if(do_it) {
		matrix.drawPixel(0, 0, matrix.Color444(r, g, b));
		matrix.swapBuffers(true);
	}

	is_ota_updating = false;
}


// Called when an OTA flash is running
void system_ota_handler(system_event_t events, uint32_t param, void* pointer) {
    is_ota_updating = true;
}


void setup() {

int i;

for (i=0; i < MAX_FACES; i++)
	faces[i] = NULL;

for (i=0; i < MAX_INTERLUDES; i++)
	interludes[i] = NULL;

#ifdef FACE_NORMAL
//	faces[0] = new face_normal();
	faces[0] = &FaceNormal;
#endif

#ifdef FACE_PONG
	faces[1] = new face_pong();
#endif
#ifdef FACE_WORDCLOCK
	faces[2] = new face_wordclock();
#endif
#ifdef FACE_JUMBLE
	faces[3] = new face_jumble();
#endif
#ifdef FACE_FFT
	faces[4] = new face_fft();
#endif
#ifdef FACE_PLASMA
	faces[5] = new face_plasma();
#endif
#ifdef FACE_MARQUEE
	faces[6] = new face_marquee();
#endif

#ifdef INTERLUDE_WEATHER
	interludes[0] = new interlude_weather();
#endif
#ifdef INTERLUDE_PACMAN
	interludes[0] = new interlude_pacman();
#endif
#ifdef INTERLUDE_DATE
	interludes[1] = new interlude_date();
#endif


	// Initialize the panel
	matrix.begin();
	matrix.setTextWrap(false); // Allow text to run off right edge
	matrix.setTextSize(1);
	matrix.setTextColor(matrix.Color333(210, 210, 210));

	// Bind to onboard RGB change
	RGB.onChange(ledChangeHandler);

	// Listen for OTA updates to blink magenta on the panel
	System.on(firmware_update, system_ota_handler);

	// Connect to the cloud
	Particle.connect();
	while(!Particle.connected()) {
	 	Particle.process();
	 	delay(1);
	}

	// May need to move to loop() if can't connect
	Particle.publish("RGBPongClock", RGBPCversion, 60, PRIVATE);
	Particle.process();	// Force processing of Particle.publish()
	// ************************************************


#ifdef FACE_MARQUEE
	// Allow a user to call a function and post their own marquee message
	Particle.function("marquee", marqueeMsg);
#endif

#ifdef FACE_WEATHER
	weather_setup();
#endif


	// !!!! May need to copy to loop() if disconnect/connect
	unsigned long resetTime;
	do {
		resetTime = Time.now();        // the current time = time of last reset
		delay(10);
	} while (resetTime < 1000000 && millis() < 20000); // wait for a reasonable epoc time, but not longer than 20 seconds


	// Needs to be set via Particle.function and store in EEPROM!!
	Time.zone(-4);

#ifdef FACE_FFT
	memset(peak, 0, sizeof(peak));
	memset(col , 0, sizeof(col));

	for(uint8_t i=0; i<32; i++) {
		minLvlAvg[i] = 0;
		maxLvlAvg[i] = 255;
	}
#endif



#ifdef FACE_PACMAN
	pacMan();
#endif

	// clock_mode = random(0, MAX_CLOCK_MODE-1);
	// Default startup clock is face_normal so init that face
	clock_mode = 0;
	faces[clock_mode]->begin();

	modeSwitch = millis();
	updateCTime = millis();		// Reset 24hr cloud time refresh counter

	update_last();
	cls();
}


void loop() {
	// !!!  Add code for re-syncing time every 24 hrs  !!!
	if ((millis() - updateCTime) > (24UL * 60UL * 60UL * 1000UL)) {
		Particle.syncTime();
		updateCTime = millis();
	}

	// Make it check for mode_changed==0, otherwise it may change the mode to a different one
	// in case the user "manually" triggered a mode change.
	// if (mode_changed==0 && millis() - modeSwitch > 300000UL) {	//Switch modes every 5 mins
	if (millis() - modeSwitch > 10000UL) {	//Switch modes every 5 mins
		mode_changed = 1;
		clock_mode++;
		modeSwitch = millis();
		if (clock_mode > MAX_CLOCK_MODE-1)
			clock_mode = 0;
	}

/*
	if(mode_quick){
		mode_quick = false;
		// activate interludes
		display_date();
		quickWeather();
		return;
	}
*/

	if (mode_changed == 1) {
		if (faces[clock_mode] == NULL) {
			clock_mode++;	// go to next clock an try again
			return;
		}
		else {
			mode_changed = 0;
			faces[clock_mode]->begin(); //init clock face only once
		}
	}

	// display the active clock face if not complete or endless loop
	if (faces[clock_mode]->isComplete() == 0 || faces[clock_mode]->isComplete() == -1)
		faces[clock_mode]->run();
//	else

	// Timer for interlude
	// If face display isComplete AND time is up, display the interludes
	// do something is faces[clock_mode].isComplete()

/*	
	
	switch (clock_mode) {
#ifdef FACE_NORMAL
	case 0:
		// normal_clock();
		faceNormal.();
		break;
#endif
#ifdef FACE_PONG
	case 1:
		pong();
		break;
#endif
#ifdef FACE_WORDCLOCK
	case 2:
		word_clock();
		break;
#endif
#ifdef FACE_JUMBLE
	case 3:
		jumble();
		break;
#endif
#ifdef FACE_FFT
	case 4:
		spectrumDisplay();
		break;
#endif
#ifdef FACE_PLASMA
	case 5:
		plasma();
		break;
#endif
#ifdef FACE_MARQUEE
	case 6:
		marquee();
		break;
#endif
#ifdef FACE_WEATHER
	case 7:
		weather();
		break;
#endif
#ifdef FACE_DATE
	case 8:
		display_date();
		break;
#endif
	default:
		clock_mode++;
	}
*/

	//if the mode hasn't changed, show the date
// #ifdef FACE_PACMAN
// 	pacClear();
// #endif
// 	//if (mode_changed == 0) {
// #ifdef FACE_DATE
// 		//display_date();
// #endif

// #ifdef FACE_PACMAN
// 		//pacClear();
// #endif

	bgProcess();
}


// int setMode(String command) {
// 	mode_changed = 0;

// 	int j = command.indexOf('=',0);
// 	if (j>0) {	// "=" is used when setting city only
// 		if(command.substring(0,j) == "city")
// 		{
// 			unsigned char tmp[20] = "";
// 			int p = command.length();
// 			command.getBytes(tmp, (p-j), j+1);
// #ifdef FACE_WEATHER
// 			strcpy(city, "{\"mycity\": \"");
// 			strcat(city, (const char *)tmp);
// 			strcat(city, "\" }");
// 			weatherGood = false;
// #endif
// 			return 1;
// 		}
// 	}
// 	else if(command == "normal")
// 	{
// 		mode_changed = 1;
// 		clock_mode = 0;
// 	}
// 	else if(command == "pong")
// 	{
// 		mode_changed = 1;
// 		clock_mode = 1;
// 	}
// 	else if(command == "word")
// 	{
// 		mode_changed = 1;
// 		clock_mode = 2;
// 	}
// 	else if(command == "jumble")
// 	{
// 		mode_changed = 1;
// 		clock_mode = 3;
// 	}
// 	else if(command == "spectrum")
// 	{
// 		mode_changed = 1;
// 		clock_mode = 4;
// 	}
// 	else if(command == "quick")
// 	{
// 		mode_quick = true;
// 		return 1;
// 	}
// 	else if(command == "plasma")
// 	{
// 		mode_changed = 1;
// 		clock_mode = 5;
// 	}
// 	else if(command == "marquee")
// 	{
// 		mode_changed = 1;
// 		clock_mode = 6;
// 	}
// 	if (mode_changed == 1) {
// 		modeSwitch = millis();
// 		return 1;
// 	}
// 	else return -1;

// }


void cls() {
	bgProcess();
	matrix.fillScreen(0);
}


void drawString(int x, int y, char* c,uint8_t font_size, uint16_t color) {
	// x & y are positions, c-> pointer to string to disp, update_s: false(write to mem), true: write to disp
	//font_size : 51(ascii value for 3), 53(5) and 56(8)
	for(uint16_t i=0; i< strlen(c); i++)
	{
		drawChar(x, y, c[i],font_size, color);
		x+=calc_font_displacement(font_size); // Width of each glyph
	}
}


int calc_font_displacement(uint8_t font_size) {
	switch(font_size)
	{
	case 51:
		return 4;  //5x3 hence occupies 4 columns ( 3 + 1(space btw two characters))
		break;

	case 53:
		return 6;
		break;
		//case 56:
		//return 6;
		//break;
	default:
		return 6;
		break;
	}
}


// Display the data depending on the font size mentioned in the font_size variable
void drawChar(int x, int y, char c, uint8_t font_size, uint16_t color)  {
	uint8_t dots;
	if ((c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z')) {
		c &= 0x1F;   // A-Z maps to 1-26
	}
	else if (c >= '0' && c <= '9') {
		c = (c - '0') + 27;
	}
	else if (c == ' ') {
		c = 0; // space
	}
	else if (c == '#'){
		c=37;
	}
	else if (c=='/'){
		c=37;
	}

	switch(font_size)
	{
	case 51:  // font size 3x5  ascii value of 3: 51

		if(c==':'){
			matrix.drawPixel(x+1,y+1,color);
			matrix.drawPixel(x+1,y+3,color);
		}
		else if(c=='-'){
			matrix.drawLine(x,y+2,3,0,color);
		}
		else if(c=='.'){
			matrix.drawPixel(x+1,y+2,color);
		}
		else if(c==39 || c==44){
			matrix.drawLine(x+1,y,2,0,color);
			matrix.drawPixel(x+2,y+1,color);
		}
		else{
			for (uint8_t row=0; row< 5; row++) {
//				dots = pgm_read_byte_near(&font3x5[(uint8_t)c][row]);
				dots = font3x5[(uint8_t)c][row];
				for (uint8_t col=0; col < 3; col++) {
					int x1=x;
					int y1=y;
					if (dots & (4>>col))
					matrix.drawPixel(x1+col, y1+row, color);
				}
			}
		}
		break;

	case 53:  // font size 5x5   ascii value of 5: 53

		if(c==':'){
			matrix.drawPixel(x+2,y+1,color);
			matrix.drawPixel(x+2,y+3,color);
		}
		else if(c=='-'){
			matrix.drawLine(x+1,y+2,3,0,color);
		}
		else if(c=='.'){
			matrix.drawPixel(x+2,y+2,color);
		}
		else if(c==39 || c==44){
			matrix.drawLine(x+2,y,2,0,color);
			matrix.drawPixel(x+4,y+1,color);
		}
		else{
			for (uint8_t row=0; row< 5; row++) {
//				dots = pgm_read_byte_near(&font5x5[(uint8_t)c][row]);
				dots = font5x5[(uint8_t)c][row];
				for (uint8_t col=0; col < 5; col++) {
					int x1=x;
					int y1=y;
					if (dots & (64>>col))  // For some wierd reason I have the 5x5 font in such a way that.. last two bits are zero..
					matrix.drawPixel(x1+col, y1+row, color);
				}
			}
		}

		break;
	default:
		break;
	}
}


//Draw number n, with x,y as top left corner, in chosen color, scaled in x and y.
//when scale_x, scale_y = 1 then character is 3x5
void vectorNumber(int n, int x, int y, int color, float scale_x, float scale_y) {
	switch (n) {
	case 0:
		matrix.drawLine(x ,y , x , y+(4*scale_y) , color);
		matrix.drawLine(x , y+(4*scale_y) , x+(2*scale_x) , y+(4*scale_y), color);
		matrix.drawLine(x+(2*scale_x) , y , x+(2*scale_x) , y+(4*scale_y) , color);
		matrix.drawLine(x ,y , x+(2*scale_x) , y , color);
		break;
	case 1:
		matrix.drawLine( x+(1*scale_x), y, x+(1*scale_x),y+(4*scale_y), color);
		matrix.drawLine(x , y+4*scale_y , x+2*scale_x , y+4*scale_y,color);
		matrix.drawLine(x,y+scale_y, x+scale_x, y,color);
		break;
	case 2:
		matrix.drawLine(x ,y , x+2*scale_x , y , color);
		matrix.drawLine(x+2*scale_x , y , x+2*scale_x , y+2*scale_y , color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y , x , y+2*scale_y, color);
		matrix.drawLine(x , y+2*scale_y, x , y+4*scale_y,color);
		matrix.drawLine(x , y+4*scale_y , x+2*scale_x , y+4*scale_y,color);
		break;
	case 3:
		matrix.drawLine(x ,y , x+2*scale_x , y , color);
		matrix.drawLine(x+2*scale_x , y , x+2*scale_x , y+4*scale_y , color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y , x+scale_x , y+2*scale_y, color);
		matrix.drawLine(x , y+4*scale_y , x+2*scale_x , y+4*scale_y,color);
		break;
	case 4:
		matrix.drawLine(x+2*scale_x , y , x+2*scale_x , y+4*scale_y , color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y , x , y+2*scale_y, color);
		matrix.drawLine(x ,y , x , y+2*scale_y , color);
		break;
	case 5:
		matrix.drawLine(x ,y , x+2*scale_x , y , color);
		matrix.drawLine(x , y , x , y+2*scale_y , color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y , x , y+2*scale_y, color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y, x+2*scale_x , y+4*scale_y,color);
		matrix.drawLine( x , y+4*scale_y , x+2*scale_x , y+4*scale_y,color);
		break;
	case 6:
		matrix.drawLine(x ,y , x , y+(4*scale_y) , color);
		matrix.drawLine(x ,y , x+2*scale_x , y , color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y , x , y+2*scale_y, color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y, x+2*scale_x , y+4*scale_y,color);
		matrix.drawLine(x+2*scale_x , y+4*scale_y , x, y+(4*scale_y) , color);
		break;
	case 7:
		matrix.drawLine(x ,y , x+2*scale_x , y , color);
		matrix.drawLine( x+2*scale_x, y, x+scale_x,y+(4*scale_y), color);
		break;
	case 8:
		matrix.drawLine(x ,y , x , y+(4*scale_y) , color);
		matrix.drawLine(x , y+(4*scale_y) , x+(2*scale_x) , y+(4*scale_y), color);
		matrix.drawLine(x+(2*scale_x) , y , x+(2*scale_x) , y+(4*scale_y) , color);
		matrix.drawLine(x ,y , x+(2*scale_x) , y , color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y , x , y+2*scale_y, color);
		break;
	case 9:
		matrix.drawLine(x ,y , x , y+(2*scale_y) , color);
		matrix.drawLine(x , y+(4*scale_y) , x+(2*scale_x) , y+(4*scale_y), color);
		matrix.drawLine(x+(2*scale_x) , y , x+(2*scale_x) , y+(4*scale_y) , color);
		matrix.drawLine(x ,y , x+(2*scale_x) , y , color);
		matrix.drawLine(x+2*scale_x , y+2*scale_y , x , y+2*scale_y, color);
		break;
	}
}


void flashing_cursor(byte xpos, byte ypos, byte cursor_width, byte cursor_height, byte repeats) {
	for (byte r = 0; r <= repeats; r++) {
		matrix.fillRect(xpos,ypos,cursor_width, cursor_height, matrix.Color333(0,3,0));
		matrix.swapBuffers(true);

		if (repeats > 0) {
			delay(400);
		}
		else {
			delay(70);
		}

		matrix.fillRect(xpos,ypos,cursor_width, cursor_height, matrix.Color333(0,0,0));
		matrix.swapBuffers(true);

		//if cursor set to repeat, wait a while
		if (repeats > 0) {
			delay(400);
		}
		bgProcess();	//Give the background process some lovin'
	}
}


void update_last() {
	hour_last = Time.hour();
	minute_last = Time.minute();
	second_last = Time.second();
}
