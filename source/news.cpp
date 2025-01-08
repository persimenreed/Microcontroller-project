/**
 * @file   news.cpp
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#include "DFRobot_RGBLCD.h"
#include "GlobalSign_RSA_OV_SSL_CA_2018.h"
#include "HTS221Sensor.h"
#include "buttons.h"
#include "mbed.h"
#include "startfile.h"
#include "string"
#include "temphum.h"

#include "wifi.h"
#include <cstdio>

#define JSON_NOEXCEPTION
#include "json.hpp"

#define WAIT_TIME_MS 2000

using json = nlohmann::json;
InterruptIn *button_set_news = init_button_news();


// FOR THE CONNECTION PHASE WE USE THE SAME CODE AS SHOWN IN LECTURES


// print the current Yahoo news. Connects to wifi and fetch from API.
int print_news() {

  lcd.init();
  lcd.display();
  lcd.printf("Loading news ...");

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
    socket.set_timeout(3000);

    result = socket.open(network);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to open TLSSocket: %s\n", get_nsapi_error_string(result));
      continue;
    }

    // https://news.yahoo.com/rss/
    const char host[] = "news.yahoo.com";

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

    // Set certificarte for yahoo page (HTTPS)
    result = socket.set_root_ca_cert(GlobalSign_RSA_OV_SSL_CA_2018);

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
    const std::string request = "GET /rss HTTP/1.1\r\n"
                                "Host: news.yahoo.com \r\n"
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

    // Buffer for HTTP responses
    static constexpr size_t HTTP_RESPONSE_BUF_SIZE = 4000;

    // Use static keyword to move response[] from stack to bss
    static char response[HTTP_RESPONSE_BUF_SIZE + 1]; // Plus 1 for '\0'
    // Read response
    result = read_response(&socket, response, HTTP_RESPONSE_BUF_SIZE);

    // Check result
    if (result < 0) {
      printf("Failed to read response: %d\n", result);
      continue;
    }

    
    response[result] = '\0';

    // Sort out items from <title>. Both headline and news comes from <title>.
    char *item_start = strstr(response, "<title>");
    int count = 0;
    char news_string[41];
    char news_string2[41];
    char news_string3[41];
    char headline[11];

    while (item_start != NULL) {
        
      // Find the start of <title>
      char *item_content_start = item_start + strlen("</title>") - 1;

      // Find the end of <title>
      char *item_end = strstr(item_content_start, "</title>");

      if (item_end != NULL) {
        // find content length
        size_t item_length = item_end - item_content_start;

        // creating temporary string, containing one element (1 news title) and allocate memory to string
        char *item_content = (char *)malloc(item_length + 1);
        strncpy(item_content, item_content_start, item_length);
        item_content[item_length] = '\0';

        // Print current item
        printf("%s\n", item_content);

        // first element is header
        if (count == 0) {
          strncpy(headline, item_content, 10);
          headline[10] = '\0';

        // second element is also header, so third element is first news article
        } else if (count == 2) {
          strncpy(news_string, item_content, 35);
          news_string[35] = '\0';
          strcat(news_string, " ... ");
          news_string[40] = '\0';

        // second news article
        } else if (count == 3) {
          strncpy(news_string2, item_content, 35);
          news_string2[35] = '\0';
          strcat(news_string2, " ... ");
          news_string2[40] = '\0';

        // third news article
        } else if (count == 4) {
          strncpy(news_string3, item_content, 35);
          news_string3[35] = '\0';
          strcat(news_string3, " ... ");
          news_string3[40] = '\0';
        }

        // Free memory to grab new item
        free(item_content);

        // search for next item
        item_start = strstr(item_end, "<title>");
        count++;
      } else {
        // end loop if </title> is not found. This will happen since buffer size is just 4000.
        break;
      }
    }

    lcd.clear();
    network->disconnect();

    state = true;
    int news_timer = 0;
    int news_count = 0;

    while (state) {

        // print first article for like 10 seconds
        if(news_count < 50){

            lcd.setCursor(1, 0);
            lcd.printf(headline);

            lcd.setCursor(0, 1);
            lcd.printstr(news_string);

        // print second article for like 10 seconds
        }else if(news_count < 100){

            lcd.setCursor(1, 0);
            lcd.printf(headline);

            lcd.setCursor(0, 1);
            lcd.printstr(news_string2);

        // print third article for like 10 seconds
        }else if(news_count < 150){

            lcd.setCursor(1, 0);
            lcd.printf(headline);

            lcd.setCursor(0, 1);
            lcd.printstr(news_string3);

        // reset timer to print first article again.
        }else{
            news_count = 0;
        }

      lcd.scrollDisplayLeft();

        // return to time screen if user presses button again
      if (button_set_news->read()) {
        return 2;
      }

      // re-print news every 5 minutes.
      if (news_timer >= 60 * 5 * 5) {
        print_news();
      }

      news_timer++;
      news_count++;
      thread_sleep_for(200);

        /*
            Note for censor: We were not able to print the whole news message. We only fetched from one news source aswell.
        */

    }
  }
  return 2;
}

// sends back to print_news
void update_news() { print_news(); }
