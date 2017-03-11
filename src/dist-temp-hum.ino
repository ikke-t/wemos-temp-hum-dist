#include "DHT.h"
#include <ESP8266WiFi.h>

const char* ssid = "ssid-name";
const char* password = "ssid-password";

WiFiServer server(80);

#define ECHO_PIN D1       // Echo pin
#define TRIG_PIN D2       // Trigger pin

#define DHTPIN D5         // DHT input pin
#define DHTTYPE DHT21     // AM2301

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin (9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

bool read_hum(float *h)
{
  bool ret = true;
  *h = dht.readHumidity();

  // check if returns are valid, if they are NaN (not a number)
  // then something went wrong!
  if (isnan(*h))
  {
    ret = false;
  }
  return ret;
}

bool read_temp(float *t)
{
  bool ret = true;
  *t = dht.readTemperature();

  // check if return is valid, if it is NaN (not a number)
  // then something went wrong!
  if (isnan(*t))
  {
    ret = false;
  }
  return ret;
}

long measure_distance()
{
  long duration;
  // The following TRIG_PIN/ECHO_PIN cycle is used to determine the
  // distance of the nearest object by bouncing soundwaves off of it.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  //Calculate the distance (in cm) based on the speed of sound.
  return duration/58.2;
}

void handle_http_api(float t, float h, long d)
{
  // http://www.esp8266learning.com/wemos-webserver-example.php
  // Check if a client has connected
  WiFiClient client = server.available();

  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  if (request.indexOf("/data_out") != -1){
    client.print("{Temperature: ");
    client.print(t);
    client.print(",");
    client.print("Humidity: ");
    client.print(h);
    client.print(",");
    client.print("Distance: ");
    client.print(d);
    client.println("}");
  }
  client.println("</html>");

  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");
  return;
}

void loop()
{
  float h;
  float t;
  long d;

  if (read_temp(&t))
  {
    Serial.print("{Temperature: ");
    Serial.print(t);
    Serial.println("}");
  } else {
    Serial.println("Failed to read temperature from DHT");
  }

  if (read_hum(&h))
  {
    Serial.print("{Humidity: ");
    Serial.print(h);
    Serial.println("}");
  } else {
    Serial.println("Failed to read humidity from DHT");
  }

  d = measure_distance();
  Serial.print("{Distance: ");
  Serial.print(d);
  Serial.println("}");

  handle_http_api(h, t, d);

  //Delay x*ms before next reading.
  delay(30*1000);
}
