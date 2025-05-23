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
  


void save_file(char* newfile);
int check_save();
void new_cb(Fl_Widget*, void*);
void open_cb(Fl_Widget*, void*);
void saveas_cb(void);
void save_cb(void);
void quit_cb (Fl_Widget*, void*);
void cut_cb(Fl_Widget*, void* v);
void copy_cb(Fl_Widget*, void* v);
void paste_cb(Fl_Widget*, void* v);
void delete_cb(Fl_Widget*, void* v);
void find_cb(Fl_Widget* w, void* v);
void find2_cb(Fl_Widget* w, void* v);
void replace_cb(Fl_Widget*, void* v);
void load_file(char *newfile, int ipos);

class EditorWindow : public Fl_Double_Window { //класс окна
    public:
        EditorWindow(int w, int h, const char* t);
        virtual ~EditorWindow() {}
        Fl_Window *replace_dlg;
        Fl_Input *replace_find;
        Fl_Input *replace_with;
        Fl_Button *replace_all;
        Fl_Return_Button *replace_next;
        Fl_Button *replace_cancel;

        Fl_Text_Editor *editor;
        char search[256];   
        void draw() override { Fl_Double_Window::draw(); }
        int handle(int event) override { return Fl_Double_Window::handle(event); }
        void resize(int x, int y, int w, int h) override { Fl_Double_Window::resize(x, y, w, h); }
        
};

int changed = 0;
char filename[256] = "";
Fl_Text_Buffer *textbuf;
int loading = 0;

void save_file(char* newfile){
    textbuf->savefile(newfile);
    strcpy(filename, newfile);
}

int check_save(){
    if (!changed) return 1;

    int r = fl_choice("Файл не сохранён. Сохранить?\n", 
        "Да", "Нет", "Отмена");

    if (r == 0) { 
    save_cb();
    return !changed;
    }
    return (r == 1) ? 1 : 0;
}

void new_cb(Fl_Widget*, void*) {
    if (changed && !check_save()) return;

    filename[0] = '\0'; 
    textbuf->select(0, textbuf->length()); // очистка текстового буфера
    textbuf->remove_selection();
    changed = 0;
    textbuf->call_modify_callbacks();
}

void open_cb(Fl_Widget*, void*) {
  if (changed && !check_save()) return;

  char* newfile = fl_file_chooser("Открыть", "*.txt", filename);
  if (newfile != NULL) load_file(newfile, -1);
}

void saveas_cb(void) {
    char* newfile;
    newfile = fl_file_chooser("Сохранить как", "*", filename);
    if (newfile != NULL) save_file(newfile);
}

void save_cb(void) {
    if (filename[0] == '\0') {
        saveas_cb();
    }
        else save_file(filename);
}



void quit_cb (Fl_Widget*, void*) {
    if (changed && !check_save()) return;
    exit(0);
}

void cut_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->editor->kf_cut(0, e->editor);;
}

void copy_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->editor->kf_copy(0, e->editor);;
}

void paste_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->editor->kf_paste(0, e->editor);;
}

void delete_cb(Fl_Widget*, void* v) {
    textbuf->remove_selection();
}

void find_cb(Fl_Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char *val;

    val = fl_input("Поиск", e->search, sizeof(e->search));
    if (val != NULL) {
        strcpy(e->search, val);
        find2_cb(w, v);
    }
}

void find2_cb(Fl_Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    if (e->search[0] == '\0') {
        find_cb(w, v);
        return;
    }

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, e->search, &pos);
    if (found){
        textbuf->select(pos, pos + strlen(e->search));
        e->editor->insert_position(pos + strlen(e->search));
        e->editor->show_insert_position();
    } else {
        fl_alert("Текст не найден.");
    }
}

void replace_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->show();
}


void load_file(char *newfile, int ipos) {
    loading = 1;
    int insert = (ipos != -1);
    changed = insert;
    if (!insert) strcpy(filename, "");

    int r;
    if (!insert) r = textbuf->loadfile(newfile);
    else r = textbuf->insertfile(newfile, ipos);
    
    if (r) {
        fl_alert("Ошибка чтения \'%s\':\n%s.", newfile, strerror(errno));
    }else
        if (!insert) strcpy(filename, newfile);

    loading = 0;
    textbuf->call_modify_callbacks();
    
}

void hide_replace_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->hide();
}

void replace_all_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char* find = e->replace_find->value();
    const char* replace = e->replace_with->value();
    if (!find || strlen(find) == 0) return;

    int pos = 0;
    int found;
    while ((found = textbuf->search_forward(pos, find, &pos))) {
        textbuf->replace(pos, pos + strlen(find), replace);
        pos += strlen(replace);
    }
}



Fl_Menu_Item menuitems[] = { //меню для команд редактора
    {"&Файл", 0, 0, 0, FL_SUBMENU},
        {"&Новый", FL_COMMAND + 'n', (Fl_Callback *)new_cb},
        {"&Открыть...", FL_COMMAND + 'o', (Fl_Callback *)open_cb },
        {"&Сохранить", FL_COMMAND + 's', (Fl_Callback *)save_cb },
        {"Сохранить &как...", FL_COMMAND + FL_SHIFT + 's', (Fl_Callback *)saveas_cb },
        {"&Выход", FL_COMMAND + 'q', (Fl_Callback *)quit_cb },
        {0},
    {"&Правка", 0, 0, 0, FL_SUBMENU},
        {"&Вырезать", FL_COMMAND + 'x', (Fl_Callback *)cut_cb },   
        {"&Копировать", FL_COMMAND + 'c', (Fl_Callback *)copy_cb },
        {"&Вставить", FL_COMMAND + 'v', (Fl_Callback *)paste_cb },
        {"&Удалить", 0, (Fl_Callback *)delete_cb },
        {0},
    {"&Поиск", 0, 0, 0, FL_SUBMENU},
        {"&Найти", FL_COMMAND + 'f', (Fl_Callback *)find_cb },
        {"&Заменить", FL_COMMAND + 'h', (Fl_Callback *)replace_cb },
    {0}
};



EditorWindow::EditorWindow(int w, int h, const char* t) : Fl_Double_Window(w, h, t) {
    const int menu_height = 30;
    Fl_Menu_Bar *m = new Fl_Menu_Bar(0, 0, w, menu_height);
    m->copy(menuitems);

    editor = new Fl_Text_Editor(0, menu_height, w, h - menu_height);
 
    this->resizable(editor);
    editor->buffer(textbuf);

    replace_dlg = new Fl_Window(300, 150, "Заменить");
    replace_find = new Fl_Input(10, 10, 180, 25, "Найти");
    replace_with = new Fl_Input(10, 40, 180, 25, "Заменить на:");
    replace_all = new Fl_Button(10, 80, 90, 25, "Заменить все");
    replace_next = new Fl_Return_Button(110, 80, 90, 25, "Найти далее");
    replace_cancel = new Fl_Button(210, 80, 80, 25, "Отмена");

    replace_all->callback(replace_all_cb, this);
    replace_next->callback(find2_cb, this);
    replace_cancel->callback(hide_replace_cb, this);


    replace_dlg->end();
    replace_dlg->set_non_modal();


    printf("Меню создано\n");
    this->end();
    this->resizable(editor);
}



EditorWindow* new_view(){
    EditorWindow* window = new EditorWindow(800, 600, "Текстовый редактор");
    window->show();
    return window;
}

int main(int argc, char **argv){
    textbuf = new Fl_Text_Buffer;

    EditorWindow* window = new_view();

    if (argc > 1) load_file(argv[1], -1);

    return Fl::run();
}