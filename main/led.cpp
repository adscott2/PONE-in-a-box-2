#include "led.h"
#include "config.h"

void LEDSystem::initLEDs(Adafruit_NeoPixel* strs, int nStrs, Adafruit_7segment* clk){

  this->strips = strs;
  this->numStrips = nStrs;
  this->clock = clk;
  this->currentTime = 0;
  for (int i = 0; i < numStrips; ++i) {
    strips[i].begin();
    strips[i].show();
    strips[i].setBrightness(100);
  }
  clear();
  clock->clear();
  for (int i = 0; i < 5; ++i) { //left to right (0:digit, 1:digit, 2:colon(?), 3:digit, 4:digit)
    clock->writeDigitNum(i, 0,false); //false disables drawing extra colons/dots around the board
  }
  clock->drawColon(true);
  clock->writeDisplay();

  int rows = 7;
  float distance = 16.0;
  generate_grid(rows, distance);
  Serial.println("Geometry Generated");
}
float LEDSystem::checkVolume(coordinate LED, coordinate coor, int radius)
{
    int xdiff=abs(LED.x-coor.x);
    int ydiff=abs(LED.y-coor.y);
    int zdiff=abs(LED.z-coor.z);
	float scale=0;
    if(xdiff<radius && ydiff<radius && zdiff<radius){
      float dist= sqrt(pow(xdiff,2)+pow(ydiff,2)+pow(zdiff,2));
      scale=1-(dist/radius);
      if (scale<0) scale=0;
	}
	return scale;
}

void LEDSystem::setVolume(coordinate coor, int radius, int r, int g, int b) {
float t1 = 1000.;
float t2 = 2000.;
float t3 = 3000.;
float t4 = 4000.;
float t5 = 5000.;
float t6 = 6000.;
if (currentTime < t1){
  r = 255;
  g = currentTime/t1*255;
  b = 0;    
  }
if (currentTime >= t1 && currentTime <= t2){
  r = (255 - (currentTime-t1)/(t2-t1)*255);
  g = 255;
  b = 0;    
}
if (currentTime >= t2 && currentTime <= t3){
  r = 0;
  g = 255;
  b = (currentTime-t2)/(t3-t2)*255;    
}
if (currentTime >= t3 && currentTime <= t4){
  r = 0;
  g = (255 - (currentTime-t3)/(t4-t3)*255);  
  b = 255;    
}      
if (currentTime >= t4 && currentTime <= t5){
  r = (currentTime-t4)/(t5-t4)*255;
  g = 0;
  b = 255;    
}
if (currentTime >= t5 && currentTime <= t6){
  r = 255;
  g = 0;
  b = (255 - (currentTime-t5)/(t6-t5)*255);
}
if (currentTime > t6){
  r = 255;
  g = 255;
  b = 255;    
}	
  for(int i=0; i<460;i++){
      float scale = checkVolume(points[i],coor, radius);
      if(scale>0){
	    setPixel(pins[i].strip,pins[i].pixel,int(scale*r),int(scale*g),int(scale*b));
    }
  }
}
void LEDSystem::drawCombined(coordinate p1, int r1, colour c1, coordinate p2, int r2, colour c2){
  for(int i=0; i<460;i++){
	float s1 = checkVolume(points[i],p1,r1);
	float s2 = checkVolume(points[i],p2,r2);
    if(s1>0 || s2>0){
	  setPixel(pins[i].strip,pins[i].pixel,int(s1*c1.r+s2*c2.r),int(s1*c1.g+s2*c2.g),int(s1*c1.b+s2*c2.b)); //should we include a scale factor?
    }
  }

}

void LEDSystem::show() {
  for (int i = 0; i < 7; i++) {
    strips[i].show();
  }
}
void LEDSystem::clear(){
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < LED_COUNT; j++) {
      strips[i].setPixelColor(j, strips[i].Color(0, 0, 0));
    }
  strips[i].show();
  }

}

void LEDSystem::generate_grid(int rows, float distance) {
  int cnt=0;
  for (int row = 0; row < rows; row++) {
    int num_points = (row % 2 == 0) ? 7 : 6;
    bool top = (row % 2 == 0);
    int ledcnt=99;
    for (int col = 0; col < num_points; col++) {
      float x = col * distance;
      float y = row * distance * sqrt(3) / 2;
      if (row % 2 == 1) x += distance / 2;      
      x += 2;
      y += (100.0-(6.0*distance*sqrt(3)/2.0))/2.0;
      for(int lay=0; lay<10; lay++){
        int ledid = top ? (ledcnt - 9 + lay) : (ledcnt - lay);
        float z=lay*10+5;
        coordinate pnt = {int(x), int(y), int(z)};
        int strip_id = get_pin_id(int(y));
        pixel_ID pxl={strip_id,ledid};
        points[cnt]=pnt;
        pins[cnt]=pxl;
        cnt++;
      }
      top = !top;
      ledcnt -= 13;
    }
  }
}
int LEDSystem::get_pin_id(int y) {
  const int keys[] = {8, 22, 36, 50, 63, 77, 91}; 
  const int values[] = {2, 3, 4, 5, 6, 7, 8};
  for (int i = 0; i < 7; i++) {
    if (keys[i] == y) {
      return values[i];
    }
  }
  Serial.println("No Valid Key found");
  return -1; // Return -1 if the key is not found
}


void LEDSystem::printMap(){
  // Print the map for verification
  Serial.println("print map");
  for (int i=0; i<460; ++i) {
    Serial.print("Coordinate: ");
    Serial.print(points[i].x);
    Serial.print(", ");
    Serial.print(points[i].y);
    Serial.print(", ");
    Serial.print(points[i].z);
    Serial.print(" -> Pixel ID: ");
    Serial.print(pins[i].strip);
    Serial.print(", ");
    Serial.println(pins[i].pixel);
  }
}
void LEDSystem::setPixel(int strip, int pixel, int r, int g, int b){
  if(r>255) r=255;
  if(g>255) g=255;
  if(b>255) b=255;
  if(r < 0) r = 0;
  if(g < 0) g = 0;
  if(b < 0) b = 0;
  if (strip < 2 || (strip - 2) >= NUM_STRIPS) return;	
  // don't overwrite the pixel if it's already on
  uint8_t r0, g0, b0;
  strips[strip-2].getPixelColor(pixel, &r0, &g0, &b0);
  // uint32_t pixcol = strips[strip-2].getPixelColor(pixel); // alternate
  // if (pixcol == 0){ // alternate
  if (r0 == 0 && g0 == 0 && b0 == 0){	
	strips[strip-2].setPixelColor(pixel, strips[strip-2].Color(r,g,b));
  // strips[strip-2].setPixelColor(pixel, r, g, b); // alternate	   
  }	  
}
void LEDSystem::lightAll()
{
  for (int i=0; i<460; ++i) {
    setPixel(pins[i].strip,pins[i].pixel,50,50,50);
  }
  show();
}
void LEDSystem::assembly(int strip, int step){  //turn on each light in a seequence, pushing the button steps the lights one posistion  
  clear();
  int pixel_list[100];
  for (int i=0; i<100; i++){
    pixel_list[i]=999;
  }
  int count = 0;
  for (int i = 0; i < 460; i++) { //If written in reverse order ie. 70-count, it could be sorted in ~10 steps
    if (pins[i].strip == strip) {
      pixel_list[count] = pins[i].pixel;
      count++;
    }
  }
  //order list
  for (int i = 0; i < 100 - 1; i++) {
    for (int j = 0; j < 100 - 1; j++) {
      if (pixel_list[j] > pixel_list[j + 1]) {
        int temp = pixel_list[j];
        pixel_list[j] = pixel_list[j + 1];
        pixel_list[j + 1] = temp;
      }
    }
  }
  int px=pixel_list[step];
  bool top;
  if((px<100 && px>89)  || (px<74 && px>63) || (px<48 && px>37) ||(px<22 && px>11)) top=true;
  else top=false;
  if(strip%2!=0) top= !top;

  if(top) setPixel(strip-2,px,255,0,0); //-2 converts between PINID and Array index
  else setPixel(strip-2,px,0,0,255); //Green light if elastic before ball, red light if ball before elastic

  show();
}
void LEDSystem::redFlash(){
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < LED_COUNT; j++) {
      strips[i].setPixelColor(j, PINK); //GBR
    }
  strips[i].show();
  }
  delay(500);
  clear();
}
void LEDSystem::resetClock()
{
    currentTime=0;
    updateClock(0);
}
void LEDSystem::updateClock(int timestep)
{
  clock->clear();
  currentTime+=timestep;
  clock->writeDigitNum(0, (currentTime / 1000), false);
  clock->writeDigitNum(1, (currentTime / 100) % 10, false);
  clock->writeDigitNum(3, (currentTime / 10) % 10, false);
  clock->writeDigitNum(4, currentTime % 10, false);
  clock->drawColon(false);
  clock->writeDisplay();
}
