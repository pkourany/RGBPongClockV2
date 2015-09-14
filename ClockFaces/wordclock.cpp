#ifdef FACE_WORDCLOCK

//print a clock using words rather than numbers
void word_clock() {
	if(Time.hour()==hour_last && Time.minute()==minute_last && has_shown==1) {
		return;
	}

	update_last();

	uint8_t hours = Time.hourFormat12();
	uint8_t mins  = Time.minute();

	char numbers[19][10]   = {
		"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten",
		"eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "7teen", "8teen", "nineteen"
	};
	char numberstens[5][7] = {
		"ten", "twenty", "thirty", "forty", "fifty"
	};

	//split mins value up into two separate digits
	int minsdigit = mins % 10;
	byte minsdigitten = (mins / 10) % 10;

	char str_top[8];
	char str_bot[8];
	char str_mid[8];

	//if mins <= 10 , then top line has to read "minsdigti past" and bottom line reads hours
	if (mins < 10) {
		strcpy (str_top,numbers[minsdigit - 1]);
		strcpy (str_mid,"PAST");
		strcpy (str_bot,numbers[hours - 1]);
	}
	//if mins = 10, cant use minsdigit as above, so soecial case to print 10 past /n hour.
	if (mins == 10) {
		strcpy (str_top,numbers[9]);
		strcpy (str_mid,"PAST");
		strcpy (str_bot,numbers[hours - 1]);
	}

	//if time is not on the hour - i.e. both mins digits are not zero,
	//then make top line read "hours" and bottom line ready "minstens mins" e.g. "three /n twenty one"
	else if (minsdigitten != 0 && minsdigit != 0) {
		strcpy (str_top,numbers[hours - 1]);

		//if mins is in the teens, use teens from the numbers array for the bottom line, e.g. "three /n fifteen"
		if (mins >= 11 && mins <= 19) {
			strcpy (str_bot, numbers[mins - 1]);
			strcpy(str_mid," ");
			//else bottom line reads "minstens mins" e.g. "three \n twenty three"
		}
		else {
			strcpy (str_mid, numberstens[minsdigitten - 1]);
			strcpy (str_bot, numbers[minsdigit -1]);
		}
	}
	// if mins digit is zero, don't print it. read read "hours" "minstens" e.g. "three /n twenty"
	else if (minsdigitten != 0 && minsdigit == 0  ) {
		strcpy (str_top, numbers[hours - 1]);
		strcpy (str_bot, numberstens[minsdigitten - 1]);
		strcpy (str_mid, " " );
	}

	//if both mins are zero, i.e. it is on the hour, the top line reads "hours" and bottom line reads "o'clock"
	else if (minsdigitten == 0 && minsdigit == 0  ) {
		strcpy (str_top,numbers[hours - 1]);
		strcpy (str_bot, "O'CLOCK");
		strcpy (str_mid, " ");
	}

	//work out offset to center top line on display.
	byte lentop = 0;
	while(str_top[lentop]) {
		lentop++;
	}; //get length of message

	byte offset_top;
	if(lentop<6) {
		offset_top = (X_MAX - ((lentop*6)-1)) / 2;
	} else {
		offset_top = (X_MAX - ((lentop - 1)*4)) / 2;
	}

	//work out offset to center bottom line on display.
	byte lenbot = 0;
	while(str_bot[lenbot]) {
		lenbot++;
	}; //get length of message

	byte offset_bot;
	if(lenbot<6) {
		offset_bot = (X_MAX - ((lenbot*6)-1)) / 2;
	} else {
		offset_bot = (X_MAX - ((lenbot - 1)*4)) / 2;
	}

	byte lenmid = 0;
	while(str_mid[lenmid]) {
		lenmid++;
	}; //get length of message

	byte offset_mid;
	if(lenmid<6) {
		offset_mid = (X_MAX - ((lenmid*6)-1)) / 2;
	} else {
		offset_mid = (X_MAX - ((lenmid - 1)*4)) / 2;
	}

	cls();

	drawString(offset_top,(lenmid>1?0:2),str_top,(lentop<6?53:51),matrix.Color333(0,1,5));

	if(lenmid>1) {
		drawString(offset_mid,5,str_mid,(lenmid<6?53:51),matrix.Color333(1,1,5));
	}

	drawString(offset_bot,(lenmid>1?10:8),str_bot,(lenbot<6?53:51),matrix.Color333(0,5,1));

	matrix.swapBuffers(false);
}

#endif