
#include <Arduino.h>
#include "WiFi.h"

// microros ===================================================================
#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <std_msgs/msg/int32.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <sensor_msgs/msg/imu.h>

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

#define LED_PIN 23

rclc_executor_t executor;
//rcl_publisher_t publisher;
rcl_subscription_t subscriber;
//std_msgs__msg__Int32 send_msg;
std_msgs__msg__Int32 msg;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

/*void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);

  int valueled = 0;

  send_msg.data = valueled;

  rcl_publish(&publisher, &send_msg, NULL);
}*/

void subscription_callback(const void * msgin)
{
	const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;

	// Dont pong my own pings
	//if(strcmp(outcoming_ping.frame_id.data, msg->frame_id.data) != 0){
		printf("Value received with %d. Turning led state.\n", msg->data);
    digitalWrite(LED_PIN, (msg->data == 0) ? LOW : HIGH);  
	//	rcl_publish(&publisher, msg, NULL);
	//}
}

void setup()
{

  //digitalWrite(ledPin, HIGH);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  int agent_port = 8888; //port
  //set_microros_wifi_transports("MEO-8E7C30_EXT", "2bc18f222e", "192.168.1.203", agent_id);
  set_microros_wifi_transports("MEO-CFEC70", "09f8a4e7d1", "192.168.1.203", agent_port);
  //set_microros_transports();

  delay(2000);

  allocator = rcl_get_default_allocator();

  //rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
	//RCCHECK(rcl_init_options_init(&init_options, allocator));

  // create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  //rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator);
  
  //create node
  rclc_node_init_default(&node, "micro_ros_node", "", &support);

  //create the publisher
  //rclc_publisher_init_default(&publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "int32_publisher");

  //create subscriber
  RCCHECK(rclc_subscription_init_default(
		&subscriber,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"led_toggle"));

  //const unsigned int timer_timeout = 1000;
  //rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(timer_timeout), timer_callback);
  //rclc_executor_init(&executor, &support.context, 1, &allocator);
  //rclc_executor_add_timer(&executor, &timer);

  //create executor and add a subscriber to it
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));

}

void loop()
{

  delay(100);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));


  //command to publish - source ros2 installation - ros2 topic pub --once /led_toggle std_msgs/msg/Int32 '{data: 1}'


  //command to run the agent in a container - sudo docker run -it --rm -v /dev:/dev --privileged --net=host microros/micro-ros-agent:foxy serial --dev /dev/ttyUSB0 --baud 115200
  // command to run installed agent - cd microros_ws - source instalation - ros2 run micro_ros_agent micro_ros_agent udp4 -p 8888
}