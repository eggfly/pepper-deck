# 1 "/var/folders/8x/lx7fqmm923542qcqqsx73vvm0000gn/T/tmprro8vcbc"
#include <Arduino.h>
# 1 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
#include <Arduino.h>

#include "esp_clk.h"

#include "FS.h"
#include "SD_MMC.h"
#include "config.h"
#include "SPI.h"
#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <Audio.h>
#include "display.h"
#include <mouse_icon.h>

#include "page.h"
#include "nc1020.h"
#include "lru.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/portmacro.h"

#include "app.h"
#include "battery_gauge.h"
#include "usb_host_hid.h"

portMUX_TYPE my_mutex = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t anotherCoreTaskHandle;

Audio audio;

std::vector<String> m_songFiles{};

const bool APP_DEBUG = false;

#define I2S_DOUT 7
#define I2S_BCLK 16
#define I2S_LRCK 15


#define LEDC_TIMER_12_BIT 12
# 54 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
uint32_t chipId = 0;

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

int clk = 13;
int cmd = 14;
int d0 = 12;
int d1 = 11;
int d2 = 10;
int d3 = 9;
bool SD_Init();
void NC1020_Init();
void set_backlight_level();
int strncmpci(const char *str1, const char *str2, size_t num);
bool startsWithIgnoreCase(const char *pre, const char *str);
bool endsWithIgnoreCase(const char *base, const char *str);
void populateMusicFileList(String path, size_t depth);
void autoPlayNextSong();
void clearSongInfo();
void startNextSong(bool isNextOrPrev);
void volumeUp();
void volumeDown();
void setup();
void anotherCoreTask(void *parameter);
void gfx_demo();
void pixelScale(uint8_t *src, uint8_t *dest);
void pixelScale2(uint8_t *src, uint8_t *dest);
void sprintfBinary(uint8_t num, char *buf);
void enlargeBuffer(uint8_t *src, uint8_t *dest);
void magnify_pixels(uint8_t *src, uint8_t *dest);
void my_lcd_zoom_in(uint8_t *src, uint8_t *dest);
inline void drawMouse();
inline void nc1020_loop();
void loop();
void handle_backlight_key();
void show_restart();
void select_boot_item(int8_t offset);
bool keyboard_callback(const char *key_str);
#line 102 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
bool SD_Init()
{
  if (!SD_MMC.setPins(clk, cmd, d0, d1, d2, d3))
  {
    Serial.println("Pin change failed!");
    return false;
  }
  if (!SD_MMC.begin())
  {
    Serial.println("Card Mount Failed");
    return false;
  }

  uint8_t cardType = MY_SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return false;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = MY_SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listDir(MY_SD, "/", 0);

  Serial.printf("Total space: %lluMB\n", MY_SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", MY_SD.usedBytes() / (1024 * 1024));

  delay(200);
  return true;
}

QueueHandle_t g_event_queue_handle = NULL;


uint8_t lcd_buff[1600];


uint8_t *lcd_buff_expanded;

uint8_t my_buff[0x8000];




std::map<std::string, uint8_t> nc1020_keymap = {
    {"A", 0x28},
    {"B", 0x34},
    {"C", 0x32},
    {"D", 0x2A},
    {"E", 0x22},
    {"F", 0x2B},
    {"G", 0x2C},
    {"H", 0x2D},
    {"I", 0x27},
    {"J", 0x2E},
    {"K", 0x2F},
    {"L", 0x19},
    {"M", 0x36},
    {"N", 0x35},
    {"O", 0x18},
    {"P", 0x1C},
    {"Q", 0x20},
    {"R", 0x23},
    {"S", 0x29},
    {"T", 0x24},
    {"U", 0x26},
    {"V", 0x33},
    {"W", 0x21},
    {"X", 0x31},
    {"Y", 0x25},
    {"Z", 0x30},
    {"Enter", 0x1d},
    {"Backspace", 0x3b},
    {"F1", 0x1a},
    {"F2", 0x1b},
    {"F3", 0x3f},
    {"F4", 0x1f},
    {"P1", 0x0b},
    {"P2", 0x0c},
    {"P3", 0x0d},
    {"P4", 0x0a},
    {"P5", 0x09},
    {"P6", 0x08},
    {"P7", 0x0e},
};
# 260 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
void NC1020_Init()
{

  auto freq = getCpuFrequencyMhz();
  Serial.printf("cpu freq=%dMHz\n", freq);

  lcd_buff_expanded = (uint8_t *)ps_malloc(320 * 160 / 8);

  g_event_queue_handle = xQueueCreate(20, sizeof(uint8_t));
  wqx::Initialize(nullptr);
  wqx::LoadNC1020();
}

const float PROGMEM backlight_levels[] = {
    0.0,
    0.02,
    0.05,
    0.15,
    0.3,
    0.5,
    0.75,
    1.0,
};

uint8_t current_backlight_level = sizeof(backlight_levels) / sizeof(backlight_levels[0]) / 2;

void set_backlight_level()
{
}

Page *currPage;

char menu_items[][128] = {
    "* Ubuntu, with Linux 5.19 Generic",
    "  WenQuXing NC1020 Emulator",
    "  EGGFLY Music Player",
    "  Console & Keyboard Test",
    "  MicroPython Shell (1.20.0)",
    "  LVGL v8.3.8 (ESP32-S3)",
    "  Arduino-ESP32 Factory Test",
};

const uint8_t menu_items_count = sizeof(menu_items) / sizeof(menu_items[0]);

Page *menu_pages[menu_items_count];

size_t selected_menu_index = 0;

void grub_loader_menu();

class BootMenuPage : public Page
{
public:
  void onDraw()
  {
    grub_loader_menu();
  }
  bool handleKey(const char *key_str)
  {
    if (strcmp(key_str, "Up") == 0)
    {
      select_boot_item(-1);
      return true;
    }
    else if (strcmp(key_str, "Down") == 0)
    {
      select_boot_item(1);
      return true;
    }
    else if (strcmp(key_str, "Enter") == 0)
    {
      currPage = menu_pages[selected_menu_index];
      currPage->initPage();
      return true;
    }
    return false;
  }
};

int strncmpci(const char *str1, const char *str2, size_t num)
{
  int ret_code = 0;
  size_t chars_compared = 0;

  if (!str1 || !str2)
  {
    ret_code = INT_MIN;
    return ret_code;
  }

  while ((chars_compared < num) && (*str1 || *str2))
  {
    ret_code = tolower((int)(*str1)) - tolower((int)(*str2));
    if (ret_code != 0)
    {
      break;
    }
    chars_compared++;
    str1++;
    str2++;
  }
  return ret_code;
}

bool startsWithIgnoreCase(const char *pre, const char *str)
{
  return strncmpci(pre, str, strlen(pre)) == 0;
}

bool endsWithIgnoreCase(const char *base, const char *str)
{
  int blen = strlen(base);
  int slen = strlen(str);
  return (blen >= slen) && (0 == strncmpci(base + blen - slen, str, strlen(str)));
}

void populateMusicFileList(String path, size_t depth)
{
  Serial.printf("search: %s, depth=%d\n", path.c_str(), depth);
  File musicDir = MY_SD.open(path);
  bool nextFileFound;
  do
  {
    nextFileFound = false;
    File entry = musicDir.openNextFile();
    if (entry)
    {
      nextFileFound = true;
      if (entry.isDirectory())
      {
        if (depth)
        {
          populateMusicFileList(entry.path(), depth - 1);
        }
      }
      else
      {
        const bool entryIsFile = entry.size() > 4096;
        if (entryIsFile)
        {
          if (APP_DEBUG)
          {
            Serial.print(entry.path());
            Serial.print(" size=");
            Serial.println(entry.size());
          }
          if (endsWithIgnoreCase(entry.name(), ".mp3") || endsWithIgnoreCase(entry.name(), ".flac") || endsWithIgnoreCase(entry.name(), ".aac") || endsWithIgnoreCase(entry.name(), ".wav"))
          {
            m_songFiles.push_back(entry.path());
          }
        }
      }
      entry.close();
    }
  } while (nextFileFound);
}

int m_activeSongIdx{-1};

char music_title[256] = "";
char music_album[256] = "";
char music_artist[256] = "";

void autoPlayNextSong()
{
  if (m_songFiles.size() == 0)
  {
    delay(100);
    return;
  }
  if (!audio.isRunning())
  {
    Serial.println("autoPlay: playNextSong()");
    startNextSong(true);
  }
}

std::unordered_set<int> m_played_songs{};

void clearSongInfo()
{
  music_title[0] = '\0';
  music_artist[0] = '\0';
  music_album[0] = '\0';
}

bool shuffle_mode = true;

void startNextSong(bool isNextOrPrev)
{
  if (m_songFiles.size() == 0)
  {
    return;
  }
  m_played_songs.insert(m_activeSongIdx);
  if (m_played_songs.size() * 2 > m_songFiles.size())
  {
    Serial.println("re-shuffle.");
    m_played_songs.clear();
  }
  if (isNextOrPrev)
  {
    m_activeSongIdx++;
  }
  else
  {
    m_activeSongIdx--;
  }
  if (shuffle_mode)
  {
    do
    {
      m_activeSongIdx = random(m_songFiles.size());
    } while (m_played_songs.find(m_activeSongIdx) != std::end(m_played_songs));
  }



  m_activeSongIdx %= m_songFiles.size();
  Serial.print("songIndex=");
  Serial.print(m_activeSongIdx);
  Serial.print(", total=");
  Serial.println(m_songFiles.size());

  if (audio.isRunning())
  {
    audio.stopSong();
  }
  clearSongInfo();





  Serial.println(m_songFiles[m_activeSongIdx].c_str());
}

uint8_t volume = 5;

void volumeUp()
{
  if (volume < 21)
  {
    volume++;
    audio.setVolume(volume);
  }
}

void volumeDown()
{
  if (volume > 0)
  {
    volume--;
    audio.setVolume(volume);
  }
  else
  {

  }
}

class MusicPlayerPage : public Page
{
public:
  void initPage()
  {
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    audio.setVolume(volume);
    populateMusicFileList("/", 1);
    Serial.print("MusicFileList length: ");
    Serial.println(m_songFiles.size());
  }
  bool handleKey(const char *key_str)
  {
    if (strcmp(key_str, "Left") == 0)
    {
      startNextSong(false);
    }
    else if (strcmp(key_str, "Right") == 0)
    {
      startNextSong(true);
    }
    else if (strcmp(key_str, "Up") == 0)
    {
      volumeUp();
    }
    else if (strcmp(key_str, "Down") == 0)
    {
      volumeDown();
    }
    return true;
  }
  void onDraw()
  {
    audio.loop();
    autoPlayNextSong();

    canvas.fillScreen(0);
    canvas.setCursor(0, 0);
    canvas.setTextColor(0xFF);
    canvas.setTextSize(3);
    if (audio.isRunning())
    {
      auto audioDuration = audio.getAudioFileDuration();
      auto currPos = audio.getAudioCurrentTime();
      auto totalPlayingTime = audio.getTotalPlayingTime();
      Serial.printf("AudioFileDuration=%d, AudioCurrentTime=%d, TotalPlayingTime=%d\n", audioDuration, currPos, totalPlayingTime);
      canvas.println(audioDuration);
      canvas.println(currPos);
      canvas.println(totalPlayingTime);
    }
    else
    {
      canvas.println("not playing.");
    }
  }
};

std::string screen_str;
class ConsolePage : public Page
{
public:
  void onDraw()
  {
    gfx_demo();
  }
  bool handleKey(const char *key_str)
  {
    screen_str += key_str;
    return true;
  }
};

class WenQuXingPage : public Page
{
public:
  void onDraw()
  {
    auto start_time = millis();
    nc1020_loop();
    drawMouse();
    canvas.flush();
    if (LOG_LEVEL <= LOG_LEVEL_VERBOSE)
    {
      Serial.printf("nc1020_loop,cost=%dms\n", millis() - start_time);
    }
  }
  bool handleKey(const char *key_str)
  {
    std::string str(key_str);


    char c = '\n';
    xQueueSend(g_event_queue_handle, &c, portMAX_DELAY);
    Serial.printf("xQueueSend, key=0x%02x\n", c);
    return true;
  }
};

void setup()
{
  Serial.begin(115200);
  usb_hid_setup();





  Serial.printf("Current CPU Freq: %u MHz\n", ESP.getCpuFreqMHz());

  initScreen();
  keyboard_setup();
  battery_setup();

  menu_pages[0] = new ConsolePage();
  menu_pages[1] = new WenQuXingPage();
  menu_pages[2] = new MusicPlayerPage();
  menu_pages[3] = new ConsolePage();
  menu_pages[4] = new ConsolePage();
  menu_pages[5] = new ConsolePage();
  menu_pages[6] = new ConsolePage();

  currPage = menu_pages[1];

  if (!SD_Init())
  {
    for (;;)
    {
    }
  }
  NC1020_Init();
  auto coreId = xPortGetCoreID();
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());
  auto anotherCoreId = coreId ? 0 : 1;
# 664 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
}

SPIClass *fspi = NULL;
static const int spiClk = 40000000;

void anotherCoreTask(void *parameter)
{

  Serial.print("anotherCoreTask running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
    portENTER_CRITICAL(&my_mutex);
# 689 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
    delayMicroseconds(10);

  }
}

void grub_loader_menu()
{
  portENTER_CRITICAL(&my_mutex);
  canvas.fillScreen(0);
  canvas.fillRect(0 + 10, 30 + 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 50, 0xFF);
  canvas.fillRect(0 + 10 + 3, 30 + 10 + 3, SCREEN_WIDTH - 20 - 6, SCREEN_HEIGHT - 50 - 6, 0x00);
  canvas.setCursor(0, 0);
  canvas.setTextColor(0xFF);
  canvas.setTextSize(3);
  canvas.println("----- GNU GRUB v2.02 -----");
  canvas.setTextSize(2);

  canvas.println();
  canvas.println();
  for (size_t i = 0; i < menu_items_count; i++)
  {
    if (selected_menu_index == i)
    {
      canvas.setTextColor(0x00, 0xFF);
    }
    else
    {
      canvas.setTextColor(0xFF);
    }
    canvas.println(menu_items[i]);
  }
  portEXIT_CRITICAL(&my_mutex);
  delay(100);
}

typedef struct
{
  uint8_t d[1280000];
} my_wrapped_array;

my_wrapped_array *my_arr = NULL;

void gfx_demo()
{
  portENTER_CRITICAL(&my_mutex);
  canvas.fillScreen(0);
# 744 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
  canvas.fillRect(400, 1, 10, 10, 1);

  canvas.setCursor(0, 0);
  canvas.setTextColor(0xFF);
  canvas.setTextSize(3);
  canvas.print("--- HELLO EGGFLY WORLD ---");
  canvas.println();
  canvas.print("Console >>>");
  canvas.println();
  canvas.print(screen_str.c_str());

  portEXIT_CRITICAL(&my_mutex);


}

void pixelScale(uint8_t *src, uint8_t *dest)
{

  const int scale = 2;


  const int srcPitch = 160 / 8;


  const int destPitch = 320 / 8;


  for (int y = 0; y < 80; y++)
  {
    for (int x = 0; x < srcPitch; x++)
    {
      uint8_t pixel = src[y * srcPitch + x];


      uint8_t expandedPixel = (pixel << 4) | pixel;


      dest[y * 2 * destPitch + 2 * x] = expandedPixel;
      dest[y * 2 * destPitch + 2 * x + 1] = expandedPixel;
      dest[(y * 2 + 1) * destPitch + 2 * x] = expandedPixel;
      dest[(y * 2 + 1) * destPitch + 2 * x + 1] = expandedPixel;
    }
  }
}
void pixelScale2(uint8_t *src, uint8_t *dest)
{

  const int scale = 2;


  const int srcPitch = 160 / 8;


  const int destPitch = 320 / 8;


  for (int y = 0; y < 80; y++)
  {
    for (int x = 0; x < srcPitch; x++)
    {
      uint8_t pixel = src[y * srcPitch + x];


      uint8_t expandedPixel = (pixel << 4) | pixel;


      dest[y * 2 * destPitch + 2 * x] = expandedPixel;
      dest[y * 2 * destPitch + 2 * x + 1] = expandedPixel;
      dest[(y * 2 + 1) * destPitch + 2 * x] = expandedPixel;
      dest[(y * 2 + 1) * destPitch + 2 * x + 1] = expandedPixel;
    }
  }
}

#define SRC_WIDTH 160
#define SRC_HEIGHT 80
#define DEST_WIDTH 320
#define DEST_HEIGHT 160

void sprintfBinary(uint8_t num, char *buf)
{
  sprintf(buf, "0b");
  buf += 2;
  for (int i = 7; i >= 0; i--)
  {
    uint8_t bit = (num >> i) & 1;
    sprintf(buf, "%d", bit);
    buf++;
  }
}


void enlargeBuffer(uint8_t *src, uint8_t *dest)
{
  memset(dest, 0, DEST_WIDTH * DEST_HEIGHT / 8);
  for (int src_y = 0; src_y < SRC_HEIGHT; src_y++)
  {
    for (int x = 0; x < SRC_WIDTH; x += 8)
    {
      int src_offset = src_y * SRC_WIDTH / 8 + x / 8;
      uint8_t srcByte = src[src_offset];
      for (int i = 0; i < 8; i++)
      {
        uint8_t pixel = (srcByte >> (7 - i)) & 1;
        int src_x = x + i;

        int dest_x0 = 2 * src_x;
        int dest_y0 = 2 * src_y;
        int dest_x1 = dest_x0 + 1;
        int dest_y1 = dest_y0;
        int dest_x2 = dest_x0;
        int dest_y2 = dest_y0 + 1;
        int dest_x3 = dest_x0 + 1;
        int dest_y3 = dest_y0 + 1;

        int dest_offset_0 = dest_y0 * DEST_WIDTH / 8 + dest_x0 / 8;
        int dest_offset_1 = dest_y1 * DEST_WIDTH / 8 + dest_x1 / 8;
        int dest_offset_2 = dest_y2 * DEST_WIDTH / 8 + dest_x2 / 8;
        int dest_offset_3 = dest_y3 * DEST_WIDTH / 8 + dest_x3 / 8;

        dest[dest_offset_0] |= (pixel << (7 - dest_x0 % 8));
        dest[dest_offset_1] |= (pixel << (7 - dest_x1 % 8));
        dest[dest_offset_2] |= (pixel << (7 - dest_x2 % 8));
        dest[dest_offset_3] |= (pixel << (7 - dest_x3 % 8));
# 883 "/Users/eggfly/github/eggfly/pepper-deck/PlatformIO/WenQuXing_NC1020_Emulator/src/WenQuXing_NC1020_Emulator.ino"
      }
    }
  }
}

void magnify_pixels(uint8_t *src, uint8_t *dest)
{
  for (int y = 0; y < 80; y++)
  {
    for (int x = 0; x < 160; x++)
    {
      uint8_t pixel = ((src[y * 20 + (x / 8)] >> (7 - (x % 8))) & 0x01) ? 0xFF : 0x00;
      for (int i = 0; i < 2; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          dest[(y * 320 + x * 2 + i) * 4 + j] = pixel;
        }
      }
    }
  }
}

void my_lcd_zoom_in(uint8_t *src, uint8_t *dest)
{

  for (size_t x = 0; x < 160; x++)
  {
    for (size_t y = 0; y < 80; y++)
    {
      size_t src_offset = y * 160 * 2 + 2 * x;
      uint8_t v1 = src[src_offset];
      uint8_t v2 = src[src_offset + 1];
      size_t dest_offset_0 = 640 * (2 * y) + 2 * (2 * x);
      size_t dest_offset_1 = 640 * (2 * y + 1) + 2 * (2 * x);
      size_t dest_offset_2 = 640 * (2 * y) + 2 * (2 * x + 1);
      size_t dest_offset_3 = 640 * (2 * y + 1) + 2 * (2 * x + 1);
      dest[dest_offset_0] = v1;
      dest[dest_offset_0 + 1] = v2;
      dest[dest_offset_1] = v1;
      dest[dest_offset_1 + 1] = v2;
      dest[dest_offset_2] = v1;
      dest[dest_offset_2 + 1] = v2;
      dest[dest_offset_3] = v1;
      dest[dest_offset_3 + 1] = v2;
    }
  }


}


uint8_t func_keys[] = {
    8,
    8,
    9,
    9,
    10,
    10,
    11,
    11,
    12,
    12,
    13,
    13,
    14,
    14,
};
const size_t func_key_size = sizeof(func_keys) / sizeof(func_keys[0]);
int curr_key_index = 0;
int key_release_countdown = 0;
int key_to_release = -1;

inline void drawMouse()
{
  if (mouse_hid_report.pos_x < 0)
  {
    mouse_hid_report.pos_x = 0;
  }
  else if (mouse_hid_report.pos_x >= DISPLAY_WIDTH)
  {
    mouse_hid_report.pos_x = DISPLAY_WIDTH - 1;
  }
  if (mouse_hid_report.pos_y < 0)
  {
    mouse_hid_report.pos_y = 0;
  }
  else if (mouse_hid_report.pos_y >= DISPLAY_HEIGHT)
  {
    mouse_hid_report.pos_y = DISPLAY_HEIGHT - 1;
  }
  int16_t mouse_x = mouse_hid_report.pos_x;
  int16_t mouse_y = mouse_hid_report.pos_y;

  canvas.drawXBitmap(mouse_x, mouse_y, mouse_xbm, mouse_xbm_width, mouse_xbm_height, RGB565_WHITE);
}
inline void nc1020_loop()
{
  auto start_time = millis();
  size_t slice = 20;
  wqx::RunTimeSlice(slice, false);
  if (LOG_LEVEL <= LOG_LEVEL_VERBOSE)
  {
    Serial.printf("slice=%d,cost=%dms\n", slice, millis() - start_time);
  }

  wqx::CopyLcdBuffer((uint8_t *)lcd_buff);

  enlargeBuffer(lcd_buff, lcd_buff_expanded);



  canvas.fillScreen(RGB565_BLACK);

  canvas.drawBitmap(
      0,
      (240 - 160) / 2,
      lcd_buff_expanded, 320, 160, WQX_COLOR_RGB565_FG, WQX_COLOR_RGB565_BG);
  canvas.setCursor(2, 2);
  canvas.setTextSize(2);
  canvas.setTextColor(RGB565_RED);
  canvas.printf("BATTERY %.4fV %.3f%%\n", battery_gauge.voltage, battery_gauge.percent);

  canvas.setCursor(2, 20);
  canvas.printf("HIGH VOLTAGE BATTERY\n");


  if (key_to_release > 0)
  {
    if (key_release_countdown == 0)
    {
      wqx::SetKey(key_to_release, false);
      Serial.printf("nc1020 set release key: %d\n", key_to_release);
      key_to_release = -1;
    }
    else
    {
      key_release_countdown--;
    }
  }
  else
  {

    if (ruler_deck::pressed_key.length() > 0)
    {
      int keycode = 0;
      if (nc1020_keymap.find(ruler_deck::pressed_key) != nc1020_keymap.end())
      {
        keycode = nc1020_keymap[ruler_deck::pressed_key];
        Serial.printf("receive key=%s -> 0x%02x\n", ruler_deck::pressed_key.c_str(), keycode);
      }
      else
      {
        Serial.printf("receive key=%s -> not found!\n", ruler_deck::pressed_key.c_str());
        keycode = -1;
      }
      wqx::SetKey(keycode, true);
      key_to_release = keycode;
      key_release_countdown = 1;

      curr_key_index++;
      curr_key_index %= func_key_size;
    }
  }
}

void loop()
{
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.println("I'm the master processor!");
  Serial.printf("ESP32 Chip model = % s Rev % d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has % d cores\n", ESP.getChipCores());
  Serial.print("Chip ID: ");
  Serial.println(chipId);
  auto lastUpdate = millis();

  for (;;)
  {
    auto currTime = millis();
    currPage->onDraw();
    keyboard_loop();
    if (currTime - lastUpdate > 1000)
    {
      lastUpdate = currTime;
      battery_loop();

    }





  }
}

void handle_backlight_key()
{
  uint8_t count = sizeof(backlight_levels) / sizeof(backlight_levels[0]);
  current_backlight_level++;
  current_backlight_level %= count;
  set_backlight_level();
}

void show_restart()
{
  portENTER_CRITICAL(&my_mutex);
  canvas.fillScreen(0);
  canvas.setCursor(0, 0);
  canvas.setTextColor(0xFF);
  canvas.setTextSize(2);
  canvas.println("---- - RESTART NOW ! ---- -");
  portEXIT_CRITICAL(&my_mutex);

}

void select_boot_item(int8_t offset)
{
  int8_t new_selected = selected_menu_index + offset;
  if (new_selected < 0)
  {
    new_selected = 0;
  }
  else if (new_selected >= menu_items_count)
  {
    new_selected = menu_items_count - 1;
  }
  selected_menu_index = new_selected;
}

bool keyboard_callback(const char *key_str)
{
  Serial.printf("keyboard_callback: % s\n", key_str);
  if (strcmp(key_str, "Backlight") == 0)
  {
    handle_backlight_key();
    return true;
  }
  else if (strcmp(key_str, "Power") == 0)
  {
    show_restart();
    ESP.restart();
    return true;
  }
  else
  {
    return currPage->handleKey(key_str);
  }
}