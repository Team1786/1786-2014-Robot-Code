#include "WPILib.h"
#include "CRioNetworking.h"

int networkMethod(void);

bool isComm;
string data;

class main : public IterativeRobot
{
	RobotDrive drivetrain;
	Joystick stick;
	Task *networking;

private:
	struct input
	{
		float rotate;
		float drive;
	};
	
	input updateJoystick()
	{
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
		stick(1)
	{
		networking = new Task("networking", (FUNCPTR)&networkMethod);
		isComm = false;
	}
	
	void AutonomousInit(void)
	{
		networking->Start();
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
		if(!isComm)
		{
			drivetrain.ArcadeDrive(js.drive,js.rotate,false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
		}
		else
		{
			//int c = data.find_first_of(',');
			//char* drive = data.substr(0, c);
			//char* rotate = (char*)data.substr(c);
			//drivetrain.ArcadeDrive(atof((char*)drive), atof((char*)rotate), false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
		}
	}
	
	void TeleopContinuous(void)
	{
		
	}

	void TestInit(void)
	{
		printf("Starting Test mode\n");
		drivetrain.SetSafetyEnabled(false); //disable watchdog so that it doesn't fill the log with useless stuff. Also, we don't really want to move in Test
		networking->Start();
		printf("network started\n");
	}

	void TestPeriodic(void) //prints debugging info to netconsole
	{
		input js=updateJoystick();
		//printf("rotate:%f,drive%f,X:%f,Y:%f,Z(Twist):%f,Twist(Throttle):%f,Throttle:%f\n", js.rotate, js.drive, stick.GetX(), stick.GetY(), stick.GetZ(), stick.GetTwist(), stick.GetThrottle()); //report what WPILIB thinks these are. Some don't match with what we think they are.
		if(stick.GetY() < -.5 && !networking->IsSuspended())
		{
			printf("Suspending\n");
			networking->Suspend();
		}
		else if(stick.GetY() > .5 && networking->IsSuspended())
		{
			printf("Resuming\n");
			networking->Resume();
		}
	}
	
	void DisabledInit(void)
	{
		printf("Stopping");
		networking->Stop();
	}
};

int networkMethod(void)
{
	while(true)
	{
		printf("Task!\n");
		sleep(1);	
	}
	//CRioNetworking* cRio = new CRioNetworking();
	//cRio->connect();
	//cRio->send("Ready!");
	while(true)
	{
		if(isComm)
		{
			//cRio->send("Give my data!!!");
			//char* data;
			while(isComm)
			{
				//data = (char*)cRio->receive();
			}
		}
	}
	return 0;
}

START_ROBOT_CLASS(main);
