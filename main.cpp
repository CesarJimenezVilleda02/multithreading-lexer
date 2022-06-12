
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <string>
#include <istream>
#include <vector>
#include <regex>

/*************************************************************
* File: main.cpp
* Author: Pablo César Jiménez Villeda y Ariann Fernando Arriaga Alcántara
* Description:
* Date: 17/06/2022
*************************************************************/
using namespace std;

pthread_mutex_t lock_write;

/*--------------------- MONO HILO ---------------------*/
/// =================================================================
// Función que imprime los tokens encontrados en un archivo.
//
// Análsisi de complejidad:
/*
La complejidad es de este algoritmo es de O(n*m), donde n es la cantidad
de posible smatches que se pueden encontrar en el string y m es la
cantidad de tokens que se buscan.
*/
// @param row, un string de cada línea del archivo que se lee
// =================================================================
void analizeRow(string row, ofstream &out) {
  // Expresiones regulare spara encontrar todos los tokens
  string variables = "[a-zA-Z][a-zA-Z0-9_]*";
  string operands = "\\/|\\+|\\-|\\*|\\=|\\(|\\)|\\^";
  string comments = "\\/\\/.*";
  string integers = "\\-*[0-9]+";
  string words = "#include|scanf|printf|auto|else|long|switch|break|enum|register|typedef|case|extern|return|union|char|float|short|unsigned|const|for|signed|void|continue|goto|sizeof|volatile|default|if|stati|while|do|int|struct_Packed|double";
  string strings = "\".*\"";
  string real = "\\-*[0-9]+\\.[0-9]+((E|e)-*[0-9]*)*";
  std::regex e(comments + "|" + strings + "|" + words + "|" + real  + "|" + variables + "|" + integers + "|" + operands + "|.");

  // Itereador de todos los matches de las expresiones regulares
  std::regex_iterator<std::string::iterator> rit( row.begin(), row.end(), e);
  std::regex_iterator<std::string::iterator> rend;

  // Buscar qué regex ha provocado el match para identificar el token
  while (rit!=rend) {
    if(regex_match(rit->str(), regex(comments))) {
      out << "<span class=\"comment\">" << rit->str() << "</span>" << endl;
    } else if(regex_match(rit->str(), regex(strings))) {
      out << "<span class=\"string\">" << rit->str() << "</span>" << endl;
    } else if(regex_match(rit->str(), regex(words))) {
      out << "<span class=\"palabraReservada\">" << rit->str() << "</span>" << endl;
    } else if(regex_match(rit->str(), regex(real))) {
      out << "<span class=\"real\">" << rit->str() << "</span>" << endl;
    } else if(regex_match(rit->str(), regex(variables))) {
      out << "<span class=\"variable\">" << rit->str() << "</span>" << endl;
    } else if(regex_match(rit->str(), regex(integers))) {
      out << "<span class=\"integer\">" << rit->str() << "</span>" << endl;
    } else if(regex_match(rit->str(), regex(operands))) {
      char curr = rit->str()[0];
      out << "<span class=\"operand\">" << curr << "</span>" << endl;
    } else out << "<span>" << rit->str() << "</span>" << endl;
    ++rit;
  }

}


// =================================================================
// Función implementa un lexer
//
// Análsisi de complejidad:
/*
La complejidad es de esta función es de O(n*m*l), donde l es la
cantidad de líneas del archivo, m es la camtidad de matches y n
el número de tokens a buscar.
*/
// @param archivo, un string del nombre del archivo a analizar
// =================================================================
void lexer(string archivo) {
  ofstream out(archivo + ".analizado.mono.html");
  out << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><link rel=\"stylesheet\" href=\"./styles.css\"><title>Actividad integral</title></head><body><div class=\"tokens\"><h3 class=\"variable\">Variables</h3><h3 class=\"palabraReservada\">Palabra Reservada</h3><h3 class=\"operand\">Operadores</h3><h3 class=\"integer\">Enteros</h3><h3 class=\"real\">Reales</h3><h3 class=\"string\">Cadena</h3><h3 class=\"comment\">Comentarios</h3></div><br>" << endl;

  fstream contentDataFile;
  contentDataFile.open(archivo, ios::in);

  string currentLine = "";
  while(getline(contentDataFile, currentLine)) {
    // O(n*m)
    out << "</br>";
    analizeRow(currentLine, out);
    out << "</br>";
  }

  out << "</body></html>" << endl;
  out.close();
}

// /*--------------------- MULTI HILO ---------------------*/
typedef struct {
  string file;
} Block;
// =================================================================
// Función implementa un lexer
//
// Análsisi de complejidad:
/*
La complejidad es de esta función es de O(n*m*l), donde l es la
cantidad de líneas del archivo, m es la camtidad de matches y n
el número de tokens a buscar.
*/
// @param param, bloque con el nombre del archivo a analizar
// =================================================================
void* lexer(void* param) {
  Block *b;
  b = (Block*) param;

  ofstream out(b->file + ".analizado.multi.html");
  out << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><link rel=\"stylesheet\" href=\"./styles.css\"><title>Actividad integral</title></head><body><div class=\"tokens\"><h3 class=\"variable\">Variables</h3><h3 class=\"palabraReservada\">Palabra Reservada</h3><h3 class=\"operand\">Operadores</h3><h3 class=\"integer\">Enteros</h3><h3 class=\"real\">Reales</h3><h3 class=\"string\">Cadena</h3><h3 class=\"comment\">Comentarios</h3></div><br>" << endl;

  fstream contentDataFile;
  contentDataFile.open(b->file, ios::in);

  string currentLine = "";
  while(getline(contentDataFile, currentLine)) {
    // O(n*m)
    out << "</br>";
    analizeRow(currentLine, out);
    out << "</br>";
  }

  out << "</body></html>" << endl;
  out.close();

  pthread_mutex_lock(&lock_write);
  cout << "Analisis de " << b->file << " terminado." << endl;
  pthread_mutex_unlock(&lock_write);

  pthread_exit(0);
}

int main(int argc, char* argv[]) {
  vector<string> allArgs(argv, argv + argc);

  // --- MONOHILO
  start_timer();

  if (argc < 1) {
    cout << "usage: " << argv[0] << " pathname\n";
    return 0;
  }

  for(int i = 1; i < allArgs.size(); i++) {
      lexer(allArgs[i]);
      cout << "Analisis de " << allArgs[i] << " terminado." << endl;
  }

  cout << "Monohilo termino en: " << stop_timer()  << "ms"<< std::endl;

  // --- MULTIHILO
  start_timer();
  pthread_t tid[allArgs.size()];
  Block blocks[allArgs.size()];

  for (int i = 1; i < allArgs.size(); i++) {
    blocks[i].file = allArgs[i];
  }

  for (int i = 1; i < allArgs.size(); i++) {
     pthread_create(&tid[i], NULL, lexer, (void*) &blocks[i]);
   }

 for (int i = 1; i < allArgs.size(); i++) {
   pthread_join(tid[i], NULL);
 }

  cout << "Multihilo termino en: " << stop_timer()  << "ms"<< std::endl;
  return 0;
}
