#include <Preferences.h>
#include <wifi.hpp>

Preferences preferences;

/// @brief Save up to three SSIDs and passwords to preferences
/// @param ssid SSID of the network
/// @param password Password of the network
void saveWiFi(String ssid, String password)
{
  // check for free space
  preferences.begin("wifi", false);

  // distribute SSID and password to free space
  if (preferences.getBytesLength("ssid") == 0)
  {
    // save to space 1
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    Serial.println("Saved SSID and password to space 1");
  }
  else if (preferences.getBytesLength("ssid2") == 0)
  {
    // save to space 2
    preferences.putString("ssid2", ssid);
    preferences.putString("password2", password);
    Serial.println("Saved SSID and password to space 2");
  }
  else if (preferences.getBytesLength("ssid3") == 0)
  {
    // save to space 3
    preferences.putString("ssid3", ssid);
    preferences.putString("password3", password);
    Serial.println("Saved SSID and password to space 3");
  }
  else
  {
    // all spaces full, overwrite first space
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    Serial.println("Overwrote SSID and password at space 1");
  }
  preferences.end();
}

/// @brief Load SSID and password from preferences
/// @return number of SSIDs and passwords found
int loadWiFi()
{
  preferences.begin("wifi", false);
  String ssid = preferences.getString("ssid", "");
  String ssid2 = preferences.getString("ssid2", "");
  String ssid3 = preferences.getString("ssid3", "");
  preferences.end();

  // count SSIDs and passwords
  int count = 0;
  if (ssid != "")
  {
    count++;
  }
  if (ssid2 != "")
  {
    count++;
  }
  if (ssid3 != "")
  {
    count++;
  }
  return count;
}

/// @brief Get SSID from preferences
/// @param index Index of SSID to return (0-2)
/// @return SSID
String getSSID(int index)
{
  preferences.begin("wifi", false);

  // return SSID
  if (index == 0)
  {
    String ssid = preferences.getString("ssid", "");
    preferences.end();
    return ssid;
  }
  else if (index == 1)
  {
    String ssid2 = preferences.getString("ssid2", "");
    preferences.end();
    return ssid2;
  }
  else if (index == 2)
  {
    String ssid3 = preferences.getString("ssid3", "");
    preferences.end();
    return ssid3;
  }
  else
  {
    return "";
  }
}

/// @brief Get passphrase from preferences
/// @param index Index of passhrase to return (0-2)
/// @return passphrase
String getPassphrase(int index)
{
  preferences.begin("wifi", false);

  // return password
  if (index == 0)
  {
    String password = preferences.getString("password", "");
    preferences.end();
    return password;
  }
  else if (index == 1)
  {
    String password2 = preferences.getString("password2", "");
    preferences.end();
    return password2;
  }
  else if (index == 2)
  {
    String password3 = preferences.getString("password3", "");
    preferences.end();
    return password3;
  }
  else
  {
    return "";
  }
}

/// @brief Delete SSID and password from preferences
void deleteAllWiFi()
{
  preferences.begin("wifi", false);
  preferences.remove("ssid");
  preferences.remove("password");
  preferences.remove("ssid2");
  preferences.remove("password2");
  preferences.remove("ssid3");
  preferences.remove("password3");
  preferences.end();
  Serial.println("Deleted SSIDs and passphrases from preferences");
}

/// @brief Delete SSID and password from preferences
/// @param index Index of SSID and password to delete (0-2)
void deleteWiFi(int index)
{
  preferences.begin("wifi", false);
  if (index == 0)
  {
    preferences.remove("ssid");
    preferences.remove("password");
  }
  else if (index == 1)
  {
    preferences.remove("ssid2");
    preferences.remove("password2");
  }
  else if (index == 2)
  {
    preferences.remove("ssid3");
    preferences.remove("password3");
  } else {
    Serial.println("Invalid index");
  }
  preferences.end();
  Serial.println("Deleted all SSIDs and passphrases from preferences");
}
