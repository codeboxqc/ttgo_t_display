#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <time.h>

// External references
extern TFT_eSPI tft;
extern int displayLine;
extern void displayMessage(String message, uint16_t color);


// Telegram constants
const char* BOT_TOKEN = "***************************";
const char* CHAT_ID = "***************************";

// RSS feed URLs
// RSS feed URLs

// RSS feed URLs
const char* rssFeeds[] = {
  // Working feeds (verified)
  "https://feeds.bbci.co.uk/news/world/rss.xml",
  "https://www.france24.com/en/tv-shows/observers-direct/podcast",  // France24 podcast feed
  "https://english.kyodonews.net/rss",                              // Kyodo News - FIXED
  "https://www.japantimes.co.jp/feed/",                             // Japan Times Main - FIXED
  "https://soranews24.com/feed/",                                   // Working
  "https://japantoday.com/feed",                                    // Working
  "https://rss.dw.com/rdf/rss-en-world",                            // Working
  "https://www3.nhk.or.jp/nhkworld/en/news/rss/",                   // NHK World - FIXED
  "https://news.un.org/feed/subscribe/en/news/all/rss.xml",        // Working
  "https://www.aljazeera.com/xml/rss/all.xml",                      // Verify manually, might work
  "https://www.cgtn.com/news/rss/headlines/rss.xml",                // CGTN Top Stories feed
  "https://www.centcom.mil/DesktopModules/ArticleCS/RSS.ashx?ContentType=1&Site=808&max=20/feed", // Working
  "http://china-defense.blogspot.com/feeds/posts/default",          // Working (HTTP)
  "https://militarywatchmagazine.com/feed/headlines.rss",          // Working
  "http://chinadefense.blogspot.co.il/feeds/posts/default",         // Working (HTTP)
  "https://russianmilitaryanalysis.wordpress.com/feed/",           // Working
  "https://www.oryxspioenkop.com/feeds/posts/default",             // Working
  "https://www.dailypress.com/tag/military/feed/",                 // Working
  "https://www.gov.uk/government/organisations/ministry-of-defence.atom", // Working
  "https://feeds.elpais.com/mrss-s/pages/ep/site/elpais.com/section/ultimas-noticias/portada", // Working
  "https://wyborcza.pl/pub/rss/najnowsze_wyborcza.xml",            // Working
  "https://rss.dw.com/rdf/rss-en-ger",                              // Working
  "https://feeds.elpais.com/mrss-s/pages/ep/site/elpais.com/section/internacional/portada", // Working
  "https://www.portugalresident.com/feed/",                         // Working
  "https://www.thelocal.dk/feeds/rss",                              // Working
  "https://www.thelocal.no/feeds/rss",                              // Working
  "https://www.thelocal.se/feeds/rss",                              // Working
  "https://www.icelandreview.com/feed/",                            // Working
  "https://www.ekathimerini.com/feed/",                             // Working
  "https://www.romania-insider.com/feed",                           // Working
  "https://brazilian.report/feed/",                                 // Working
  "https://santiagotimes.cl/feed/",                                 // Working
  "https://havanatimes.org/feed/",                                  // Working
  "https://newsroompanama.com/feed",                                // Working
  "https://mexiconewsdaily.com/feed/",                              // Working
  "https://caretas.pe/feed/",                                       // Peru - Working
  "https://buenosairesherald.com/feed",                             // Argentina - Working
  "https://www.irishtimes.com/arc/outboundfeeds/rss/",              // Ireland - Working
  "https://lapresse.ca/actualites/rss",                             // Working
  "https://korben.info/feed",                                       // Working
  "https://globalnews.ca/montreal/feed/",                           // Working
  "https://www.journaldemontreal.com/spectacles/jetset/rss.xml",    // Working
  "https://rss.cnn.com/rss/cnn_us.rss",                             // Working
  "https://rss.cnn.com/rss/cnn_topstories.rss",                     // Working
  "https://mobile.abc.net.au/news/feed/51120/rss.xml",              // Working
  "https://www.journaldemontreal.com/jm/techno/rss.xml",            // Working
  "https://www.abc.net.au/news/feed/936/rss.xml",                   // Working
  "https://www.lapresse.ca/international/rss",                      // Working
  "https://www.theguardian.com/news/series/ten-best-photographs-of-the-day/rss", // Working
  "https://feeds.skynews.com/feeds/rss/world.xml",                  // Working
  "https://feeds.bbci.co.uk/news/england/rss.xml",                  // Working
  "https://www.theguardian.com/world/rss",                          // Working
  "https://www.bbc.co.uk/blogs/internet/rss",                       // Working
  "https://www.independent.co.uk/rss",                               // Working
  "https://www.independent.co.uk/news/science/rss",                  // Working
  "https://www.ouest-france.fr/rss-en-continu.xml",                  // Working
  "https://www.hongkongfp.com/feed/",                               // Working
  "https://rss.cnn.com/rss/edition_world.rss",                      // Working
  "https://www.tomshardware.com/feeds/all",                         // Working
  "https://www.cbsnews.com/latest/rss/world",                       // Working
  "https://www.nasa.gov/rss/dyn/breaking_news.rss",                 // Working
  "https://www.yahoo.com/news/rss/world/",                          // Working
  "https://www.thesun.co.uk/news/worldnews/feed/",                  // Working
  "https://montreal.citynews.ca/feed/",                             // Working
  "https://toronto.citynews.ca/feed/",                              // Working
  "https://stackoverflow.blog/feed/atom/",                          // Working
  "https://www.bleepingcomputer.com/feed/",                         // Working
  "https://timesofindia.indiatimes.com/rssfeedstopstories.cms",     // Working
  "https://www.japantimes.co.jp/feed/topstories/",                  // Working
  "https://lemonde.fr/en/rss/une.xml",                              // Working
  "https://www.repubblica.it/rss/homepage/rss2.0.xml",              // Working
  "https://www.cnbc.com/id/100727362/device/rss/rss.html",          // Working
  "https://abcnews.go.com/abcnews/internationalheadlines",          // Working
  "https://www.aljazeera.com/xml/rss/all.xml",                      // Working
  "https://rss.cnn.com/rss/cnn_showbiz.rss",                        // Working
  "https://en.yna.co.kr/RSS/news.xml",                              // Working
  "https://defence-blog.com/feed/",                                 // Working
  "https://www.wired.com/feed/rss",                                 // Working
  "https://rss.cnn.com/rss/edition_asia.rss",                       // Working
  "https://gizmodo.com/rss",                                        // Working
  "https://www.cnet.com/rss/news/",                                 // Working
  "https://www.smh.com.au/rssheadlines/world/article/rss.xml",      // Working
  "https://www.nationalobserver.com/front/rss",                     // Working
  "https://abcnews.go.com/abcnews/topstories",                      // Working
  "https://www.yahoo.com/news/rss",                                 // Working
  "https://www.washingtontimes.com/rss/headlines/news",             // Working
  "https://nypost.com/feed/",                                       // Working
  "https://rss.nytimes.com/services/xml/rss/nyt/HomePage.xml",      // Working
  "https://abc7news.com/feed/",                                     // Working
  "https://www.texasobserver.org/feed/",                            // Working
  "https://www.nydailynews.com/feed/",                              // Working
  "https://wsvn.com/feed/",                                         // Working
  "https://www.kron4.com/feed/",                                    // Working
  "https://feeds.nbcnews.com/nbcnews/public/news",                  // Working
  "https://bair.berkeley.edu/blog/feed.xml",                        // Working
  "https://abc13.com/feed/",                                        // Working
  "https://www.engadget.com/rss.xml",                               // Working
  "https://gizmodo.com/rss",  
  "https://rss.nytimes.com/services/xml/rss/nyt/World.xml",         
  "https://www.nbcnewyork.com/?rss=y",  
  "https://security.googleblog.com/feeds/posts/default",        
  "https://www.cisa.gov/feeds/alerts.xml",                        
  "https://krebsonsecurity.com/feed/",                             
  "https://feeds.feedburner.com/TheHackRead",                      
  "https://portswigger.net/research/rss",  
  "https://www.gdacs.org/xml/rss.xml",                             
  "https://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/all_hour.atom", 
  "https://alerts.weather.gov/cap/us.atom", 
  "https://news.ycombinator.com/rss",                             
  "https://martinfowler.com/feed.atom",                            
  "https://developers.googleblog.com/feeds/posts/default", 
  "https://feeds.arstechnica.com/arstechnica/index",               
  "https://techcrunch.com/feed/", 
  "https://www.military.com/rss-feeds/content?limit=20&tags=news", 
  "https://www.defenseone.com/rss/all/",                           
  "https://www.defensenews.com/arc/outboundfeeds/rss/", 
  "https://technode.com/feed/"                                     
};

const int numFeeds = sizeof(rssFeeds) / sizeof(rssFeeds[0]);
 

String removeHTMLTags(String text);


// -------------------- URL Encoding --------------------
String urlEncode(const String& str) {
  String encoded = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (c == ' ') {
      encoded += "%20";
    } else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else {
      encoded += '%';
      if (c < 16) encoded += '0';
      encoded += String(c, HEX);
    }
  }
  return encoded;
}








// -------------------- Telegram Functions --------------------
void postToTelegram(const String& title, const String& link) {
  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate verification
  
  HTTPClient https;
  String message = "*New Article*\n[" + title + "](" + link + ")";
  String encodedMessage = urlEncode(message);

  String apiUrl = String("https://api.telegram.org/bot") + BOT_TOKEN +
                  "/sendMessage?chat_id=" + CHAT_ID +
                  "&text=" + encodedMessage + "&parse_mode=Markdown";

  https.begin(client, apiUrl);
  https.setTimeout(1000);
  
  int httpCode = https.GET();

  if (httpCode == 200) {
    displayMessage("TeGram OK", TFT_OLIVE);
  } else {
    displayMessage("Telegram: Error " + String(httpCode), TFT_RED);
  }

  https.end();
  delay(1300); // Rate limiting
}

// -------------------- XML Parsing --------------------
String extractXMLContent(const String& xml, const String& tag) {
  String openTag = "<" + tag + ">";
  String closeTag = "</" + tag + ">";
  
  int start = xml.indexOf(openTag);
  if (start == -1) {
    // Try with CDATA
    openTag = "<" + tag + "><![CDATA[";
    closeTag = "]]></" + tag + ">";
    start = xml.indexOf(openTag);
    if (start == -1) return "";
    start += openTag.length();
    int end = xml.indexOf(closeTag, start);
    if (end == -1) return "";
    return xml.substring(start, end);
  }
  
  start += openTag.length();
  int end = xml.indexOf(closeTag, start);
  if (end == -1) return "";
  
  String content = xml.substring(start, end);
  
  // Clean CDATA if present
  content.replace("<![CDATA[", "");
  content.replace("]]>", "");
  
  // Remove HTML entities
  content.replace("&quot;", "\"");
  content.replace("&amp;", "&");
  content.replace("&lt;", "<");
  content.replace("&gt;", ">");
  content.replace("&nbsp;", " ");
  content.replace("&apos;", "'");
  
  // Remove HTML tags
  while (content.indexOf("<") != -1 && content.indexOf(">") != -1) {
    int tagStart = content.indexOf("<");
    int tagEnd = content.indexOf(">", tagStart);
    if (tagEnd != -1) {
      content.remove(tagStart, tagEnd - tagStart + 1);
    } else {
      break;
    }
  }
  
  content.trim();
  return content;
}

 



// -------------------- Improved RSS/Atom Feed Processing --------------------
// Added parameter 'depth' to prevent infinite redirects
void fetchRSSFeed(const char* url, size_t maxItems, int recursionDepth = 0) {
  
  // 1. Safety Check: Stop recursion if redirected too many times
  if (recursionDepth > 3) {
    displayMessage("Too many redirects! Skipping.", TFT_RED);
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    displayMessage("WiFi lost. Skipping.", TFT_RED);
    return;
  }

  // Extract domain name for display
  String urlStr = String(url);
  String domain = "Unknown";
  int startPos = urlStr.indexOf("://");
  if (startPos > 0) {
    int endPos = urlStr.indexOf("/", startPos + 3);
    if (endPos == -1) endPos = urlStr.length();
    domain = urlStr.substring(startPos + 3, endPos);
  }
  displayMessage("Fetch> " + domain, TFT_YELLOW);

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate verification

  HTTPClient https;
  
  // 2. Wrap begin() in a check to see if connection is possible
  if (!https.begin(client, url)) {
    displayMessage("Connect Failed! Skipping.", TFT_RED);
    return;
  }

  https.addHeader("User-Agent", "Mozilla/5.0 (ESP32 RSS Reader)");
  https.setTimeout(8000); // Increased timeout to 8 seconds
  https.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS); // We handle redirects manually for safety

  int httpResponseCode = https.GET();

  // 3. Handle Redirects Manually (Safely)
  if (httpResponseCode == 301 || httpResponseCode == 302 || httpResponseCode == 307 || httpResponseCode == 308) {
    String newUrl = https.getLocation();
    https.end();
    if (newUrl.length() > 0 && newUrl != urlStr) {
      displayMessage("Redirect >>", TFT_BLUE);
      fetchRSSFeed(newUrl.c_str(), maxItems, recursionDepth + 1); // Recurse with depth + 1
      return;
    } else {
      displayMessage("Bad Redirect. Skipping.", TFT_RED);
      return;
    }
  }

  // 4. Handle HTTP Errors
  if (httpResponseCode != 200) {
    displayMessage("HTTP Err: " + String(httpResponseCode), TFT_RED);
    Serial.printf("Error on %s: %d\n", url, httpResponseCode);
    https.end();
    return; // This acts as the BYPASS
  }

  // 5. check payload size to prevent RAM overflow (optional but recommended)
  int len = https.getSize();
  if (len > 100000) { // If XML is larger than 100kb, it might crash ESP32
     displayMessage("File too big! Skipping.", TFT_RED);
     https.end();
     return;
  }

  String xmlContent = https.getString();
  https.end();

  if (xmlContent.length() < 50) {
    displayMessage("Empty/Bad XML. Skipping.", TFT_RED);
    return;
  }

  // Parsing logic (Previous logic was fine, just keeping it consistent)
  int itemCount = 0;
  int itemStart = 0;

  // ... (Keep your existing parsing loop here) ...
  while (itemCount < maxItems) {
    int itemTagStart = xmlContent.indexOf("<item>", itemStart);
    int entryTagStart = xmlContent.indexOf("<entry>", itemStart);

    if (itemTagStart == -1 && entryTagStart == -1) break;

    bool isAtom = (entryTagStart != -1 && (entryTagStart < itemTagStart || itemTagStart == -1));
    itemStart = isAtom ? entryTagStart : itemTagStart;
    String endTag = isAtom ? "</entry>" : "</item>";

    int itemEnd = xmlContent.indexOf(endTag, itemStart);
    if (itemEnd == -1) break;

    String itemXML = xmlContent.substring(itemStart, itemEnd + endTag.length());
    String title = extractXMLContent(itemXML, "title");
    String link;

    if (isAtom) {
      int linkPos = itemXML.indexOf("<link ");
      if (linkPos != -1) {
        int hrefStart = itemXML.indexOf("href=\"", linkPos);
        if (hrefStart != -1) {
          hrefStart += 6;
          int hrefEnd = itemXML.indexOf("\"", hrefStart);
          link = itemXML.substring(hrefStart, hrefEnd);
        }
      }
    } else {
      link = extractXMLContent(itemXML, "link");
    }

    if (title.length() > 0 && link.length() > 0) {
      String displayTitle = title.length() > 30 ? title.substring(0, 27) + "..." : title;
      displayMessage(">" + displayTitle, TFT_WHITE); // Simplified display
      postToTelegram(title, link);
      itemCount++;
    }

    itemStart = itemEnd;
    yield(); 
  }

  if (itemCount == 0) {
     displayMessage("No items found.", TFT_ORANGE);
  } else {
     displayMessage("Done. " + String(itemCount) + " sent.", TFT_GREEN);
  }
}

// -------------------- RSS Feed Processing --------------------
// -------------------- RSS/Atom Feed Processing --------------------
void fetchRSSFeed2(const char* url, size_t maxItems = 13) {

 

  if (WiFi.status() != WL_CONNECTED) {
    displayMessage("WiFi not connected", TFT_RED);
    return;
  }

  // Extract domain name for display
  String urlStr = String(url);
  int startPos = urlStr.indexOf("://") + 3;
  int endPos = urlStr.indexOf("/", startPos);
  String domain = urlStr.substring(startPos, endPos);
  displayMessage("Fetch>" + domain, TFT_YELLOW);

  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate verification

  HTTPClient https;
  https.begin(client, url);
  https.addHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
  https.addHeader("Referer", "https://google.com");
  https.addHeader("Accept-Language", "en-US,en;q=0.9");
  https.setTimeout(5000);
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpResponseCode = https.GET();

  if (httpResponseCode == 301 || httpResponseCode == 302 || httpResponseCode == 307 || httpResponseCode == 308) {
    String newUrl = https.getLocation();
    https.end();
    if (newUrl.length() > 0) {
      displayMessage("Redirected to: " + newUrl.substring(0, 30) + "...", TFT_BLUE);
      delay(1000);
      fetchRSSFeed(newUrl.c_str(), maxItems); // Recursive call
      return;
    }
  }

  if (httpResponseCode != 200) {
    displayMessage("HTTP Error: " + String(httpResponseCode), TFT_RED);
    https.end();
    return;
  }

  String xmlContent = https.getString();
  https.end();

  if (xmlContent.length() < 100) {
    displayMessage("Invalid RSS content", TFT_RED);
    return;
  }

  displayMessage("Parsing feed...", TFT_CYAN);

  // Parse RSS or Atom items
  int itemCount = 0;
  int itemStart = 0;

  while (itemCount < maxItems) {
    int itemTagStart = xmlContent.indexOf("<item>", itemStart);
    int entryTagStart = xmlContent.indexOf("<entry>", itemStart);

    if (itemTagStart == -1 && entryTagStart == -1) break;

    bool isAtom = (entryTagStart != -1 && (entryTagStart < itemTagStart || itemTagStart == -1));
    itemStart = isAtom ? entryTagStart : itemTagStart;
    String endTag = isAtom ? "</entry>" : "</item>";

    int itemEnd = xmlContent.indexOf(endTag, itemStart);
    if (itemEnd == -1) break;

    String itemXML = xmlContent.substring(itemStart, itemEnd + endTag.length());

    String title = extractXMLContent(itemXML, "title");
    String link;

    if (isAtom) {
      int linkPos = itemXML.indexOf("<link ");
      if (linkPos != -1) {
        int hrefStart = itemXML.indexOf("href=\"", linkPos);
        if (hrefStart != -1) {
          hrefStart += 6;
          int hrefEnd = itemXML.indexOf("\"", hrefStart);
          link = itemXML.substring(hrefStart, hrefEnd);
        }
      }
    } else {
      link = extractXMLContent(itemXML, "link");
    }

    if (title.length() > 0 && link.length() > 0) {
      String displayTitle = title.length() > 40 ? title.substring(0, 37) + "..." : title;
      displayMessage("POST:" + String(itemCount + 1) + ": " + displayTitle, TFT_DARKGREEN);
      postToTelegram(title, link);
      itemCount++;
    }

    itemStart = itemEnd;
    yield(); // Prevent watchdog timeout
  }

  displayMessage("Found " + String(itemCount) + " items", TFT_GREEN);
  delay(100);
}








// Simple HTML tag remover
String removeHTMLTags(String text) {
  String result = text;
  int startPos = result.indexOf('<');
  while (startPos != -1) {
    int endPos = result.indexOf('>', startPos);
    if (endPos == -1) break;
    result.remove(startPos, endPos - startPos + 1);
    startPos = result.indexOf('<');
  }
  return result;
}

 String getFormattedDateTime() {
  time_t now = time(nullptr);
  struct tm* timeInfo = localtime(&now);

  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
  return String(buffer);
}







void processRSSFeeds() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  displayLine = 0;
  
  displayMessage("=== RSS Processing ===", TFT_CYAN);

 String timestamp = getFormattedDateTime();
 String message = "📅 Daily RSS Update\nTime: " + timestamp;
 postToTelegram(message, "https://t.me/RssFido");
  
  for (int i = 0; i < numFeeds; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      displayMessage("WiFi lost - skipping", TFT_RED);
      continue;
    }
    
    displayMessage("Feed " + String(i + 1) + "/" + String(numFeeds), TFT_MAGENTA);
    fetchRSSFeed(rssFeeds[i], 10); // Max 13 items per feed
    
    delay(2000); // Pause between feeds
    yield();
  }
  
  displayMessage("=== Processing Done ===", TFT_GREEN);
}
