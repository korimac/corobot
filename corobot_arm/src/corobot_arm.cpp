#include "ros/ros.h"
#include <stdio.h>
#include <math.h>

#include "std_msgs/Float64.h"
#include "corobot_msgs/MoveArm.h"
#include "corobot_msgs/ServoPosition.h"
#include "corobot_msgs/ServoType.h"

//Enumeration to identifies the position of a servo
typedef enum
{
	base_rotation,
	shoulder,
	elbow,
	wrist_flex,
	wrist_rotation,
	gripper
} servo_type;

//Enumeration to identify the servo controller used for the arm
typedef enum
{
	arbotix,
	ssc32,
	phidget
} hardware_controller;

// servo type that save data about the connected servos.
typedef struct
{
	servo_type type;
	int id;
	int min_angle;
	int max_angle; 
} servo;


ros::Publisher position_pub, type_pub;
double centerOffset = 0.0; //servo offset, in degrees
servo * servos = NULL; // list of connected servos
int number_servo = 0; //number of connected servos
hardware_controller controller; //which servo controller id used?
ros::Publisher * arbotix_publisher = NULL; //table of publishers, as we need one publisher per servo for the arbotix controller.


void setServoPosition(const corobot_msgs::MoveArm &msg)
/**
 * @brief Topic to move the arm of the Corobot
 */ 
{
	corobot_msgs::ServoPosition msg_sending;
	
	for(int i; i<number_servo; i++)
	{
		msg_sending.index = -1;

		//We received the indication on which servo we want to move, so now we are finding the index corresponding to the servo.  
		if(msg.index == msg.BASE_ROTATION && servos[i].type == base_rotation)
			msg_sending.index = servos[i].id;
		if(msg.index == msg.SHOULDER && servos[i].type == shoulder)
			msg_sending.index = servos[i].id;
		if(msg.index == msg.ELBOW && servos[i].type == elbow)
			msg_sending.index = servos[i].id;
		if(msg.index == msg.WRIST_FLEX && servos[i].type == wrist_flex)
			msg_sending.index = servos[i].id;
		if(msg.index == msg.WRIST_ROTATION && servos[i].type == wrist_rotation)
			msg_sending.index = servos[i].id;
		if(msg.index == msg.GRIPPER && servos[i].type == gripper)
			msg_sending.index = servos[i].id;

		//if we found the corresponding servo, we send it to the driver of the controller board
		if(msg_sending.index != -1)
		{
			msg_sending.position = msg.position + centerOffset;

			//make sure the position is in the range
			if(msg_sending.position < servos[i].min_angle)
				msg_sending.position = servos[i].min_angle;
			else if(msg_sending.position > servos[i].max_angle)
				msg_sending.position = servos[i].max_angle;

		
			if(controller == arbotix)
			{
				std_msgs::Float64 msg_arbotix;
				msg_arbotix.data = (msg_sending.position / 180) * M_PI; // the arbotix controller code take angles in radian and not degrees
				arbotix_publisher[i].publish(msg_arbotix);
			}
			else
				position_pub.publish(msg_sending);
		}
	}
}


void init_servos_db(XmlRpc::XmlRpcValue dynamixels, ros::NodeHandle n)
/**
 * @brief initialise the list of servos (variable servos) and the number of servos connected. It reads this from the yaml file. 
 */ 
{
	if(dynamixels.hasMember("base"))
	{
		servos[number_servo].type = base_rotation;
		servos[number_servo].id = (int) dynamixels["base"]["id"];
		servos[number_servo].min_angle = (int) dynamixels["base"]["min_angle"];
		servos[number_servo].max_angle = (int) dynamixels["base"]["max_angle"];

		if(controller == arbotix) //the arbotix controller driver needs one topic per servo motor
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/base/command", 100);

		number_servo++;
	}
	if(dynamixels.hasMember("shoulder"))
	{
		servos[number_servo].type = shoulder;
		servos[number_servo].id = (int) dynamixels["shoulder"]["id"];
		servos[number_servo].min_angle = (int) dynamixels["shoulder"]["min_angle"];
		servos[number_servo].max_angle = (int) dynamixels["shoulder"]["max_angle"];

		if(controller == arbotix)
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/shoulder/command", 100);

		number_servo++;
	}
	if(dynamixels.hasMember("shoulder2"))
	{
		servos[number_servo].type = shoulder;
		servos[number_servo].id = (int) dynamixels["shoulder2"]["id"];
		servos[number_servo].min_angle = (int) dynamixels["shoulder2"]["min_angle"];
		servos[number_servo].max_angle = (int) dynamixels["shoulder2"]["max_angle"];

		if(controller == arbotix)
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/shoulder2/command", 100);

		number_servo++;
	}
	if(dynamixels.hasMember("elbow"))
	{
		servos[number_servo].type = elbow;
		servos[number_servo].id = (int) dynamixels["elbow"]["id"];
		servos[number_servo].min_angle = (int) dynamixels["elbow"]["min_angle"];
		servos[number_servo].max_angle = (int) dynamixels["elbow"]["max_angle"];

		if(controller == arbotix)
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/elbow/command", 100);
	
		number_servo++;
	}
	if(dynamixels.hasMember("elbow2"))
	{
		servos[number_servo].type = elbow;
		servos[number_servo].id = (int) dynamixels["elbow2"]["id"];
		servos[number_servo].min_angle = (int) dynamixels["elbow2"]["min_angle"];
		servos[number_servo].max_angle = (int) dynamixels["elbow2"]["max_angle"];

		if(controller == arbotix)
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/elbow2/command", 100);

		number_servo++;
	}
	if(dynamixels.hasMember("wrist_flex"))
	{
		servos[number_servo].type = wrist_flex;
		servos[number_servo].id = (int) dynamixels["wrist_flex"]["id"];
		servos[number_servo].min_angle = (int) dynamixels["wrist_flex"]["min_angle"];
		servos[number_servo].max_angle = (int) dynamixels["wrist_flex"]["max_angle"];

		if(controller == arbotix)
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/wrist_flex/command", 100);

		number_servo++;
	}
	if(dynamixels.hasMember("wrist_rotation"))
	{
		servos[number_servo].type = wrist_rotation;
		servos[number_servo].id = (int) dynamixels["wrist_rotation"]["id"];
		servos[number_servo].min_angle = (int) dynamixels["wrist_rotation"]["min_angle"];
		servos[number_servo].max_angle = (int) dynamixels["wrist_rotation"]["max_angle"];

		if(controller == arbotix)
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/wrist_rotation/command", 100);

		number_servo++;
	}
	if(dynamixels.hasMember("gripper"))
	{
		servos[number_servo].type = gripper;
		servos[number_servo].id = (int) dynamixels["gripper"]["id"];
		servos[number_servo].min_angle = -180;
		servos[number_servo].max_angle = 180;

		if(controller == arbotix)
			arbotix_publisher[number_servo] = n.advertise<std_msgs::Float64>("/gripper/command", 100);

		number_servo++;
	}
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "corobot_arm");
	ros::NodeHandle n;
	ros::NodeHandle n_private("~");


	//Subscribe to topics
	ros::Subscriber armPos= n.subscribe("armPosition", 100, setServoPosition); //Command received by corobot_teleop or any other controlling node


	//Set up an offset, in case the servo is not at its default position when it is supposed to  
	n_private.param("offset", centerOffset, 0.0);


	//Read yalm file parameters.
	XmlRpc::XmlRpcValue dynamixels;
	XmlRpc::XmlRpcValue controller_type;
   	n_private.param("dynamixels", dynamixels, dynamixels);
	n_private.param("controller_type", controller_type, controller_type);

	//Read the type of arm we are controlling
	if (strcmp(static_cast<std::string>(controller_type).c_str(), "arbotix") == 0)
	{
			controller = arbotix;
			arbotix_publisher = new ros::Publisher[dynamixels.size()]; //set the size of the table
	}
	else if (strcmp(static_cast<std::string>(controller_type).c_str(), "ssc32") == 0)
	{
			controller = ssc32;
	}
	else if (strcmp(static_cast<std::string>(controller_type).c_str(), "phidget") == 0)
	{
			controller = phidget;
	}
	//Read information about the servos
	servos = (servo*) malloc(dynamixels.size() * sizeof(servo));
	init_servos_db(dynamixels, n_private);


	//Declare the necessary topics
	if(controller == phidget) 
	{
		position_pub = n.advertise<corobot_msgs::ServoPosition>("setPositionServo", 100); //Set the position of the servo. Use for phidgetServo and corobot_ssc32
		type_pub = n.advertise<corobot_msgs::ServoType>("phidgetServo_setType", 100); //Set the type of servos. PhidgetServo needs it.

		corobot_msgs::ServoType typeMsg;
	}
	else if (controller == ssc32)
	{
		position_pub = n.advertise<corobot_msgs::ServoPosition>("setPositionServo", 100); //Set the position of the servo. Use for phidgetServo and corobot_ssc32
	}


	if(controller == phidget || controller == ssc32)
	{ 
		//Make sure that the arm is in the default initial position for non arbotix arms (wrist horizontal, gripper open and shoulder and elbow angle at 0)
		corobot_msgs::ServoPosition msg;

		ros::spinOnce();
		ros::Rate loop_rate(0.5);  //We wait 2s to make sure that the node PhidgetServo has already subscribed to the topic
		loop_rate.sleep();

		for (int i=0; i<number_servo; i++) //we initialize the servos
		{
			msg.index = servos[i].id;
			if(servos[i].type == base_rotation)
				msg.position = 90;
			else if (servos[i].type == wrist_rotation)
				msg.position = 90;
			else if (servos[i].type == wrist_flex)
				msg.position = 90;
			else
				msg.position = 0;
			position_pub.publish(msg);
		}
	}

        ros::spin();
	
	free(servos);
	delete[] arbotix_publisher;
	return 0;
}
