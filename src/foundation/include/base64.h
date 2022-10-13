#ifndef TRACKER_BASE64_H
#define TRACKER_BASE64_H

#include <iostream>
#include <vector>
#include <string>

std::string base64_encode(unsigned char const *bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);

#endif //TRACKER_BASE64_H
