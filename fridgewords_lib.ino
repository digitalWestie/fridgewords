void scroll(){
  for (byte i=0; i<3; i++){
    //Serial.print("Printing ");
    //Serial.println(character_ints[i]);
    
    if(i >= message.length()){ break; } //in case message is shorter than 3
    
    if (character_ints[i] >= 32 && character_ints[i] <= 34) { //punctuation
      drawCharacter(punctuation[(character_ints[i]-32)], scroll_shifts[i]);
    } else if (character_ints[i] >= 65 && character_ints[i] <= 90){ //capitals
      drawCharacter(capitals[(character_ints[i]-65)], scroll_shifts[i]);
    } else { //don't know, make it a space
      drawCharacter(punctuation[0], scroll_shifts[i]);
    }
    
    /*Serial.print("Char ");
    Serial.print(i);
    Serial.print(" scroll: ");
    Serial.println(scroll_shifts[i]);*/
    
    scroll_shifts[i]++; //increment amount that character has been scrolled
  }
  
  if (scroll_shifts[0] == 4){ //moved on one
    Serial.println("Moving on ...");
    
    draw_count++; //increment number of characters drawn
    Serial.print("Draw count is now ");
    Serial.println(draw_count);
    
    //Move up the drawn shift and character counts 
    character_ints[0] = character_ints[1];
    scroll_shifts[0] = scroll_shifts[1];
    character_ints[1] = character_ints[2];
    scroll_shifts[1] = scroll_shifts[2];
    
    //Get and set the next character
    if (draw_count >= message.length()){
      Serial.println("message too long inserting space");
      character_ints[2] = 32;
    } else { //There is another character
      character_ints[2] = getCharacterInt(draw_count+2);
    }
    
    //int val of next character
    next_char = getCharacterInt(draw_count+2); 
    
    //Set scroll offset for next character
    if (next_char >= 65 && next_char <= 90){
      scroll_shifts[2] = scroll_shifts[1] - capitals[next_char-65][0];
    } else if (next_char >= 32 && next_char <= 34) {
      scroll_shifts[2] = scroll_shifts[1] - punctuation[next_char-32][0];
    }
  }
  
  if(draw_count == message.length()){ 
    Serial.println("resetting scroller..."); 
    resetScroller(); 
  }
}

void resetScroller(){
  draw_count = 0;
  int counter = 0;
  for (byte i=0; i<3; i++){
    if(i >= message.length()){ break; } //in case message is shorter than 3
    //get length of character
    if (getCharacterInt(i) >= 65 && getCharacterInt(i) <= 90){
      counter = counter - capitals[getCharacterInt(i)-65][0];
    } else if (getCharacterInt(i) >= 32 && getCharacterInt(i) <= 34){
      counter = counter - punctuation[getCharacterInt(i)-32][0];
    }
    scroll_shifts[i] = counter;
    character_ints[i] = getCharacterInt(draw_count + i);
  }
  
  /*Serial.print("Scroll shift 0: ");
  Serial.println(scroll_shifts[0]);
  Serial.print("Scroll shift 1 ");
  Serial.println(scroll_shifts[1]);
  Serial.print("Scroll shift 2 ");
  Serial.println(scroll_shifts[2]);*/
}

int getCharacterInt(byte message_index){
  return int(message.charAt(message_index));
  //return char_int;  
 //else if (char_int >= 48 && char_int <= 57) { char_int = char_int - 45; }
}

void drawCharacter(byte character[], int shift){  
  byte mask = 1;
  byte pixel_i = 0;
  byte strip_i = 0;
  
  //character[0] is the array size / width of character
  for (byte i = 1; i <= character[0]; i++) { 
    mask = 1;
    pixel_i = 0;
    for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask      
      if (strip_i + shift > 4) { break; }// all drawn
      if (strip_i + shift < 0) { break; } //not there yet
      if (character[i] & mask){
        strips[(strip_i + shift)].setPixelColor(pixel_i, 255, 255, 255);
      }
      pixel_i++;
      if (pixel_i > 4) { break; } //out of pixels
    }
    strip_i++;
  }
}

void showStrips(){
  for(uint16_t i=0; i < 5; i++) {
    strips[i].show();// Initialize all pixels to 'off'
  }
}

void stripsOff(){
  for(uint16_t i=0; i<5; i++) {
    for(uint16_t x=0; x<5; x++) {
      strips[i].setPixelColor(x, 0, 0, 0);
    }
  }
}

void printMacAddress() {
  // the MAC address of your Wifi shield
  byte mac[6];                     

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
}

void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
  { 
    Serial.println("Couldn't get a wifi connection");
    while(true);
  } 

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
  }
}

void serverConnect(){
  if (client.connect(server, 80) > 0) {
    Serial.println("Connected to server");
    client.println("GET /fridgewords/latest HTTP/1.1");
    client.println("Host: rorygianni.me.uk");
    client.println();
    has_connected = true;
  } else { 
    Serial.println("Connection failed"); 
    client.stop();
    has_connected = false;
  }
}

void readPage(){   //read the page, and capture & return everything between '<' and '>'
  stringPos = 0;
  memset(&inString, 0, 100); //clear inString memory
  while(true){
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
      //'{' is our begining character
      if (c == '{' ) { 
        //Ready to start reading the part 
        startRead = true; 
      } else if(startRead){
        if(c != '}'){ //'}' is our ending character
          inString[stringPos] = c;
          stringPos ++;
        } else {
          //got what we need here! We can disconnect now
          startRead = false;
          client.stop();
          client.flush();
          messageSet = true;
          message = inString;
          message.trim();
          Serial.print("Message set to ");
          Serial.println(message);
          return;
        }
      }
    } else {
      Serial.println("No stream available");
      messageSetRetries--;
      messageSet = false;
      return;
    } 
  }
}

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
  case ENC_TYPE_WEP:
    Serial.println("WEP");
    break;
  case ENC_TYPE_TKIP:
    Serial.println("WPA");
    break;
  case ENC_TYPE_CCMP:
    Serial.println("WPA2");
    break;
  case ENC_TYPE_NONE:
    Serial.println("None");
    break;
  case ENC_TYPE_AUTO:
    Serial.println("Auto");
    break;
  } 
}

