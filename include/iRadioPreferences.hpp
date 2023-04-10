/**
 * @file iRadioPreferences.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier werden alle Einstellungen verwaltet, die in den NVM kommen. Dazu gehören die Stationen
 * und weitere Einstellungen.
 * **Stationen**
 * Hier werden die _Sender_ verwaltet. Es soll durch eine einfache API möglich sein,
 * Sender zu speichern und auf dem Display auszuwählen. Damit die Sender nicht nach dem Ausschalten
 * weg sind, werden sie in den NVM (non-volatile-memory), den nicht flüchtigen Speicher geschrieben.
 * Das Arduino-Framework hat dafür das `Preferences.h`. Hier befindet sich alles Nötige.
 * NVM speichert Werte nach dem Prinzip `key`->`value`. Das ist hier zusätzlich noch sehr hilfreich,
 * denn es ersetzt ein Assoziatives Array. Das würde man sonst brauchen, weil noch nicht klar ist, wie viele
 * Einträge die Sender-Liste hat. Eine Implementierung eines assoziativen Arrays gibt es zwar, die
 * braucht aber sehr viel Speicher. NVM löst also beide Probleme elegant.
 *
 * ** Weitere Einstellungen **
 * Weitere Einstellungen werde in der Klasse `Settings` gespeichert. Diese Klasse bietet für jeden gespeicherten
 * Parameter einen eigenen Getter und Setter.
 *
 * Eine gute Zusammenfassung zum Thema NVM findet sich [hier](https://deepbluembedded.com/esp32-flash-memory-spi-flash-arduino/)
 * @version 0.1
 * @date 2023-03-31
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef IRADIOPREFERENCES_HPP_
#define IRADIOPREFERENCES_HPP_

#include <Arduino.h>
#include <log.h>

extern String stations[];

#define ALLOW_STATIONS 100 ///< Es werden 100 Stationen gespeichert.

/**
 * @brief Diese Klasse verwaltet eine einzelne Station. Eine Station hat einen Namen und eine URL wo wir den
 * Datenstrom des Audios erreichen können. Die Klasse hat die Aufgabe nicht nur diese Daten in einem Objekt zu
 * bündeln sondern auch gleich die Aufbereitung der Daten für den NVM _zu verbergen_. Damit ist gemeint, das
 * das restliche Program nicht na der Implementierung beteiligt ist und damit Änderungen (z.B. Fehlerbehebungen)
 * wirklich nur hier stattfinden.
 *
 * Der String, der in den NVM geschrieben wird, ist einfach die Summe `name+separator+url`
 */
class Station
{
private:
    const char separator = '|'; ///< Trennzeichen für den zusammengesetzten String für den NVM

public:
    String url;            ///< url des Streams
    String name;           ///< Name der Station
    u_int8_t indexNVM = 0; ///< index der Station im NVM

    Station()
    {
    }

    /// @brief Konstruktor für das Neuanlegen eines Senders, der noch nicht bekannt ist.
    /// @param url ///< url des Streams
    /// @param name ///< Name der Station
    Station(String url, String name)
    {
        this->name = name;
        this->url = url;
    }

    /**
     * @brief Konstruktor für das Auslesen eines Senders aus dem NVM
     *
     * @param val Wert, der im NVM gefunden wurde.
     */
    Station(String val);

    /**
     * @brief Erzeugt einen String, der in den NVM geschrieben werden kann.
     *
     * @return String Der String für den NVM
     */
    String toPreferences() { return name + separator + url; };
};

class iRadioStations
{
private:
    /** @brief Sollte im NVM noch etwas anderes abgespeichert werden als Sender, müssen wir die
     * unsere Einträge wiederfinden, deshalb nutzen wir diesen Namespace. Hier finden wir nur Stations.
     */
    const char *stationNamespace = "Station";

public:
    /**
     * @brief Fügt eine neue Station zum NVM hinzu
     *
     * @param newStation
     */
    void addStation(Station newStation);

    /**
     * @brief Löscht eine Station
     *
     * @param nrStation Index im NVM von der Station
     */
    void removeStation(u_int8_t nrStation);
    /**
     * @brief Liest alle Indizes der Stationen aus dem NVM. Damit lässt sich dann später
     * die Anzeige der Stationen realisieren.
     *
     * @param stations Pointer auf ein bereits ausreichend großes Feld von Indizes. Die abgelegten
     * Indizes sind der Größe nach sortiert.
     */
    void getStations(u_int8_t *stations);

    /**
     * @brief Getter für ein Station Objekt
     *
     * @param nrStation Index im NVM von der Station
     * @return Station Zugehörige Station
     */
    Station getStation(u_int8_t nrStation);

    /**
     * @brief Getter für die Anzahl der Stationen im NVM.
     *
     * **Achtung**: Es wird erwartet, dass der Namespace bereits gesetzt ist.
     *
     * @return u_int8_t Gefundene Einträge im NVM.
     */
    u_int8_t getAnzahlStations();
};

class Settings
{
private:
    /** @brief Sollte im NVM noch etwas anderes abgespeichert werden als Sender, müssen wir die
     * unsere Einträge wiederfinden, deshalb nutzen wir diesen Namespace. Hier finden wir nur Stations.
     */
    const char *prefNamespace = "Prefs";

public:
    u_int8_t getCurrentStation();
    void setCurrentStation(u_int8_t theStation);
};


#endif // IRADIOPREFERENCES_HPP_