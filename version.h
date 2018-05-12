/*
 * version.h
 *
 *  Created on: 2015/6/27
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_VERSION_H_
#define USER_ELECTROMBILE_VERSION_H_


/*
 * Changelog
 * 0.1.0: initial framework
 * 0.2.2: gps funziona e celle, da controllare la velocità di aggiornamento e la ricerca duplicati
 * 0.2.3: aggiustato calcolo distanza
 * 0.2.4: modulo can compila, fix a upgrade per errore 85 (FS)-> reset del modulo e download template per successive richieste
 * 0.2.5: fix messaggio login
 */

#define VERSION_MAJOR   0
#define VERSION_MINOR   2
#define VERSION_MICRO   6

#define VERSION_INT(a, b, c)    (a << 16 | b << 8 | c)
#define VERSION_DOT(a, b, c)    a##.##b##.##c
#define VERSION(a, b, c)        VERSION_DOT(a, b, c)

#define STRINGIFY(s)         TOSTRING(s)
#define TOSTRING(s) #s

#define VERSION_STR STRINGIFY(VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO))

#define VERSION_NUM VERSION_INT(VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO)

#define DEVICE_TYPE 2

#endif /* USER_ELECTROMBILE_VERSION_H_ */
