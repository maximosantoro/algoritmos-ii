
#include "calc_helper.h"
#include "strutil.h"


#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




//
// Prototipos de funciones internas a calc_helper.c
//
static bool parse_num(const char *tok, calc_num *dest);


//
// Convierte palabra a struct calc_token (número, operador, o paréntesis).
//
// Args:
//    tok: la palabra a convertir
//    dest: dónde guardar el token extraído
//
// Returns:
//    verdadero si la palabra formaba un token válido, falso en caso contrario
//
bool calc_parse(const char *tok, struct calc_token *parsed, bool str) {
    if (parse_num(tok, &parsed->value)) {
        parsed->type = TOK_NUM;
        return true;
    }
    else {
        parsed->type = TOK_OPER;
    }

    if (strlen(tok) == 1) {
        char op = tok[0];
        if (op == '+') {
            parsed->oper.op = OP_ADD;
            parsed->oper.operandos = 2;
            parsed->oper.precedencia = 2;
            parsed->oper.asociatividad = ASSOC_LEFT;
            if (str)
                parsed->oper.texto = strdup("+ ");
            else 
                parsed->oper.texto = NULL;
            return true;
        }
        else if (op == '-') {
            parsed->oper.op = OP_SUB;
            parsed->oper.operandos = 2;
            parsed->oper.precedencia = 2;
            parsed->oper.asociatividad = ASSOC_LEFT;
            if (str)
                parsed->oper.texto = strdup("- ");
            else 
                parsed->oper.texto = NULL;
            return true;
        }
        else if (op == '*') {
            parsed->oper.op = OP_MUL;
            parsed->oper.operandos = 2;
            parsed->oper.precedencia = 3;
            parsed->oper.asociatividad = ASSOC_LEFT;
            if (str)
                parsed->oper.texto = strdup("* ");
            else 
                parsed->oper.texto = NULL;
            return true;
            
        }
        else if (op == '/') {
            parsed->oper.op = OP_DIV;
            parsed->oper.operandos = 2;
            parsed->oper.precedencia = 3;
            parsed->oper.asociatividad = ASSOC_LEFT;
            if (str)
                parsed->oper.texto = strdup("/ ");
            else 
                parsed->oper.texto = NULL;
            return true;
        }
        else if (op == '^') {
            parsed->oper.op = OP_POW;
            parsed->oper.operandos = 2;
            parsed->oper.precedencia = 4;
            parsed->oper.asociatividad = ASSOC_RIGHT;
            if (str)
                parsed->oper.texto = strdup("^ ");
            else 
                parsed->oper.texto = NULL;
            return true;
        }
        else if (op == '?') {
            parsed->oper.op = OP_TERN;
            parsed->oper.operandos = 3;
            parsed->oper.texto = NULL;
            return true;
        }
        else if (op == '(') {
            parsed->type = TOK_LPAREN;
            parsed->oper.asociatividad = ASSOC_PAREN;
            parsed->oper.texto = NULL;
            return true;
        }
        else if (op == ')') {
            parsed->type = TOK_RPAREN;
            parsed->oper.asociatividad = ASSOC_PAREN;
            parsed->oper.texto = NULL;
            return true;
        }
        return false;
    }
    else if (strcmp(tok, "log") == 0) {
        parsed->oper.op = OP_LOG;
        parsed->oper.operandos = 2;
        parsed->oper.asociatividad = ASSOC_LEFT;
        parsed->oper.texto = NULL;
        return true;
    }
    else if (strcmp(tok, "sqrt") == 0) {
        parsed->oper.op = OP_RAIZ;
        parsed->oper.operandos = 1;
        parsed->oper.asociatividad = ASSOC_LEFT;
        parsed->oper.texto = NULL;
        return true;
    }
    return false;
}


//
// Primitivas de pilaint_t.
//

pilanum_t *pilanum_crear(void) {
    return pila_crear();
}

void apilar_num(pilanum_t *pila, calc_num num) {
    calc_num *dyn;
    // Por comodidad, queremos que apilar_num() sea void. Añadimos
    // sendos asserts para que, si ocurriese que malloc() falla, fuera
    // obvio qué está pasando.
    assert((dyn = malloc(sizeof(calc_num))));
    *dyn = num;
    assert(pila_apilar(pila, dyn));
}


bool desapilar_num(pilanum_t *pila, calc_num *num) {
    if (pila_esta_vacia(pila))
        return false;

    calc_num *dyn = pila_desapilar(pila);
    *num = *dyn;
    free(dyn);
    return true;
}


void pilanum_destruir(pilanum_t *pila) {
    while (!pila_esta_vacia(pila)) {
        void *elem = pila_desapilar(pila);
        free(elem);
    }

    pila_destruir(pila);
}


//
// Implementaciones de dc_split e infix_split.
//
// En ambos casos el argumento es una línea de texto, y el valor de retorno
// un arreglo dinámico de cadenas dinámicas. Si la cadena estaba vacía o no
// tenía tokens, se devuelve un arreglo de longitud cero: {NULL}.
//

char **dc_split(const char *linea) {
    const char *delim = " \n";
    char *parse = strdup(linea);  // No modificar línea, que es const.
    char *ptr, *tok = strtok_r(parse, delim, &ptr);
    char **strv = calloc(strlen(linea) + 1, sizeof(char *));  // (¹)

    // Separa por espacios (o múltiples espacios), e ignorando saltos de línea.
    for (size_t i = 0; tok != NULL; tok = strtok_r(NULL, delim, &ptr), i++) {
        strv[i] = strdup(tok);
    }

    free(parse);
    return strv;
}


char **infix_split(const char *linea) {
    size_t i = 0;
    const char *s = linea;
    char **strv = calloc(strlen(linea) + 1, sizeof(char *));  // (¹)

    // Implementa lo especificado en la consigna: solamente números positivos,
    // y garantía de input siempre válido. Por tanto, es como un split en la
    // barrera dígito/no-espacio.
    while (*s != '\0') {
        size_t n = 1;

        while (isspace(*s)) {
            s++;
        }

        if (isdigit(*s)) {
            n = strspn(s, "0123456789");
        }

        if (*s) {
            strv[i++] = strndup(s, n);
            s += n;
        }
    }

    return strv;
}


/** Notas al pie.
 *
 * (¹) Lo ideal siempre es reservar la cantidad exacta de memoria que se
 * precisa, como se hace en split() contando por adelantado el número de
 * separadores). En este caso es un poco más molesto hacer el cálculo por
 * adelantado, dada la fusión de separadores contiguos. A fin de mantener
 * la brevedad y legibilidad de las funciones, optamos por simplemente
 * reservar una cantidad de memoria que de seguro es suficiente pero que,
 * con toda probabilidad, es más del doble de la que se necesita.
 *
 * Una solución más elegante sería implementar la redimensión de un strv.
 * Una propuesta de prototipo, sin pérdida de legibilidad para dc_split,
 * sería el siguiente:
 *
 *    static char **strv_redim(char **argv, size_t idx, size_t *cur_size);
 *
 * Inicializando simplemente las variables a NULL (en lugar de calloc):
 *
 *    size_t size = 0;
 *    char **strv = NULL;
 *
 * el cuerpo del ciclo sería:
 *
 *    strv = strv_redim(strv, i, &size);  // Asegura poder guardar en strv[i]
 *    srtv[i] = strdup(tok);
 */


//
// Convierte una cadena a entero, indicando si se pudo.
//
// Args:
//    tok: la cadena a convertir
//    dest: dónde guardar el valor convertido
//
// Returns:
//    verdadero si la cadena era en su totalidad un entero válido.
//
static bool parse_num(const char *tok, calc_num *dest) {
    char *end;
    *dest = strtol(tok, &end, 10);
    return *end == '\0';
}

// funcion para copiar un struct calc_token. 
//
// arg: recibe el token a copiar.
// si se puede pedir la memoria y el token a copiar es valido, devuelve la copia.
// si no se cumplen las condiciones previas, devuelve NULL.
struct calc_token* token_copiar(struct calc_token* a_copiar) {
    if (!a_copiar) return NULL;
    struct calc_token* t = malloc(sizeof(struct calc_token));
    if (!t) return NULL;
    t->type = a_copiar->type;
    t->value = a_copiar->value;
    t->oper = a_copiar->oper;
    t->oper.precedencia = a_copiar->oper.precedencia;
    t->oper.operandos = a_copiar->oper.operandos;
    t->oper.op = a_copiar->oper.op;
    t->oper.asociatividad = a_copiar->oper.operandos;
    if (a_copiar->oper.texto)
        t->oper.texto = a_copiar->oper.texto;
    else 
        t->oper.texto = NULL;
    return t;
}

void destruir_token(struct calc_token* t) {
    if (t->type == TOK_OPER && t->oper.texto)
        free(t->oper.texto);
    free(t);
}