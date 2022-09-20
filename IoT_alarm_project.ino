#include <cstdlib>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>


#define FIREBASE_API_KEY ""
#define FIREBASE_DATABASE_URL "" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

//#define WIFI_SSID "Net_578463"
//#define WIFI_PASSWORD "RLENSOLSQBRLENSOLSQB"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""


#define echoPin D2
#define trigPin D3
#define buzzerPin D6
#define ledPin D7


// firebase variables
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// setup variables
bool signupOK = false;




// Alarm variables
const int password = 1312;
int input = -1;
bool alarmIsTriggered = false;
bool alarmIsActive = false;

int defaultDistance = 30;
int currentDistance = 5;

int distance;
long duration;



void setup()
{
    Serial.begin(9600);
    
    pinMode(echoPin, INPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(ledPin, OUTPUT);


    // connect to wifi.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_DATABASE_URL;
    
    Firebase.reconnectWiFi(true);

    Serial.print("Sign up new user... ");

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;
    }
    else
        Serial.printf("%s\n", config.signer.signupError.message.c_str());

    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    SetDefaults();

    defaultDistance = GetDefaultDistance();
}



void loop()
{
  if (Firebase.ready())
  {
    input = GetPassword();

    if (password == input)
    {
        TurnOffAlarm();
        
        SetActiveStatus(!alarmIsActive); // change active status
        
        ResetPassword();
    }

    Randomize();

    currentDistance = GetSensorReadings();

    if (alarmIsActive && (abs(defaultDistance - currentDistance) > 5))
    {
        TriggerAlarm();
    }
  }
}

int GetSensorReadings()
{
    // Clears the trigPin condition
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
    // Displays the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    return distance;
}

void Randomize()
{
    int randomNumber = random(300);
    
    Firebase.set(fbdo, "IoT/random", randomNumber);
}

int GetDefaultDistance(){
    Firebase.RTDB.getInt(&fbdo, "/IoT/DefaultDistance");

    return fbdo.to<int>();
}

int GetPassword()
{
    Firebase.RTDB.getInt(&fbdo, "/IoT/Password");

    return fbdo.to<int>();
}

void ResetPassword()
{
    Firebase.set(fbdo, "IoT/Password", -1);
}

void SetActiveStatus(bool newStatus)
{
    alarmIsActive = newStatus;

    if (alarmIsActive)
    {
        digitalWrite(ledPin, HIGH);
    }
    
    Firebase.set(fbdo, "IoT/AlarmIsActive", alarmIsActive);
}

void SetTriggeredStatus(bool newStatus)
{
    alarmIsTriggered = newStatus;
    
    Firebase.set(fbdo, "IoT/AlarmIsTriggered", alarmIsTriggered);
}

void TriggerAlarm() 
{
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, LOW);

    if (!alarmIsTriggered)
    {
        SetTriggeredStatus(true);
    }
}

void TurnOffAlarm() 
{
    if (alarmIsTriggered)
    {
      digitalWrite(ledPin, LOW); // transition period
      digitalWrite(buzzerPin, HIGH); // transition period  // buzzer
      delay(100);
      digitalWrite(ledPin, HIGH);
      digitalWrite(buzzerPin, LOW); // buzzer
      delay(100);
      digitalWrite(ledPin, LOW);
      digitalWrite(buzzerPin, HIGH); // buzzer
      delay(100);
      digitalWrite(ledPin, HIGH);
      digitalWrite(buzzerPin, LOW); // buzzer
      delay(100);
      digitalWrite(ledPin, LOW); // off
      digitalWrite(buzzerPin, HIGH); // off  // buzzer

      SetTriggeredStatus(false);
    }
    else
    {
      digitalWrite(ledPin, LOW); // keep off
      digitalWrite(buzzerPin, HIGH); // keep off  // buzzer
    }
}

void SetDefaults()
{
    SetActiveStatus(false);
    SetTriggeredStatus(false);
    ResetPassword();
    TurnOffAlarm();
}






