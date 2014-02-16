#include "WPILib.h"
#include "CRioNetworking.h"

void networkMethod(void);

struct input
{
	float rotate;
	float drive;
	float power;
};

input netData;
CRioNetworking cRio=CRioNetworking();

class main : public IterativeRobot
{
	RobotDrive drivetrain;
	Task *networking;
	Joystick driveStick;
	Encoder leftEncoder;
	Encoder rightEncoder;

private:
	input updateJoystick()
	{
		static bool invertButtonHeld=false, commButtonHeld=false;
		static int invertDrive=1; //1 for normal, -1 for inverted
		if(driveStick.GetRawButton(1)&&!invertButtonHeld){
			invertDrive=-invertDrive; //if invert button changed and new button state is pressed, invert invertDrive
		}
		invertButtonHeld=driveStick.GetRawButton(1); //update stored value for button
		if(driveStick.GetRawButton(2)&&!commButtonHeld){
			isAuton=!isAuton; //if invert button changed and new button state is pressed, invert isAuton
			printf("%s\n", isAuton?"Targeting":"Driving");
			if(isAuton)
			{
				cRio.send("start");
			}
		}
		commButtonHeld=driveStick.GetRawButton(2); //update stored value for button
		float throttleScale=((1-driveStick.GetTwist())/2); //make throttle 0-1 for scaling the joystick input
		return (input){-driveStick.GetX()*throttleScale*invertDrive, -driveStick.GetY()*throttleScale}; //get X & Y, scale by throttle, and apply drive inversion
	}
	bool isAuton;

public:
	main(void):
		//init the Joystick and RobotDrive (numbers refer to ports)
		drivetrain(1,2),
		driveStick(1),
		leftEncoder(1,2), rightEncoder(3,4)
	{
		cRio.connect();
		networking = new Task("networking", (FUNCPTR)&networkMethod);
		networking->Start();
		isAuton=false;
		
		leftEncoder.SetDistancePerPulse((3.1415926535*8)/250);
		rightEncoder.SetDistancePerPulse((3.1415926535*8)/250);
		leftEncoder.Start();
		rightEncoder.Start();
	}

	void AutonomousInit(void)
	{
		leftEncoder.Reset();
		rightEncoder.Reset();
		drivetrain.SetSafetyEnabled(false); //disable watchdog
	}

	void AutonomousPeriodic(void)
	{
		//drivetrain.ArcadeDrive(0.5,0);
		printf("Left Encoder:%f\t Raw:%i <--> Right Encoder:%f\t Raw:%i\n", leftEncoder.GetDistance(), (int)leftEncoder.GetRaw(), rightEncoder.GetDistance(), (int)rightEncoder.GetRaw());
		SmartDashboard::PutNumber("Left Encoder", leftEncoder.GetDistance());
		SmartDashboard::PutNumber("Right Encoder", rightEncoder.GetDistance());
		if(leftEncoder.GetDistance()<60)
		{
			drivetrain.ArcadeDrive(.3,0);
		}
		else
		{
			if(netData.power>0)
			{
				printf("WHEEEEEEE! We should have scored here!"); //TODO: when merged with shooter, make it shoot here
				isAuton=false;
			}
			else
			{
				drivetrain.ArcadeDrive(0, netData.rotate, false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
			}
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
		if(isAuton)
		{
			drivetrain.ArcadeDrive(0.0,0.0); //TODO: Remember to remove this
			if(netData.power>0)
			{
				printf("WHEEEEEEE! We should have scored here!"); //TODO: when merged with shooter, make it shoot here
				isAuton=false;
			}
			else
			{
				drivetrain.ArcadeDrive(0, netData.rotate, false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
			}
		}
		else
		{
			drivetrain.ArcadeDrive(js.drive,js.rotate,false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
		}
	}

	void TestInit(void)
	{
		printf("Starting Test mode\n");
		drivetrain.SetSafetyEnabled(false); //disable watchdog so that it doesn't fill the log with useless stuff. Also, we don't really want to move in Test
		isAuton=true;
	}

	void TestPeriodic(void) //prints debugging info to netconsole
	{
		input js=updateJoystick();
		//printf("rotate:%f,drive%f,X:%f,Y:%f,Z(Twist):%f,Twist(Throttle):%f,Throttle:%f\n", js.rotate, js.drive, driveStick.GetX(), driveStick.GetY(), driveStick.GetZ(), driveStick.GetTwist(), driveStick.GetThrottle()); //report what WPILIB thinks these are. Some don't match with what we think they are.
	}

	void DisabledInit(void)
	{
		printf("Stopping");
		isAuton=false;
	}
};

void networkMethod(void)
{
	while(true)
	{
		char data[20];
		cRio.receive(data, 20);
		netData.rotate = atof(strtok(data, ","));
		netData.power = atof(strtok(NULL, ","));
		nanosleep(&(timespec){0, 50000000},NULL);
	}
}

START_ROBOT_CLASS(main);
