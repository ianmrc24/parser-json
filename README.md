# Analizador Sintáctico LL(1) para JSON Simplificado

## Autores:

1. Ian Delvalle - 5550211
2. Matias Gaona - 4553979

Este proyecto implementa un **analizador sintáctico predictivo LL(1)** para el lenguaje **JSON simplificado** utilizando el lenguaje de programación C. El analizador toma como entrada un archivo `.json`, realiza un análisis léxico y sintáctico, e informa si la entrada es válida según la gramática definida para un subconjunto de JSON.

## Características

* **Reconocimiento de tokens**:

  * Llaves (`{`, `}`)
  * Corchetes (`[`, `]`)
  * Coma (\`,")
  * Dos puntos (`:`)
  * Cadenas literales (`"texto"`)
  * Números (incluye notación científica)
  * Booleanos: `true`, `false`
  * `null`
* **Gramática LL(1)** simplificada basada en la estructura JSON:

  * Soporte para objetos y arreglos anidados.
  * Manejo de listas de atributos y elementos.
* **Modo de recuperación de errores (panic mode)** mediante conjuntos *follow* para permitir la sincronización y continuar con el análisis.
* Mensajes de error sintáctico informativos con número de línea y token actual.
* Informe de aceptación o rechazo de la entrada.

## Estructura del Proyecto

* `main()` inicia el proceso cargando el archivo fuente y llamando a la función principal `json()`.
* `getNextToken()` obtiene los tokens uno por uno del archivo fuente.
* Funciones del parser:

  * `json()`, `element()`, `object()`, `array()`
  * `attributes_list()`, `attribute()`, `attribute_value()`
  * `element_list()` y funciones auxiliares para manejar listas separadas por comas
* `syntaxError()` y `scanUntilFollow()` implementan la recuperación de errores

## Instalación y Ejecución

### Requisitos

* Compilador C (como `gcc`)
* Sistema compatible: Linux, Windows, macOS

### Compilación

```bash
gcc -o parser parser.c
```

### Ejecución

```bash
./parser archivo.json
```

### Ejemplo de uso

```json
{
  "nombre": "Ian",
  "edad": 21,
  "activo": true,
  "hobbies": ["leer", "programar"]
}
```

Salida:

```text
Entrada JSON válida.
```

Si hay errores sintácticos, se mostrarán mensajes como:

```text
Error de sintaxis en línea 4: Se esperaba ':' (token actual: '"edad"')
```
