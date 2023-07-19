/**
 * @file iRadioConfig.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier sind alle Config-Anzeigen zusammengefasst
 * @version 0.1
 * @date 2023-04-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef IRADIOCONFIG_HPP_
#define IRADIOCONFIG_HPP_

#include <iRadioDisplay.hpp>

struct OptionValue
{
    String name;
    int value;
};

enum How2Continue
{
    stay,
    leave
};

How2Continue configReset();
How2Continue configInternet();
How2Continue configZeitZone();
How2Continue configSommerZeit();
How2Continue configClock();
How2Continue configDisplay();
void configRadio();

extern OptionValue speedOpts[];
extern OptionValue brightOpts[];

#endif // IRADIOCONFIG_HPP_