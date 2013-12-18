#include "WPILib.h"

class main : public IterativeRobot
{
	RobotDrive drivetrain;
	Joystick stick;
	float stickY;
	float stickX;
	
public:
	main(void):
		//init the Joystick and RobotDrive
		drivetrain(1,2),
		stick(1)
	{
	}
	
	void autonomousPeriodic(void)
	{
	}
	
	void TeleopInit(void)
	{
		drivetrain.SetExpiration(2);
		drivetrain.SetSafetyEnabled(true);
		printf("Starting Teleop mode");
	}
	
	void TeleopPeriodic(void)
	{
		stickY=stick.GetY()*((-stick.GetTwist()+1)/2);
		stickX=stick.GetX()*((-stick.GetTwist()+1)/2);
		printf("X:%f,Y%f\n", stickX, stickY);
		drivetrain.ArcadeDrive(-stickY,stickX,false); //pass the joystick information to the drivetrain using the simplified ArcadeDrive
	}
	
	void TestInit(void)
	{
		printf("Starting Test");
		drivetrain.SetSafetyEnabled(false);
	}
	void TestPeriodic(void) //echos debugging info through netconsole
	{
		stickY=stick.GetY()*((-stick.GetTwist()+1)/2);
		stickX=stick.GetX()*((-stick.GetTwist()+1)/2);
		printf("X:%f,Y%f,Z:%f,Twist:%f,Throttle:%f\n", stickX, stickY, stick.GetZ(), stick.GetTwist(), stick.GetThrottle());
	}
};

START_ROBOT_CLASS(main);
