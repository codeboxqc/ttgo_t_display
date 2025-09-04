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
const char* BOT_TOKEN = "*************";
const char* CHAT_ID = "*******";

// RSS feed URLs
const char* rssFeeds[] = {
  "https://lapresse.ca/actualites/rss",
  "https://korben.info/feed",
  "https://globalnews.ca/montreal/feed/",
  "https://www.journaldemontreal.com/spectacles/jetset/rss.xml", // Fixed to HTTPS
  "https://rss.cnn.com/rss/cnn_us.rss", // Fixed to HTTPS
  "https://rss.cnn.com/rss/cnn_topstories.rss", // Fixed to HTTPS
  "https://mobile.abc.net.au/news/feed/51120/rss.xml",
  "https://www.journaldemontreal.com/jm/techno/rss.xml", // Fixed to HTTPS
  "https://www.abc.net.au/news/feed/936/rss.xml",
  "https://www.lapresse.ca/international/rss",
  "https://www.theguardian.com/news/series/ten-best-photographs-of-the-day/rss",
  "https://feeds.skynews.com/feeds/rss/world.xml",
  "https://feeds.bbci.co.uk/news/england/rss.xml",
  "https://www.theguardian.com/world/rss",
  "https://www.bbc.co.uk/blogs/internet/rss",
  "https://www.independent.co.uk/rss",
  "https://www.independent.co.uk/news/science/rss",
  "https://www.ouest-france.fr/rss-en-continu.xml",
  "https://www.hongkongfp.com/feed/",
  "https://rss.cnn.com/rss/edition_world.rss", // Fixed to HTTPS
  "https://www.tomshardware.com/feeds/all",
  "https://www.cbsnews.com/latest/rss/world",
  "https://www.nasa.gov/rss/dyn/breaking_news.rss",
  "https://www.yahoo.com/news/rss/world/",
  "https://www.thesun.co.uk/news/worldnews/feed/",
  "https://montreal.citynews.ca/feed/",
  "https://toronto.citynews.ca/feed/",
  "https://stackoverflow.blog/feed/atom/",
  "https://www.bleepingcomputer.com/feed/",
  "https://timesofindia.indiatimes.com/rssfeedstopstories.cms",
  "https://www.japantimes.co.jp/feed/topstories/",
  "https://lemonde.fr/en/rss/une.xml",
  "https://www.repubblica.it/rss/homepage/rss2.0.xml",
  "https://www.cnbc.com/id/100727362/device/rss/rss.html",
  "https://abcnews.go.com/abcnews/internationalheadlines", // Fixed to HTTPS
  "https://rss.cnn.com/rss/cnn_us.rss", // Fixed to HTTPS
  "https://www.aljazeera.com/xml/rss/all.xml", // Fixed to HTTPS
  "https://rss.cnn.com/rss/cnn_showbiz.rss", // Fixed to HTTPS
  "https://en.yna.co.kr/RSS/news.xml",
  "https://defence-blog.com/feed/",
  "https://www.wired.com/feed/rss",
  "https://rss.cnn.com/rss/edition_asia.rss", // Fixed to HTTPS
  "https://gizmodo.com/rss",
  "https://www.cnet.com/rss/news/",
  "https://www.engadget.com/rss.xml",
  "https://www.smh.com.au/rssheadlines/world/article/rss.xml", // Fixed to HTTPS
  "https://www.nationalobserver.com/front/rss",
  "https://abcnews.go.com/abcnews/topstories", // Fixed to HTTPS
  "https://www.yahoo.com/news/rss",
  "https://www.washingtontimes.com/rss/headlines/news",
  "https://nypost.com/feed/",
  "https://rss.nytimes.com/services/xml/rss/nyt/HomePage.xml",
  "https://abc7news.com/feed/",
  "https://www.texasobserver.org/feed/",
  "https://www.nydailynews.com/feed/",
  "https://wsvn.com/feed/",
  "https://www.kron4.com/feed/",
  "https://feeds.nbcnews.com/nbcnews/public/news",
  "https://bair.berkeley.edu/blog/feed.xml",
  "https://abc13.com/feed/",
  "https://www.engadget.com/rss.xml",
  "https://gizmodo.com/rss",
  "https://rss.nytimes.com/services/xml/rss/nyt/World.xml",
  "https://www.nbcnewyork.com/?rss=y",
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
    displayMessage("Telegram: OK", TFT_GREEN);
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

 





// -------------------- RSS Feed Processing --------------------
// -------------------- RSS/Atom Feed Processing --------------------
void fetchRSSFeed(const char* url, size_t maxItems = 13) {
  if (WiFi.status() != WL_CONNECTED) {
    displayMessage("WiFi not connected", TFT_RED);
    return;
  }

  // Extract domain name for display
  String urlStr = String(url);
  int startPos = urlStr.indexOf("://") + 3;
  int endPos = urlStr.indexOf("/", startPos);
  String domain = urlStr.substring(startPos, endPos);
  displayMessage("Fetching: " + domain, TFT_YELLOW);

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
      displayMessage("Item " + String(itemCount + 1) + ": " + displayTitle, TFT_WHITE);
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