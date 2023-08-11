#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <driver/i2c.h>

// ESP32でLovyanGFXを独自設定で利用する場合の設定例

/// 独自の設定を行うクラスを、LGFX_Deviceから派生して作成します。
class LGFX : public lgfx::LGFX_Device
{
  //lgfx::Bus_I2C       _bus_instance; 

public:

  lgfx::Bus_RGB      _bus_instance;
  lgfx::Panel_ST7701 _panel_instance;
  lgfx::Light_PWM _light_instance;
  //lgfx::Touch_GT911  _touch_instance;
  lgfx::Touch_FT5x06  _touch_instance;
  
  // コンストラクタを作成し、ここで各種設定を行います。
  // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
  LGFX(void)
  {
    {
      auto cfg = _panel_instance.config();

      cfg.memory_width  = 480;
      cfg.memory_height = 480;
      cfg.panel_width  = 480;
      cfg.panel_height = 480;

      cfg.offset_x = 0;
      cfg.offset_y = 0;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();

      cfg.use_psram = 1;
      cfg.pin_cs = 1; //1
      cfg.pin_sclk = 41; //12
      cfg.pin_mosi = 48; //11

      _panel_instance.config_detail(cfg);
    }

    {
      auto cfg = _bus_instance.config();
     
      cfg.panel = &_panel_instance;
      cfg.pin_d0  = GPIO_NUM_15;  // B0
      cfg.pin_d1  = GPIO_NUM_14;  // B1
      cfg.pin_d2  = GPIO_NUM_13; // B2
      cfg.pin_d3  = GPIO_NUM_12; // B3
      cfg.pin_d4  = GPIO_NUM_11;  // B4
      cfg.pin_d5  = GPIO_NUM_10;  // G0
      cfg.pin_d6  = GPIO_NUM_9;  // G1
      cfg.pin_d7  = GPIO_NUM_8; // G2
      cfg.pin_d8  = GPIO_NUM_7; // G3
      cfg.pin_d9  = GPIO_NUM_6; // G4
      cfg.pin_d10 = GPIO_NUM_5;  // G5
      cfg.pin_d11 = GPIO_NUM_4; // R0
      cfg.pin_d12 = GPIO_NUM_3; // R1
      cfg.pin_d13 = GPIO_NUM_2; // R2
      cfg.pin_d14 = GPIO_NUM_1; // R3
      cfg.pin_d15 = GPIO_NUM_0; // R4

      cfg.pin_henable = GPIO_NUM_18; //45
      cfg.pin_vsync   = GPIO_NUM_17; //4
      cfg.pin_hsync   = GPIO_NUM_16; //5
      cfg.pin_pclk    = GPIO_NUM_21; //21
      cfg.freq_write  = 14000000;

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 10;
      cfg.hsync_pulse_width = 8;
      cfg.hsync_back_porch  = 50;
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 10;
      cfg.vsync_pulse_width = 8;
      cfg.vsync_back_porch  = 20;
      cfg.pclk_idle_high    = 0;
      cfg.de_idle_high      = 1;
      
      _bus_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);

    {
      
      auto cfg = _touch_instance.config();
      cfg.x_min      = 0;
      cfg.x_max      = 480;
      cfg.y_min      = 0;
      cfg.y_max      = 480;
      //cfg.pin_int = -1; //追加
      cfg.bus_shared = true; //false
      cfg.offset_rotation = 0;

      cfg.i2c_port   = I2C_NUM_1; //NUM_1
      cfg.i2c_addr = 0x48; //20,48
      cfg.pin_int    = GPIO_NUM_NC;
      cfg.pin_sda    = GPIO_NUM_39;
      cfg.pin_scl    = GPIO_NUM_40;
      cfg.pin_rst    = GPIO_NUM_NC; //38

      /*
      cfg.pin_sclk  = 41;
      cfg.pin_mosi  = 48;
      //cfg.pin_miso  = 
      */


      cfg.freq       = 600000; //400000
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
      
    }

     {
       auto cfg = _light_instance.config();
       cfg.pin_bl = GPIO_NUM_45;
       _light_instance.config(cfg);
     }
     _panel_instance.light(&_light_instance);

    setPanel(&_panel_instance);
  }
};

// 準備したクラスのインスタンスを作成します。
LGFX display;

void setup(void)
{
  // SPIバスとパネルの初期化を実行すると使用可能になります。
  Serial.begin(9600);
  Serial.println("set1");
  display.init();
  display.setColorDepth(16);
  //display.setBrightness(128);
  

  display.setTextSize((std::max(display.width(), display.height()) + 255) >> 8);
  display.fillScreen(TFT_BLACK);
  
  // タッチが使用可能な場合のキャリブレーションを行います。（省略可）
  /*
  if (display.touch())
  {
    if (display.width() < display.height()) display.setRotation(display.getRotation() ^ 1);

    // 画面に案内文章を描画します。
    display.setTextColor(TFT_WHITE);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("touch the arrow marker.", display.width()>>1, display.height() >> 1);
    display.setTextDatum(textdatum_t::top_left);

    // タッチを使用する場合、キャリブレーションを行います。画面の四隅に表示される矢印の先端を順にタッチしてください。
    std::uint16_t fg = TFT_WHITE;
    std::uint16_t bg = TFT_BLACK;
    if (display.isEPD()) std::swap(fg, bg);
    uint16_t calibrateData[8];
    display.calibrateTouch(calibrateData, fg, bg, std::max(display.width(), display.height()) >> 3);
    Serial.printf("[0] x_min = %d\n", calibrateData[0]);
    Serial.printf("[1] y_min = %d\n", calibrateData[1]);
    Serial.printf("[2] x_min = %d\n", calibrateData[2]);
    Serial.printf("[3] y_max = %d\n", calibrateData[3]);
    Serial.printf("[4] x_max = %d\n", calibrateData[4]);
    Serial.printf("[5] y_min = %d\n", calibrateData[5]);
    Serial.printf("[6] x_max = %d\n", calibrateData[6]);
    Serial.printf("[7] y_max = %d\n", calibrateData[7]);
    Serial.printf("x_min = %d\n", (calibrateData[0] + calibrateData[2]) / 2);
    Serial.printf("x_max = %d\n", (calibrateData[4] + calibrateData[6]) / 2);
    Serial.printf("y_min = %d\n", (calibrateData[1] + calibrateData[5]) / 2);
    Serial.printf("y_max = %d\n", (calibrateData[3] + calibrateData[3]) / 2);
  }
  */
  display.fillScreen(TFT_BLACK);
  
}

uint32_t count = ~0;
void loop(void)
{
  //Serial.println("set2");
  display.startWrite();
  display.setRotation(++count & 7);
  display.setColorDepth((count & 8) ? 16 : 24);

  display.setTextColor(TFT_WHITE);
  display.drawNumber(display.getRotation(), 16, 0);

  display.setTextColor(0xFF0000U);
  display.drawString("R", 30, 16);
  display.setTextColor(0x00FF00U);
  display.drawString("G", 40, 16);
  display.setTextColor(0x0000FFU);
  display.drawString("B", 50, 16);

  display.drawRect(30,30,display.width()-60,display.height()-60,count*7);
  display.drawFastHLine(0, 0, 10);

  display.endWrite();

  int32_t x, y;
  if (display.getTouch(&x, &y)) {
    
    lgfx::touch_point_t tp;
    display.getTouchRaw(&tp);
    Serial.printf("raw_x = %d, raw_y = %d\n", tp.x, tp.y);
    //display.fillRect(x-2, y-2, 5, 5, count*7);
    display.fillRect(x-2, y-2, 7, 7, TFT_WHITE);
  }
  //Serial.println("test");
}
