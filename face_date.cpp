#ifdef FACE_DATE

void display_date() {
	uint16_t color = matrix.Color333(0,1,0);
	cls();
	matrix.swapBuffers(true);
	//read the date from the DS1307
	//it returns the month number, day number, and a number representing the day of week - 1 for Tue, 2 for Wed 3 for Thu etc.
	byte dow = Time.weekday()-1;		//we  take one off the value the DS1307 generates, as our array of days is 0-6 and the DS1307 outputs  1-7.
	byte date = Time.day();
	byte mont = Time.month()-1;

	//array of day and month names to print on the display. Some are shortened as we only have 8 characters across to play with
	char daynames[7][9]={"Sunday", "Monday","Tuesday", "Wed", "Thursday", "Friday", "Saturday"};
	char monthnames[12][9]={"January", "February", "March", "April", "May", "June", "July", "August", "Sept", "October", "November", "December"};

	//call the flashing cursor effect for one blink at x,y pos 0,0, height 5, width 7, repeats 1
	flashing_cursor(0,0,3,5,1);

	//print the day name
	int i = 0;
	while(daynames[dow][i])
	{
		flashing_cursor(i*4,0,3,5,0);
		drawChar(i*4,0,daynames[dow][i],51,color);
		matrix.swapBuffers(true);
		i++;
	}

	//pause at the end of the line with a flashing cursor if there is space to print it.
	//if there is no space left, dont print the cursor, just wait.
	if (i*4 < 32){
		flashing_cursor(i*4,0,3,5,1);
	}
	else {
		bgProcess();	//Give the background process some lovin'
		delay(300);
	}

	//flash the cursor on the next line
	flashing_cursor(0,8,3,5,0);

	//print the date on the next line: First convert the date number to chars
	char buffer[3];
	itoa(date,buffer,10);

	//then work out date 2 letter suffix - eg st, nd, rd, th etc
	char suffix[4][3]={"st", "nd", "rd", "th"};
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

	//print the 1st date number
	drawChar(0,8,buffer[0],51,color);
	matrix.swapBuffers(true);

	//if date is under 10 - then we only have 1 digit so set positions of sufix etc one character nearer
	byte suffixposx = 4;

	//if date over 9 then print second number and set xpos of suffix to be 1 char further away
	if (date > 9){
		suffixposx = 8;
		flashing_cursor(4,8,3,5,0);
		drawChar(4,8,buffer[1],51,color);
		matrix.swapBuffers(true);
	}

	//print the 2 suffix characters
	flashing_cursor(suffixposx, 8,3,5,0);
	drawChar(suffixposx,8,suffix[s][0],51,color);
	matrix.swapBuffers(true);

	flashing_cursor(suffixposx+4,8,3,5,0);
	drawChar(suffixposx+4,8,suffix[s][1],51,color);
	matrix.swapBuffers(true);

	//blink cursor after
	flashing_cursor(suffixposx + 8,8,3,5,1);

	//replace day name with date on top line - effectively scroll the bottom line up by 8 pixels
	for(int q = 8; q>=0; q--){
		cls();
		int w =0 ;
		while(daynames[dow][w])
		{
			drawChar(w*4,q-8,daynames[dow][w],51,color);

			w++;
		}

		matrix.swapBuffers(true);
		//date first digit
		drawChar(0,q,buffer[0],51,color);
		//date second digit - this may be blank and overwritten if the date is a single number
		drawChar(4,q,buffer[1],51,color);
		//date suffix
		drawChar(suffixposx,q,suffix[s][0],51,color);
		//date suffix
		drawChar(suffixposx+4,q,suffix[s][1],51,color);
		matrix.swapBuffers(true);
		delay(50);
	}
	//flash the cursor for a second for effect
	flashing_cursor(suffixposx + 8,0,3,5,0);

	//print the month name on the bottom row
	i = 0;
	while(monthnames[mont][i])
	{
		flashing_cursor(i*4,8,3,5,0);
		drawChar(i*4,8,monthnames[mont][i],51,color);
		matrix.swapBuffers(true);
		i++;

	}

	//blink the cursor at end if enough space after the month name, otherwise juts wait a while
	if (i*4 < 32){
		flashing_cursor(i*4,8,3,5,2);
	}
	else {
		delay(1000);
	}

	for(int q = 8; q>=-8; q--){
		cls();
		int w =0 ;
		while(monthnames[mont][w])
		{
			drawChar(w*4,q,monthnames[mont][w],51,color);

			w++;
		}

		matrix.swapBuffers(true);
		//date first digit
		drawChar(0,q-8,buffer[0],51,color);
		//date second digit - this may be blank and overwritten if the date is a single number
		drawChar(4,q-8,buffer[1],51,color);
		//date suffix
		drawChar(suffixposx,q-8,suffix[s][0],51,color);
		//date suffix
		drawChar(suffixposx+4,q-8,suffix[s][1],51,color);
		matrix.swapBuffers(true);
		delay(50);
	}
}
#endif