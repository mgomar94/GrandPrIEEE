/* Code found on https://arduining.com/2014/03/26/using-the-linear-sensor-array-tsl201r-with-arduino/
 * Written by user ardunaut on the arduining website
 */

//</pre>
/*Pro_TSL201RViewer_01.pde    Arduning.com  28 AUG 2015
 
Processing Viewer for the Linear Array Sensor TSL201R (AMS):
  64 x 1 sensors (200 DPI)
 
Data is received at 115200 bauds (ASCII messages ending with CR and LF)
One frame (64x1 image) has the followinf format:
      >5890    Indicates the integration period in microseconds.
      543      this value must be discarded.
      234      value of the pixel 1
      242      value of the pixel 2
       .         .
       .         .
      245      value of the pixel 64
 
Works with the Sketch:TSL201R_Scanning.ino in the Arduino.    
 
------------------------------------------------------------------------------*/
//---------------- Labels -----------------------------------------------------
String    heading = "TSL201R Viewer";
String    credit  = "Arduining.com";
//------------------------- Graph dimensions (constants) ----------------------
//System variables width and height are defined with size(width,height) in setup().
 
int       leftMargin=   80;   //from Graph to the left margin of the window
int       rightMargin=  80;   //from Graph to the right margin of the window
int       topMargin=    70;   //from Graph to the top margin of the window.
int       bottonMargin= 70;   //from Graph to the botton margin of the window.
//-------------------- Colors -------------------------------------------------
final  color  frameColor =  color(0,0,255);    // Main window background color.(Blue)
final  color  graphBack =   color(0,0,128);    // Graphing window background color. (Dark black)
final  color  borderColor=  color(128);        // Border of the graphing window.(grey)
final  color  barColor=     color(255,255,0);  // Vertical bars color. (yellow)
final  color  titleColor=   color(255,255,0);  // Title color. (yellow)
final  color  textColor=    color(200);        // Text values color. (grey)
//-------------------- Serial port library and variables ----------------------
import    processing.serial.*;
 
Serial    myPort;
boolean   COMOPENED = false;      // to flag that the COM port is opened.
final  char  LINE_FEED=10;        //mark the end of the frame.
final  int  numRecords=3;         //Number of data records per frame.
String[]  vals;                   //array to receibe data.
String    dataIn= null;           //string received
int       pixelCount= 64;
int       lightIntensity;
int       Integration= 0;         //Integration period in milliseconds
int       timeStamp;              // used to calculate frames per second (FPS).
int       lastTimeStamp;          // used to calculate frames per second (FPS).
float     framePeriod=0;          // used to calculate frames per second (FPS).
PFont     fontArial;
PFont     boldArial;
int       graphBase,graphWidth,graphHeight;
 
//-------- Variables to allocate the parameters in "settings.txt" --------------
String    comPortName;
float     Gain= 1;
 
//-------- Default content of "settings.txt" (one parameter per line) ----------
String[] lines = {
                 "Port:COM4",      //Serial Port
                 "Gain:1"};        //Gain (data amplification)
 
//------------------------------------------------------------------------------
//  setup
//------------------------------------------------------------------------------
void setup(){
//  String[] fontList = PFont.list();
//  println(fontList);
//  exit();
 
  size(800,600);        // setting system variables width and height.
 
  FillParams();         //Read "settings.txt" an fill the values.
  PortManager();        //Deal with the port selection and opening.
 
  graphBase= height-bottonMargin;
  graphHeight= height-bottonMargin-topMargin;
  graphWidth= width-leftMargin-rightMargin;
 
  fontArial = createFont("Arial",12);
  boldArial = createFont("Arial Bold Italic",12);
 
  timeStamp= millis();
  lastTimeStamp = timeStamp;
 
  background(frameColor);     // Main window background color.
  clearGrahp();
  showLabels();
  showParams();               // Display variables (usefull for debugging)
}
 
//==============================================================================
//  draw (main loop)
//==============================================================================
void draw(){
 
  while (myPort.available() > 0) {
    dataIn = myPort.readStringUntil(LINE_FEED);
    if (dataIn != null) {
      print(dataIn);                //show dataIn for debugging
      dataIn= trim(dataIn);         //removes spaces, CR and LF.
      PlotData();
      showParams();
    }
  }
}
 
//------------------------------------------------------------------------------
//Plot the incoming data in the Graph.
//------------------------------------------------------------------------------
void PlotData(){
 
  if (dataIn.charAt(0)=='>'){
    Integration= int(dataIn.substring(1,dataIn.length())); // removes '>'.
    timeStamp= millis();             //calculate period between frames.
    framePeriod= 1000/float(timeStamp - lastTimeStamp);
    lastTimeStamp= timeStamp;
    println("         (" + Integration +")");   // print Integration period.
    pixelCount=-2;            //reset pixel counter (-2 to skip first reading).
  }
 
  else if((pixelCount >= 0) && (pixelCount < 64)){
    lightIntensity= int(dataIn)/2;
    println("light= " + lightIntensity);
 
// ------- Clear the pixel region. -------------------
    noStroke();                 //no border
    fill(graphBack);            //Dark blue background
    rect(1+leftMargin +(graphWidth/64)*pixelCount, graphBase, (graphWidth/64)-2, 1-graphHeight);
 
// ------- Draw pixel intensity value as a vertical bar. -------
//    strokeWeight(1);
//    stroke(0);                //black border
    noStroke();
    fill(barColor);              //Color of the vertical bars
    rect(1+leftMargin +graphWidth*pixelCount/64, graphBase,(graphWidth/64)-2, -constrain(lightIntensity*Gain, 0, graphHeight-1));
  }
  pixelCount++;       //pointer to draw next pixel.
  println("--------" + pixelCount + "-"); //show pixel for debugging
}
 
//------------------------------------------------------------------------------
//Display diferent values for debugging.
//------------------------------------------------------------------------------
void showParams(){
  noStroke();                 //no border
  fill(frameColor);           //Blue background
  rect(1,height, width-250, 1-bottonMargin);  //Clean parameters region.
 
  textFont(fontArial,18);
  textAlign(LEFT,BOTTOM);    //Horizontal and vertical alignment.
  fill(textColor);
 
//---------- Show COM Port -----------------------------
  if (COMOPENED){
    text(lines[0],20, height-15);                          //Show the COM port.
    text("Gain: "+ Gain ,150, height-15);                  //Show Vertical Gain.
    text("Integration: " + Integration,270, height-15);    //Show Integration Period.
    text("FPS: "+ nf(framePeriod, 1, 1) ,450, height-15);  //Show frames Per Second (FPS)
  }
  else {
    fill(255,0,0);
    text(lines[0]+" not available",20, height-15);   //COMM port not available.
  }
}
 
//------------------------------------------------------------------------------
//Clear graph area.
//------------------------------------------------------------------------------
void clearGrahp(){
 
  stroke(borderColor);        //Graph border color.(gray)
  fill(graphBack);            //Dark Blue background
  rect(leftMargin, graphBase,graphWidth,-graphHeight);
}
 
//------------------------------------------------------------------------------
//Write the labels.
//------------------------------------------------------------------------------
void showLabels(){
  textAlign(CENTER,CENTER);
  textFont(fontArial,36);
  fill(titleColor);
  text(heading,width/2,topMargin/2);     //Draw graph title.
  textAlign(RIGHT,BOTTOM);
  textFont(boldArial,24);
  fill(textColor);
  text(credit,width-25,height-15);      //Draw credits.
}
 
//------------------------------------------------------------------------------
// Look for the values in "settings.txt" and set the variables accordingly.
//------------------------------------------------------------------------------
void  FillParams(){
  String splitLine[];
//-------- If "settings.txt" doesn't exist, create it.-------------
  if (loadStrings("settings.txt")== null){
    saveStrings("settings.txt", lines);         //each string is a line.
  }
  else{  //Reading "settings.txt" and update variables:
    lines = loadStrings("settings.txt");        //lines is a string array
    splitLine = split(lines[0], ':');           //split line[0].
    comPortName= splitLine[1];                  //the substring after '='
    splitLine = split(lines[1], ':');           //split line[1].
    Gain= Float.parseFloat(splitLine[1]);       //the substring after '='
  }  
 
//---------- Show the content of "settings.txt" in the the console -------------
  println("there are " + lines.length + " lines in \"settings.tx\" file");
  for (int i=0; i < lines.length; i++) {
      println(lines[i]);                        //Send to console.
  }
 
  saveStrings("settings.txt", lines);           //Save the settings.
}
 
//------------------------------------------------------------------------------
//Try to open the port specified in "settings.txt".
//------------------------------------------------------------------------------
void PortManager(){
 
//------------- List the available COM ports in this system --------------------
  String[] portlist = Serial.list();  //Create a list of available ports.
  println("There are " + portlist.length + " COM ports:");
  //println(portlist);
 
//----------- If COM port in "settings.txt" is available, open it  -------------
  for (int i=0; i < portlist.length; i++) {
      if(comPortName.equals(portlist[i]) == true){
          myPort = new Serial(this, portlist[i], 115200); //open serial port.
          COMOPENED = true;
          println(portlist[i]+" opened");
      }
  }
 
//-------- Procedure when COM port is not available --------------------
  if(!COMOPENED){
      println(comPortName + " is not available");    //Anounce the problem
//      SelComPort();         //Procedure to select another port...
  }
}
//<pre>