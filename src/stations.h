#include <Arduino.h>
#ifdef STATIONS_H
#define STATIONS_H
#else

// structure for station list
typedef struct
{
    const char *url;  // stream url
    const char *name; // stations name
} Station;

#define STATIONS 26 // number of stations in the list

// station list can easily be modified to support other stations
Station stationlist[STATIONS] PROGMEM = {
    {"http://icecast.ndr.de/ndr/ndr2/niedersachsen/mp3/128/stream.mp3", "NDR2 Niedersachsen"},                  // 0
    {"http://icecast.ndr.de/ndr/ndr1niedersachsen/hannover/mp3/128/stream.mp3", "NDR1 Hannover"},               // 1
    {"http://wdr-1live-live.icecast.wdr.de/wdr/1live/live/mp3/128/stream.mp3", "WDR1"},                         // 2
    {"http://wdr-cosmo-live.icecast.wdr.de/wdr/cosmo/live/mp3/128/stream.mp3", "WDR COSMO"},                    // 3
    {"http://radiohagen.cast.addradio.de/radiohagen/simulcast/high/stream.mp3", "Radio Hagen"},                 // 4
    {"http://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3", "Deutschlandfunk"},                              // 5
    {"http://player.ffn.de/radiobollerwagen.mp3", "Radio Bollerwagen"},                                         // 6
    {"http://dispatcher.rndfnk.com/br/br3/live/mp3/low", "Bayern3"},                                            // 7
    {"http://dispatcher.rndfnk.com/hr/hr3/live/mp3/48/stream.mp3", "Hessen3"},                                  // 8
    {"http://stream.antenne.de/antenne", "Antenne Bayern"},                                                     // 9
    {"http://stream.1a-webradio.de/saw-deutsch/", "Radio 1A Deutsche Hits"},                                    // 10
    {"http://stream.1a-webradio.de/saw-rock/", "Radio 1A Rock"},                                                // 11
    {"http://streams.80s80s.de/ndw/mp3-192/streams.80s80s.de/", "Neue Deutsche Welle"},                         // 12
    {"http://dispatcher.rndfnk.com/br/brklassik/live/mp3/low", "Bayern Klassik"},                               // 13
    {"http://mdr-284280-1.cast.mdr.de/mdr/284280/1/mp3/low/stream.mp3", "MDR"},                                 // 14
    {"http://icecast.ndr.de/ndr/njoy/live/mp3/128/stream.mp3", "N-JOY"},                                        // 15
    {"http://dispatcher.rndfnk.com/rbb/rbb888/live/mp3/mid", "RBB"},                                            // 16
    {"http://dispatcher.rndfnk.com/rbb/antennebrandenburg/live/mp3/mid", "Antenne Brandenburg"},                // 17
    {"http://wdr-wdr3-live.icecastssl.wdr.de/wdr/wdr3/live/mp3/128/stream.mp3", "WDR 3"},                       // 18
    {"http://wdr-wdr2-aachenundregion.icecastssl.wdr.de/wdr/wdr2/aachenundregion/mp3/128/stream.mp3", "WDR 2"}, // 19
    {"http://rnrw.cast.addradio.de/rnrw-0182/deinschlager/low/stream.mp3", "NRW Schlagerradio"},                // 20
    {"http://rnrw.cast.addradio.de/rnrw-0182/deinrock/low/stream.mp3", "NRW Rockradio"},                        // 21
    {"http://rnrw.cast.addradio.de/rnrw-0182/dein90er/low/stream.mp3", "NRW 90er"},                             // 22
    {"http://mp3.hitradiort1.c.nmdn.net/rt1rockwl/livestream.mp3", "RT1 Rock"},                                 // 23
    {"http://0n-80s.radionetz.de:8000/0n-70s.mp3", "radionetz"},                                                // 24
    {"http://stream.srg-ssr.ch/rsc_de/mp3_128.m3u", "Radio Swiss Classic"}};                                    // 25
#endif