#include "WPILib.h"
#include "CRioNetworking.h"

void networkMethod(void);

struct input
{
	float rotate;
	float drive;
};

input netData;
CRioNetworking cRio = CRioNetworking();

class main : public IterativeRobot
{
	Task *networking;
	RobotDrive drivetrain;
	Talon kicker, kicker2, lifter;
	Victor spinnerLeft, spinnerRight;
	DigitalInput kickerLimiter;
	Encoder leftEncoder, rightEncoder;
	Joystick driveStick, shooterStick;
	Timer timer;
	
private:
	bool isAuton;
	input updateJoystick()
	{
		static bool invertButtonHeld = false, commButtonHeld = false;
		static int invertDrive = 1;  //1 for normal, -1 for inverted
		float throttleScale = ((1 - driveStick.GetTwist()) / 2), kickerScale = ((1 - shooterStick.GetTwist()) / 2);  //make throttles 0 - 1 for scaling the joystick input
		
		if(driveStick.GetRawButton(11) && !invertButtonHeld)
			invertDrive = -invertDrive;  //if invert button changed and new button state is pressed, invert invertDrive
		invertButtonHeld = driveStick.GetRawButton(11);  //update stored value for button

		if(shooterStick.GetRawButton(2) && !commButtonHeld){
			isAuton = !isAuton;  //if invert button changed and new button state is pressed, invert isAuton
			printf("%s\n", isAuton?"Targeting":"Driving");
			if(isAuton)
			{
				cRio.send("start");
				netData.rotate = 0.01;
				netData.drive = 0.01;
			}
		}
		commButtonHeld = driveStick.GetRawButton(2);  //update stored value for button
		
		if(shooterStick.GetRawButton(1)) kick(kickerScale);  //if the button is pressed, shoot
		return (input){-driveStick.GetX() * throttleScale, -driveStick.GetY() * throttleScale * invertDrive};  //get X & Y, scale by throttle, and apply drive inversion
	}
	
	bool kick(float kickerPower)
	{
		static bool kickerLimiterHeld;
		float power;
		if(!kicker.Get())  //if the kicker is currently unset, bring the kicker back
		{
			power = 0.5;
			timer.Reset();
		}
		else if(kickerLimiter.Get() && kicker.Get() > 0)
			power = -kickerPower;  //if the limiter is hit, and the kicker is currently going backwards, set the kicker forwards at the given power
		else if((kickerLimiter.Get() && kicker.Get() < 0 && !kickerLimiterHeld) || timer.Get() > .75)
			power = 0;//if the limiter is hit, the kicker is currently going forwards, and the limiter has been released since we set it to kick, stop the kicker
		kickerLimiterHeld = kickerLimiter.Get();
		kicker.Set(power);
		kicker2.Set(power);
		return power;
	}

	bool autonShoot()
	{
		if(netData.rotate || netData.drive) drivetrain.ArcadeDrive(netData.drive, netData.rotate, false);
		else
		{
			printf("Time to shoot");
			return kick(1);
			drivetrain.ArcadeDrive(0.0, 0.0);
		}
		return true;
	}

public:
	main(void):
		//init the Joystick, RobotDrive and Talon motors (numbers refer to ports)
		drivetrain(1, 2),
		kicker(3), kicker2(7),
		lifter(4), spinnerLeft(5), spinnerRight(6),
		kickerLimiter(5),
		leftEncoder(1, 2), rightEncoder(3, 4),
		driveStick(1), shooterStick(2),
		timer()
	{
		cRio.connect();
		networking = new Task("networking", (FUNCPTR)&networkMethod);
		networking->Start();
		isAuton = false;
		
		leftEncoder.SetDistancePerPulse((3.1415926535 * 8) / 250);
		rightEncoder.SetDistancePerPulse((3.1415926535 * 8) / 250);
		leftEncoder.Start();
		rightEncoder.Start();
		timer.Start();
	}

	void AutonomousInit(void)
	{
		leftEncoder.Reset();
		rightEncoder.Reset();
		drivetrain.SetSafetyEnabled(false);  //disable watchdog
	}

	void AutonomousPeriodic(void)
	{
		static bool shoot = false, doneShooting = false;
		SmartDashboard::PutNumber("Left Encoder", leftEncoder.GetDistance());
		SmartDashboard::PutNumber("Right Encoder", rightEncoder.GetDistance());
		if(leftEncoder.GetDistance() < 60 && !shoot) drivetrain.ArcadeDrive(.5, 0);
		else if(!shoot && !doneShooting)
		{
			cRio.send("start");
			shoot = true;
			netData.rotate = 0.01;
			netData.drive = 0.01;
		}
		else if(shoot && !doneShooting) doneShooting = autonShoot();
		else drivetrain.ArcadeDrive(0.0, 0.0);
	}

	void TeleopInit(void)
	{
		printf("Starting Teleop mode");
		drivetrain.SetExpiration(2);  //set the timeout for the watchdog
		drivetrain.SetSafetyEnabled(true);  //enable watchdog
		leftEncoder.Reset();
		rightEncoder.Reset();
	}

	void TeleopPeriodic(void)
	{
		input js = updateJoystick();
		SmartDashboard::PutNumber("Left Encoder", leftEncoder.GetDistance());
		SmartDashboard::PutNumber("Right Encoder", rightEncoder.GetDistance());
		SmartDashboard::PutBoolean("isAuton", isAuton);
		if(isAuton) autonShoot();
		else
		{
			drivetrain.ArcadeDrive(js.drive, js.rotate, false);  //pass the joystick information to the drivetrain using the WPILib method ArcadeDrive
			lifter.Set(-shooterStick.GetY() * 0.5);  //TODO: figure out which of these should be inverted
			spinnerLeft.Set((shooterStick.GetRawButton(3) + -shooterStick.GetRawButton(4)) * ((1 - shooterStick.GetTwist()) / 2));
			spinnerRight.Set((-shooterStick.GetRawButton(3) + shooterStick.GetRawButton(4)) * ((1 - shooterStick.GetTwist()) / 2));
		}
	}

	void TestInit(void)
	{
		printf("Starting Test mode\n");
		drivetrain.SetSafetyEnabled(false);  //disable watchdog so that it doesn't fill the log with useless stuff. Also, we don't really want to move in Test
	}

	void TestPeriodic(void)  //prints debugging info to netconsole
	{
		input js = updateJoystick();
		printf("rotate:%f, drive%f, X:%f, Y:%f, Z(Twist):%f, Twist(Throttle):%f, Throttle:%f\n", js.rotate, js.drive, driveStick.GetX(), driveStick.GetY(), driveStick.GetZ(), driveStick.GetTwist(), driveStick.GetThrottle());  //report what WPILIB thinks these are. Some don't match with what we think they are.
	}

	void DisabledInit(void)
	{
		printf("Stopping");
	}
};

void networkMethod(void)
{
	while(true)
	{
		char data[20];
		cRio.receive(data, 20);
		netData.rotate = atof(strtok(data, ","));
		netData.drive = atof(strtok(NULL, ","));
		nanosleep(&(timespec){0, 50000000}, NULL);
	}
}

START_ROBOT_CLASS(main);
