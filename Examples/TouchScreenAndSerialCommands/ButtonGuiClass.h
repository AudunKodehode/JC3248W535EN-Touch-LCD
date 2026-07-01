#include <JC3248W535EN-Touch-LCD.h>

//quick and dirty Button Class
class ButtonGuiClass {
  private:
    int PrivateVariable;

  public:
	  //Arduino_GFX *gfx;
    JC3248W535EN *screen;            //The main display variable

    int16_t X;          //start of button location x
    int16_t Y;          //start of button location y
    int16_t W;          //width of button
    int16_t H;          //height of button
    uint8_t Red_FG;     //foreground font color
    uint8_t Green_FG;   //foreground font color
    uint8_t Blue_FG;    //foreground font color
    uint8_t Red_BG;     //background font color
    uint8_t Green_BG;   //background font color 
    uint8_t Blue_BG;    //background font color
    uint8_t fontSize;
    String Text;

    ButtonGuiClass(void){
      Text = "ClassNotInit";
    }

    ButtonGuiClass(JC3248W535EN *screenClass){
      screen = screenClass;
      Red_BG   = 255;
      Green_BG = 255;
      Blue_BG  = 255;
      Text = "TextBox";
    }

    ButtonGuiClass(JC3248W535EN *screenClass, int16_t XX, int16_t YY, int16_t WW, int16_t HH, uint8_t RedBG, uint8_t GreenBG, uint8_t BlueBG, String Textt, uint8_t fontSizee)
    {
      screen = screenClass;
      X = XX;
      Y = YY;
      W = WW;
      H = HH;

      //make sure we are within the bounds of the screen
      if (X > screen->width){ X = screen->width;}
      if (Y > screen->height){ Y = screen->height;}
      if (W > screen->width){ W = screen->width;}
      if (H > screen->height){ H = screen->height;}
      
      //default font/foreground color to black
      Red_FG   = 0;
      Green_FG = 0;
      Blue_FG  = 0;

      Red_BG   = RedBG;
      Green_BG = GreenBG;
      Blue_BG  = BlueBG;
      Text  = Textt;
      fontSize = fontSizee;
    }

    //this one adds font/foreground color setup along with the background color
    ButtonGuiClass(JC3248W535EN *screenClass, int16_t XX, int16_t YY, int16_t WW, int16_t HH, uint8_t RedBG, uint8_t GreenBG, uint8_t BlueBG, uint8_t RedFG, uint8_t GreenFG, uint8_t BlueFG, String Textt, uint8_t fontSizee)
    {
      screen = screenClass;
      X = XX;
      Y = YY;
      W = WW;
      H = HH;

      //make sure we are within the bounds of the screen
      if (X > screen->width){ X = screen->width;}
      if (Y > screen->height){ Y = screen->height;}
      if (W > screen->width){ W = screen->width;}
      if (H > screen->height){ H = screen->height;}
      
      //font/foreground color
      Red_FG   = RedBG;
      Green_FG = GreenBG;
      Blue_FG  = BlueBG;

      //background color
      Red_BG   = RedBG;
      Green_BG = GreenBG;
      Blue_BG  = BlueBG;

      Text  = Textt;
      fontSize = fontSizee;
    }

    void displayButt(void){
      // Draw Button. Right now its a solid backgrund color. Could make it just an outlined color button or something fancier
      screen->setColor(Red_BG, Green_BG, Blue_BG);
      screen->drawFillRect(X, Y, W, H);
      //screen->setColor(255,255,255); //make white text the default for now
      //screen->setColor(0,0,0); //make black text the default for now
      screen->setColor(0,0,0); //make black text the default for now
      screen->prt(Text, X + 2, Y + 4, fontSize);
    }

    void AnimateButtPush(void){
      screen->setColor(255, 255, 255);  //simulate white flash
      screen->drawFillRect(X, Y, W, H);
      screen->flush();   //anytime you want the screen to update you have to do a flush
      //delay(5);

      // Draw Button 
      screen->setColor(Red_BG, Green_BG, Blue_BG);
      screen->drawFillRect(X, Y, W, H);
      //screen->setColor(255,255,255);              //make white text the default for now
      screen->setColor(Red_FG, Green_FG, Blue_FG);  //make black text the default for now
      screen->prt(Text, X + 2, Y + 4, fontSize);    //default the text to offset a little outside the bounds quick and dirty. Should make this change based on font size..
    }

    void updateText(String newText){
      Text = newText;
      displayButt();
    }

    void updateText(String newText, bool updateNow){
      Text = newText;
      if (updateNow){
        displayButt();
      }
    }

    //could use this to potentially add some motion to a box
    void updateButtonLocation(int16_t XX, int16_t YY, int16_t WW, int16_t HH, bool updateNow){
      X = XX;
      Y = YY;
      W = WW;
      H = HH;

      //make sure we are within the bounds of the screen
      if (X > screen->width){ X = screen->width;}
      if (Y > screen->height){ Y = screen->height;}
      if (W > screen->width){ W = screen->width;}
      if (H > screen->height){ H = screen->height;}
      if (updateNow){
        displayButt();
      }
    }

    void updateBackgroundColor(uint8_t RedBG, uint8_t GreenBG, uint8_t BlueBG, bool updateNow){
      Red_BG   = RedBG;
      Green_BG = GreenBG;
      Blue_BG  = BlueBG;
      if (updateNow){
        displayButt();
      }
    }

    void updateFontColor(uint8_t RedBG, uint8_t GreenBG, uint8_t BlueBG, bool updateNow){
      Red_FG   = RedBG;
      Green_FG = GreenBG;
      Blue_FG  = BlueBG;
      if (updateNow){
        displayButt();
      }
    }

    //check if xy click is in our button
    bool checkIfClicked(uint16_t touchX, uint16_t touchY){
      if (touchX > screen->width){touchX = screen->width; }
      if (touchY > screen->height){touchY = screen->height; }

      if ( touchX >= X && touchX <= X+W && touchY >= Y && touchY <= Y+H){
        Serial.println("Button " + Text + " Clicked");
        AnimateButtPush();
        return true;
      }
      else{
        return false;
      }
    }
};  //end ButtonGuiClass