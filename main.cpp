#include "WPILib.h"

class main : public IterativeRobot
{
	RobotDrive drivetrain;
	Joystick stick;
	Encoder encoder;

private:
	struct input
	{
		float rotate;
		float drive;
	};

	input updateJoystick(){
		input js;
		static bool invertButtonHeld=false;
		static int invertDrive=1; //1 for normal, -1 for inverted
		if(stick.GetRawButton(1)!=invertButtonHeld&&stick.GetRawButton(1)) invertDrive=-invertDrive; //if invert button changed and new button state is pressed, invert invertDrive
		invertButtonHeld=stick.GetRawButton(1); //update stored value for button
		float throttleScale=((1-stick.GetTwist())/2); //make throttle 0-1 for scaling the joystick input
		js.rotate=-stick.GetX()*throttleScale*invertDrive; //get X and multiply by throttleScale and invertDrive //TODO check which of these (X/Y) need to be inverted to begin with
		js.drive=stick.GetY()*throttleScale*invertDrive; //do the same with Y
		return js;
	}

public:
	main(void):
		//init the Joystick and RobotDrive (numbers refer to ports)
		drivetrain(1,2),
		stick(1),
		encoder(1,2)
	{
		encoder.SetDistancePerPulse((3.1415926535*6)/250); //TODO: change 6 to 8 when wheels changed
		encoder.Start();
	}

	void AutonomousInit(void)
	{
		encoder.Reset();
		drivetrain.SetSafetyEnabled(false); //disable watchdog
	}

	void AutonomousPeriodic(void)
	{
		//drivetrain.ArcadeDrive(0.5,0);
		printf("Encoder:%f\t Raw:%i\n", encoder.GetDistance(), (int)encoder.GetRaw());
		SmartDashboard::PutNumber("Encoder", encoder.GetDistance());
		if(encoder.GetDistance()<60)
		{
			drivetrain.ArcadeDrive(.5,0);
		}
		else
		{
			drivetrain.ArcadeDrive(0.0 ,0);
		}
	}

	void TeleopInit(void)
	{
		printf("Starting Teleop mode");
		drivetrain.SetExpiration(2); //set the timeout for the watchdog
		drivetrain.SetSafetyEnabled(true); //enable watchdog
	}

	void TeleopPeriodic(void)
	{
		input js=updateJoystick();
		drivetrain.ArcadeDrive(js.drive,js.rotate,false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
	}

	void TestInit(void)
	{
		printf("Starting Test mode");
		drivetrain.SetSafetyEnabled(false); //disable watchdog so that it doesn't fill the log with useless stuff. Also, we don't really want to move in Test
	}

	void TestPeriodic(void) //prints debugging info to netconsole
	{
		input js=updateJoystick();
		printf("rotate:%f,drive%f,X:%f,Y:%f,Z(Twist):%f,Twist(Throttle):%f,Throttle:%f\n", js.rotate, js.drive, stick.GetX(), stick.GetY(), stick.GetZ(), stick.GetTwist(), stick.GetThrottle()); //report what WPILIB thinks these are. Some don't match with what we think they are.
	}
};

START_ROBOT_CLASS(main);
