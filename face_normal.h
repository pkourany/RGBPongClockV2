
#include "application.h"
#include "face_class.h"

class face_normal : public FaceClass
{
  public:

	// Constructor - face gets display size from GFX function
	face_normal()
	
	void begin(void);				// Reset face for running
    void run(void);					// Update the face animation
	int isComplete(void);			// True if animation completed, false if not, -1 if not applicable
	
  private:
	byte hours, mins;

	int  msHourPosition, lsHourPosition, msMinPosition, lsMinPosition;
	int  msLastHourPosition, lsLastHourPosition, msLastMinPosition, lsLastMinPosition;
	int c1, c2, c3, c4;
	
	//unsigned long showtime;
	
	float scale_x, scale_y;
	char lastHourBuffer[3], lastMinBuffer[3];

};
