#include "common.h"
#include "cJSON.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "tui.h"
#include <ctype.h>

#define MAX_VISIBLE 2048

typedef struct { cJSON *item; int depth; } ViewNode;

typedef struct ExpandNode {
    const cJSON *ptr;
    int expanded;
    struct ExpandNode *next;
} ExpandNode;

static ExpandNode *expand_state = NULL;

/* Expansion state helpers */
static int get_expanded(const cJSON *item) {
    for (ExpandNode *e = expand_state; e; e = e->next)
        if (e->ptr == item) return e->expanded;
    return 0;
}
static void set_expanded(const cJSON *item, int val) {
    for (ExpandNode *e = expand_state; e; e = e->next)
        if (e->ptr == item) { e->expanded = val; return; }
    ExpandNode *e = malloc(sizeof(ExpandNode));
    e->ptr = item;
    e->expanded = val;
    e->next = expand_state;
    expand_state = e;
}
static void toggle_expanded(const cJSON *item) {
    set_expanded(item, !get_expanded(item));
}

/* Build flattened visible tree */
static void build_visible(cJSON *node, int depth, ViewNode *out, int *count) {
    for (cJSON *cur = node; cur; cur = cur->next) {
        if (*count >= MAX_VISIBLE) return;
        out[*count].item = cur;
        out[*count].depth = depth;
        (*count)++;
        if ((cJSON_IsObject(cur) || cJSON_IsArray(cur)) && get_expanded(cur))
            build_visible(cur->child, depth + 1, out, count);
    }
}

static char *popup_input(const char *title, const char *initial) {
    int width = 50, height = 5;
    int startx = (COLS - width)/2;
    int starty = (LINES - height)/2;
    WINDOW *win = newwin(height, width, starty, startx);
    box(win,0,0);

    mvwprintw(win, 1, 2, "%s:", title);

    char buf[256];
    int pos = 0;

    if (initial) {
        strncpy(buf, initial, sizeof(buf)-1);
        buf[sizeof(buf)-1] = '\0';
        pos = strlen(buf);
    } else buf[0] = '\0';

    int ch;
    keypad(win, TRUE);
    curs_set(1);
    wrefresh(win);

    while (1) {
        // Draw prompt and current buffer
        mvwprintw(win, 2, 2, "> %-*s", width-4, buf);
        wmove(win, 2, 4 + pos);
        wrefresh(win);

        ch = wgetch(win);

        if (ch == '\n' || ch == KEY_ENTER) break;      // Enter -> done
        else if (ch == KEY_LEFT && pos > 0) pos--;     // Move left
        else if (ch == KEY_RIGHT && pos < strlen(buf)) pos++; // Move right
        else if ((ch == KEY_BACKSPACE || ch == 127) && pos > 0) {
            memmove(buf+pos-1, buf+pos, strlen(buf)-pos+1);
            pos--;
        }
        else if (isprint(ch) && strlen(buf) < sizeof(buf)-1) {
            memmove(buf+pos+1, buf+pos, strlen(buf)-pos+1);
            buf[pos++] = ch;
        }
    }

    curs_set(0);
    delwin(win);
    return strdup(buf);
}




/* Find parent node (LEFT nav) */
static int find_parent_index(ViewNode *vlist, int vcount, int idx) {
    int depth = vlist[idx].depth;
    for (int i = idx-1; i>=0; i--)
        if (vlist[i].depth < depth) return i;
    return idx;
}

/* Draw single entry */
static void draw_line(int y, int highlight, int depth, cJSON *n) {
    move(y, depth*4);
    if (highlight) attron(A_REVERSE);

    if (cJSON_IsObject(n) || cJSON_IsArray(n)) {
        int exp = get_expanded(n);
        printw("%c %s : %s",
               exp ? '-' : '+',
               n->string ? n->string : "(index)",
               cJSON_IsObject(n) ? "{...}" : "[...]");
    } else if (cJSON_IsString(n)) {
        printw("  %s : \"%s\"", n->string?n->string:"(str)", n->valuestring);
    } else if (cJSON_IsBool(n)) {
        printw("  %s : %s", n->string?n->string:"(bool)", cJSON_IsTrue(n)?"true":"false");
    } else if (cJSON_IsNumber(n)) {
        printw("  %s : %g", n->string?n->string:"(num)", n->valuedouble);
    } else if (cJSON_IsNull(n)) {
        printw("  %s : null", n->string?n->string:"(null)");
    }

    if (highlight) attroff(A_REVERSE);
}

/* === MAIN UI LOOP === */
void run_ui(cJSON *root) {
    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    int highlight=0, ch;

    while (1) {
        clear();
        mvprintw(0,0,"JSON Editor - <up>/<down> nav  <enter> expand/collapse  <e> edit  <a> add  <q> quit");

        ViewNode vlist[MAX_VISIBLE];
        int vcount=0;
        build_visible(root,0,vlist,&vcount);

        int y=2;
        for (int i=0;i<vcount;i++)
            draw_line(y+i, i==highlight, vlist[i].depth, vlist[i].item);

        refresh();
        ch = getch();

        if (ch=='q') break;
        else if (ch==KEY_UP) highlight=(highlight-1+vcount)%vcount;
        else if (ch==KEY_DOWN) highlight=(highlight+1)%vcount;
        else if (ch==KEY_LEFT) {
            cJSON *sel=vlist[highlight].item;
            if ((cJSON_IsObject(sel)||cJSON_IsArray(sel)) && get_expanded(sel))
                set_expanded(sel,0);
            else highlight=find_parent_index(vlist,vcount,highlight);
        }
        else if (ch==KEY_RIGHT||ch=='\n'||ch==KEY_ENTER) {
            cJSON *sel=vlist[highlight].item;
            if (cJSON_IsObject(sel)||cJSON_IsArray(sel)) toggle_expanded(sel);
        }
        else if (ch=='e') {
            cJSON *sel=vlist[highlight].item;
            if (cJSON_IsString(sel)) {
                char *newv=popup_input("Edit string", sel->valuestring);
                if (newv) { cJSON_SetValuestring(sel,newv); free(newv); }
            } else if (cJSON_IsNumber(sel)) {
                char tmp[32]; snprintf(tmp,sizeof(tmp),"%g",sel->valuedouble);
                char *newv=popup_input("Edit number", tmp);
                if (newv) { sel->valuedouble=atof(newv); sel->valueint=atoi(newv); free(newv); }
            } else if (cJSON_IsBool(sel)) {
                sel->type=(sel->type==cJSON_True)?cJSON_False:cJSON_True;
            }
        }
        else if (ch=='a') {
    cJSON *sel = vlist[highlight].item;

    int type_ch = 0;
    if (cJSON_IsObject(sel) || cJSON_IsArray(sel)) {
        // Ask for type
        mvprintw(LINES-2, 0, "Add type: s=string, n=number, o=object, a=array, b=bool ");
        refresh();
        type_ch = getch();
        mvprintw(LINES-2, 0, "%*s", COLS, " "); // clear prompt
    }

    cJSON *new_item = NULL;
    char *key = NULL;

    if (cJSON_IsObject(sel)) {
        key = popup_input("New key", "key");

        if (!key) break;

        switch(type_ch) {
            case 's': {
                char *val = popup_input("Value", "value");
                new_item = cJSON_CreateString(val ? val : "");
                free(val);
                break;
            }
            case 'n': {
                char tmp[32];
                char *val = popup_input("Value", "0");
                new_item = cJSON_CreateNumber(val ? atof(val) : 0);
                free(val);
                break;
            }
            case 'o': new_item = cJSON_CreateObject(); break;
            case 'a': new_item = cJSON_CreateArray(); break;
            case 'b': new_item = cJSON_CreateBool(1); break; // default true
            default: break;
        }

        if (new_item) {
            cJSON_AddItemToObject(sel, key, new_item);
            set_expanded(sel,1);
        }
        free(key);

    } else if (cJSON_IsArray(sel)) {
        switch(type_ch) {
            case 's': {
                char *val = popup_input("New array value", "value");
                new_item = cJSON_CreateString(val ? val : "");
                free(val);
                break;
            }
            case 'n': {
                char *val = popup_input("New array number", "0");
                new_item = cJSON_CreateNumber(val ? atof(val) : 0);
                free(val);
                break;
            }
            case 'o': new_item = cJSON_CreateObject(); break;
            case 'a': new_item = cJSON_CreateArray(); break;
            case 'b': new_item = cJSON_CreateBool(1); break;
            default: break;
        }

        if (new_item) {
            cJSON_AddItemToArray(sel, new_item);
            set_expanded(sel,1);
        }
    }
}

    }
    endwin();
}
