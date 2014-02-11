#include "WPILib.h"
#include "CRioNetworking.h"

int networkMethod(void);

bool isComm;
string data;

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
		static bool invertButtonHeld=false;
		static int invertDrive=1; //1 for normal, -1 for inverted
		if(driveStick.GetRawButton(1)&&!invertButtonHeld){
			invertDrive=-invertDrive; //if invert button changed and new button state is pressed, invert invertDrive
		}
		invertButtonHeld=driveStick.GetRawButton(1); //update stored value for button
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
		if(driveStick.GetRawButton(2))
		{
			isComm = true;
			if(networking->IsSuspended())
			{
				networking->Resume();
				printf("Resuming\n");
			}
			printf("true\n");
		}
		else
		{
			isComm = false;
			if(!networking->IsSuspended())
			{
				printf("Suspending: %d", networking->Suspend());
				//printf("Suspending\n");
			}
			printf("false\n");
		}
		if(!isComm)
		{
			drivetrain.ArcadeDrive(js.drive,js.rotate,false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
			printf("Driving\n");
		}
		else
		{
			printf("Targeting\n");
			if(data.length())
			{
				int c = data.find_first_of(',');
				char* drive = (char*)data.substr(0, c).c_str();
				char* rotate = (char*)data.substr(c).c_str();
				drivetrain.ArcadeDrive(atof((char*)drive), atof((char*)rotate), false); //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
			}
		}
	}

	void TestInit(void)
	{
		printf("Starting Test mode\n");
		drivetrain.SetSafetyEnabled(false); //disable watchdog so that it doesn't fill the log with useless stuff. Also, we don't really want to move in Test
		networking->Start();
		printf("network started\n");
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
		networking->Stop();
	}
};

int networkMethod(void)
{
	CRioNetworking* cRio = new CRioNetworking();
	char data[20];
	cRio->connect();
	while(!isComm);
	sleep(1);
	while(true)
	{
		if(isComm)
		{
			cRio->send("start"); //send the signal to the OBL to start imageProc
			while(isComm)
			{
				cRio->receive(data, 20);
				printf("buf=%s\n", data);
			}
		}
		sleep(1);
	}
	return 0;
}

START_ROBOT_CLASS(main);
