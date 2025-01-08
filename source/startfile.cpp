/**
 * @file   startfile.cpp
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#include "startfile.h"
#include "cloudflare_inc_ECC_CA_3.h"
#include "mbed.h"
#include "wifi.h"
#include <cstdio>
#include "string"

#include "DFRobot_RGBLCD.h"
#include "HTS221Sensor.h"

#define JSON_NOEXCEPTION
#include "json.hpp"

#define WAIT_TIME_MS 2000

using json = nlohmann::json;

// https://api.ipgeolocation.io/timezone?apiKey=d27ee442a9b84045aa545601f65e8dce


// FOR THE CONNECTION PHASE WE USE THE SAME CODE AS SHOWN IN LECTURES



DFRobot_RGBLCD lcd(16, 2, D14, D15);
Timer t;

// function to print unix time to lcd for 2 seconds
void print_unix(int date_time_unix) {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Unix epoch time:");
  lcd.setCursor(0, 1);
  lcd.printf("%i", date_time_unix);

  printf("Unix epoch time: %i\n", date_time_unix);
  thread_sleep_for(WAIT_TIME_MS);
  return;
}

// function to print latitude and longitude to lcd for 2 seconds
void print_lat_lon(std::string latitude, std::string longitude) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Lat: %s", latitude.c_str());
  lcd.setCursor(0, 1);
  lcd.printf("Lon: %s", longitude.c_str());

  printf("Lat: %s, Lon: %s\n", latitude.c_str(), longitude.c_str());
  thread_sleep_for(WAIT_TIME_MS);
  return;
}

// function to print city to lcd for 2 seconds
void print_city(std::string city_name) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("City: ");
  lcd.setCursor(0, 1);
  lcd.printf("%s", city_name.c_str());

  printf("City: %s\n", city_name.c_str());
  thread_sleep_for(WAIT_TIME_MS);
  return;
}

// function that eventually returns struct containing lat, lon, and unix. Check startfile.h
Values startfile() {

  // thread_sleep_for(WAIT_TIME_MS);
  Values v;

  lcd.init();
  lcd.display();
  lcd.printf("Loading ...");
  int date_time_unix;

  // Initialise the digital pin LED1 as an output
  DigitalOut led1(LED1);

  // Get pointer to default network interface
  NetworkInterface *network = NetworkInterface::get_default_instance();

  if (!network) {
    printf("Failed to get default network interface\n");
    while (1)
      ;
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
    socket.set_timeout(1000);

    result = socket.open(network);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to open TLSSocket: %s\n", get_nsapi_error_string(result));
      continue;
    }

    const char host[] = "api.ipgeolocation.io";

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
    result = socket.set_root_ca_cert(cloudflare_inc_ECC_CA_3);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to set root certificate of the web site: %s\n",
             get_nsapi_error_string(result));
      continue;
    }

    // ### IMPORTANT ###
    // Often a server has several virtual hosts to serve, and
    // it is therefore necessary to tell which host to connect to,
    // in order to let the TLS handshake succeed
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
    const char request[] =
        "GET /timezone?apiKey=d27ee442a9b84045aa545601f65e8dce HTTP/1.1\r\n"
        "Host: api.ipgeolocation.io\r\n"
        "Connection: close\r\n"
        "\r\n";

    // Send request
    result = send_request(&socket, request);

    // Check result
    if (result < 0) {
      printf("Failed to send request: %d\n", result);
      continue;
    }

    static char buffer[2000];

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

    // unix time
    document["date_time_unix"].get_to(date_time_unix);

    // Count time since we got unix time
    t.reset();
    t.start();

    // fetch latitude
    std::string latitude;
    document["geo"]["latitude"].get_to(latitude);

    // fetch longitude
    std::string longitude;
    document["geo"]["longitude"].get_to(longitude);

    // fetch city
    std::string city_name;
    document["geo"]["city"].get_to(city_name);

    // add lat and lon to struct
    v.lat = std::stof(latitude);
    v.lon = std::stof(longitude);

    // Print info in functions:
    print_unix(date_time_unix);
    print_lat_lon(latitude, longitude);
    print_city(city_name);

    network->disconnect();
    state = false;
  }

  // Find UNIX time + time since UNIX time (approx. 6 seconds)
  int rtc = date_time_unix +
            duration_cast<std::chrono::seconds>(t.elapsed_time()).count();

    v.rtc = rtc;

  // return struct containing lat, lon and rtc
  return v;
}