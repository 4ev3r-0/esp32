#include "Arduino.h"
#include "LittleFS.h"
#include "src/h/display.h"
#include <LuaWrapper.h>

display lcd;
LuaWrapper lua;

int lua_lcd_print(lua_State* LState) {
    const char* text = luaL_checkstring(LState, 1);
    int x = luaL_checkinteger(LState, 2);
    int y = luaL_checkinteger(LState, 3);

    lcd.drawString(text, x, y);

    return 0;
}

int lua_load_text_file(lua_State* LState) {
    const char* path = luaL_checkstring(LState, 1);

    if (!LittleFS.exists(path)) {
        return 0;
    }

    File file = LittleFS.open(path, "r");

    if (!file) {
        return 0;
    }

    lua_newtable(LState);

    int lineIndex = 1;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.replace("\r", "");

        lua_pushstring(LState, line.c_str());
        lua_rawseti(LState, -2, lineIndex++);
    }

    file.close();

    return 1;
}

int lua_lcd_clear(lua_State* LState) {
    uint32_t color = luaL_checkinteger(LState, 1);

    lcd.fillScreen(color);

    return 0;
}

int lua_read_button(lua_State* LState) {
    int pin = luaL_checkinteger(LState, 1);

    lua_pushboolean(LState, digitalRead(pin) == LOW);

    return 1;
}

void setup() {
    Serial.begin(115200);

    if (!LittleFS.begin(true)) {
        Serial.println("FS Error");
    }

    const uint8_t pins[] = {1, 2, 3, 4, 5, 6};

    for (int i = 0; i < 6; i++) {
        pinMode(pins[i], INPUT_PULLUP);
    }

    lcd.init();
    lcd.setRotation(3);
    lcd.fillScreen(0x0000);

    if (!LittleFS.exists("/splash_anim/main.lua")) {
        Serial.println("Lua file missing");
        return;
    }

    lua.Lua_register("lcd_print", lua_lcd_print);
    lua.Lua_register("load_text_file", lua_load_text_file);
    lua.Lua_register("lcd_clear", lua_lcd_clear);
    lua.Lua_register("read_button", lua_read_button);

    File file = LittleFS.open("/splash_anim/main.lua", "r");

    if (!file) {
        Serial.println("Failed to open Lua file");
        return;
    }

    String scriptContent;

    while (file.available()) {
        scriptContent += (char)file.read();
    }

    file.close();

    String result = lua.Lua_dostring(&scriptContent);

    if (result.length() > 0) {
        Serial.printf("Lua Setup Error: %s\n", result.c_str());
    }
}

void loop() {
    String updateScript = "if update then update() end";

    String result = lua.Lua_dostring(&updateScript);

    if (result.length() > 0) {
        Serial.printf("Lua Loop Error: %s\n", result.c_str());
    }

    delay(10);
}
