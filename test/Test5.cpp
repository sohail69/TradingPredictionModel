
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

// Function to convert ISO 8601 timestamp to epoch time (seconds since Unix epoch)
double convertISO8601ToEpoch(const std::string& iso8601) {
    std::tm tm = {};
    std::istringstream ss(iso8601);
    // Expected format: YYYY-MM-DDTHH:MM:SSZ
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        std::cerr << "Failed to parse timestamp: " << iso8601 << std::endl;
        return 0.0;
    }
    // Convert tm to time_t in UTC
    // Since mktime assumes local time, use timegm for UTC
    // Note: timegm is a GNU extension and may not be available on all platforms
    #ifdef _WIN32
        // Windows does not have timegm, use _mkgmtime
        return static_cast<double>(_mkgmtime(&tm));
    #else
        // Unix-like systems
        return static_cast<double>(timegm(&tm));
    #endif
}

// Function to parse JSON and extract bid prices, ask prices, and timestamps
void parseOrderBooks(
    const std::string& jsonResponse,
    std::vector<double>& bidPrices,
    std::vector<double>& askPrices,
    std::vector<double>& epochTimestamps
) {
    size_t pos = 0;
    size_t len = jsonResponse.length();

    while (pos < len) {
        // Find the start of the next key (currency pair)
        size_t keyStart = jsonResponse.find('"', pos);
        if (keyStart == std::string::npos) {
            break; // No more keys found
        }

        // Find the end of the key
        size_t keyEnd = jsonResponse.find('"', keyStart + 1);
        if (keyEnd == std::string::npos) {
            break; // Malformed JSON
        }

        // Extract the currency pair (not stored as double)
        // std::string pair = jsonResponse.substr(keyStart + 1, keyEnd - keyStart - 1);
        // Optionally, you can store or use the pair if needed

        // Find the start of the object associated with the currency pair
        size_t objStart = jsonResponse.find('{', keyEnd);
        if (objStart == std::string::npos) {
            break; // Malformed JSON
        }

        // Find the end of the object
        size_t objEnd = jsonResponse.find('}', objStart);
        if (objEnd == std::string::npos) {
            break; // Malformed JSON
        }

        // Extract the object content
        std::string objContent = jsonResponse.substr(objStart + 1, objEnd - objStart - 1);

        // Extract "bid" value
        size_t bidKey = objContent.find("\"bid\"");
        double bid = 0.0;
        if (bidKey != std::string::npos) {
            size_t colon = objContent.find(':', bidKey);
            if (colon != std::string::npos) {
                size_t comma = objContent.find(',', colon);
                if (comma == std::string::npos) {
                    comma = objContent.length();
                }
                std::string bidStr = objContent.substr(colon + 1, comma - colon - 1);
                bidStr.erase(0, bidStr.find_first_not_of(" \t\n\r")); // Trim leading whitespace
                bidStr.erase(bidStr.find_last_not_of(" \t\n\r") + 1); // Trim trailing whitespace
                bid = std::stod(bidStr);
            }
        } else {
            std::cerr << "Warning: 'bid' field not found." << std::endl;
        }

        // Extract "ask" value
        size_t askKey = objContent.find("\"ask\"");
        double ask = 0.0;
        if (askKey != std::string::npos) {
            size_t colon = objContent.find(':', askKey);
            if (colon != std::string::npos) {
                size_t comma = objContent.find(',', colon);
                if (comma == std::string::npos) {
                    comma = objContent.length();
                }
                std::string askStr = objContent.substr(colon + 1, comma - colon - 1);
                askStr.erase(0, askStr.find_first_not_of(" \t\n\r")); // Trim leading whitespace
                askStr.erase(askStr.find_last_not_of(" \t\n\r") + 1); // Trim trailing whitespace
                ask = std::stod(askStr);
            }
        } else {
            std::cerr << "Warning: 'ask' field not found." << std::endl;
        }

        // Extract "timestamp" value and convert to epoch time
        size_t tsKey = objContent.find("\"timestamp\"");
        double epochTime = 0.0;
        if (tsKey != std::string::npos) {
            size_t colon = objContent.find(':', tsKey);
            if (colon != std::string::npos) {
                size_t quoteStart = objContent.find('"', colon);
                if (quoteStart != std::string::npos) {
                    size_t quoteEnd = objContent.find('"', quoteStart + 1);
                    if (quoteEnd != std::string::npos) {
                        std::string timestampStr = objContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                        epochTime = convertISO8601ToEpoch(timestampStr);
                    }
                }
            }
        } else {
            std::cerr << "Warning: 'timestamp' field not found." << std::endl;
        }

        // Populate the vectors
        bidPrices.push_back(bid);
        askPrices.push_back(ask);
        epochTimestamps.push_back(epochTime);

        // Move the position past the current object for the next iteration
        pos = objEnd + 1;
    }
}
