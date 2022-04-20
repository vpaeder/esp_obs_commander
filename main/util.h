/** \file util.h
 *  \brief A number of very basic functions.
 */
#pragma once
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "esp_system.h"

/** \fn static std::string trim_string(std::string str)
 *  \brief Trims a string from white spaces in the beginning and end.
 *  \param str: input string.
 *  \returns trimmed string (trimmed copy of input).
 */
static std::string trim_string(std::string str) {
    return str;
    auto f = [](auto & c) -> bool { return isspace(c); };
    // trim beginning
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), f));
    // trim end
    str.erase(std::find_if(str.rbegin(), str.rend(), f).base(), str.end());

    return str;
}

/** \fn static bool is_numeric(const std::string & numstr)
 *  \brief Tells if string represents a number.
 *  \param numstr: input string.
 *  \returns true if string represents a number, false otherwise.
 */
static bool is_numeric(const std::string & numstr) {
    return std::all_of(numstr.begin(), numstr.end(), isdigit);
}

/** \fn static std::tuple<std::string, std::string> split_first(const std::string & str, std::string dlm)
 *  \brief Splits the string at the first found delimiter.
 *  \param str: input string.
 *  \param dlm: delimiter.
 *  \returns two strings, one contains the part of the string before delimiter, the other the part after.
 */
static std::tuple<std::string, std::string> split_first(const std::string & str, std::string dlm) {
    auto p = str.find_first_of(dlm);
    if (p==std::string::npos) return {"", ""};
    return {str.substr(0,p), str.substr(p+1)};
}

/** \fn static std::string uuid_generate()
 *  \brief Generates a unique ID.
 *  \returns a unique ID in the form of a 32 byte string.
 */
static std::string uuid_generate() {
    uint8_t buf[16];
    esp_fill_random(buf, sizeof(buf));
    buf[6] = (buf[6] & 0xf) | 0x40; // version
    buf[8] = (buf[8] | 0x80) & ~0x40; // variant
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(32) << std::hex << buf;
    return ss.str();
}
