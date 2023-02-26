void setup() {
  
  Serial.begin(115200);
  Serial.println("Ready");
}


void loop(){
//quick case for single char commands
 if(Serial.available()){    
    switch(Serial.read()){
    case 'a': break;
    case 'b': break;
    case 'c': break;
    case 'd': break;
    case 'e': break;
    case 'f':  break;
    case 'g': break;
    case 'h': break;
    case 'i':  break;
    case 'j':  break;
    default:  break;
    }
  }
}