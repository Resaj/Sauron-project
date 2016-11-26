import processing.serial.*;

Serial myPort;  // Create object from Serial class
int val;      // Data received from the serial port
int t = 0;
int x = 0, y = 0;
int i = 0, j = 0;
int fps = 0;
int frames = 0;
int inByte = 0;

final int windowSizeX = 300;
final int windowSizeY = 350;

int numSamplesX = 3;
int numSamplesY = 3;
int[][] picture = new int[numSamplesX][numSamplesY];
int pictureX_min = 10, pictureX_max = windowSizeX-10;
int pictureY_min = 10, pictureY_max = windowSizeY-60;

String m[] = {"X", nf(numSamplesX), "Y", nf(numSamplesY), "S"};
String msg = join(m, "");

boolean initialized = false;

void setup() 
{
  size(300, 350);
  // I know that the first port in the serial list on my mac
  // is always my  FTDI adaptor, so I open Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 115200);
}

void draw()
{
  if(!initialized)
  {
    background(255,0,0);
    textSize(20);
    fill(0);
    text("Initializing...", 50, 50);
    
    if(myPort.available() > 0)
    {
      inByte = myPort.read();
      if(inByte == 'E')
        text("Dimension error!!", 50, 100);
      else if(inByte == 'K')
        initialized = true;
        t = 0;
    }
    else if(millis() - t > 200)
    {
      myPort.write(msg);
      t = millis();
    }
  }
  else
  {
    if(myPort.available() > 0)
    {
      inByte = myPort.read();

      if(inByte == 'S')
      {
        x = 0;
        y = 0;
        frames++;
      }
      else if(inByte == '0' || inByte == '1' || inByte == '2')
      {
        picture[x][y] = inByte - 48;
        x++;
        if(x == numSamplesX)
        {
          x = 0;
          y++;
        }
      }
    
      if(millis() - t > 1000)
      {
        fps = frames;
        frames = 0;
        t = millis();
      }
    }
    
    background(0,255,0);
    fill(0);
    text("fps:", 50, windowSizeY-25);
    text(fps, 100, windowSizeY-25);

    for(j=0; j<numSamplesY; j++)
    {
      for(i=0; i<numSamplesX; i++)
      {
        switch(picture[i][j])
        {
          case 0:
            fill(255);
            break;
          case 1:
            fill(127);
            break;
          case 2:
            fill(0);
        }
        rect(/*x_min*/pictureX_min + i*floor((pictureX_max-pictureX_min)/numSamplesX),
             /*y_min*/pictureY_min + j*floor((pictureY_max-pictureY_min)/numSamplesY),
             /*x_dist*/ceil((pictureX_max-pictureX_min)/numSamplesX),
             /*y_dist*/ceil((pictureY_max-pictureY_min)/numSamplesY));
      }
    }
  }
}