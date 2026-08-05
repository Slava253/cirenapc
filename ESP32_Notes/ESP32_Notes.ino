/*
  ESP32 Notes Server
  Веб-блокнот с сохранением заметок во внутреннюю память ESP32
  Сервер: встроенный WebServer
*/

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// ===== Wi-Fi настройки =====

const char* ssid = "ESP32-Notes";
const char* password = "12345678";

WebServer server(80);


// ===== Файл хранения =====

const char* notesFile = "/notes.json";


// ===== Структура заметки =====

struct Note {
  int id;
  String title;
  String text;
  bool favorite;
  String date;
};


DynamicJsonDocument notesDoc(16384);


// ===== Загрузка заметок =====

void loadNotes() {

  if (!LittleFS.exists(notesFile)) {

    notesDoc.clear();
    notesDoc.createNestedArray("notes");

    saveNotes();
    return;
  }


  File file = LittleFS.open(notesFile, "r");

  if (!file) {
    return;
  }


  DeserializationError error =
    deserializeJson(notesDoc, file);


  file.close();


  if (error) {

    notesDoc.clear();
    notesDoc.createNestedArray("notes");

    saveNotes();
  }
}


// ===== Сохранение заметок =====

void saveNotes() {

  File file = LittleFS.open(notesFile, "w");

  if (!file) {
    return;
  }


  serializeJson(notesDoc, file);

  file.close();
}


// ===== Получить все заметки =====

void handleGetNotes() {

  String output;

  serializeJson(notesDoc, output);

  server.send(
    200,
    "application/json",
    output
  );
}


// ===== Добавление заметки =====

void handleAddNote() {


  if (!server.hasArg("plain")) {

    server.send(
      400,
      "text/plain",
      "Нет данных"
    );

    return;
  }


  DynamicJsonDocument request(2048);


  deserializeJson(
    request,
    server.arg("plain")
  );


  JsonArray notes =
    notesDoc["notes"];


  JsonObject note =
    notes.createNestedObject();


  int newId = millis();


  note["id"] = newId;

  note["title"] =
    request["title"].as<String>();

  note["text"] =
    request["text"].as<String>();

  note["favorite"] =
    false;

  note["date"] =
    String(millis());


  saveNotes();


  server.send(
    200,
    "application/json",
    "{\"status\":\"ok\"}"
  );
}



// ===== Удаление заметки =====

void handleDeleteNote() {


  if (!server.hasArg("id")) {

    server.send(
      400,
      "text/plain",
      "Нет ID"
    );

    return;
  }


  int id =
    server.arg("id").toInt();



  JsonArray notes =
    notesDoc["notes"];


  for (int i = 0; i < notes.size(); i++) {


    if (notes[i]["id"] == id) {


      notes.remove(i);

      break;
    }
  }


  saveNotes();


  server.send(
    200,
    "application/json",
    "{\"status\":\"deleted\"}"
  );
}



// ===== Очистить все заметки =====

void handleClearNotes() {


  notesDoc.clear();

  notesDoc.createNestedArray("notes");


  saveNotes();


  server.send(
    200,
    "application/json",
    "{\"status\":\"cleared\"}"
  );
}
// ===== Редактирование заметки =====

void handleEditNote() {

  if (!server.hasArg("plain")) {

    server.send(
      400,
      "text/plain",
      "Нет данных"
    );

    return;
  }


  DynamicJsonDocument request(2048);


  deserializeJson(
    request,
    server.arg("plain")
  );


  int id =
    request["id"];


  JsonArray notes =
    notesDoc["notes"];


  for (JsonObject note : notes) {


    if (note["id"] == id) {


      note["title"] =
        request["title"].as<String>();

      note["text"] =
        request["text"].as<String>();

      break;
    }
  }


  saveNotes();


  server.send(
    200,
    "application/json",
    "{\"status\":\"edited\"}"
  );
}



// ===== Изменение избранного =====

void handleFavorite() {


  if (!server.hasArg("id")) {

    server.send(
      400,
      "text/plain",
      "Нет ID"
    );

    return;
  }


  int id =
    server.arg("id").toInt();


  JsonArray notes =
    notesDoc["notes"];


  for (JsonObject note : notes) {


    if (note["id"] == id) {


      note["favorite"] =
        !note["favorite"].as<bool>();

      break;
    }
  }


  saveNotes();


  server.send(
    200,
    "application/json",
    "{\"status\":\"favorite\"}"
  );
}



// ===== Экспорт заметок =====

void handleExport() {


  String text = "";


  JsonArray notes =
    notesDoc["notes"];


  for (JsonObject note : notes) {


    text += "================\n";

    text += note["title"].as<String>();

    text += "\n\n";

    text += note["text"].as<String>();

    text += "\n";
  }


  server.send(
    200,
    "text/plain",
    text
  );
}



// ===== Главная страница =====

void handleRoot() {


  File file =
    LittleFS.open(
      "/index.html",
      "r"
    );


  if (!file) {

    server.send(
      404,
      "text/plain",
      "index.html не найден"
    );

    return;
  }


  server.streamFile(
    file,
    "text/html"
  );


  file.close();
}



// ===== CSS =====

void handleCSS() {


  File file =
    LittleFS.open(
      "/style.css",
      "r"
    );


  server.streamFile(
    file,
    "text/css"
  );


  file.close();
}



// ===== JavaScript =====

void handleJS() {


  File file =
    LittleFS.open(
      "/script.js",
      "application/javascript"
    );


  server.streamFile(
    file,
    "application/javascript"
  );


  file.close();
}



// ===== Запуск =====

void setup() {


  Serial.begin(115200);



  if (!LittleFS.begin(true)) {

    Serial.println(
      "Ошибка LittleFS"
    );

    return;
  }



  loadNotes();



  WiFi.softAP(
    ssid,
    password
  );



  Serial.println();

  Serial.println(
    "WiFi запущен"
  );

  Serial.print(
    "Адрес: "
  );

  Serial.println(
    WiFi.softAPIP()
  );



  server.on(
    "/",
    handleRoot
  );


  server.on(
    "/style.css",
    handleCSS
  );


  server.on(
    "/script.js",
    handleJS
  );


  server.on(
    "/api/notes",
    HTTP_GET,
    handleGetNotes
  );


  server.on(
    "/api/add",
    HTTP_POST,
    handleAddNote
  );


  server.on(
    "/api/edit",
    HTTP_POST,
    handleEditNote
  );


  server.on(
    "/api/delete",
    HTTP_GET,
    handleDeleteNote
  );


  server.on(
    "/api/favorite",
    HTTP_GET,
    handleFavorite
  );


  server.on(
    "/api/clear",
    HTTP_GET,
    handleClearNotes
  );


  server.on(
    "/api/export",
    HTTP_GET,
    handleExport
  );



  server.begin();


  Serial.println(
    "Сервер запущен"
  );
}



void loop() {

  server.handleClient();

}
