#ifdef FACE_WEATHER

int stringPos;

// Function prototypes
void quickWeather();
void getWeather();
void processWeather(const char *name, const char *data);
void showWeather();
void drawWeatherIcon(uint8_t x, uint8_t y, int id);
void drawDayOfWeek();


static const uint8_t cloud[] PROGMEM = {
	0x0 , 0x0 , 0x0 , 0xe0, 0x1 , 0xf8, 0x3b, 0xfc, 0x7f, 0xfe, 0xff, 0xff, 0xff,
	0xff, 0x7f, 0xff, 0x3f, 0xfe, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};

static const uint8_t small_sun[] PROGMEM= {
	0x0 , 0x0 , 0xe , 0x0 , 0x3f, 0x80, 0x3f, 0x80, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f,
	0xc0, 0x3f, 0x80, 0x3f, 0x80, 0xe , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};

static const uint8_t big_sun[] PROGMEM= {
	0x0 , 0x0 , 0x81, 0x2 , 0x41, 0x4 , 0x20, 0x8 , 0x3 , 0x80, 0xf , 0xe0, 0xf ,
	0xe0, 0x1f, 0xf0, 0xdf, 0xf6, 0x1f, 0xf0, 0xf , 0xe0, 0xf , 0xe0, 0x3 , 0x80,
	0x20, 0x8 , 0x41, 0x4 , 0x81, 0x2 ,
};

static const uint8_t lightning[] PROGMEM = {
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x1 , 0xc0, 0x0 , 0xe0, 0x0 , 0x70, 0x0 , 0xe0,
	0x1 , 0xc0, 0x3 , 0x0 , 0x2 , 0x0 ,
};

static const uint8_t cloud_outline[] PROGMEM= {
	0x0 , 0x0 , 0x0 , 0xe0, 0x1 , 0x18, 0x3a, 0x4 , 0x4e, 0x2 , 0x80, 0x1 , 0x80,
	0x1 , 0x40, 0x1 , 0x3f, 0xfe, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
};


/***** Weather webhook definitions *****/
#define HOOK_RESP		"hook-response/rgbpc_weather_hook"
#define HOOK_PUB		"rgbpc_weather_hook"
/***************************************/


bool weatherGood = false;
int badWeatherCall = 0;
char w_temp[8][7] = {""};
char w_id[8][4] = {""};

/*** Set default city for weather webhook ***/
char city[40] = WEATHER_CITY;

boolean wasWeatherShownLast = true;
unsigned long lastWeatherTime = 0;

uint32_t weather_expire = 15 * 60 * 1000;
uint32_t weather_refresh = 0;

bool weather_hook_has_published = false;
uint32_t weather_hook_retry = 0;
uint32_t display_weather_icon = 0;


static char daynames[7][4] = {
	"Sun", "Mon","Tue", "Wed", "Thu", "Fri", "Sat"
};

uint8_t weather_dow = 0;

void weather_setup() {
	// Lets listen for the hook response
	Particle.subscribe(HOOK_RESP, processWeather, MY_DEVICES);
	Particle.process();

	Particle.publish(HOOK_PUB, city, 60, PRIVATE);
	Particle.process();

	weather_hook_has_published = true;
	weather_hook_retry = millis() + 3000;
}


void weather() {
	// If the weather is still good and we've hit our timer, reset weatherGood
	// to force another update
	if(weatherGood && (weather_refresh==0 || millis() > weather_refresh))
		weatherGood = false;

	// If the weather isn't good, we've published already, and still don't have
	// anything back, retry the publish
	if(!weatherGood && millis() > weather_hook_retry) {
		badWeatherCall++;
		weather_hook_has_published = false;
	}

	// If we've failed the hook too many times, reset the MCU and start over
	if(!weatherGood && badWeatherCall>3) {
		System.reset();
	} else {
		badWeatherCall = 0;
	}

	// If the weather isn't good
	if(!weatherGood && !weather_hook_has_published) {
		Particle.publish(HOOK_PUB, city, 60, PRIVATE);
		weather_hook_has_published = true;
		weather_hook_retry = millis() + 3000;
	} else if(weatherGood)
		showWeather();
}


void processWeather(const char *name, const char *data) {
	weatherGood = true;
	lastWeatherTime = millis();
	stringPos = strlen((const char *)data);

	memset(&w_temp,0,8*7);
	memset(&w_id,0,8*4);
	int dayCounter =0;
	int itemCounter = 0;
	int tempStringLoc=0;
	boolean dropChar = false;

	for (int i=0; i<stringPos; i++){
		if(data[i]=='~'){
			itemCounter++;
			tempStringLoc = 0;
			dropChar = false;
			if(itemCounter>1){
				dayCounter++;
				itemCounter=0;
			}
		}
		else if(data[i]=='.' || data[i]=='"'){
			//if we get a . we want to drop all characters until the next ~
			dropChar=true;
		}
		else{
			if(!dropChar){
				switch(itemCounter){
				case 0:
					w_temp[dayCounter][tempStringLoc++] = data[i];
					break;
				case 1:
					w_id[dayCounter][tempStringLoc++] = data[i];
					break;
				}
			}
		}
	}

	weather_refresh = millis() + weather_expire;
}


void showWeather() {
	//matrix.swapBuffers(true);

	if(millis() >= display_weather_icon) {
		weather_dow++;

		if(weather_dow>=7)
			weather_dow = 0;

		display_weather_icon = millis()+5000;
		cls();
	} else {
		drawWeatherIcon(16,0,atoi(w_id[weather_dow]));
		drawDayOfWeek();
		matrix.swapBuffers(true);
	}
}


void drawDayOfWeek() {
	byte dow = Time.weekday()-1;

	int numTemp = atoi(w_temp[weather_dow]);

	//fix within range to generate colour value
	if (numTemp < -14) numTemp = -10;
	if (numTemp > 34) numTemp = 30;

	//add 14 so it falls between 0 and 48
	numTemp = numTemp +14;

	//divide by 3 so value between 0 and 16
	numTemp = numTemp / 3;

	int tempColor;
	if(numTemp<8) {
		tempColor = matrix.Color444(0,tempColor/2,7);
	} else {
		tempColor = matrix.Color444(7,(7-numTemp/2) ,0);
	}

	//Display the day on the top line.
	if(weather_dow==0) {
		drawString(2,2,(char*)"Now",51,matrix.Color444(1,1,1));
	} else {
		drawString(2,2,daynames[(dow+weather_dow-1) % 7],51,matrix.Color444(0,1,0));
	}

	//put the temp underneath
	boolean positive = !(w_temp[weather_dow][0]=='-');
	for(int t=0; t<7; t++) {
		if(w_temp[weather_dow][t]=='-') {
			matrix.drawLine(3,10,4,10,tempColor);
		} else if(!(w_temp[weather_dow][t]==0)) {
			vectorNumber(w_temp[weather_dow][t]-'0',t*4+2+(positive*2),8,tempColor,1,1);
		}
	}


}


void drawWeatherIcon(uint8_t x, uint8_t y, int id) {
	unsigned long start = millis();
	static int rain[12];

	for(int r=0; r<13; r++){
		rain[r]=random(9,15);
	}

	int rainColor = matrix.Color333(0,0,1);
	byte intensity=id-(id/10)*10 + 1;

	int deep = 0;
	boolean raining = false;

	switch(id/100) {
		case 2:
			//Thunder
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,0));
			matrix.drawBitmap(x,y,cloud_outline,16,16,matrix.Color333(1,1,1));
			if(random(0,10)==3){
				int pos = random(-5,5);
				matrix.drawBitmap(pos+x,y,lightning,16,16,matrix.Color333(1,1,1));
			}
			raining = true;
			break;
		case 3:
			//drizzle
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,0));
			matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
			raining=true;
			break;
		case 5:
			//rain was 5
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,0));

			if(intensity<3){
				matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
			}
			else{
				matrix.drawBitmap(x,y,cloud_outline,16,16,matrix.Color333(1,1,1));
			}
			raining = true;
			break;
		case 6:
			//snow was 6
			rainColor = matrix.Color333(4,4,4);
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,0));

			deep = (millis()-start)/500;
			if(deep>6) deep=6;

			if(intensity<3){
				matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
				matrix.fillRect(x,y+16-deep/2,16,deep/2,rainColor);
			}
			else{
				matrix.drawBitmap(x,y,cloud_outline,16,16,matrix.Color333(1,1,1));
				matrix.fillRect(x,y+16-(deep),16,deep,rainColor);
			}
			raining = true;
			break;
		case 7:
			//atmosphere
			matrix.drawRect(x,y,16,16,matrix.Color333(1,0,0));
			drawString(x+2,y+6,(char*)"FOG",51,matrix.Color333(1,1,1));
			break;
		case 8:
			//cloud
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,1));
			if(id==800){
				matrix.drawBitmap(x,y,big_sun,16,16,matrix.Color333(2,2,0));
			}
			else{
				if(id==801){
					matrix.drawBitmap(x,y,big_sun,16,16,matrix.Color333(2,2,0));
					matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
				}
				else{
					if(id==802 || id ==803){
						matrix.drawBitmap(x,y,small_sun,16,16,matrix.Color333(1,1,0));
					}
					matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
					matrix.drawBitmap(x,y,cloud_outline,16,16,matrix.Color333(0,0,0));
				}
			}
			break;
		case 9:
			//extreme
			matrix.fillRect(x,y,16,16,matrix.Color333(0,0,0));
			matrix.drawRect(x,y,16,16,matrix.Color333(7,0,0));
			if(id==906){
				raining =true;
				intensity=3;
				matrix.drawBitmap(x,y,cloud,16,16,matrix.Color333(1,1,1));
			};
			break;
		default:
			matrix.fillRect(x,y,16,16,matrix.Color333(0,1,1));
			matrix.drawBitmap(x,y,big_sun,16,16,matrix.Color333(2,2,0));
			break;
	}

	if(raining) {
		for(int r = 0; r<13; r++) {
			matrix.drawPixel(x+r+2, rain[r]++, rainColor);
			if(rain[r]==16) rain[r]=9;
		}
	}

	// matrix.swapBuffers(true);

	delay(( 50 -( intensity * 10 )) < 0 ? 0: 50-intensity*10);
}

#endif