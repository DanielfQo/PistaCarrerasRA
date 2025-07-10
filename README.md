# PistaCarrerasRA 

Este es un proyecto universitario desarrollado como trabajo final del curso de **Computación Gráfica**. La aplicación utilizará **Realidad Aumentada (RA)** para detectar un patrón visual , sobre el cual se proyecta un modelo 3D interactivo. Además, se integra **reconocimiento de gestos** usando visión por computadora para controlar el comportamiento del objeto virtual.

---

## Autores

- **Leonardo Gaona**
- **Daniel Quiñones**
- **Kevin Rodriguez**
- **Michael Ticona**

---

## Objetivo del proyecto

- Detectar un **patrón físico** usando la cámara .
- Proyectar un **modelo 3D escaneado** sobre el patrón.
- Reconocer **gestos con la mano** (cerrada / abierta) usando OpenCV.
- Controlar el comportamiento del objeto virtual con dichos gestos (ej: avanzar, rotar, detenerse).

---

## Tecnologías empleadas

| Tecnología     | Descripción                                         |
|----------------|-----------------------------------------------------|
| **C++**      | Lenguaje principal del proyecto                     |
| **OpenCV ** | Para captura de video, detección de color y gestos  |
| **OpenGL**     | Para el renderizado del modelo 3D         |
| **CMake**      | Para la construcción multiplataforma             |

---

## Estructura del Proyecto

La arquitectura es modular, utilizando el patrón **MVC **.

