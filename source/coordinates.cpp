/**
 * @file   coordinates.cpp
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#include "DFRobot_RGBLCD.h"
#include "HTS221Sensor.h"
#include "buttons.h"
#include "mbed.h"
#include "startfile.h"
#include "temphum.h"

#include "USERTrust-RSA-Certification-Authority.h"
#include "wifi.h"
#include <cstdio>

#define JSON_NOEXCEPTION
#include "json.hpp"
#define WAIT_TIME_MS 2000
using json = nlohmann::json;

#define WAIT_TIME_MS 1000

AnalogIn ain(A5, 3.3f);


// FOR THE CONNECTION PHASE WE USE THE SAME CODE AS SHOWN IN LECTURES



// Function to set latitude
float set_lat() {
  InterruptIn *button_set_weather = init_button_weather();
  float lat = 0;

  bool state = true;
  while (state) {

    printf("lat: %.4f\n", ain.read() * 90);

    lat = ain.read() * 90;

    lcd.clear();
    lcd.printf("Set latitude:");
    lcd.setCursor(0, 1);
    lcd.printf("%.4f", lat);

    if (button_set_weather->read()) {
      state = false;
    }

    thread_sleep_for(400);
  }

  lat = ain.read() * 90;

  return lat;
}


// Function to set longitude
float set_lon() {
  InterruptIn *button_set_weather = init_button_weather();
  float lon = 0;

  bool state = true;
  while (state) {

    printf("lon: %.4f\n", ain.read() * 180);

    lon = ain.read() * 180;

    lcd.clear();
    lcd.printf("Set longitude:");
    lcd.setCursor(0, 1);
    lcd.printf("%.4f", lon);

    if (button_set_weather->read()) {
      state = false;
    }

    thread_sleep_for(400);
  }

  lon = ain.read() * 180;

  return lon;
}

// Function which connects to wifi and API
void print_weather(float lat, float lon) {

  lcd.init();
  lcd.display();
  lcd.printf("Loading weather ...");
  int date_time_unix;

  // Initialise the digital pin LED1 as an output
  DigitalOut led1(LED1);

  // Get pointer to default network interface
  NetworkInterface *network = NetworkInterface::get_default_instance();

  if (!network) {
    printf("Failed to get default network interface\n");
    while (1);
  }

  nsapi_size_or_error_t result;

  do {
    printf("Connecting to the network...\n");
    result = network->connect();

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to connect to network: %s\n",
             get_nsapi_error_string(result));
    }
  } while (result != NSAPI_ERROR_OK);

  SocketAddress address;
  result = network->get_ip_address(&address);

  if (result != NSAPI_ERROR_OK) {
    printf("Failed to get local IP address: %s\n",
           get_nsapi_error_string(result));
    while (1)
      ;
  }

  printf("Connected to WLAN and got IP address %s\n", address.get_ip_address());

  bool state = true;
  while (state) {
    led1 = !led1; // Toggle "running" LED
    thread_sleep_for(WAIT_TIME_MS);

    TLSSocket socket;

    // Configure timeout on socket receive
    // (returns NSAPI_ERROR_WOULD_BLOCK on timeout)
    socket.set_timeout(3000);

    result = socket.open(network);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to open TLSSocket: %s\n", get_nsapi_error_string(result));
      continue;
    }

    const char host[] = "api.weatherbit.io";

    // Get IP address of host (web server) by name
    result = network->gethostbyname(host, &address);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to get IP address of host %s: %s\n", host,
             get_nsapi_error_string(result));
      continue;
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());

    // Set server TCP port number, 443 for HTTPS
    address.set_port(443);

    // Set the root certificate of the web site.
    // See include/ipify_org_ca_root_certificate.h for how to download the
    // certificate and add it to source code
    result = socket.set_root_ca_cert(USERTrust_RSA_Certification_Authority);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to set root certificate of the web site: %s\n",
             get_nsapi_error_string(result));
      continue;
    }

    socket.set_hostname(host);

    // Connect to server at the given address
    result = socket.connect(address);

    // Check result
    if (result != NSAPI_ERROR_OK) {
      printf("Failed to connect to server at %s: %s\n", host,
             get_nsapi_error_string(result));
      continue;
    }

    printf("Successfully connected to server %s\n", host);

    // Create HTTP request
    const std::string request = "GET /v2.0/current?lat=" + std::to_string(lat) +
                            "&lon=" + std::to_string(lon) +
                            "&key=73d85b6bbb9e46fd9c3ecf00d2155bee HTTP/1.1\r\n"
                            "Host: api.weatherbit.io\r\n"
                            "User-Agent: persimenreed.no persr18@uia.no\r\n"
                            "Connection: close\r\n"
                            "\r\n";



    // Send request
    result = send_request(&socket, request.c_str());

    // Check result
    if (result < 0) {
      printf("Failed to send request: %d\n", result);
      continue;
    }

    // We need to read the response into memory. The memory object is called a
    // buffer. If you make this buffer static it will be placed in bss (memory
    // for global and static variables) and won't use the main thread stack
    // memory
    static char buffer[4000];

    // Read response
    result = read_response(&socket, buffer, sizeof(buffer));

    // Check result
    if (result < 0) {
      printf("Failed to read response: %d\n", result);
      continue;
    }

    // Find the start and end of the JSON data.
    // If the JSON response is an array you need to replace this with [ and ]
    char *json_begin = strchr(buffer, '{'); // Search first occurence of {
    char *json_end = strrchr(buffer, '}');  // Search last occurence of }

    // Check if we actually got JSON in the response
    if (json_begin == nullptr || json_end == nullptr) {
      printf("Failed to find JSON in response\n");
      continue;
    }

    // End the string after the end of the JSON data in case the response
    // contains trailing data
    json_end[1] = 0;

    printf("\nJSON response:\n%s\n", json_begin);

    // Parse response as JSON, starting from the first {
    json document = json::parse(json_begin);

    if (document.is_discarded()) {
      printf("The input is invalid JSON\n");
      continue;
    }


    // Get description from API. ex: Cloudy
    std::string description;
    document["data"][0]["weather"]["description"].get_to(description);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("%s", description.c_str());
    printf("Description: %s\n", description.c_str());

    // Get temperature from API. Value is rounded to nearest int.
    float temp;
    document["data"][0]["app_temp"].get_to(temp);

    lcd.setCursor(0, 1);
    lcd.printf("%2.f degrees", temp);
    printf("Temp: %2.f\n", temp);


    // Disconnect from network after info is fetched.
    network->disconnect();
    state = false;

  }
}

// function to manually set coordinates
int set_coordinates() {

    InterruptIn *button_set_coordinates = init_button_set_coordinates();
    InterruptIn *button_set_weather = init_button_weather();

  float lat = set_lat();
  float lon = set_lon();

  printf("Latitude: %.4f, Longitude: %.4f\n", lat, lon);

  print_weather(lat, lon);

  bool state = true;
  int weather_timer = 0;

  while(state){

      if(button_set_weather->read()){
          state=false;
      }

    // fetch weather again after 15 minutes
      if(weather_timer >= 60*15){
            print_weather(lat, lon);
        }
        
         weather_timer++;
         printf("Time since update: %i\n", weather_timer);
         thread_sleep_for(1000);

  }
  return 2;
}

// main function in the file.
int set_weather(float lat, float lon){
    int weather_timer = 0;
    InterruptIn *button_set_coordinates = init_button_set_coordinates();
    InterruptIn *button_set_weather = init_button_weather();

    // print weather from latitude and longitude we got from startfile.cpp.
    print_weather(lat, lon);

    bool state = true;
    while(state){

        // check if user wants to enter coords manually
        if(button_set_coordinates->read()){
            set_coordinates();
        };

        // check if user wants to go back
        if(button_set_weather->read()){
            state=false;
        };

        // fetch weather again after 15 minutes
        if(weather_timer >= 60*15){
            print_weather(lat, lon);
        }
        
         weather_timer++;
         printf("Time since update: %i\n", weather_timer);
         thread_sleep_for(1000);
    }
    return 2;
}
