#include <kipr/botball.h>
#include <stdbool.h>
/*
LIST OF EACH TYPE OF SENSOR/MOTOR AND WHICH PORT THEY GO TO
Camera - USB port, camera-0
grabber detector - digital(9), returns 1 if pressed
left motor - motor(0)
right motor - motor(1)
holder sensor - analog(0), checks if there is an object in the holder
corner/tape sensor - analog(1), checks if we have reached a corner after we have an object
other corner/tape sensor - analog(2), check for tape line

used for toy robot
left light sensor - analog(2)
right light sensor 2 - analog(3)

holding arm - servo(0), move up and down to move arm

Channel Numbers
0 - Blue cube
1 - blue strip
2 - ping pong ball
3 - orange strip
*/

//arm vals
const int arm_up = 800;
const int arm_down = 2000;

//screen vals
const int right_side_of_screen = 107;
const int left_side_of_screen = 53;
const int top_of_screen = 40;

const int left_line_sensor_thresh = 80;//analog(1)
const int right_line_sensor_thresh = 75;//analog(2)

//int sort_objects(bool have_objects, int channel);
int place_toy();
int have_object();


int main()
{
    //vars to keep track of which job it needs to do
    int channel = 0;
    int objects_sorted = 0;    
    bool have_object = false;
    
        
    //enable servos
    enable_servos();
    //enable camera
    camera_open();
    //initial setup
    set_servo_position(0, arm_up);
    
    //can turn to false if we arent doing the toy part
    bool toy = false;
    //bool darkroom = false;
    bool found_line = false;
    
    while(toy){
        if (get_servo_position(0) != arm_down)
            set_servo_position(0, arm_down);
        //update frame
        //camera_update();
        //if we arent in the dark room, look for it
        //channel will be 0
        if (!found_line){
            printf("Looking for line\n");
            motor(0, 5);
            motor(1, 5);
            //we are line following
            //move forward until you detect a dark line
            if (analog(1) > left_line_sensor_thresh || analog(2) > right_line_sensor_thresh){
                printf("Found line\n");
                found_line = true;
            }
        }
        //NOTE: analog(1) reports ~5 greater than analog(2)
        else if (found_line){
            
        	if (analog(1) < left_line_sensor_thresh && analog(2) < right_line_sensor_thresh){
                printf("Line is centered\n");
                motor(0, 10);
                motor(1, 10);
            }
            //line is leftish, move right
            else if (analog(1) > left_line_sensor_thresh && analog(2) < right_line_sensor_thresh){
                printf("Left sensor detects line, move right to correct\n");
            	motor(0, -5);
                motor(1, 30);
            }
            //line if rightish, move left
            else if (analog(1) < left_line_sensor_thresh && analog(2) > right_line_sensor_thresh){
                printf("Right sensor detects line, move left to correct\n");
             	motor(0, 30);
                motor(1, -5);
            }
            else if (analog(1) > 120 && analog(2) > 125){
             	printf("Found the dark room!\n");
                set_servo_position(0, arm_up);
                msleep(1000);
                //back up
                motor(0, -50);
                motor(1, -50);
                msleep(3000);
                toy = false;
            }
        }
        
        //else we are in the dark room, rely on analog sensors
        else {
            //at white strip, drop robot
            if(analog(1) < 120){
                printf("Backing up\n");
                set_servo_position(0, arm_up);
                msleep(1000);
                //back up
                motor(0, -50);
                motor(1, -50);
                msleep(3000);
                toy = false;
                return 0;
            }
            
            if (analog(3) > analog(4)){
            	//move left
            	motor(0, 0);
                motor(1, 50);
            }
            else if (analog(4) > analog(3)){
            	//move right
            	motor(0, 50);
                motor(1, 0);
            }
            else{
            	//drive straight
            	motor(0, 50);
                motor(1, 50);
            }
        }
    }
            
 	///////////////////////////////////////////////////////////////
    while(toy == false){
        //new frame
        camera_update();
        //if we arent holding an object
        if(have_object == false){
            //reset grabber arm
            if(arm_up != get_servo_position(0))
                set_servo_position(0, arm_up);
            
			//dont see anything? turn in place
            if(get_object_count(channel) == 0){
                printf("Looking for object: %d\n", get_object_count(channel));
                motor(0, -30);
                motor(1, 30);
            }
            //else we detect an object, how do we respond?
            else{
            	//make sure we detect an object below half the screen
                if(get_object_center_row(channel, 0) < top_of_screen){
                    printf("Found object\n");
                    //if object on the left, turn left
                    if (get_object_center_column(channel, 0) < left_side_of_screen){
                        printf("Turning left\n");
                        motor(0, 0);
                        motor(1, 50);
                    }
                    //if object on the right, turn right
                    if (get_object_center_column(channel, 0) > right_side_of_screen){
                        printf("Turning right\n");
                        motor(0, 50);
                        motor(1, 0);
                    }
                    //if object is centered, drive straight
                    if((get_object_center_column(channel, 0) >= left_side_of_screen) && (get_object_center_column(channel, 0) <= right_side_of_screen)){
                        printf("Driving straight\n");
                        motor(0, 50);
                        motor(1, 50);
                    }
                }
			}
            if (analog(0) < 2700)
                have_object = true;
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //WE HAVE AN OBJECT, move it to its corner
        else{
            //make sure we caught the object
            if (analog(0) > 2700)
                have_object = false;
            
            //if we are working on blue, find blue strip
            if (channel == 0)
                channel = 1;
            //if we are working on ping pong ball, find orange strip
            if (channel == 2)
                channel = 3;
            
            //close the grabber to secure contents
            if (get_servo_position(0) != arm_down)
        		set_servo_position(0, arm_down);
            
            //turn in place until you see another object with same color
            if(get_object_count(channel) == 0){
                printf("Looking for corner\n");
                //turn in place until you detect an object
                //servo0 @ 100, servo1 @ -100
                motor(0, -50);
                motor(1, 50);
            }
            //if we detect object with same color above the ground
            else if (get_object_center_row(channel, 0) < top_of_screen){
                printf("Found the corner\n");
                //if object on the left, turn left
                if (get_object_center_column(channel, 0) < left_side_of_screen){
                    printf("Turning left\n");
                    //drive_straight = false;
                    motor(0, 0);
                    motor(1, 50);
                }
                //if object on the right, turn right
                if (get_object_center_column(channel, 0) > right_side_of_screen){
                    printf("Turning right\n");
                    //drive_straight = false;
                    motor(0, 50);
                    motor(1, 0);
                }
                //if object is centered, drive straight
                if ((get_object_center_column(channel, 0) >= left_side_of_screen) && (get_object_center_column(channel, 0) <= right_side_of_screen)){
                    printf("Driving straight\n");
                    //drive_straight = true;
                    motor(0, 50);
                    motor(1, 50);
                }
            }    
            //stop when analog(1) > 85 (we reached the colored tape aka corner)
            if(((analog(1) >= (left_line_sensor_thresh)) || (analog(2) >= (right_line_sensor_thresh))) && have_object){
                printf("Object put in correct place\n");
                //release grabber arm
                set_servo_position(0, arm_up);
                //have_object = false;
                
                //back up
                motor(0, -50);
                motor(1, -50);
                msleep(3000);
                //turn 90 degrees and restart search
                motor(0, -100);
                motor(1, 100);
                msleep(500);
                
                motor(0, 0);
                motor(1, 0);
                
                if (analog(0) > 2800){
                    have_object = false;
                    objects_sorted++;
                    printf("Sorted %d objects\n", objects_sorted);
                    //if we sorted all the objects, increase channel count
                    if (objects_sorted >= 3){
                        objects_sorted = 0;
                        if (channel == 3){
                            printf("Mission Complete!\n");
                            return 0;
                        }
                        channel++;
                        if (channel > 3){
                            printf("MISSION COMPLETE\n");
                            return 0;
                        }
                	}
                }
            }
        }
    }
    
	//clean up
    ao();
    disable_servos();
    camera_close();
    return 0;
}

//if we have object, returns 1, else 0
int have_object(){
	if (analog(1) > 85)
        return 1;
    else
        //dont have object
        return 0;
}
