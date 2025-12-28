#pragma once
#include "esphome.h"
#include "esp_http_client.h"
#include "ArduinoJson.h"

class HAHttpSensor : public PollingComponent, public Sensor {
 public:
  HAHttpSensor(const std::string &url, const std::string &token, uint32_t update_interval_ms)
      : PollingComponent(update_interval_ms), url_(url), token_(token) {}

  void setup() override {
    // เริ่ม setup ถ้าจำเป็น
  }

  void update() override {
    // เรียก HTTP GET
    esp_http_client_config_t config = {};
    config.url = url_.c_str();
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_http_client_set_header(client, "Authorization", ("Bearer " + token_).c_str());
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
      int content_length = esp_http_client_get_content_length(client);
      std::vector<char> buffer(content_length + 1);
      int len = esp_http_client_read(client, buffer.data(), content_length);
      if (len > 0) {
        buffer[len] = 0;
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, buffer.data());
        if (doc.containsKey("state")) {
          float value = atof(doc["state"].as<const char*>());
          this->publish_state(value);
        }
      }
    }

    esp_http_client_cleanup(client);
  }

 protected:
  std::string url_;
  std::string token_;
};
