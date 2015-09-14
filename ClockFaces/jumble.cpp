#ifdef FACE_JUMBLE

//show time and date and use a random jumble of letters transition each time the time changes.
void jumble() {

	char days[7][4] = {
		"SUN","MON","TUE", "WED", "THU", "FRI", "SAT"                  }; //DS1307 outputs 1-7
	char allchars[37] = {
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"                  };
	char endchar[16];
	byte counter[16];
	byte mins = Time.minute();
	byte seq[16];

	DEBUGpln("in Jumble");
	cls();

	for (int show = 0; show < SHOWCLOCK ; show++) {
		if (mode_changed == 1)
			return;
			
		if(mode_quick){
			mode_quick = false;
			display_date();

#ifdef FACE_WEATHER
			quickWeather();
#endif
			jumble();
			return;
		}

		if ( show == 0 || mins != Time.minute()  ) {
			//fill an arry with 0-15 and randomize the order so we can plot letters in a jumbled pattern rather than sequentially
			for (int i=0; i<16; i++) {
				seq[i] = i;  // fill the array in order
			}
			//randomise array of numbers
			for (int i=0; i<(16-1); i++) {
				int r = i + (rand() % (16-i)); // Random remaining position.
				int temp = seq[i];
				seq[i] = seq[r];
				seq[r] = temp;
			}

			//reset these for comparison next time
			mins = Time.minute();
			byte hours = Time.hour();
			byte dow   = Time.weekday() - 1; // the DS1307 outputs 1 - 7.
			byte date  = Time.day();

			byte alldone = 0;

			//set counters to 50
			for(byte c=0; c<16 ; c++) {
				counter[c] = 3 + random (0,20);
			}

			//set final characters
			char buffer[3];
			itoa(hours,buffer,10);

			//fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
			if (hours < 10) {
				buffer[1] = buffer[0];
				buffer[0] = '0';
			}

			endchar[0] = buffer[0];
			endchar[1] = buffer[1];
			endchar[2] = ':';

			itoa (mins, buffer, 10);
			if (mins < 10) {
				buffer[1] = buffer[0];
				buffer[0] = '0';
			}

			endchar[3] = buffer[0];
			endchar[4] = buffer[1];

			itoa (date, buffer, 10);
			if (date < 10) {
				buffer[1] = buffer[0];
				buffer[0] = '0';
			}

			//then work out date 2 letter suffix - eg st, nd, rd, th etc
			char suffix[4][3]={
				"st", "nd", "rd", "th"                                                      };
			byte s = 3;
			if(date == 1 || date == 21 || date == 31) {
				s = 0;
			}
			else if (date == 2 || date == 22) {
				s = 1;
			}
			else if (date == 3 || date == 23) {
				s = 2;
			}
			//set topline
			endchar[5] = ' ';
			endchar[6] = ' ';
			endchar[7] = ' ';

			//set bottom line
			endchar[8] = days[dow][0];
			endchar[9] = days[dow][1];
			endchar[10] = days[dow][2];
			endchar[11] = ' ';
			endchar[12] = buffer[0];
			endchar[13] = buffer[1];
			endchar[14] = suffix[s][0];
			endchar[15] = suffix[s][1];

			byte x = 0;
			byte y = 0;

			//until all counters are 0
			while (alldone < 16){

				//for each char
				for(byte c=0; c<16 ; c++) {

					if (seq[c] < 8) {
						x = 0;
						y = 0;
					}
					else {
						x = 8;
						y = 8;
					}

					//if counter > 1 then put random char
					if (counter[ seq[c] ] > 1) {
						matrix.fillRect((seq[c]-x)*4,y,3,5,matrix.Color333(0,0,0));
						drawChar((seq[c] - x) *4, y, allchars[random(0,36)],51,matrix.Color444(1,0,0));
						counter[ seq[c] ]--;
						matrix.swapBuffers(true);
					}

					//if counter == 1 then put final char
					if (counter[ seq[c] ] == 1) {
						matrix.fillRect((seq[c]-x)*4,y,3,5,matrix.Color444(0,0,0));
						drawChar((seq[c] - x) *4, y, endchar[seq[c]],51,matrix.Color444(0,0,1));
						counter[seq[c]] = 0;
						alldone++;
						matrix.swapBuffers(true);
					}

					//if counter == 0 then just pause to keep update rate the same
					if (counter[seq[c]] == 0) {
						delay(4);
					}

					if (mode_changed == 1)
					return;
				}
				bgProcess();//Give the background process some lovin'
			}
		}
		delay(50);
	} //showclock
}

#endif