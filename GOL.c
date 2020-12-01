#ifdef _WIN32
#define WINDOWS
#else
#define UNIX
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef WINDOWS

#include "unistd.h"
#include <conio.h>
#include "dirent.h"
#define _CRT_SECURE_NO_WARNINGS

#define KEYCODE1 224
//#define KEYCODE2 224
#define KEYCODE_UP 72
#define KEYCODE_DOWN 80
#define KEYCODE_LEFT 75
#define KEYCODE_RIGHT 77

#define getc getch
#define CONSOLE_CLEAR "cls"

#else

#include <unistd.h>
#include <dirent.h>

#define KEYCODE1 27
#define KEYCODE2 91
#define KEYCODE_UP 65
#define KEYCODE_DOWN 66
#define KEYCODE_LEFT 68
#define KEYCODE_RIGHT 67

#define CONSOLE_CLEAR "clear"
#define CONSOLE_CLEAR "clear"

#endif



#define LINE_MAX 10
#define SPEED 0.7 // delay in s
#define DEFAULT_WIDTH 20
#define DEFAULT_HEIGHT 20
#define FILE_LIST 5 // file list count
#define FILE_HEADER "GOL82874"


//#define DEBUG

char randomField = 0;

#ifdef WINDOWS
void sleep(int millis) {
    clock_t startTime = clock();
    while (clock() < startTime + millis);
}
#endif

typedef struct gamefield {
    int n;
    int m;
    char step;
    int generation;
    char* data;
} Field;

typedef struct cursor {
    int x;
    int y;
} Cursor;

char readInput(char* buf, int* numeric) {
    fflush(stdin);
    char* end;
    fgets(buf, sizeof(buf), stdin);
    buf[strlen(buf) - 1] = 0;
    *numeric = strtol(buf, &end, 10);
#ifdef DEBUG
    printf("Read:%d, *end == NULL: %d, strlen(buf): %d\n", *numeric, *end == NULL, strlen(buf));
#endif
    if (*end == NULL) return 1; // INTEGER GELESEN
    else return 0; // char* gelesen
}

char checkError(char* buf, Field* gamefield) { // retured 1 wenn restarted werden soll
    if (buf[0] == 'q') {
        if (gamefield->data != NULL)
            free(gamefield->data);
        exit(0); // programm beenden
    }
    else {
#ifndef DEBUG
        system(CONSOLE_CLEAR);
#endif
        if (buf[0] != 'r') printf("Fehlerhafte Eingabe!\n\n");
        return 1; // menu neustarten
    }
}

char askYN(char* message, char* ret, Field* gamefield) {
    char buf[LINE_MAX];
    int intBuf;

    printf("%s [y/N]\n", message);
    readInput(buf, &intBuf);
    if (strlen(buf) > 0 && buf[0] != 'n' && buf[0] != 'N') {
        if (buf[0] == 'y' || buf[0] == 'Y') *ret = 1;
        else if (checkError(buf, gamefield)) return 0;
    }
    else *ret = 0;
    return 1;
}

#ifdef WINDOWS
char random1(char percent) {
    return (char)((rand() % 100) < percent);
}
#endif

void generateField(Field* gamefield, int fillPercentage) {
    srand((unsigned)time(NULL));

    char percent = (fillPercentage > -1) ? fillPercentage : rand() % 101;

    if (fillPercentage == -1) printf("Fillpercentage: %d\n", percent);

    char* field = malloc(sizeof(char) * gamefield->n * gamefield->m);

    gamefield->data = field;

#ifdef UNIX
    char random() {
        return ((rand() % 100) < percent);
    }
#endif

    for (int i = 0; i < gamefield->n; i++) {
        for (int j = 0; j < gamefield->m; j++) {
#ifdef UNIX
            *(field + i * gamefield->m + j) = random();
#else
            *(field + i * gamefield->m + j) = random1(percent);
#endif
        }
    }

}

#ifdef WINDOWS
char formatted1(char val) {
    return (val == 1) ? '*' : ' ';
}
#endif

void printField(Field* gamefield, char printHeader) {
#ifdef UNIX
    char formatted(char val) {
        return (val == 1) ? '*' : ' ';
    }
#endif

    if (printHeader) {
#ifndef DEBUG
        system(CONSOLE_CLEAR);
#endif
        printf("Generation: %d ", gamefield->generation);
        if (gamefield->step) printf("Press <Enter> to continue, q to quit.\n");
        else printf("Press q to quit.\n");
    }


    for (int i = 0; i < gamefield->n; i++) {
        for (int j = 0; j < gamefield->m; j++) {
            printf("%c ", 
#ifdef UNIX
                formatted(
#else
                formatted1(
#endif
                *(gamefield->data + i * gamefield->n + j)));
        }
        printf("\n");
    }
}

void generateNext(Field* gamefield) {

    char* newfield = malloc(sizeof(char) * gamefield->n * gamefield->m);

    for (int i = 0; i < gamefield->n; i++) {
        for (int j = 0; j < gamefield->m; j++) {

            char currVal = *(gamefield->data + i * gamefield->n + j);
            char neighbours = 0;
            char newVal = currVal;

            for (int k = i - 1; k <= i + 1; k++) {
                for (int l = j - 1; l <= j + 1; l++) {
                    if ((k < 0 || k >= gamefield->n) ||
                        (l < 0 || l >= gamefield->m) ||
                        (k == i && l == j))
                        continue;
                    neighbours += *(gamefield->data + k * gamefield->n + l);
                }
            }

            if (currVal == 1 && (neighbours < 2 || neighbours > 3)) newVal = 0;
            else if (currVal == 0 && neighbours == 3) newVal = 1;

#ifdef DEBUG
            printf("%d:%d->%d has %d neighbours->%d\n", i, j, currVal, neighbours, newVal);
#endif

            * (newfield + i * gamefield->m + j) = newVal;
        }
    }

    // neues spielfeld linken, altes freen

    free(gamefield->data);
    gamefield->data = newfield;
    gamefield->generation++;
}

#ifdef WINDOWS
char formatted2(char val, Cursor *cursor, Cursor *currPos) {
    char ret;
    if (cursor->x == currPos->x && cursor->y == currPos->y) {
        ret = (val == 1) ? 35 : 48;
    }
    else {
        ret = (val == 1) ? '*' : ' ';
    }
    return ret;
}
#endif

void openEditor(Field* gamefield) {
    //void openEditor(){
    Cursor cursor;
    cursor.x = 0;
    cursor.y = 0;

    Cursor currPos;
    currPos.x = 0;
    currPos.y = 0;

    int xmax = gamefield->n - 1;
    int ymax = gamefield->m - 1;

#ifdef UNIX
    char formatted(char val) {
        char ret;
        if (cursor.x == currPos.x && cursor.y == currPos.y) {
            ret = (val == 1) ? 35 : 48;
        }
        else {
            ret = (val == 1) ? '*' : ' ';
        }
        return ret;
    }
#endif

    int ch1, ch2;
    while (1) {
#ifndef DEBUG
        system(CONSOLE_CLEAR);
#endif

        printf("Arrow-Keys to move around; <Space> to toggle cell; <Enter> to finish\n");

        for (int i = 0; i < gamefield->n; i++) {
            for (int j = 0; j < gamefield->m; j++) {
                currPos.x = i;
                currPos.y = j;
#ifdef UNIX
                printf("%c ", formatted(*(gamefield->data + i * gamefield->n + j)));
#else
                printf("%c ", formatted2(*(gamefield->data + i * gamefield->n + j), &cursor, &currPos));

#endif
            }
            printf("\n");
        }

#ifdef UNIX
        system("/bin/stty raw");
#endif


        ch1 = getc(stdin);
        printf("%d ", ch1);
        if (ch1 == KEYCODE1) {
#ifdef UNIX // unter unix kommt noch ein key dazwischen mit
            ch2 = getc(stdin);
            if (ch2 == KEYCODE2) {
#endif
                ch2 = getc(stdin);
                switch (ch2) {
                case KEYCODE_UP: if (cursor.x > 0) cursor.x--; break;
                case KEYCODE_DOWN: if (cursor.x < xmax) cursor.x++; break;
                case KEYCODE_LEFT: if (cursor.y > 0) cursor.y--; break;
                case KEYCODE_RIGHT: if (cursor.y < ymax) cursor.y++; break;
                }
#ifdef UNIX
            }
#endif
        }
        else if (ch1 == 32) {
            //toggle cell
            *(gamefield->data + cursor.x * gamefield->n + cursor.y) = !*(gamefield->data + cursor.x * gamefield->n + cursor.y);
        }
        else break;
#ifdef UNIX
        system("/bin/stty cooked");
#endif
    }
#ifdef UNIX
    system("/bin/stty cooked");
#endif
}

char openFileSaveDialog(Field* gamefield) {
    char buf[255];
    int intBuf;
    char done = 0;
    while (!done) {
        printf("Please enter a valid Filename to save to: (c = cancel)");
        if (readInput(buf, &intBuf) || strlen(buf) == 0) continue; // kein valid filename, nochmal
        else {
            if (strlen(buf) == 1 && (buf[0] == 'q' || buf[0] == 'r')) { if (checkError(buf, gamefield)) return 0; } // return 0 = restart (damit immer 'r' geht)
            else if (strlen(buf) == 1 && buf[0] == 'c') break; // c = cancel
            else {
                // valid filename
                // -> construct file
                // checken ob datei existiert, fragen ob wir überschreiben sollen
                if (access(buf, F_OK) != -1) {
                    // file exists
                    char overwrite = 0;
                    if (!askYN("File already exists. Do you want to overwrite?", &overwrite, gamefield)) return 1; // 1 = fertig
                    if (!overwrite) continue; // wenn es nicht überschrieben werden soll von vorne anfangen
                }
                // speichern
                FILE* saveFile = fopen(buf, "w");
                fputs(FILE_HEADER, saveFile);
                fputc('\n', saveFile);
                for (int i = 0; i < gamefield->n; i++) {
                    for (int j = 0; j < gamefield->m; j++) {
                        fputc((*(gamefield->data + i * gamefield->n + j) == 1) ? '1' : '0', saveFile);
                    }
                    fputc('\n', saveFile);
                }
                fclose(saveFile);
                done = 1;
                printf("\n"); // newline damit es schöner ist
            }
        }

    }
    return 1;
}

char openFileReadDialog(Field* gamefield) {
    char buf[255];
    int intBuf = 0;
    char done = 0;

    DIR* folder;
    struct dirent* entry;


    while (!done) {

        folder = opendir(".");
        if (folder == NULL) {
            printf("Unable to read directory\n");
            return 0; // hier 0 returnen weil wir ja neu anfangen müssen
        }

        // x dateien im aktuell Verzeichnis drucken
        int i = 0;
        FILE* file;
        while ((entry = readdir(folder)) && i < FILE_LIST) {

            //check file format (header)
            if (entry->d_name[0] == '.') continue;
            file = fopen(entry->d_name, "r");
            if (file == NULL) continue;
#ifdef WINDOWS
            char header[sizeof(FILE_HEADER) / sizeof(FILE_HEADER[0])];
#else
            char header[strlen(FILE_HEADER)];
#endif
            fgets(header, strlen(FILE_HEADER) + 1, file);
            fclose(file);
            if (strcmp(header, FILE_HEADER) == 0) {
                i++;
                printf("%d - %s\n", i, entry->d_name);
            }

        }
        closedir(folder);
        printf("Enter File Number or other filename:");

        char fileNo = 0;

        //checken ob der filename oder die nummer eingegeben wurde
        if (readInput(buf, &intBuf) && intBuf > 0 && intBuf <= 5) fileNo = (char)intBuf;
        else {
#ifdef DEBUG
            printf("input: %s\n", buf);
#endif
            if (strlen(buf) > 1) {
                //checken ob die datei existiert
                if (access(buf, F_OK) == -1) {
                    printf("Files does not exist\n");
                    continue;
                }
                // richtiger filename im buffer
                file = fopen(buf, "r");
#ifdef WINDOWS
                char header[sizeof(FILE_HEADER)/sizeof(FILE_HEADER[0])];
#else
                char header[strlen(FILE_HEADER)];
#endif
                fgets(header, strlen(FILE_HEADER) + 1, file);
                if (strcmp(header, FILE_HEADER) != 0) {
                    printf("File not in the right format");
                    fclose(file);
                    continue;
                }
            }
            else {
#ifdef DEBUG
                printf("checking input errors\n");
#endif
                if (checkError(buf, gamefield)) return 0; //restart
            }
        }

        if (fileNo != 0) {
            // nochmal durchgehen um die nummer zu holen
            folder = opendir(".");
            if (folder == NULL) {
                printf("Unable to read directory\n");
                return 0; // hier 0 returnen weil wir ja neu anfangen müssen
            }
            int i = 0;
            while ((entry = readdir(folder))) {

                //check file format (header) (muss wieder gemacht werden damit die reihenfolge gleich bleibt)
                if (entry->d_name[0] == '.') continue;
                file = fopen(entry->d_name, "r");
                if (file == NULL) continue;
#ifdef WINDOWS
                char header[sizeof(FILE_HEADER)/sizeof(FILE_HEADER[0])];
#else
                char header[strlen(FILE_HEADER)];
#endif
                fgets(header, strlen(FILE_HEADER) + 1, file);
                fclose(file);
                if (strcmp(header, FILE_HEADER) != 0) continue;

                i++;
                if (i == fileNo) {
                    strcpy(buf, entry->d_name);
                    file = fopen(buf, "r");

                    break;
                }
            }
        }

        //read file
        char c;
        int x = -1; // weils am anfang noch eine newline nach dem header gibt
        int y = 0;
        while ((c = fgetc(file)) != EOF) {
            //printf("%c",c);
            if (c != '\n') {
                y++;
            }
            else {
                if (x == 0) gamefield->m = y;
                x++;
                y = 0;
            }
        }
        gamefield->n = x;

        char* field = malloc(sizeof(char) * gamefield->n * gamefield->m);

        gamefield->data = field;

        fseek(file, 0, SEEK_SET); // fp nochmal zurücksetzen damit wir nochmal durchgehen können

        x = -1, y = -1;
        while ((c = fgetc(file)) != EOF) { // nochmal neu durch die datei gehen
#ifdef DEBUG
            printf("%d:%d:%c ", x, y, c);
#endif
            if (c == '\n') {
                y = 0;
                x++;
            }
            else {
                if (x > -1) *(field + x * gamefield->m + y) = (c == '1') ? 1 : 0;
                y++;
            }
        }

        fclose(file);

        break;
    }
    return 1;
}

char printMenu(Field* gamefield) {
    char buf[LINE_MAX];
    int intBuf = 0;

    printf("Welcome to the GAME OF LIFE\n");
    printf("\nYou can enter 'q' (quit) or 'r' (restart) at any time.\n");

    printf("\n1 - Generate random start [1]\n2 - Load start from file:\n");
    if (readInput(buf, &intBuf)) randomField = (intBuf == 1 || intBuf == 0); //standardwert
    else if (checkError(buf, gamefield)) return 0;

    if (!randomField) {
        printf("Opening file dialog\n");
        if (!openFileReadDialog(gamefield)) return 0;
    }
    else {

        printf("\nPlease enter width [%d]:", DEFAULT_WIDTH);
        if (readInput(buf, &intBuf) && intBuf >= 0) gamefield->n = (intBuf == 0) ? DEFAULT_WIDTH : intBuf;
        else if (checkError(buf, gamefield)) return 0;

        printf("Please enter height [%d]:", DEFAULT_HEIGHT);
        if (readInput(buf, &intBuf) && intBuf >= 0) gamefield->m = (intBuf == 0) ? DEFAULT_HEIGHT : intBuf;
        else if (checkError(buf, gamefield)) return 0;

        // randomField generieren
        // ask for fill percentage
        printf("\nEnter fill percentage (0-100; empty for random):");
        if (readInput(buf, &intBuf) && intBuf <= 100) {
            if (strlen(buf) == 0) intBuf = -1;
        }
        else if (checkError(buf, gamefield)) return 0;

        generateField(gamefield, intBuf);
    }

    printField(gamefield, 0);

    char edit = 0;
    if (!askYN("Do you want to edit?", &edit, gamefield)) return 0;

    if (edit) openEditor(gamefield);


    char save = 0;
    if (!askYN("Do you want to save the current Layout?", &save, gamefield)) return 0;

    if (save) {
        if (!openFileSaveDialog(gamefield)) return 0;
    }


    if (!askYN("Do you want to step through the generations yourself?", &gamefield->step, gamefield)) return 0;
#ifdef DEBUG
    printf("step: %d", gamefield->step);
#endif

    //START
    return 1;

}

int main() {

    //openEditor();
    Field gamefield;
    gamefield.data = NULL;

    char menuStatus = 0;

    system(CONSOLE_CLEAR);

    while (!menuStatus) {
        menuStatus = printMenu(&gamefield);
        if (!menuStatus) free(gamefield.data);
    }

    //start game
    gamefield.generation = 0;
    printField(&gamefield, 1);
    while (1) {
        if (gamefield.step) {
            char x = getchar();
            if (x == 'q') break;
        } else {
#ifdef WINDOWS
            if (kbhit()) { if (getc(stdin) == 'q') break; }
#else            
            //if (!feof(stdin)) { if (getc(stdin) == 'q') break; }
#endif
#ifdef UNIX
            sleep(SPEED); // TODO q zum quitten wenn kein steppen
#else
            sleep(SPEED * 1000); // TODO q zum quitten wenn kein steppen
#endif
        }
        generateNext(&gamefield);
        printField(&gamefield, 1);
    }

    free(gamefield.data); //aufräumen
    return 0;
}












