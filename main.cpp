#include "WPILib.h"

class main : public IterativeRobot
{
	RobotDrive drivetrain;
	Joystick stick;
	
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
		drivetrain.SetSafetyEnabled(true);
		printf("Starting Teleop mode");
	}
	
	void TeleopPeriodic(void)
	{
		//drivetrain.MecanumDrive_Polar(stick.GetMagnitude(), stick.GetDirectionDegrees(), stick.GetTwist()); //pass the joystick information to the drivetrain using the built in MecanumDrive
		drivetrain.ArcadeDrive(stick); //pass the joystick information to the drivetrain using the simplified ArcadeDrive
	}
	
	void Test()
	{
	}
};

START_ROBOT_CLASS(main);
