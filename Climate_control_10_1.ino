/*
 * Embedded systems LAB SS17 : Task : Climate Control
 * 
 */

#define SET_TEMPERATURE    A0    //  A0 pin is set temperature value
#define TEMPERATURE_SENSOR 15    
#include <Servo.h> 
#include <OneWire.h>

#include <Wire.h> 
#include <SPI.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>

#define LINE_1    1
#define LINE_2    2
#define LINE_3    3

#define INCREASING_TEMPERATURE  4
#define DECREASING_TEMPERATURE  5
#define NO_ACTION               6
#define SERVO_CONTROL 9


/* ################################################################################################################################# */
//Global initializations and constants settings

const int loopDelay =1000;
const double TemperatureOffset =0.5; // in degree centigrade         ===============> change this as per the user requirement.Recommended value =0.5

/**Calibrate the initial position of the Servo motor to vertical(neutral) position **/
const int servo_angle_calibration_offset = 8;// in degrees           ===============> change this as per the user requirement
const int neutral_pos_angle =90+servo_angle_calibration_offset;//    ===============> change this as per the user requirement

const int angle_of_rotation_from_neutral_pos = 18;// in degrees  //  ===============> change this as per the user requirement
const int min_angle = neutral_pos_angle-angle_of_rotation_from_neutral_pos;
const int max_angle = neutral_pos_angle+angle_of_rotation_from_neutral_pos;

/** duration in seconds **/
const int button_press_duration=  1; //in seconds                   ================> change this as per requirement .Recommended value = 2  to 3 seconds
/** repeatition_interval in seconds **/
const int button_press_repeatition_interval= 10; //in seconds       ================> change this as per requirement .Recommended value = 10 minutes ie 600 seconds

/* #################################################################################################################################### */

int decrease_temperature_counter=0;
int increase_temperature_counter=0;
Servo servo;

   MI0283QT2 lcd;

//Temperature chip I/o
OneWire ds(TEMPERATURE_SENSOR);

/**
  *Setup and initialization.This function does the initial setup of inputs,servo motor and the LCD
  */                            

void setup() 

{
//=========================================
  pinMode(SET_TEMPERATURE, INPUT); 
  pinMode(TEMPERATURE_SENSOR, INPUT);
  
  pinMode(SERVO_CONTROL, OUTPUT);
 servo.attach(SERVO_CONTROL); 
//==================================

  // Basic initial display
  lcd.begin();
  lcd.fillScreen(RGB(255, 255, 255));
 
  lcd.drawText(0, 5, "** CLIMATE **", RGB(0, 0, 0), RGB(255, 150, 80), 3);
  lcd.drawText(0, 40,"** CONTROL **", RGB(0, 0, 0), RGB(255, 150, 80), 3);
  lcd.drawText(0, 65,"====================", RGB(0, 0, 0), RGB(200, 200, 200), 2);
  lcd.drawText(2, 85,"Temperature value:   ", RGB(0, 0, 0), RGB(153, 204, 255), 2);
  lcd.drawText(2, 105,"  (in Celsius)      ", RGB(0, 0, 0), RGB(153, 204, 255), 2);
  lcd.drawText(0, 180,"--------------------", RGB(0, 0, 0), RGB(255, 255, 102), 2);



}
//==========================================================




/**
  *This is the Main Loop of the program which is ececuted periodically
  */                            


void loop() 
{

 
 //---------------------------------------------------------
     //Read values from inputs POT
     
      int  setTempereatureValue = analogRead(SET_TEMPERATURE);
     
    //---------------------------------------------------------
      double  set_temp_Celcius   = Compute_set_temprature_in_Celcius(setTempereatureValue); 
      double  set_temp_Farenheit = Compute_temprature_in_Farenheit(set_temp_Celcius);
      
     // Display the temperature on LCD
     Display_temperature(set_temp_Celcius,LINE_1);  

 //---------------------------------------------------------------  
      

     // Get the temerature reading from the temperature sensor
      double  curr_temp_Celcius   = get_temprature_in_Celcius();
      double  curr_temp_Farenheit = Compute_temprature_in_Farenheit(curr_temp_Celcius);
      
        // Display the temperature on LCD
      Display_temperature(curr_temp_Celcius,LINE_2);

 //---------------------------------------------------------------        

     // calculate offset between current and set temeperature.
      double offset = (set_temp_Celcius - curr_temp_Celcius); 

      //Take action based on offset
      if(offset> TemperatureOffset)  // if offset is positive
      {
           // servo rotates clockwise and presses "HEAT" button so that temperatrure increases.
            IncreaseTemperature(); 
    
            Display_action(INCREASING_TEMPERATURE);
     
      }
      
      else if( offset< -TemperatureOffset) // if offset is negative
      {
         // servo rotates counter clockwise and presses "COLD" button so that temperatrure decreases.
          DecreaseTemperature();

         
           Display_action(DECREASING_TEMPERATURE);
       
        
      }

       else  // if the offset is within the range 
      {
         //Then position the Servo motor in neutral postion so that none of the buttons are pressed

         decrease_temperature_counter=increase_temperature_counter=0;
          Servo_in_neutral_position();

      
           Display_action(NO_ACTION);
   
        
      }
      
    delay(loopDelay);

 
  
} // main loop ends

//==========================================================================

//Helper functions
/**
  *This method computes the set temerature in degree celcius.It takes the Analog  value from the POT as input
  */        
double Compute_set_temprature_in_Celcius(int value)
 {
  
   
 
  double highResolution_SetTemperature=  double(value*5 *6)/1024 +10 ;
  return  highResolution_SetTemperature;
    
 }
//=============================================================================
/**
  *This method can be used to convert the temperature ffrom degree celcius to degree farenheit.
  */     
double Compute_temprature_in_Farenheit( double temp_celcius)
{
    return (temp_celcius*9.0/5.0+32);
}

//=============================================================================

int curr_position=0;

/**
  *This method takes action to increase the temperature by initiating the servo motor to rotate clock-wise
  */   
void IncreaseTemperature()
  {
   
  
       if(increase_temperature_counter%button_press_repeatition_interval==0 || increase_temperature_counter>button_press_repeatition_interval)
       {
        increase_temperature_counter=0;
         RotateServoClockwise();
       }

       if(increase_temperature_counter==button_press_duration)
        {
         Servo_in_neutral_position();
        }

   increase_temperature_counter++;    
     
  }
/**
  *This method sends commands to the servo motor to rotate clock-wise
  */   
void RotateServoClockwise()
{
     while ( curr_position <= max_angle ) 
      {
        servo.write(curr_position); 
     
        curr_position ++; delay(10);
      
      }

  
}


  
//==========================================================================
/**
  *This method takes action to decrease the temperature by initiating the servo motor to rotate counter clock-wise
  */   
void  DecreaseTemperature()
  {
    
   
       if(decrease_temperature_counter%button_press_repeatition_interval==0 || decrease_temperature_counter>button_press_repeatition_interval)
       {
        decrease_temperature_counter=0;
         RotateServoAntiClockwise();
       }

       if(decrease_temperature_counter==button_press_duration)
        {
         Servo_in_neutral_position();
       }

   decrease_temperature_counter++;    
  }
/**
  *This method sends commands to the servo motor to rotate counter clock-wise
  */   
void RotateServoAntiClockwise()
{

 while ( curr_position  >= min_angle ) 
      {
        servo.write(curr_position); 
       
         curr_position --; delay(10);
      }

} 
 //=============================================================================
/**
  *This method sends commands to the servo motor to return to neutral position
  */   
  
  void  Servo_in_neutral_position()
  {
    /* if the difference between " Set temperature" and  "Current temperature" is within the range
     *  then position the servo in neutral position ( ie 90 degrees ) so that neither "HEAT" or "COLD" button is pressed.
     */
        if (curr_position <=min_angle)
        {

        
           while ( curr_position <= neutral_pos_angle ) 
           {
            servo.write(curr_position);
            curr_position ++; delay(10);
           }
        }
      
      
         else if (curr_position >=max_angle)
        {
          
           while ( curr_position >= neutral_pos_angle ) 
           {
            servo.write(curr_position);
            curr_position --; delay(10);
           }
        }
  }
 
//====================================================================================
/**
  *This method is responsible for displaying the dynamically changing value of the temperature at the specified line on the LCD.
  */   

void Display_temperature(double temperature ,int line_num)
{
    switch(line_num)
    { 
      
      char data[30];
      
        case LINE_1:
      
    
        sprintf(data, "SET     : %02i.%02i        ",  ( int)temperature,(unsigned int)(abs(temperature)*100)%100);
        lcd.drawText(2, 135, data, RGB(0, 0, 0), RGB(200, 200, 200), 2);
        
        break;
        
        case LINE_2 :
  
        sprintf(data, "CURRENT : %02i.%02i     ",  ( int)temperature,(unsigned int)(abs(temperature)*100)%100);
        lcd.drawText(2, 160, data, RGB(0, 0, 0), RGB(200, 200, 200), 2);
        
        break;
    
    }
}
/**
  *This method is responsible for displaying the current action executed like Increasing or Decreasing temperature or no action.
  */   
int current_state=0;

void Display_action( int action)
{
    
      if(action == INCREASING_TEMPERATURE && current_state != INCREASING_TEMPERATURE)
        {
        current_state=INCREASING_TEMPERATURE;
        lcd.drawText(20,200,"                            ", RGB(255, 255, 2555), RGB(255, 255, 255), 2);
        lcd.drawText(20,220,"                            ", RGB(255, 255, 2555), RGB(255, 255, 255), 2);
        lcd.drawText(2,200,"Increasing the     ", RGB(0, 0, 0),  RGB(255, 180, 180), 2);
        lcd.drawText(2,220,"current Temperature", RGB(0, 0, 0),  RGB(255, 180, 180), 2);
        }
       
        
        else if(action == DECREASING_TEMPERATURE && current_state != DECREASING_TEMPERATURE)
        {
         current_state=DECREASING_TEMPERATURE;
        lcd.drawText(20,200,"                            ", RGB(255, 255, 2555), RGB(255, 255, 255), 2);
        lcd.drawText(20,220,"                            ", RGB(255, 255, 2555), RGB(255, 255, 255), 2);
        lcd.drawText(2,200,"Decreasing the     ", RGB(0, 0, 0), RGB(255, 180, 180), 2);
        lcd.drawText(2,220,"current Temperature", RGB(0, 0, 0),  RGB(255, 180, 180), 2);
        }
        
       else if( action == NO_ACTION && current_state != NO_ACTION )
        {
          current_state=NO_ACTION;
        
        lcd.drawText(0,200,"                        ", RGB(255, 255, 2555), RGB(255, 255, 255), 2);
        lcd.drawText(0,220,"                        ", RGB(255, 255, 255),  RGB(255, 255, 255), 2);
        lcd.drawText(2,200,"TEMPERATURE is OK  ", RGB(0, 0, 0), RGB(51, 255, 51), 2);
        lcd.drawText(2,220,"( within  range )  ", RGB(0, 0, 0), RGB(51, 255, 51), 2);
        action = 7;
        }

        
        
    }

 //========================================================================
/**
  *This method reads the temerature sensor DFR0024 DFRobot and converts the reading into equivalent temerature in degree celcuis.
  */   
 
  double get_temprature_in_Celcius()
  {
       
      
      byte data[12];
      byte addr[8];
      
      if ( !ds.search(addr)) 
      {
          
          ds.reset_search();
          return -1000;
      }
      
      if ( OneWire::crc8( addr, 7) != addr[7])
      {
          
          return -1000;
      }
      
      if ( addr[0] != 0x10 && addr[0] != 0x28)
      {
        
          return -1000;
      }
      
      ds.reset();
      ds.select(addr);
      ds.write(0x44,1); 
      
      byte present = ds.reset();
      ds.select(addr);    
      ds.write(0xBE); // Read Scratchpad
      
        
      for (int i = 0; i < 9; i++)
      { 
        data[i] = ds.read();
      }
        
      ds.reset_search();
        
      byte MSB = data[1];
      byte LSB = data[0];
      
      float tempRead = ((MSB << 8) | LSB); 
      float TemperatureSum = tempRead / 16;
        
      return TemperatureSum;
        
}
/*************************************************** END OF PROGRAM **********************************************/


