#include "iRadioStations.hpp"
#include "Preferences.h"

// Logging-Tag für Easy-Logger
static const char *TAG = "STATIONS";

/**
 * @brief Initiale Liste von Stationen. Diese können so, wie sie geschrieben sind direkt in 
 * den NVM geschrieben werden.
 */
String defaultStations[] = {
    "NDR1 Hannover|http://icecast.ndr.de/ndr/ndr1niedersachsen/hannover/mp3/128/stream.mp3",
    "NDR2 Niedersachsen|http://icecast.ndr.de/ndr/ndr2/niedersachsen/mp3/128/stream.mp3",
    "Eins Live|http://wdr-1live-live.icecast.wdr.de/wdr/1live/live/mp3/128/stream.mp3",
    "WDR 2|http://wdr-wdr2-aachenundregion.icecastssl.wdr.de/wdr/wdr2/aachenundregion/mp3/128/stream.mp3",
    "WDR 3|http://wdr-wdr3-live.icecastssl.wdr.de/wdr/wdr3/live/mp3/128/stream.mp3",
    "WDR COSMO|http://wdr-cosmo-live.icecast.wdr.de/wdr/cosmo/live/mp3/128/stream.mp3",
    "Bayern3|http://dispatcher.rndfnk.com/br/br3/live/mp3/low",
    "Bayern Klassik|http://dispatcher.rndfnk.com/br/brklassik/live/mp3/low",
    "Hessen3|http://dispatcher.rndfnk.com/hr/hr3/live/mp3/48/stream.mp3",
    "MDR|http://mdr-284280-1.cast.mdr.de/mdr/284280/1/mp3/low/stream.mp3",
    "RBB|http://dispatcher.rndfnk.com/rbb/rbb888/live/mp3/mid",
    "Deutschlandfunk|http://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3",
    "Antenne Bayern|http://stream.antenne.de/antenne",
    "Antenne Brandenburg|http://dispatcher.rndfnk.com/rbb/antennebrandenburg/live/mp3/mid",
    "Radio Hagen|http://radiohagen.cast.addradio.de/radiohagen/simulcast/high/stream.mp3",
    "Radio 1A Deutsche Hits|http://stream.1a-webradio.de/saw-deutsch/",
    "Radio 1A Rock|http://stream.1a-webradio.de/saw-rock/",
    "N-JOY|http://icecast.ndr.de/ndr/njoy/live/mp3/128/stream.mp3",
    "NRW Rockradio|http://rnrw.cast.addradio.de/rnrw-0182/deinrock/low/stream.mp3",
    "NRW 90er|http://rnrw.cast.addradio.de/rnrw-0182/dein90er/low/stream.mp3",
    "NRW Schlagerradio|http://rnrw.cast.addradio.de/rnrw-0182/deinschlager/low/stream.mp3",
    "RT1 Rock|http://mp3.hitradiort1.c.nmdn.net/rt1rockwl/livestream.mp3",
    "On 80s Radionetz|http://0n-80s.radionetz.de:8000/0n-70s.mp3",
    "Neue Deutsche Welle|http://streams.80s80s.de/ndw/mp3-192/streams.80s80s.de/",
    "Radio Swiss Classic|http://stream.srg-ssr.ch/rsc_de/mp3_128.m3u",
    "Radio Bollerwagen|http://player.ffn.de/radiobollerwagen.mp3",
    };
    
Preferences iRadioPrefs;

// Puffer für die Umrechnung
char key[] = "000";

/**
 * @brief Sucht einen freien Index im NVM. Der NVM ist ein Assoziatives Array, dass nach einem Key
 * sucht und den zugehörigen Value zurückgibt. Wir machen uns das hier ganz einfach und nehmen eine Zahl
 * genannt Index als Key.
 *
 * **Warum nicht den Namen als Key?**
 *
 * Grundsätzlich sollte man Werte, die man speichern will nicht zur Ordnung nutzen. In unserem Fall
 * bedeutet das konkret, würde man den Namen verwenden:
 * - Man könnte keine zwei Einträge mit dem gleichen Namen haben
 * - Man kann die Sortierreihenfolge der Einträge nicht bestimmen.
 * - ...
 *
 * **Achtung**: Es wird erwartet, dass der Namespace bereits gesetzt ist.
 * @return u_int8_t Den nächsten verwendbaren Index.
 */
u_int8_t findFreeNumber()
{

    // Alle Zahlen abklappern
    for (u_int8_t index = 1; index < ALLOW_STATIONS; index++)
    {
        // Die Zahl in einen `char *` umrechnen
        itoa(index, key, 10);

        // Den Schlüssel im NVM suchen
        if (!iRadioPrefs.isKey(key))
            return index;
    }

    return 0;
}

u_int8_t iRadioStations::getAnzahlStations()
{
    u_int8_t foundStations = 0;

    // Alle Zahlen abklappern
    for (u_int8_t index = 1; index < ALLOW_STATIONS; index++)
    {
        // Die Zahl in einen `char *` umrechnen
        itoa(index, key, 10);

        // Den Schlüssel im NVM suchen
        if (iRadioPrefs.isKey(key))
            foundStations++;
    }

    return foundStations;
}

void iRadioStations::addStation(Station newStation)
{
    iRadioPrefs.begin(stationNamespace, false);
    // Suche nächst möglichen Eintrag im NVM
    u_int8_t index = findFreeNumber();
    // Leider nichts mehr frei...
    if (index == 0)
    {
        LOG_ERROR(TAG, "Fehler beim Versuch eine neue Station abzuspeichern. Name: " << newStation.name);
        return;
    }

    // Die Zahl in einen `char *` umrechnen
    itoa(index, key, 10);

    iRadioPrefs.putString(key, newStation.toPreferences());
    iRadioPrefs.end();

    newStation.indexNVM = index;
}

void iRadioStations::removeStation(u_int8_t nrStation)
{
    iRadioPrefs.begin(stationNamespace, false);

    // Die Zahl in einen `char *` umrechnen
    itoa(nrStation, key, 10);

    /* Sollte die Stations-Nr nicht existieren gibt `remove` ein `false` zurück. Das
     * interessiert nur fürs debugging.
     */
    if (!iRadioPrefs.remove(key))
        LOG_ERROR(TAG, "Habe versucht die Station '" << key << "' zu löschen, die nicht im NVM '" << stationNamespace << "'war.");

    iRadioPrefs.end();
}

void iRadioStations::getStations(u_int8_t *stations)
{
    iRadioPrefs.begin(stationNamespace, false);

    u_int8_t anzahlStations = getAnzahlStations();

    // Erster Aufruf? Gibt es noch keine Stationen im NVM?
    if(anzahlStations==0){
        //Alle default-Stationen in den NVM schreiben.
        u_int8_t anzDefaultStations=sizeof(stations)/sizeof(String *);
        for(u_int8_t s=0;s<anzDefaultStations;s++){
            addStation(Station(defaultStations[s]));
        }
    }

    u_int8_t stationIndex = 0;

    // Alle Zahlen abklappern
    for (u_int8_t index = 1; index < ALLOW_STATIONS; index++)
    {
        // Die Zahl in einen `char *` umrechnen
        itoa(index, key, 10);

        // Den Schlüssel im NVM suchen
        if (iRadioPrefs.isKey(key))
        {
            stations[stationIndex] = index;
        }

        stationIndex++;
    }

    iRadioPrefs.end();
}

/**
 * @brief Sucht eine Station mit einem bestimmten Index im NVM
 *
 * @param nrStation Index der Station im NVM
 * @return Station Gefundene Station
 */
Station iRadioStations::getStation(u_int8_t nrStation)
{
    iRadioPrefs.begin(stationNamespace, false);

    // Die Zahl in einen `char *` umrechnen
    itoa(nrStation, key, 10);

    // Finde den passenden Value im NVM und erzeuge gleich daraus eine Station
    Station foundStation = Station(iRadioPrefs.getString(key, ""));
    iRadioPrefs.end();

    return foundStation;
}