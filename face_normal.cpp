#ifdef FACE_NORMAL

#include "face_normal.h"


void face_normal::face_normal()
{
	hours = 0;
	mins = 0;
}

void face_normal::begin() {
	matrix.setTextWrap(false); // Allow text to run off right edge
	matrix.setTextSize(2);
	matrix.setTextColor(matrix.Color333(2, 3, 2));

	cls();
	
	hours = Time.hour();
	mins = Time.minute();

	msHourPosition = 0;
	lsHourPosition = 0;
	msMinPosition = 0;
	lsMinPosition = 0;
	msLastHourPosition = 0;
	lsLastHourPosition = 0;
	msLastMinPosition = 0;
	lsLastMinPosition = 0;

	//Start with all characters off screen
	c1 = -17;
	c2 = -17;
	c3 = -17;
	c4 = -17;

	scale_x =2.5;
	scale_y =3.0;

	strcpy(lastHourBuffer, "  ");
	strcpy(lastMinBuffer, "  ");

	//showTime = Time.now();
}

void face_normal::run() {

		cls();

/* Move to loop()
		if (mode_changed == 1)
			return;
		if(mode_quick){
			mode_quick = false;
			display_date();
			quickWeather();
			normal_clock();
			return;
		}
*/

		//udate mins and hours with the new time
		mins = Time.minute();
		hours = Time.hour();

		char buffer[3];

		itoa(hours,buffer,10);
		//fix - as otherwise if num has leading zero, e.g. "03" hours, itoa coverts this to chars with space "3 ".
		if (hours < 10) {
			buffer[1] = buffer[0];
			buffer[0] = '0';
		}

		if(lastHourBuffer[0]!=buffer[0] && c1==0) c1= -17;
		if( c1 < 0 )c1++;
		msHourPosition = c1;
		msLastHourPosition = c1 + 17;

		if(lastHourBuffer[1]!=buffer[1] && c2==0) c2= -17;
		if( c2 < 0 )c2++;
		lsHourPosition = c2;
		lsLastHourPosition = c2 + 17;

		//update the display
		//shadows first
		vectorNumber((lastHourBuffer[0]-'0'), 2, 2+msLastHourPosition, matrix.Color444(0,0,1),scale_x,scale_y);
		vectorNumber((lastHourBuffer[1]-'0'), 9, 2+lsLastHourPosition, matrix.Color444(0,0,1),scale_x,scale_y);
		vectorNumber((buffer[0]-'0'), 2, 2+msHourPosition, matrix.Color444(0,0,1),scale_x,scale_y);
		vectorNumber((buffer[1]-'0'), 9, 2+lsHourPosition, matrix.Color444(0,0,1),scale_x,scale_y);

		vectorNumber((lastHourBuffer[0]-'0'), 1, 1+msLastHourPosition, matrix.Color444(1,1,1),scale_x,scale_y);
		vectorNumber((lastHourBuffer[1]-'0'), 8, 1+lsLastHourPosition, matrix.Color444(1,1,1),scale_x,scale_y);
		vectorNumber((buffer[0]-'0'), 1, 1+msHourPosition, matrix.Color444(1,1,1),scale_x,scale_y);
		vectorNumber((buffer[1]-'0'), 8, 1+lsHourPosition, matrix.Color444(1,1,1),scale_x,scale_y);

		if(c1==0) lastHourBuffer[0]=buffer[0];
		if(c2==0) lastHourBuffer[1]=buffer[1];

		matrix.fillRect(16,5,2,2,matrix.Color444(0,0,Time.second()%2));
		matrix.fillRect(16,11,2,2,matrix.Color444(0,0,Time.second()%2));

		matrix.fillRect(15,4,2,2,matrix.Color444(Time.second()%2,Time.second()%2,Time.second()%2));
		matrix.fillRect(15,10,2,2,matrix.Color444(Time.second()%2,Time.second()%2,Time.second()%2));

		itoa (mins, buffer, 10);
		if (mins < 10) {
			buffer[1] = buffer[0];
			buffer[0] = '0';
		}

		if(lastMinBuffer[0]!=buffer[0] && c3==0) c3= -17;
		if( c3 < 0 )c3++;
		msMinPosition = c3;
		msLastMinPosition= c3 + 17;

		if(lastMinBuffer[1]!=buffer[1] && c4==0) c4= -17;
		if( c4 < 0 )c4++;
		lsMinPosition = c4;
		lsLastMinPosition = c4 + 17;

		vectorNumber((buffer[0]-'0'), 19, 2+msMinPosition, matrix.Color444(0,0,1),scale_x,scale_y);
		vectorNumber((buffer[1]-'0'), 26, 2+lsMinPosition, matrix.Color444(0,0,1),scale_x,scale_y);
		vectorNumber((lastMinBuffer[0]-'0'), 19, 2+msLastMinPosition, matrix.Color444(0,0,1),scale_x,scale_y);
		vectorNumber((lastMinBuffer[1]-'0'), 26, 2+lsLastMinPosition, matrix.Color444(0,0,1),scale_x,scale_y);

		vectorNumber((buffer[0]-'0'), 18, 1+msMinPosition, matrix.Color444(1,1,1),scale_x,scale_y);
		vectorNumber((buffer[1]-'0'), 25, 1+lsMinPosition, matrix.Color444(1,1,1),scale_x,scale_y);
		vectorNumber((lastMinBuffer[0]-'0'), 18, 1+msLastMinPosition, matrix.Color444(1,1,1),scale_x,scale_y);
		vectorNumber((lastMinBuffer[1]-'0'), 25, 1+lsLastMinPosition, matrix.Color444(1,1,1),scale_x,scale_y);

		if(c3==0) lastMinBuffer[0]=buffer[0];
		if(c4==0) lastMinBuffer[1]=buffer[1];

		matrix.swapBuffers(false);
		
		//bgProcess();	//Give the background process some lovin'
}

int face_normal::isComplete() {
	return -1;		// normal face never "completes"
}
#endif
