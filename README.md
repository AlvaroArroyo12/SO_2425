Sistema de Archivos Simulado.

Este proyecto es un sistema de archivos en C que permite realizar operaciones básicas sobre una partición simulada.

Comandos disponibles:

1. **info**: Muestra información del superbloque, como bloques libres e inodos disponibles.
2. **bytemaps**: Muestra el estado de los bytemaps de bloques e inodos.
3. **dir**: Lista los archivos en el directorio raíz con su número de inodo.
4. **imprimir <nombre>**: Imprime el contenido del archivo indicado.
5. **rename <nombre_antiguo> <nombre_nuevo>**: Cambia el nombre de un archivo.
6. **remove <nombre>**: Elimina un archivo, liberando sus bloques.
7. **copy <origen> <destino>**: Crea una copia de un archivo existente.
8. **salir**: Guarda los cambios y finaliza la ejecución.

Instrucciones:

1. Compila el programa con:
   gcc simul_ext.c -o simul_ext
2. Ejecuta el programa con:
   ./simul_ext

