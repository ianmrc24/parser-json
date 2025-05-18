#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
    L_CORCHETE, R_CORCHETE, L_LLAVE, R_LLAVE, COMA, DOS_PUNTOS,
    LITERAL_CADENA, LITERAL_NUM, PR_TRUE, PR_FALSE, PR_NULL,
    EOF_TOKEN, TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[128];
} Token;

const char* tokenTypeToString(TokenType type);

Token currentToken;
bool errorReported = false;
bool anyError = false;
FILE *fuente;
int lastChar = ' ';
int currentLine = 1;

void initLexer(const char *filename) {
    fuente = fopen(filename, "r");
    if (!fuente) {
        perror("No se pudo abrir el archivo fuente");
        exit(1);
    }
}

int nextChar() {
    int c = fgetc(fuente);
    if (c == '\n') currentLine++;
    return c;
}

void skipWhitespace() {
    while (isspace(lastChar)) lastChar = nextChar();
}

Token getNextToken() {
    Token t;
    t.lexeme[0] = '\0';
    skipWhitespace();

    if (lastChar == EOF) {
        t.type = EOF_TOKEN;
        strcpy(t.lexeme, "EOF");
        return t;
    }

    if (lastChar == '{') {
        t.type = L_LLAVE; strcpy(t.lexeme, "{"); lastChar = nextChar();
    } else if (lastChar == '}') {
        t.type = R_LLAVE; strcpy(t.lexeme, "}"); lastChar = nextChar();
    } else if (lastChar == '[') {
        t.type = L_CORCHETE; strcpy(t.lexeme, "["); lastChar = nextChar();
    } else if (lastChar == ']') {
        t.type = R_CORCHETE; strcpy(t.lexeme, "]"); lastChar = nextChar();
    } else if (lastChar == ':') {
        t.type = DOS_PUNTOS; strcpy(t.lexeme, ":"); lastChar = nextChar();
    } else if (lastChar == ',') {
        t.type = COMA; strcpy(t.lexeme, ","); lastChar = nextChar();
    } else if (lastChar == '"') {
        int i = 0; lastChar = nextChar();
        while (lastChar != '"' && lastChar != EOF && i < 120) {
            t.lexeme[i++] = lastChar; 
            lastChar = nextChar();
        }
        t.lexeme[i] = '\0';
        if (lastChar == '"') {
            t.type = LITERAL_CADENA; lastChar = nextChar();
        } else {
            t.type = TOKEN_INVALID;
        }
    } else if (isdigit(lastChar) || lastChar == '-') {
        int i = 0;
        while (isdigit(lastChar) || lastChar == '.' || lastChar == '-' || lastChar == 'e' || lastChar == 'E') {
            t.lexeme[i++] = lastChar; lastChar = nextChar();
        }
        t.lexeme[i] = '\0';
        t.type = LITERAL_NUM;
    } else if (lastChar == 't') {
        char buffer[5] = {0}; buffer[0] = 't';
        for (int i = 1; i < 4; i++) buffer[i] = nextChar();
        buffer[4] = '\0';
        if (strcmp(buffer, "true") == 0) {
            t.type = PR_TRUE; strcpy(t.lexeme, "true"); lastChar = nextChar();
        } else {
            t.type = TOKEN_INVALID; strcpy(t.lexeme, buffer); lastChar = nextChar();
        }
    } else if (lastChar == 'f') {
        char buffer[6] = {0}; buffer[0] = 'f';
        for (int i = 1; i < 5; i++) buffer[i] = nextChar();
        buffer[5] = '\0';
        if (strcmp(buffer, "false") == 0) {
            t.type = PR_FALSE; strcpy(t.lexeme, "false"); lastChar = nextChar();
        } else {
            t.type = TOKEN_INVALID; strcpy(t.lexeme, buffer); lastChar = nextChar();
        }
    } else if (lastChar == 'n') {
        char buffer[5] = {0}; buffer[0] = 'n';
        for (int i = 1; i < 4; i++) buffer[i] = nextChar();
        buffer[4] = '\0';
        if (strcmp(buffer, "null") == 0) {
            t.type = PR_NULL; strcpy(t.lexeme, "null"); lastChar = nextChar();
        } else {
            t.type = TOKEN_INVALID; strcpy(t.lexeme, buffer); lastChar = nextChar();
        }
    } else {
        t.type = TOKEN_INVALID;
        sprintf(t.lexeme, "%c", lastChar);
        lastChar = nextChar();
    }
    return t;
}

void syntaxError(const char* msg) {
    if (!errorReported) {
        printf("Error de sintaxis en línea %d: %s (token actual: '%s')\n", currentLine, msg, currentToken.lexeme);
        errorReported = true;
        anyError = true;
    }
}

void match(TokenType expected) {
    if (currentToken.type == expected) {
        currentToken = getNextToken();
    } else {
        char buffer[128];
        sprintf(buffer, "Se esperaba el token %s", tokenTypeToString(expected));
        syntaxError(buffer);
    }
}

bool inFollowSet(TokenType tok, TokenType follow[], int size) {
    for (int i = 0; i < size; i++) if (tok == follow[i]) return true;
    return false;
}

void scanUntilFollow(TokenType follow[], int size) {
    while (!inFollowSet(currentToken.type, follow, size) && currentToken.type != EOF_TOKEN)
        currentToken = getNextToken();
    errorReported = false;
}

void json();
void element();
void object();
void array();
void element_list();
void element_list_p();
void attributes_list();
void attributes_list_p();
void attribute();
void attribute_value();
void syntaxError(const char* msg);

void json() {
    element();
    if (currentToken.type != EOF_TOKEN)
        syntaxError("Se esperaba fin de archivo");
    else if (!anyError)
        printf("Entrada JSON válida.\n");
}

void element() {
    if (currentToken.type == L_LLAVE)
        object();
    else if (currentToken.type == L_CORCHETE)
        array();
    else {
        TokenType follow[] = { COMA, R_CORCHETE, R_LLAVE, EOF_TOKEN };
        syntaxError("Se esperaba '{' o '['");
        scanUntilFollow(follow, 4);
    }
}

void object() {
    match(L_LLAVE);
    if (currentToken.type == LITERAL_CADENA)
        attributes_list();
    match(R_LLAVE);
}

void array() {
    match(L_CORCHETE);
    if (currentToken.type != R_CORCHETE)
        element_list();
    match(R_CORCHETE);
}

void element_list() {
    element();
    element_list_p();
}

void element_list_p() {
    while (currentToken.type == COMA) {
        match(COMA);
        element();
    }
}

void attributes_list() {
    attribute();
    attributes_list_p();
}

void attributes_list_p() {
    while (currentToken.type == COMA) {
        match(COMA);
        attribute();
    }
}

void attribute() {
    match(LITERAL_CADENA);
    match(DOS_PUNTOS);
    attribute_value();
}

void attribute_value() {
    switch (currentToken.type) {
        case LITERAL_CADENA: match(LITERAL_CADENA); break;
        case LITERAL_NUM: match(LITERAL_NUM); break;
        case PR_TRUE: match(PR_TRUE); break;
        case PR_FALSE: match(PR_FALSE); break;
        case PR_NULL: match(PR_NULL); break;
        case L_LLAVE: object(); break;
        case L_CORCHETE: array(); break;
        default: {
            TokenType follow[] = { COMA, R_LLAVE, R_CORCHETE, EOF_TOKEN };
            syntaxError("Valor de atributo inválido");
            scanUntilFollow(follow, 4);
        }
    }
}

const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case L_CORCHETE: return "'['";
        case R_CORCHETE: return "']'";
        case L_LLAVE: return "'{'";
        case R_LLAVE: return "'}'";
        case COMA: return "','";
        case DOS_PUNTOS: return "':'";
        case LITERAL_CADENA: return "cadena";
        case LITERAL_NUM: return "número";
        case PR_TRUE: return "true";
        case PR_FALSE: return "false";
        case PR_NULL: return "null";
        case EOF_TOKEN: return "fin de archivo";
        case TOKEN_INVALID: return "token inválido";
        default: return "desconocido";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s archivo.json\n", argv[0]);
        return 1;
    }
    initLexer(argv[1]);
    lastChar = nextChar();
    currentToken = getNextToken();
    json();
    fclose(fuente);
    return 0;
}
