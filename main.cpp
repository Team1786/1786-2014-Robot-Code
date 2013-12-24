#include "WPILib.h"

class main : public IterativeRobot
{
	RobotDrive drivetrain;
	Joystick stick;
	float stickY;
	float stickX;
	int invertDriving;
	bool invertDrivingNext;
	
public:
	main(void):
		//init the Joystick and RobotDrive (numbers refer to ports)
		drivetrain(1,2),
		stick(1)
	{
		invertDriving=1;
	}
	
	void updateJoystick(){
		stickY=stick.GetY()*((-stick.GetTwist()+1)/2); //get Y and multiply by Twist (which is actually Throttle), after making it 0-1
		stickX=stick.GetX()*((-stick.GetTwist()+1)/2); //do the same with X
		if(stick.GetRawButton(1)) invertDrivingNext=true; //check if button pressed and change flag to invert driving when the button is released
		if(!stick.GetRawButton(1)&&invertDrivingNext) //check if button released and flag set
		{
			invertDriving=-invertDriving; //invert driving
			invertDrivingNext=false; //reset flag so that it doesn't keep switching
		}
	}
	
	void autonomousPeriodic(void)
	{
	}
	
	void TeleopInit(void)
	{
		drivetrain.SetExpiration(2); //set the timeout for the watchdog
		drivetrain.SetSafetyEnabled(true); //enable watchdog
		printf("Starting Teleop mode");
	}
	
	void TeleopPeriodic(void)
	{
		updateJoystick();
		//printf("X:%f,Y%f\n", stickX, stickY);
		drivetrain.ArcadeDrive(invertDriving*stickY,-stickX,false); //pass the joystick information to the drivetrain using the simplified ArcadeDrive
	}
	
	void TestInit(void)
	{
		printf("Starting Test");
		drivetrain.SetSafetyEnabled(false); //disable watchdog so that it doesn't fill the log with useless stuff. Also, we don't really want to move here
	}
	
	void TestPeriodic(void) //echos debugging info through netconsole
	{
		updateJoystick();
		printf("X:%f,Y%f,Z:%f,Twist:%f,Throttle:%f\n", stickX, stickY, stick.GetZ(), stick.GetTwist(), stick.GetThrottle()); //report what WPILIB thinks these are. Some don't match with what we think they are.
	}
};

START_ROBOT_CLASS(main);
