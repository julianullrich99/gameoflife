#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LINE_MAX 10
#define SPEED 1 // delay in ms
#define DEFAULT_WIDTH 20
#define DEFAULT_HEIGHT 20

//#define DEBUG

char randomField = 0;

typedef struct gamefield {
  int n;
  int m;
  char step;
  int generation;
  char *data;
} Field;

char readInput(char *buf, int *numeric){
  fflush(stdin);
  char *end;
  fgets(buf,sizeof(buf),stdin);
  buf[strlen(buf) - 1] = 0;
  *numeric = strtol(buf,&end,10);
#ifdef DEBUG
  printf("Read:%d, *end == NULL: %d\n",*numeric,*end == NULL);
#endif
  if (*end == NULL) return 1; // INTEGER GELESEN
  else return 0; // char* gelesen
}

char checkError(char *buf, Field *gamefield){ // retured 1 wenn restarted werden soll
  if (buf[0] == 'q') {
    free(gamefield->data);
    exit(0); // programm beenden
  } else {
#ifndef DEBUG
    system("cls||clear");
#endif
    if (buf[0]  != 'r') printf("Fehlerhafte Eingabe!\n\n");
    return 1; // menu neustarten
  }
}

void generateField(Field *gamefield, int fillPercentage){
  srand((unsigned) time(NULL));

  char percent = (fillPercentage > -1) ? fillPercentage : rand()%101;

  if (fillPercentage == -1) printf("Fillpercentage: %d\n",percent);

  char *field = malloc(sizeof(char) * gamefield->n * gamefield->m);
  
  gamefield->data = field;

  char random(){
    return ((rand()%100) < percent);
  }

  for (int i = 0; i < gamefield->n; i++){
    for (int j = 0; j < gamefield->m; j++){
      *(field + i*gamefield->m + j) = random();
    }
  }

}

void printField(Field *gamefield, char printHeader){

  char formatted(char val){
    return (val == 1) ? '*' : ' ';
  }
  if (printHeader) {
#ifndef DEBUG
    system("cls||clear");
#endif
    printf("Generation: %d",gamefield->generation);
    if (gamefield->step) printf("Press <Enter> to continue, q to quit.\n");
    else printf("Press q to quit.\n");
  }


  for (int i = 0; i < gamefield->n; i++){
    for (int j = 0; j < gamefield->m; j++){
      printf("%c ",formatted(*(gamefield->data + i*gamefield->n + j)));
    }
    printf("\n");
  }
}

void generateNext(Field *gamefield){

  char *newfield = malloc(sizeof(char) * gamefield->n * gamefield->m);

  for (int i = 0; i < gamefield->n; i++){
    for (int j = 0; j < gamefield->m; j++){

      char currVal = *(gamefield->data + i*gamefield->n + j);
      char neighbours = 0;
      char newVal = currVal;

      for (int k = i-1; k <= i+1; k++){
        for (int l = j-1; l <= j+1; l++){
          if ((k < 0 || k >= gamefield->n) ||
              (l < 0 || l >= gamefield->m) ||
              (k == i && l == j))
            continue;
          neighbours += *(gamefield->data + k*gamefield->n + l);
        }
      }

    
      if (currVal == 1 && (neighbours < 2 || neighbours > 3)) newVal = 0;
      else if (currVal == 0 && neighbours == 3) newVal = 1;

#ifdef DEBUG
      printf("%d:%d->%d has %d neighbours->%d\n",i,j,currVal,neighbours,newVal);
#endif

      *(newfield + i*gamefield->m + j) = newVal;
    }
  }

  // neues spielfeld linken, altes freen

  free(gamefield->data);
  gamefield->data = newfield;
  gamefield->generation++;

}

char printMenu(Field *gamefield){
  char buf[LINE_MAX];
  int intBuf = 0;

  printf("Welcome to the GAME OF LIFE\n");
  printf("\nYou can enter 'q' (quit) or 'r' (restart) at any time.\n");

  printf("\nPlease enter width [%d]:",DEFAULT_WIDTH);
  if (readInput(buf,&intBuf) && intBuf >= 0) gamefield->n = (intBuf == 0) ? DEFAULT_WIDTH : intBuf;
  else if (checkError(buf,gamefield)) return 0;

  printf("Please enter height [%d]:",DEFAULT_HEIGHT);
  if (readInput(buf,&intBuf) && intBuf >= 0) gamefield->m = (intBuf == 0) ? DEFAULT_HEIGHT : intBuf;
  else if (checkError(buf,gamefield)) return 0;

  printf("\n1 - Generate random start [1]\n2 - Load start from file:\n");
  if (readInput(buf,&intBuf)) randomField = (intBuf == 1 || intBuf == 0); //standardwert
  else if (checkError(buf,gamefield)) return 0;

  if (!randomField) {
    // TODO datei laden
  } else  {
    // randomField generieren
    // ask for fill percentage
    printf("\nEnter fill percentage (0-100; empty for random):");
    if (readInput(buf,&intBuf) && intBuf <= 100) {
      if (strlen(buf) == 0) intBuf = -1;
    } else if (checkError(buf,gamefield)) return 0;

    generateField(gamefield, intBuf);
  }

  printField(gamefield,0);

  char edit = 0;

  printf("Do you want to edit? [y/N]\n");
  readInput(buf,&intBuf);
  if (strlen(buf) > 0 && buf[0] != 'n' && buf[0] != 'N') {
    if (buf[0] == 'y' || buf[0] == 'Y') edit = 1;
    else if (checkError(buf,gamefield)) return 0;
  }

#ifdef DEBUG
  printf("edit: %d",edit);
#endif

  if (edit) {
    // TODO: editor schreiben
  }


  printf("Do you want to step through the generations yourself? [y/N]\n");
  readInput(buf,&intBuf);
  if (strlen(buf) > 0 && buf[0] != 'n' && buf[0] != 'N') {
    if (buf[0] == 'y' || buf[0] == 'Y') gamefield->step = 1;
    else if (checkError(buf,gamefield)) return 0;
  } else gamefield->step = 0;

  printf("step: %d",gamefield->step);

  //START
  return 1;

}

int main(){
  Field gamefield;

  char menuStatus = 0;

  system("cls||clear");
  while (!menuStatus) {
    menuStatus = printMenu(&gamefield);
    if (!menuStatus) free(gamefield.data);
  }

  //start game
  gamefield.generation = 0;
  printField(&gamefield,1);
  while (1){
    if (gamefield.step) {
      char x = getchar();
      if (x == 'q') break;
    } else sleep(SPEED); // TODO q zum quitten wenn kein steppen
    generateNext(&gamefield);
    printField(&gamefield,1);
  }

  free(gamefield.data); //aufräumen

  return 0;
}












