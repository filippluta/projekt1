#include <LiquidCrystal.h>

#define X_MIN_PIN           3
#define X_MAX_PIN           2

#define Y_MIN_PIN          14
#define Y_MAX_PIN          15

#define Z_MIN_PIN          18
#define Z_MAX_PIN          19

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62

#define E0_STEP_PIN        26
#define E0_DIR_PIN         28
#define E0_ENABLE_PIN      24

#define SPEED              100

uint8_t x_max_pin_value, y_max_pin_value, z_max_pin_value;
int x_steps=0, y_steps=0, z_steps=0, e_steps=0;
int temp_steps;

String s;
bool wyslane = true;

// TEMP
#define TEMP 13
#define HEAT 10
#define FAN 44

#define A -6.458565579810880e-07
#define B 0.001142619771655
#define C -0.758914926930814
#define D 2.765483204012756e+02

int temp;
double real_temp;

float Kp = 5, Ki = 4;
float error;
float i_error = 0;
float pid_output;
int temp_setpoint = 0;
int e_counter = 0;

LiquidCrystal lcd(16,17,23,25,27,29);

long counter = 0;
float dt;

void setup() 
{
  Serial.begin(57600);

  configure_limits();
  configure_steppers();
  configure_heater();
  
  counter = micros();
}

void loop() 
{
  
  x_max_pin_value = digitalRead(X_MAX_PIN);
  y_max_pin_value = digitalRead(Y_MAX_PIN);
  z_max_pin_value = digitalRead(Z_MAX_PIN); 

  move_steppers();
  
  delayMicroseconds(SPEED);

  digitalWrite(X_STEP_PIN, LOW); 
  digitalWrite(Y_STEP_PIN, LOW);
  digitalWrite(Z_STEP_PIN, LOW);
  digitalWrite(E0_STEP_PIN, LOW);

  delayMicroseconds(SPEED);

  read_matlab();
  heat();
  
  dt = micros() - counter;
  counter = micros();
}

void calculate_pid()
{
  error = (float)temp_setpoint - real_temp;
  
  i_error += error * Ki * dt;
  if (i_error > 100) i_error = 100;
  
  pid_output = (255 * error*Kp / (temp_setpoint - 20)) + i_error;
  if (pid_output >= 255) pid_output = 255;
}

void move_steppers()
{
  if (x_max_pin_value == 0 && x_steps > 0)
  {
    digitalWrite(X_STEP_PIN, HIGH); 
    x_steps--;
  } 
  
  if(y_max_pin_value == 0 && y_steps > 0)
  {
    digitalWrite(Y_STEP_PIN, HIGH);
    y_steps--;
  }

  if (z_max_pin_value == 0 && z_steps > 0)
  {
    digitalWrite(Z_STEP_PIN, HIGH);
    z_steps--;
  }

  if (e_steps > 0)
  {
    if (e_counter >= 10)
    {
      digitalWrite(E0_STEP_PIN, HIGH);
      e_counter = 0;
    }
    e_steps--;
    e_counter++; 
  }
}

void check_direction()
{
   if (x_steps > 0)
  {
    digitalWrite(X_DIR_PIN, LOW); 
  }
  else
  {
    x_steps *= -1;
    digitalWrite(X_DIR_PIN, HIGH);
  }

  if (y_steps > 0)
  {
    digitalWrite(Y_DIR_PIN, LOW); 
  }
  else
  {
    y_steps *= -1;
    digitalWrite(Y_DIR_PIN, HIGH);
  }

  if (z_steps > 0)
  {
    digitalWrite(Z_DIR_PIN, LOW);   
  }
  else
  {
    z_steps *= -1;
    digitalWrite(Z_DIR_PIN, HIGH);
  }

  if (e_steps > 0)
  {
    digitalWrite(E0_DIR_PIN, LOW);   
  }
  else
  {
    e_steps *= -1;
    digitalWrite(E0_DIR_PIN, HIGH);
  }
}

void configure_steppers()
{
   // SILNIK KROKOWY X
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);

  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(X_DIR_PIN, LOW);
  digitalWrite(X_STEP_PIN, LOW);

  // SILNIK KROKOWY Y
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);

  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Y_DIR_PIN, LOW);
  digitalWrite(Y_STEP_PIN, LOW);

  // SILNIK KROKOWY Z
  pinMode(Z_STEP_PIN, OUTPUT);
  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);

  digitalWrite(Z_ENABLE_PIN, LOW);
  digitalWrite(Z_DIR_PIN, LOW);
  digitalWrite(Z_STEP_PIN, LOW);

  // SILNIK KROKOWY E0
  pinMode(E0_STEP_PIN, OUTPUT);
  pinMode(E0_DIR_PIN, OUTPUT);
  pinMode(E0_ENABLE_PIN, OUTPUT);

  digitalWrite(E0_ENABLE_PIN, LOW);
  digitalWrite(E0_DIR_PIN, LOW);
  digitalWrite(E0_STEP_PIN, LOW);
}

void configure_limits()
{
  // LIMITY
  pinMode(X_MAX_PIN, INPUT);
  digitalWrite(X_MAX_PIN, HIGH);

  pinMode(Y_MAX_PIN, INPUT);
  digitalWrite(Y_MAX_PIN, HIGH);

  pinMode(Z_MAX_PIN, INPUT);
  digitalWrite(Z_MAX_PIN, HIGH);

}

void configure_heater()
{
  pinMode(HEAT, OUTPUT);
  digitalWrite(HEAT, LOW);

  pinMode(FAN, OUTPUT);
  digitalWrite(FAN, HIGH);

  lcd.begin(20,4);
}

void heat()
{
  temp = analogRead(TEMP);
  real_temp = A * (double)pow(temp,3) + B * (double)pow(temp,2) + C * (double)temp + D;

  if (real_temp < temp_setpoint)
  {
    calculate_pid();    
    analogWrite(HEAT, floor(pid_output));
  }
  else 
  {
    digitalWrite(HEAT, LOW);
  }

  lcd.setCursor(0,0);
  lcd.print(real_temp);
}

void read_matlab()
{
  while (Serial.available() > 0)
  {
    s = Serial.readStringUntil('\n');
    sscanf(s.c_str(), "x %d y %d z %d e %d temp %d", &x_steps, &y_steps, &z_steps, &e_steps, &temp_setpoint);
    wyslane = false;
    lcd.setCursor(0,1);
    lcd.print(temp_setpoint);
    check_direction();
  }

  if (x_steps == 0 && y_steps == 0 && z_steps == 0 && !wyslane)
  {
    Serial.println("ok");
    wyslane = true;
  }
}

