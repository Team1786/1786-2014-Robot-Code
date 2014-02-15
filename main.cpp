#include "WPILib.h"
#include "CRioNetworking.h"

void networkMethod(void);

bool isComm, sendStart=true;
char data[20];

class main : public IterativeRobot
{
	RobotDrive drivetrain;
	Task *networking;
	Joystick driveStick;

private:
	struct input
	{
		float rotate;
		float drive;
	};

	input updateJoystick()
	{
		static bool invertButtonHeld=false, commButtonHeld=false;
		static int invertDrive=1; //1 for normal, -1 for inverted
		if(driveStick.GetRawButton(1)&&!invertButtonHeld){
			invertDrive=-invertDrive; //if invert button changed and new button state is pressed, invert invertDrive
		}
		invertButtonHeld=driveStick.GetRawButton(1); //update stored value for button
		if(driveStick.GetRawButton(2)&&!commButtonHeld){
			isComm=!isComm; //if invert button changed and new button state is pressed, invert isComm
			sendStart=true;
			printf("%s\n", isComm?"Targeting":"Driving");
		}
		commButtonHeld=driveStick.GetRawButton(2); //update stored value for button
		float throttleScale=((1-driveStick.GetTwist())/2); //make throttle 0-1 for scaling the joystick input
		return (input){-driveStick.GetX()*throttleScale*invertDrive, -driveStick.GetY()*throttleScale}; //get X & Y, scale by throttle, and apply drive inversion
	}

public:
	main(void):
		//init the Joystick and RobotDrive (numbers refer to ports)
		drivetrain(1,2),
		driveStick(1)
	{
		networking = new Task("networking", (FUNCPTR)&networkMethod);
		isComm = false;
		networking->Start();
	}

	void AutonomousInit(void)
	{
	}

	void AutonomousPeriodic(void)
	{
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
		if(isComm)
		{
			drivetrain.ArcadeDrive(0.0,0.0); //TODO: Remember to remove this
			if(data[0]!='\0')
			{
				float rotate = atof(strtok(data, ","));
				float power = atof(strtok(NULL, ","));
				if(power>0)
				{
					//TODO: when merged with shooter, make it shoot here
				}
				else
				{
					drivetrain.ArcadeDrive(0, rotate, false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
				}
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
		isComm=true;
	}

	void TestPeriodic(void) //prints debugging info to netconsole
	{
		input js=updateJoystick();
		//printf("rotate:%f,drive%f,X:%f,Y:%f,Z(Twist):%f,Twist(Throttle):%f,Throttle:%f\n", js.rotate, js.drive, driveStick.GetX(), driveStick.GetY(), driveStick.GetZ(), driveStick.GetTwist(), driveStick.GetThrottle()); //report what WPILIB thinks these are. Some don't match with what we think they are.
	}

	void DisabledInit(void)
	{
		printf("Stopping");
		isComm=false;
	}
};

void networkMethod(void)
{
	CRioNetworking* cRio = new CRioNetworking();
	cRio->connect();
	while(true)
	{
		if(sendStart)
		{
			cRio->send("start"); //send the signal to the OBL to start imageProc
			sendStart=false;
		}
		if(cRio->receive(data, 20)==-1) //TODO: The socket still seems to be blocking, preventing this part from working (issue: if the obl is not availible at boot, networking will never start
		{
			sendStart=true;
		}
		nanosleep(&(timespec){0, 50000000},NULL);
	}
}

START_ROBOT_CLASS(main);
