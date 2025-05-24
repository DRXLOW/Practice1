# Руководство по созданию текстового редактора на C++ с использованием FLTK

## Описание проекта

Простой текстовый редактор с графическим интерфейсом, реализованный на языке C++ с использованием библиотеки [FLTK (Fast Light Toolkit)](https://www.fltk.org/ "Официальный сайт FLTK"). Поддерживает функции открытия, редактирования, сохранения файлов, а также операции поиска и замены текста.

## Условия

Перед началом убедитесь, что у вас установлены:

- Компилятор C++ (g++/clang++)
- FLTK 1.3+ (или новее)
- Cmake

## Установка FLTK

1. Скачайте [FLTK](https://www.fltk.org/software.php).
1. Откройте MSYS2 MinGW 64-bit.
1. Перейдите в папку с исходниками (Например, fltk-1.3.9).
   `cd /c/fltk-1.3.9/`
1. Создайте папку для сборки и перейдите в нее:
   `mkdir build && cd build`
1. Запустите CMake (укажите путь для установки C:/FLTK):
   `cmake -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:/FLTK ..`
1. Соберите проект:
   `mingw32-make -j4`
1. Установите библиотеку:
   `mingw32-make install`

## Сборка и запуск

Использование g++:
`g++ src/main.cpp -o src/editor.exe -IC:/FLTK/include -LC:/FLTK/lib -lfltk -lfltk_images -lfltk_png -lfltk_jpeg -lole32 -luuid -lcomctl32 -lgdi32 -lws2_32 -lgdiplus -lwinspool -mwindows -static `

## Руководство по коду

### Подключаем заголовки

```#include <string>
#include <cstring>
#include <cerrno>
#include <cstdlib>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.h>
#include <FL/Fl_Input.h>
#include <FL/Fl_Window.h>
#include <FL/Fl_Button.h>
#include <FL/Fl_Return_Button.h>
#include <FL/Fl_Text_Editor.h>
#include <FL/Fl_Menu_Bar.h>
#include <FL/Fl_Choice.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Text_Buffer.H>
```

### Глобальные переменные

```int changed = 0;
std::string filename;
Fl_Text_Buffer *textbuf;
int loading = 0;
```

### Объявляем основные функции

```void set_status(const char* msg);
void save_file(char* newfile);
int check_save();
void new_cb(Fl_Widget*, void*);
void open_cb(Fl_Widget*, void*);
void save_cb(Fl_Widget*, void*);
void saveas_cb(Fl_Widget*, void*);
void quit_cb (Fl_Widget*, void*);
void cut_cb(Fl_Widget*, void* v);
void copy_cb(Fl_Widget*, void* v);
void paste_cb(Fl_Widget*, void* v);
void delete_cb(Fl_Widget*, void* v);
void find_cb(Fl_Widget* w, void* v);
void find2_cb(Fl_Widget* w, void* v);
void replace_cb(Fl_Widget*, void* v);
void load_file(char *newfile, int ipos);
```

### Класс окна редактора

```class EditorWindow : public Fl_Double_Window {
public:
    EditorWindow(int w, int h, const char* t);
    virtual ~EditorWindow() {}

    Fl_Window *replace_dlg;
    Fl_Input *replace_find;
    Fl_Input *replace_with;
    Fl_Button *replace_all;
    Fl_Return_Button *replace_next;
    Fl_Button *replace_cancel;

    Fl_Window *find_dlg;
    Fl_Input *find_input;
    Fl_Button *find_next_btn;
    Fl_Button *find_cancel_btn;

    Fl_Text_Editor *editor;
    char search[256];

    void draw() override { Fl_Double_Window::draw(); }
    int handle(int event) override { return Fl_Double_Window::handle(event); }
    void resize(int x, int y, int w, int h) override { Fl_Double_Window::resize(x, y, w, h); }
};
```

### Реализация функций

Сохранение файла:

```void save_file(char* newfile){
    textbuf->savefile(newfile);
    textbuf->savefile(filename.c_str());
}
```

Проверка сохранения текста:

```
int check_save(){
    if (!changed) return 1;

    int r = fl_choice("Файл не сохранён. Сохранить?\n",
        "Да", "Нет", "Отмена");

    if (r == 0) {
    save_cb(nullptr, nullptr);
    return !changed;
    }
    return (r == 1) ? 1 : 0;
}
```

Создание нового файла:

```
void new_cb(Fl_Widget*, void*) {
    if (changed && !check_save()) return;

    filename[0] = '\0';
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
    changed = 0;
    textbuf->call_modify_callbacks();
}
```

Открытие файла:

```void open_cb(Fl_Widget*, void*) {
  if (changed && !check_save()) return;

  char* newfile = fl_file_chooser("Открыть", "*.txt", filename.c_str());
  if (newfile != NULL) load_file(newfile, -1);
}
```

Сохранение файла:

```void save_cb(Fl_Widget*, void*) {
    if (filename[0] == '\0') {
        saveas_cb(nullptr, nullptr);
    } else {
        textbuf->savefile(filename.c_str());
    }
}
```

Сохранение файла как:

```void saveas_cb(Fl_Widget*, void*) {
    char* newfile = fl_file_chooser("Сохранить как", "*", filename.c_str());
    if (newfile != NULL) save_file(newfile);
}
```

Выйти:

```void quit_cb (Fl_Widget*, void*) {
    if (changed && !check_save()) return;
    exit(0);
}
```

Вырезать текст:

```
void cut_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->editor->kf_cut(0, e->editor);;
}
```

Копировать текст:

```
void copy_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->editor->kf_copy(0, e->editor);;
}
```

Вставить текст:

```
void paste_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->editor->kf_paste(0, e->editor);;
}
```

Удаление текста:

```
void delete_cb(Fl_Widget*, void* v) {
    textbuf->remove_selection();
}
```

Поиск текста:

```
void find_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->find_dlg->show();
}
```

```
void find2_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char* text = e->find_input->value();
    if (!text || strlen(text) == 0) {
        fl_alert("Введите текст для поиска.");
        return;
    }

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, text, &pos);
    if (found){
        textbuf->select(pos, pos + strlen(text));
        e->editor->insert_position(pos + strlen(text));
        e->editor->show_insert_position();
    } else {
        fl_alert("Текст не найден.");
    }
}
```

```
void hide_find_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->find_dlg->hide();
}
```

Замена текста:

```
void replace_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->show();
}

void hide_replace_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->hide();
}

void replace_all_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char* find = e->replace_find->value();
    const char* replace = e->replace_with->value();

    if (!find || strlen(find) == 0) {
        fl_alert("Введите текст для поиска.");
        return;
    }

    int pos = 0;
    int found;
    int find_len = strlen(find);
    int replace_len = strlen(replace);
    int count = 0;

    while ((found = textbuf->search_forward(pos, find, &pos))) {
        textbuf->replace(pos, pos + find_len, replace);
        pos += replace_len;
        count++;
    }

    if (count == 0) {
        fl_alert("Совпадений не найдено.");
    } else {
        fl_message("Заменено %d совпадений.", count);
    }
}

void replace_find_next_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char* val = e->replace_find->value();
    if (!val || strlen(val) == 0) {
        fl_alert("Введите текст для поиска.");
        return;
    }

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, val, &pos);
    if (found) {
        textbuf->select(pos, pos + strlen(val));
        e->editor->insert_position(pos + strlen(val));
        e->editor->show_insert_position();
    } else {
        fl_alert("Текст не найден.");
    }
}
```

Загрузка файла:

```
void load_file(char *newfile, int ipos) {
    loading = 1;
    int insert = (ipos != -1);
    changed = insert;
    if (!insert) filename.clear();

    int r;
    if (!insert) r = textbuf->loadfile(newfile);
    else r = textbuf->insertfile(newfile, ipos);

    if (r) {
        fl_alert("Ошибка чтения \'%s\':\n%s.", newfile, strerror(errno));
    }else
        if (!insert) {
            textbuf->savefile(filename.c_str());
            filename = newfile;
        }

    loading = 0;
    textbuf->call_modify_callbacks();

}
```

### Главное окно и меню

```EditorWindow::EditorWindow(int w, int h, const char* t) : Fl_Double_Window(w, h, t) {
    const int menu_height = 30;
    Fl_Menu_Bar *m = new Fl_Menu_Bar(0, 0, w, menu_height);
    m->add("&Файл/&Новый", FL_COMMAND + 'n', new_cb, this);
    m->add("&Файл/&Открыть...", FL_COMMAND + 'o', open_cb, this);
    m->add("&Файл/&Сохранить", FL_COMMAND + 's', save_cb, this);
    m->add("&Файл/Сохранить &как...", FL_COMMAND + FL_SHIFT + 's', saveas_cb, this);
    m->add("&Файл/&Выход", FL_COMMAND + 'q', quit_cb, this);

    m->add("&Правка/&Вырезать", FL_COMMAND + 'x', cut_cb, this);
    m->add("&Правка/&Копировать", FL_COMMAND + 'c', copy_cb, this);
    m->add("&Правка/&Вставить", FL_COMMAND + 'v', paste_cb, this);
    m->add("&Правка/&Удалить", 0, delete_cb, this);

    m->add("&Поиск/&Найти", FL_COMMAND + 'f', find_cb, this);
    m->add("&Поиск/&Заменить", FL_COMMAND + 'h', replace_cb, this);

    editor = new Fl_Text_Editor(0, menu_height, w, h - menu_height);

    this->resizable(editor);
    editor->buffer(textbuf);

    replace_dlg = new Fl_Window(320, 130, "Заменить");
    replace_dlg->begin();

    replace_find = new Fl_Input(100, 10, 200, 25, "Найти:");
    replace_with = new Fl_Input(100, 40, 200, 25, "Заменить на:");

    replace_find->tooltip("Введите текст, который нужно найти");
    replace_with->tooltip("Введите текст для замены");

    replace_next = new Fl_Return_Button(20, 80, 90, 30, "Найти далее");
    replace_all = new Fl_Button(120, 80, 90, 30, "Заменить все");
    replace_cancel = new Fl_Button(220, 80, 80, 30, "Отмена");

    replace_next->callback(replace_find_next_cb, this);
    replace_all->callback(replace_all_cb, this);
    replace_cancel->callback(hide_replace_cb, this);


    replace_dlg->end();
    replace_dlg->set_non_modal();

    find_dlg = new Fl_Window(300, 100, "Поиск");
    find_input = new Fl_Input(90, 10, 180, 25, "Найти:");
    find_next_btn = new Fl_Return_Button(90, 50, 90, 30, "Найти далее");
    find_cancel_btn = new Fl_Button(190, 50, 80, 30, "Отмена");

    find_input->tooltip("Введите текст для поиска");
    find_next_btn->callback(find2_cb, this);
    find_cancel_btn->callback(hide_find_cb, this);

    find_dlg->end();
    find_dlg->set_non_modal();


    printf("Меню создано\n");
    this->end();
    this->resizable(editor);
}
```

Вывод главного окна

```
EditorWindow* new_view(){
    EditorWindow* window = new EditorWindow(800, 600, "Текстовый редактор");
    window->show();
    return window;
}
```

Сама функция main()

```int main(int argc, char **argv){
    textbuf = new Fl_Text_Buffer;


    EditorWindow* window = new_view();

    if (argc > 1) load_file(argv[1], -1);

    return Fl::run();
}
```

### Сборка и запуск

```
g++ src/main.cpp -o src/editor.exe -IC:/FLTK/include -LC:/FLTK/lib -lfltk -lfltk_images -lfltk_png -lfltk_jpeg -lole32 -luuid -lcomctl32 -lgdi32 -lws2_32 -lgdiplus -lwinspool -mwindows -static
```
