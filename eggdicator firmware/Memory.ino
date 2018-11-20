bool download_config(){
  
  int conf_values[CONF_SIZE];
  Serial.flush();
  Serial.println("x: ");
  Serial.flush();
  while (!Serial.available());
  if (Serial.available() > 0)
    Serial.readBytesUntil('\n', read_buffer, 200);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(read_buffer);
  if (root.success()) 
  {
    JsonArray& gen_configs = root["confs"];
    JsonArray& led_color = root["leds"];
    for (int i = 0; i < CONF_SIZE; i++)
      conf_values[i] = gen_configs[i];
  }
}
