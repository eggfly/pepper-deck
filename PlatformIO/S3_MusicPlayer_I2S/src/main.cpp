#include <Arduino.h>
#include <Audio.h> /* https://github.com/schreibfaul1/ESP32-audioI2S */
#include <SD_MMC.h>

/* M5Stack Node I2S pins */
#define I2S_BCK 15
#define I2S_WS 16
#define I2S_DOUT 7
// #define I2S_DIN 4
#define I2S_MCLK 5

#define ENABLE_WIFI 0

#define DAC_SD_MODE 39
#define UDA_MUTE 46

Audio audio;

void sdmmc_setup();

void setup()
{
       pinMode(DAC_SD_MODE, OUTPUT);
       // pinMode(UDA_MUTE, OUTPUT);
       digitalWrite(DAC_SD_MODE, LOW); // LOW to set speaker to mute
       // digitalWrite(UDA_MUTE, HIGH);   // HIGH to set earphone to mute
       // delay(2000);
       Serial.begin(115200);
       Serial.println("PCM5102A I2S DAC test");
       sdmmc_setup();
       /* set the i2s pins */
       audio.setPinout(I2S_BCK, I2S_WS, I2S_DOUT, I2S_MCLK);
       if (ENABLE_WIFI)
       {
              WiFi.begin("MIWIFI8", "12345678");
              while (!WiFi.isConnected())
              {
                     delay(10);
              }
              log_i("Connected. Starting MP3...");
       }
       else
       {
              log_i("Starting MP3 without WIFI...");
       }
       // /* Song F */audio.connecttohost("http://42.193.120.65:8002/%E8%BE%BE%E8%BE%BE-Song%20F.mp3");
       /* 阳光照进回忆里 */
       // audio.connecttohost("http://42.193.120.65:8002/%E9%80%83%E8%B7%91%E8%AE%A1%E5%88%92-%E9%98%B3%E5%85%89%E7%85%A7%E8%BF%9B%E5%9B%9E%E5%BF%86%E9%87%8C.mp3");
       // audio.connecttohost("http://42.193.120.65:8002/2019%E4%B9%90%E9%98%9F%E7%9A%84%E5%A4%8F%E5%A4%A9/%E7%AC%AC%E5%8D%81%E4%BA%8C%E6%9C%9F/%E6%9C%B4%E6%A0%91%20-%20No%20Fear%20In%20My%20Heart%20%28Live%29.mp3");
       // audio.connecttohost("http://42.193.120.65:8002/2019%E4%B9%90%E9%98%9F%E7%9A%84%E5%A4%8F%E5%A4%A9/%E7%AC%AC%E4%B8%89%E6%9C%9F/%E5%88%BA%E7%8C%AC%E4%B9%90%E9%98%9F%20-%20%E7%81%AB%E8%BD%A6%E9%A9%B6%E5%90%91%E4%BA%91%E5%A4%96%EF%BC%8C%E6%A2%A6%E5%AE%89%E9%AD%82%E4%BA%8E%E4%B9%9D%E9%9C%84%20%28Live%29.mp3");
       // http://42.193.120.65:8002/520AM.mp3
       // http://42.193.120.65:8002/1%20-%20Hotel%20California.mp3
       //
       // /* 渡口 */ http://42.193.120.65:8002/%E8%94%A1%E7%90%B4%20-%20%E6%B8%A1%E5%8F%A3.mp3
       audio.connecttoFS(SD_MMC, "/server/逃跑计划-阳光照进回忆里.mp3");
       audio.setVolume(6);
       auto vol = audio.getVolume();
       Serial.println(vol);
}

void loop()
{
       audio.loop();
       if (Serial.available())
       {
              // put file path in serial monitor
              audio.stopSong();
              String r = Serial.readString();
              log_i("serial_input=%s", r.c_str());
              r.trim();
              if (r.length() > 5)
              {
                     // /server/马赛克-霓虹甜心.flac
                     // /万能青年旅店/04. 大石碎胸口.flac
                     // /yemao.mp3
                     // /达达-Song F.mp3
                     // /蔡琴 - 渡口.mp3
                     // /server/南方 (Live) - 达达乐队.mp3
                     // /刺猬乐队-生之响往（2018）[Flac]/01 二十一世纪，当我们还年轻时.flac
                     audio.connecttoFS(SD_MMC, r.c_str());
              }
              log_i("free heap=%i", ESP.getFreeHeap());
       }
}

// optional
void audio_info(const char *info)
{
       Serial.print("info        ");
       Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
       Serial.print("id3data     ");
       Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
       Serial.print("eof_mp3     ");
       Serial.println(info);
}
void audio_showstation(const char *info)
{
       Serial.print("station     ");
       Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
       Serial.print("streamtitle ");
       Serial.println(info);
}
void audio_bitrate(const char *info)
{
       Serial.print("bitrate     ");
       Serial.println(info);
}
void audio_commercial(const char *info)
{ // duration in sec
       Serial.print("commercial  ");
       Serial.println(info);
}
void audio_icyurl(const char *info)
{ // homepage
       Serial.print("icyurl      ");
       Serial.println(info);
}
void audio_lasthost(const char *info)
{ // stream URL played
       Serial.print("lasthost    ");
       Serial.println(info);
}
void audio_eof_speech(const char *info)
{
       Serial.print("eof_speech  ");
       Serial.println(info);
}
