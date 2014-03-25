#include "ardrone/ardrone.h"
#include "Leap.h"

// --------------------------------------------------------------------------
// main(Number of arguments, Argument values)
// Description  : This is the entry point of the program.
// Return value : SUCCESS:0  ERROR:-1
// --------------------------------------------------------------------------

int main(int argc, char **argv)
{
	static bool mLeapDebugPrint = true;
	static bool mLeapnot = false;
	static bool mNonDronDebug = true;
	static bool mTakOffFlag = false;
	
	float pitch = 0;	//前p：-0.5　後p： 0.9
	float yaw = 0;		//左y：-1.0　右y： 0.7
	float roll = 0;		//左R： 0.8　右R：-1.0

	float PosX = 0;		//左右　　　左  -150　～　右 150
	float PosY = 0;		//上下昇降　下    50　～　上 300
	float PosZ = 0;		//前後　    手前-100　～　奥 100


	// AR.Drone class
    ARDrone ardrone;

	if(mNonDronDebug == false)
	{ 
		// Initialize
		if (!ardrone.open()) {
			printf("Failed to initialize.\n");
			return -1;
		}
	}

    // Battery
    printf("Battery = %d%%\n", ardrone.getBatteryPercentage());

    // Instructions
    printf("***************************************\n");
    printf("*       CV Drone sample program       *\n");
    printf("*           - How to Play -           *\n");
    printf("***************************************\n");
    printf("*                                     *\n");
    printf("* - Controls -                        *\n");
    printf("*    'Space' -- Takeoff/Landing       *\n");
    printf("*    'Up'    -- Move forward          *\n");
    printf("*    'Down'  -- Move backward         *\n");
    printf("*    'Left'  -- Turn left             *\n");
    printf("*    'Right' -- Turn right            *\n");
    printf("*    'Q'     -- Move upward           *\n");
    printf("*    'A'     -- Move downward         *\n");
    printf("*                                     *\n");
    printf("* - Others -                          *\n");
    printf("*    'C'     -- Change camera         *\n");
    printf("*    'Esc'   -- Exit                  *\n");
    printf("*                                     *\n");
    printf("***************************************\n\n");

		//2014.01.15 KN
	Leap::Controller leapController;

    while (1) 
	{
        // Key input
        int key = cvWaitKey(33);
		if (key == 0x1b) break;

        // Update
        if (!ardrone.update()) break;

        // Get an image
        IplImage *image = ardrone.getImage();

        //2014.01.15 add
		if(mLeapnot != true)
		{
			if( leapController.isConnected()) //controller is a Controller object
			{
				//Frame previous = leapController.frame(1); //The previous frame
				Leap::Frame frame = leapController.frame(); // controller is a Leap::Controller object
				Leap::HandList hands = frame.hands();
				Leap::Hand firstHand = hands[0];
				pitch = firstHand.direction().pitch();//前p：-0.5　後p： 0.9
				yaw = firstHand.direction().yaw();    //左y：-1.0　右y： 0.7
				roll = firstHand.palmNormal().roll(); //左R： 0.8　右R：-1.0

				PosX = frame.pointables().leftmost().tipPosition().x;       //左右　　　左-150　～　右 150
				PosY = frame.pointables().leftmost().tipPosition().y;       //上下昇降　下  50　～　上 300
				PosZ = frame.pointables().leftmost().tipPosition().z * (-1);//前後　  手前-100　～　奥 100

				if(mLeapDebugPrint == true)
				{
					printf("X:%03.03f Y:%03.03f Z:%03.03f p:%03.03f y:%03.03f r:%03.03f\n",PosX,PosY,PosZ,pitch,yaw,roll);
				}
			}
		}

        // Take off / Landing 
        if (key == ' ') 
		{
			if (ardrone.onGround())
			{
				ardrone.takeoff();
				mTakOffFlag = true;
			}else
			{
				ardrone.landing();
				mTakOffFlag = false;
			}
        }

        // Move
        double vx = 0.0, vy = 0.0, vz = 0.0, vr = 0.0;
        if (key == 0x260000) vx =  1.0;
        if (key == 0x280000) vx = -1.0;
        if (key == 0x250000) vr =  1.0;
        if (key == 0x270000) vr = -1.0;
        if (key == 'q')      vz =  1.0;
        if (key == 'a')      vz = -1.0;

		//LeapMotion Value set
		if((PosY < 55) && (mTakOffFlag == true))
		{
			if (!ardrone.onGround())
			{
				ardrone.landing();
				mTakOffFlag = false;
			}
		}

		if((PosY > 200) && (mTakOffFlag == false))
		{
			if (ardrone.onGround()){
				ardrone.takeoff();
				mTakOffFlag = true;
			}
		}

		if(pitch < -0.5){
			vx =  1.0;
		}

		if(pitch > 0.8){
			vx =  -1.0;
		}

		if(yaw < -1.0){
			vr = 1.0;
		}
		if(yaw > 0.7){
			vr = -1.0;
		}

		if(roll < -0.8){
			vy = 1.0;
		}
		if(roll > 0.8){
			vy = -1.0;
		}

        ardrone.move3D(vx, vy, vz, vr);

        // Change camera
        static int mode = 0;
        if (key == 'c') ardrone.setCamera(++mode%4);

        // Display the image
        cvShowImage("camera", image);
        //cvMoveWindow( "camera", 50, 0 );
    }

    // See you
    ardrone.close();
    return 0;
}
