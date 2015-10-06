#ifndef _FACE_CLASS_H
#define _FACE_CLASS_H

#include "application.h"

class FaceClass
{
  public:
  	FaceClass() { }
	virtual ~FaceClass() = 0;
	
	// These must be defined by the subclass
	virtual void begin(void) = 0;				// Reset face for running
    virtual void run(void) = 0;					// Update the face animation
	virtual int isComplete(void) = 0;			// True if animation completed, false if not, -1 if not applicable
	
//  private:
//	byte hours, mins;
};

#endif
