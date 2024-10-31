#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Generators
{
    class StringGenerator
    {
    public:
        StringGenerator(size_t size,
                        const std::string& charset = "abcdefghijklmnopqrstuvwxyz",
                        const std::string& start = "",
                        const std::string& end = "")
            : size_(size), charset_(charset), exhausted_(false)
        {
            if (size_ == 0)
                throw std::invalid_argument("String size must be at least 1.");
            if (charset_.empty())
                throw std::invalid_argument("Character set must contain at least one character.");
            
            // check sorted and has unique characters
            std::string sorted_charset = charset_;
            std::sort(sorted_charset.begin(), sorted_charset.end());
            auto last = std::unique(sorted_charset.begin(), sorted_charset.end());
            sorted_charset.erase(last, sorted_charset.end());
            charset_ = sorted_charset;
            
            // If start and end are not provided, generate from first to last string
            if (start.empty() && end.empty())
            {
                current_.assign(size_, charset_[0]);
                end_ = generateLastString();
                start_provided_ = false;
            }
            else
            {
                if (start.empty() || end.empty())
                    throw std::invalid_argument("Both start and end strings must be provided to define a range.");
                
                if (start.size() != size_ || end.size() != size_)
                    throw std::invalid_argument("Start and end strings must be of the specified size.");
                
                if (!isValidString(start))
                    throw std::invalid_argument("Start string contains characters not in the character set.");
                if (!isValidString(end))
                    throw std::invalid_argument("End string contains characters not in the character set.");
                
                if (start > end)
                    throw std::invalid_argument("Start string must be lexicographically less than or equal to end string.");
                
                current_ = start;
                end_ = end;
                start_ = start;
                start_provided_ = true;
            }
        }

        bool hasNext() const
        {
            return !exhausted_;
        }

        std::string next()
        {
            if (exhausted_)
                throw std::out_of_range("No more strings to generate.");

            std::string result = current_;

            // Increment the current string to the next combination
            increment();

            return result;
        }

        // Resets the generator to the initial state
        void reset()
        {
            if (start_provided_)
            {
                current_ = start_;
                exhausted_ = false;
            }
            else
            {
                current_.assign(size_, charset_[0]);
                end_ = generateLastString();
                exhausted_ = false;
            }
        }

        // Converts a position in the series to its value in the series ("aaa" -> 0, "aab" -> 1 etc.)
        static std::string numberToString(size_t size, size_t position, const std::string& charset = "abcdefghijklmnopqrstuvwxyz")
        {
            if (size == 0)
                throw std::invalid_argument("String size must be at least 1.");
            if (charset.empty())
                throw std::invalid_argument("Character set must contain at least one character.");

            // Ensures charset is sorted and has unique characters
            std::string sorted_charset = charset;
            std::sort(sorted_charset.begin(), sorted_charset.end());
            auto last = std::unique(sorted_charset.begin(), sorted_charset.end());
            sorted_charset.erase(last, sorted_charset.end());
            std::string unique_charset = sorted_charset;

            size_t base = unique_charset.size();

            // Calculates total number of combinations: base^size
            // To prevent overflow, uses iterative multiplication
            size_t total = 1;
            for (size_t i = 0; i < size; ++i)
            {
                if (base == 0)
                    throw std::invalid_argument("Character set size cannot be zero.");

                // Check for potential overflow
                if (total > (std::numeric_limits<size_t>::max() / base))
                    throw std::overflow_error("Total number of combinations exceeds size_t limits.");

                total *= base;
            }

            if (position >= total)
                throw std::out_of_range("Position is out of the valid range for the given size and charset.");

            // Initialize string with all first characters
            std::string result(size, unique_charset[0]);

            size_t current_position = position;

            // Fill the string from the end to the beginning
            for (int i = static_cast<int>(size) - 1; i >= 0; --i)
            {
                size_t index = current_position % base;
                result[i] = unique_charset[index];
                current_position /= base;
            }

            return result;
        }

        // Calculates the total number of possible combinations based on string size and charset.
        static size_t totalCombinations(size_t size, const std::string& charset = "abcdefghijklmnopqrstuvwxyz")
        {
            if (size == 0)
                throw std::invalid_argument("String size must be at least 1.");
            if (charset.empty())
                throw std::invalid_argument("Character set must contain at least one character.");

            // Ensure charset is sorted and has unique characters
            std::string sorted_charset = charset;
            std::sort(sorted_charset.begin(), sorted_charset.end());
            auto last = std::unique(sorted_charset.begin(), sorted_charset.end());
            sorted_charset.erase(last, sorted_charset.end());
            std::string unique_charset = sorted_charset;

            size_t base = unique_charset.size();

            // Calculate total number of combinations: base^size
            size_t total = 1;
            for (size_t i = 0; i < size; ++i)
            {
                if (base == 0)
                    throw std::invalid_argument("Character set size cannot be zero.");

                // Check for potential overflow
                if (total > (std::numeric_limits<size_t>::max() / base))
                    throw std::overflow_error("Total number of combinations exceeds size_t limits.");

                total *= base;
            }

            return total;
        }


    private:
        size_t size_;                 
        std::string charset_;         
        std::string current_;         
        std::string end_;             
        bool exhausted_;              
        bool start_provided_ = false; 
        std::string start_;           

        // Helper function to check if a string contains only characters from the charset
        bool isValidString(const std::string& str) const
        {
            for (char c : str)
            {
                if (charset_.find(c) == std::string::npos)
                    return false;
            }
            return true;
        }

        // Helper function to generate the last possible string based on the charset and size
        std::string generateLastString() const
        {
            return std::string(size_, charset_.back());
        }

        // Helper function to increment the current string lexicographically
        void increment()
        {
            for (int i = static_cast<int>(size_) - 1; i >= 0; --i)
            {
                size_t pos = charset_.find(current_[i]);
                if (pos == std::string::npos)
                {
                    throw std::logic_error("Current character not found in charset.");
                }

                if (pos < charset_.size() - 1)
                {
                    current_[i] = charset_[pos + 1];
                    // Reset all characters to the right to the first character in the charset
                    for (size_t j = i + 1; j < size_; ++j)
                    {
                        current_[j] = charset_[0];
                    }
                    // Check if we've exceeded the end string
                    if (start_provided_ && current_ > end_)
                    {
                        exhausted_ = true;
                    }
                    return;
                }
                current_[i] = charset_[0];
            }
            exhausted_ = true;
        }
    };
}
